#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileSystemModel>
#include <QMessageBox>
#include <QLabel>
#include "LoggingForm.h"
#include "RemoteListModel.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

signals:
    void progressBarValueChanged(int value);
private:
    Ui::MainWindow *ui;
    QLabel *statusLabel;

    QFileSystemModel *localModel;
    RemoteListModel *remoteModel;
    LoggingForm *logForm;
    bool connectionStatus = false;
    bool actionStatus = false;
    void updateWindow();

    // for PAIN
    const QString path = "C:" + QString(QDir::separator()) + "Qt" + QString(QDir::separator()) + "QtProjects" + QString(QDir::separator()) +
            "FileArchiveSystemClient" + QString(QDir::separator()) + "local" + QString(QDir::separator());
private slots:
    void on_actionConnect_triggered();
    void on_actionDisconnect_triggered();
    void on_actionRefresh_triggered();
    void on_actionRename_triggered();
    void on_actionDelete_triggered();
    void on_actionCancel_triggered();

    void connectToSystem(QString &login, QString &password, QString &address);
    void connectedToSystem(bool connected);
    void disconnected();
    void refreshed(bool connected);
    void deletedFile(bool connected);
    //void renamedFIle(bool connected);
    void uploadFile();
    void gotUploadACK(bool connected, QString fileName, int progressBarValue);
    void gotUploadAccept(bool connected, QString fileName);
    void downloadFile();
    void gotDownloadACK(bool connected, int progressBarValue, QString fileName);

    void closeEvent(QCloseEvent *event) override;
};

#endif // MAINWINDOW_H
