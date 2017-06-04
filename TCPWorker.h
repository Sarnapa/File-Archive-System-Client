#ifndef TCPWORKER_H
#define TCPWORKER_H

#include <QList>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QHostAddress>
#include <algorithm>
#include <QDateTime>
#include "SerializationLayer.h"
#include "TransportLayer.h"
#include "FileService.h"

enum STATUS
{
    DISCONNECTED = 0,
    CONNECTED = 1,
    LOGGED = 2,
    REFRESH_ACTION = 3,
    RENAME_ACTION = 4,
    DELETE_FILE_ACTION = 5,
    START_UPLOAD_FILE_ACTION = 6,
    UPLOAD_FILE_ACTION = 7,
    WAIT_FOR_UPDATE_ACCEPT = 8,
    DOWNLOAD_FILE_ACTION = 9
};

class TCPWorker : public QObject
{
    Q_OBJECT
public:
    explicit TCPWorker(QObject *parent = 0);
    ~TCPWorker();

public slots:
    void connectToSystem(QString login, QString password, QString address);
    void disconnect();
    void cancel();
    void refresh();
    void deleteFile(QString fileName);
    void uploadFile(QFileInfo fileInfo);
    void downloadFile(MyFileInfo fileInfo);

signals:
    void connectedToSystemSignal(bool connected, QList<MyFileInfo> *userFiles);
    void disconnectedSignal();
    void refreshedSignal(bool connected, QList<MyFileInfo> *userFiles);
    void deletedFileSignal(bool connected, QString fileName);
    void gotUploadACKSignal(bool connected, MyFileInfo fileInfo, qint64 currentSize);
    void gotUploadAcceptSignal(bool connected, MyFileInfo fileInfo);
    void gotDownloadACKSignal(bool connected, QString fileName);
private:
    QTcpSocket *socket;
    QDataStream socketStream;
    QString login, password;
    bool isStopped = false;
    STATUS currentStatus = DISCONNECTED;
    Command receivedCommand;
    SerializationLayer cmdSerial;
    TransportLayer *transportLayer;
    FileService *fileService;
    QString currentFileName;
    qint64 currentSize = 0;
    QList<MyFileInfo> *userFiles;

    inline bool isConnected() { return socket->state() == QTcpSocket::ConnectedState; }
    void getResp();
    void gotLoggingResponse();
    void gotRefreshResponse();
    void gotDeleteResponse();
    void sendUploadChunks();
private slots:
    void connected();
    void disconnected();
    void gotError(QAbstractSocket::SocketError error);
    //void gotResp();
};

#endif // TCPWORKER_H
