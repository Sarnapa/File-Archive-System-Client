#ifndef LOGGINGFORM_H
#define LOGGINGFORM_H

#include <QDialog>
#include <QRegExpValidator>

namespace Ui {
class LoggingForm;
}

class LoggingForm : public QDialog
{
    Q_OBJECT

public:
    explicit LoggingForm(QWidget *parent = 0);
    ~LoggingForm();

signals:
    void connectSignal(QString &login, QString &password, QString &address);

private slots:
    void on_connectButton_clicked();
    void activateConnectButton();
private:
    Ui::LoggingForm *ui;

};

#endif // LOGGINGFORM_H
