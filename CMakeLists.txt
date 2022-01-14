cmake_minimum_required(VERSION 3.21)

project(GeoTiffDEMViewer LANGUAGES CXX VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)
# Instruct CMake to run resource files automatically when needed.
set(CMAKE_AUTORCC ON)
# Instruct CMake to run uic automatically when needed.
set(CMAKE_AUTOUIC ON)

###################
# SETTING LIB/EXE #
###################
# Headers list
set(GTIFFDEMVIEWER_HEADERS # GeoTiff DEM
                           src/geotiffdem.h
                           src/qgeotiffdem.h
                           # UI
                           src/ui/geotiffdemviewerwindow.h
                           src/ui/qdemcolormap.h)

# Sources list
set(GTIFFDEMVIEWER_SOURCES # GeoTiff DEM
                           src/geotiffdem.cpp
                           src/qgeotiffdem.cpp
                           # UI
                           src/ui/geotiffdemviewerwindow.cpp
                           src/ui/qdemcolormap.cpp
                           # Main
                           GeoTiffDEMViewer.cpp
                           # Resources files
                           GeoTiffDEMViewer.qrc)

add_executable(${PROJECT_NAME} ${GTIFFDEMVIEWER_HEADERS} ${GTIFFDEMVIEWER_SOURCES})
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
    set(GDAL_CUSTOM_PATH "D:/oboisot/msys64/mingw64") # MSYS2 mingw-w64-x86_64-gdal
else()
    set(GDAL_CUSTOM_PATH "/home/oboisot/anaconda3") # Anaconda3 gdal
endif()
# Append lib and include directories to CMAKE_PREFIX_PATH
list(APPEND CMAKE_PREFIX_PATH "${GDAL_CUSTOM_PATH}/lib" "${GDAL_CUSTOM_PATH}/include")

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
    message("-- Found Qt6")
    set(QCUSTOMPLOT_QT6 ON)
else()
    find_package(Qt5 COMPONENTS Core)
    if (Qt5_FOUND)
        message("-- Found Qt5")
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


