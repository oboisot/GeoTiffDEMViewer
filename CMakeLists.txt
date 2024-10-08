cmake_minimum_required(VERSION 3.21)

project(GeoTiffDEMViewer LANGUAGES CXX VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)
# Instruct CMake to run resource files automatically when needed.
set(CMAKE_AUTORCC ON)
# Instruct CMake to run uic automatically when needed.
set(CMAKE_AUTOUIC ON)

# Set MSYS2 path
if (WIN32)
    set(MSYS2_PATH "D:/oboisot/msys64/mingw64") # MSYS2 mingw-w64-x86_64-gdal (MSVCRT)
#    set(MSYS2_PATH "D:/oboisot/msys64/ucrt64") # MSYS2 mingw-w64-x86_64-gdal (UCRT)
endif()

###################
# SETTING LIB/EXE #
###################
# Headers list
set(GTIFFDEMVIEWER_HEADERS # GeoTiff DEM
                           src/geotiffdem.h
                           src/qgeotiffdem.h
                           # UI
                           src/ui/geotiffdemviewerwindow.h
                           src/ui/qdemcolormap.h
                           src/ui/infosdialog.h)
set(FMT_HEADERS fmt/core.h fmt/format.h fmt/format-inl.h) # {fmt} library
# Sources list
set(GTIFFDEMVIEWER_SOURCES # GeoTiff DEM
                           src/geotiffdem.cpp
                           src/qgeotiffdem.cpp
                           # UI
                           src/ui/geotiffdemviewerwindow.cpp
                           src/ui/qdemcolormap.cpp
                           src/ui/infosdialog.cpp
                           # Main
                           GeoTiffDEMViewer.cpp
                           # Resources files
                           GeoTiffDEMViewer.qrc)
set(FMT_SOURCES src/fmt/format.cc)  # {fmt} library
add_executable(${PROJECT_NAME} ${GTIFFDEMVIEWER_HEADERS} ${GTIFFDEMVIEWER_SOURCES}
                               ${FMT_HEADERS} ${FMT_SOURCES})
set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX # Needed for header files only
                                                 VERSION ${PROJECT_VERSION})

#########################################
# ADDING SUB-DIRECTORIES TO THE PROJECT #
#########################################
# UI sources
target_include_directories(${PROJECT_NAME} PUBLIC src/ui)
# Shared libraries directories
target_include_directories(${PROJECT_NAME} PUBLIC libs/qcustomplot) # QCustomPlot

################
# GDAL library #
################
# Ajout du path 'anaconda' pour la recherche de librairie pour CMake
if (WIN32)
    set(GDAL_CUSTOM_PATH ${MSYS2_PATH})
else()
    set(GDAL_CUSTOM_PATH "/home/oboisot/anaconda3") # Anaconda3 gdal
endif()
# Append lib and include directories to CMAKE_PREFIX_PATH
list(APPEND CMAKE_PREFIX_PATH "${GDAL_CUSTOM_PATH}/lib" "${GDAL_CUSTOM_PATH}/include")

message("GDAL_CUSTOM_PATH = ${GDAL_CUSTOM_PATH}")
message("MSYS2_PATH = ${MSYS2_PATH}")

find_package(GDAL REQUIRED)
message("   -> GDAL include dir: ${GDAL_INCLUDE_DIRS}")
message("   -> GDAL libraries: ${GDAL_LIBRARIES}")
target_include_directories(${PROJECT_NAME} PUBLIC ${GDAL_INCLUDE_DIRS}) # Ajout du dossier des headers
target_link_libraries(${PROJECT_NAME} GDAL::GDAL) # Linkage de la librairie pour la compilation

######
# Qt #
######
# Version de Qt pour QCustomPlot
find_package(Qt6)
if (Qt6_FOUND)
    message("-- Found Qt6: ${Qt6_DIR}")
    set(QCUSTOMPLOT_QT6 ON)
else()
    find_package(Qt5 COMPONENTS Core)
    if (Qt5_FOUND)
        message("-- Found Qt5: ${Qt5_DIR}")
        set(QCUSTOMPLOT_QT6 OFF)
    else()
        message(FATAL_ERROR "Qt6 or Qt5 versions are necessary to link QCustomPlot library !")
    endif()
