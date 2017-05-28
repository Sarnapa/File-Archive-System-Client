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
    UPLOAD_FILE_ACTION = 6,
    UPLOADED_FILE = 7,
    DOWNLOAD_FILE_ACTION = 8
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
    void uploadFile(QFileInfo fileInfo);//(QString fileName, qlonglong size, QDateTime lastModified);
    void downloadFile(QString fileName);

signals:
    void connectedToSystemSignal(bool connected, QList<QFileInfo> *userFiles);
    void disconnectedSignal();
    void refreshedSignal(bool connected, QList<QFileInfo> *userFiles);
    void deletedFileSignal(bool connected, QString fileName);
    void gotUploadACKSignal(bool connected, QFileInfo fileInfo, qint64 currentSize);
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
    qint64 currentSize = 0;
    QList<QFileInfo> *userFiles;

    inline bool isConnected() { return socket->state() == QTcpSocket::ConnectedState; }
    void sendUploadChunks();
private slots:
    void connected();
    void disconnected();
    void gotError(QAbstractSocket::SocketError error);
    void gotResp();
};

#endif // TCPWORKER_H
