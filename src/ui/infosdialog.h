#ifndef INFOSDIALOG_H
#define INFOSDIALOG_H

#include <QDialog>
#include <QPlainTextEdit>
#include <QFontDatabase>
#include <QVBoxLayout>
#include <QPushButton>

class InfosDialog : public QDialog
{
    Q_OBJECT
public:
    explicit InfosDialog(QWidget *parent = nullptr);
    ~InfosDialog();
    void setText(const QString &text);

private:
    QPlainTextEdit *m_textEdit;
};

#endif // INFOSDIALOG_H