endif()
#################################
# Qt libraries for QCustomPlot #
#################################
if (QCUSTOMPLOT_QT6)
    find_package(Qt6 COMPONENTS Core Widgets Gui PrintSupport Concurrent REQUIRED)
    target_link_libraries(${PROJECT_NAME} Qt6::Core Qt6::Widgets Qt6::Gui Qt6::PrintSupport Qt6::Concurrent)
else()
    find_package(Qt5 COMPONENTS Core Widgets Gui PrintSupport Concurrent REQUIRED)
    target_link_libraries(${PROJECT_NAME} Qt5::Core Qt5::Widgets Qt5::Gui Qt5::PrintSupport Qt6::Concurrent)
endif()

###############
# QcustomPlot #
###############
if(QCUSTOMPLOT_QT6)
    if (WIN32)
        set(QCUSTOMPLOT_SODIR "${CMAKE_CURRENT_SOURCE_DIR}/libs/qcustomplot/libQt6QCustomPlot2.dll")
    else()
        set(QCUSTOMPLOT_SODIR "${CMAKE_CURRENT_SOURCE_DIR}/libs/qcustomplot/libQt6QCustomPlot2.so")
    endif()
else()
    if (WIN32)
        set(QCUSTOMPLOT_SODIR "${CMAKE_CURRENT_SOURCE_DIR}/libs/qcustomplot/libQt5QCustomPlot2.dll")
    else()
        set(QCUSTOMPLOT_SODIR "${CMAKE_CURRENT_SOURCE_DIR}/libs/qcustomplot/libQt5QCustomPlot2.so")
    endif()
endif()
# Linking with QCustomPlot library
target_link_libraries(${PROJECT_NAME} ${QCUSTOMPLOT_SODIR})
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_if_different
                   ${QCUSTOMPLOT_SODIR} ${CMAKE_BINARY_DIR})


###########
# INSTALL #
###########
if (WIN32)
    set(MY_DEPENDENCY_PATHS "${GDAL_CUSTOM_PATH}/bin" "D:/oboisot/Qt/6.2.2/mingw_64/bin")
else()
    set(MY_DEPENDENCY_PATHS "${GDAL_CUSTOM_PATH}/lib" "/home/oboisot/Qt/6.2.2/gcc_64/lib")
endif()
# Transfer the value of ${MY_DEPENDENCY_PATHS} into the install script
install(CODE "set(MY_DEPENDENCY_PATHS \"${MY_DEPENDENCY_PATHS}\")")
install(CODE [[
 file(GET_RUNTIME_DEPENDENCIES
      EXECUTABLES $<TARGET_FILE:GeoTiffDEMViewer>
      RESOLVED_DEPENDENCIES_VAR _r_deps
      UNRESOLVED_DEPENDENCIES_VAR _u_deps
      CONFLICTING_DEPENDENCIES_PREFIX _c_deps_pref
      DIRECTORIES ${MY_DEPENDENCY_PATHS}
      PRE_EXCLUDE_REGEXES "api-ms-*"
      POST_EXCLUDE_REGEXES ".*system32/.*\\.dll"
 )
 foreach(_file ${_r_deps})
     message("Installing file: ${_file}")
     file(INSTALL
          DESTINATION "${CMAKE_BINARY_DIR}/lib"
          TYPE SHARED_LIBRARY
          FOLLOW_SYMLINK_CHAIN
          FILES "${_file}"
     )
 endforeach()
 list(LENGTH _u_deps _u_length)
 if("${_u_length}" GREATER 0)
   message(WARNING "Unresolved dependencies detected!")
 endif()
 list(LENGTH _c_deps_pref_FILENAMES _c_deps_pref_length)
 if("${_c_deps_pref_length}" GREATER 0)
     foreach(lib_conflict ${_c_deps_pref_FILENAMES})
         message(STATUS "${lib_conflict} found at: ${_c_deps_pref_${lib_conflict}}")
     endforeach()
 endif()
]])
