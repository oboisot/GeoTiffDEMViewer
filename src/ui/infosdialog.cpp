#include "infosdialog.h"

InfosDialog::InfosDialog(QWidget *parent) : QDialog(parent)
{
    m_textEdit = new QPlainTextEdit(this);
    m_textEdit->setReadOnly(true);
    m_textEdit->document()->setDefaultFont(QFontDatabase::systemFont(QFontDatabase::FixedFont)); // Define a fixed-size font
    // Close Button
    QPushButton *closeButton = new QPushButton(this);
    closeButton->setToolTip("Close window");
    closeButton->setIcon(QIcon(":/qss/dark/icons/svg@96x96/application-exit.svg"));
    closeButton->setMaximumSize(40, 40);
    connect(closeButton, &QPushButton::released, this, [&](){ this->close(); });
    // Add QPlainTextEdit, closeButton to a layout
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(m_textEdit);
    layout->addWidget(closeButton);
    layout->setAlignment(closeButton, Qt::AlignHCenter);
    // QDialog customization
    this->setSizeGripEnabled(true);
}

InfosDialog::~InfosDialog() {}

void InfosDialog::setText(const QString &text)
{
    m_textEdit->setPlainText(text);
}
