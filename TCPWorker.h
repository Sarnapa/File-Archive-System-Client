#ifndef TCPWORKER_H
#define TCPWORKER_H

#include <QObject>
#include <QThread>
#include <QDebug>
#include <QList>
#include <QByteArray>
#include <QDir>
#include <QFile>
#include <QTcpSocket>
#include <QHostAddress>
#include <algorithm>
#include <QDataStream>
#include <stdio.h>
#include "Command.h"
#include "MyFileInfo.h"


class TCPWorker : public QObject
{
    Q_OBJECT
public:
    explicit TCPWorker(QObject *parent = 0);
    ~TCPWorker();

    void connectToSystem(QString login, QString password, QString address);
    void disconnect();
    void refresh();
    void deleteFile(QString fileName);
    void uploadFile(QString fileName, qlonglong size, QDateTime lastModified);
    void downloadFile(QString fileName);
signals:
    void connectedToSystemSignal(bool connected, QList<MyFileInfo> *userFiles);
    void disconnectedSignal();
    void refreshedSignal(bool connected, QList<MyFileInfo> *userFiles);
    void deletedFileSignal(bool connected, QString fileName);
    void gotUploadACKSignal(bool connected, QString fileName, qlonglong size, QDateTime lastModified);
    void gotDownloadACKSignal(bool connected, QString fileName);
    // for PAIN
    void onTimeout();
private:
    QByteArray *receivedData;
    QTcpSocket *socket;
    QString login, password , address;
    //for PAIN
    int actionId;
    bool isConnected = false;
    const QString adminLogin = "admin";
    const QString adminPassword = "admin";
    QList<MyFileInfo> *userFiles;
    //for deleting file
    QString currentFileName;
    //for updating/downloading file
    MyFileInfo currentFile;
    QList<MyFileInfo>* getFilesFromSystem() const;
private slots:
    void connected();
    void disconnected();
    void gotResp();
    // PAIN
    void gotResponse();
};

#endif // TCPWORKER_H
