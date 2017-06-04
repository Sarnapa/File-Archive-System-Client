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

enum DISCONNECT_REASON
{
    NORMAL = 0,
    REMOTE_HOST_CLOSED_ERROR = 1,
    NETWORK_ERROR = 2,
    ERROR_LOGGING = 3,
    NOT_RECEIVE_FILES_LIST = 4,
    WRONG_CMD_ERROR = 5,
    SEND_ERROR = 6
};

Q_DECLARE_METATYPE(DISCONNECT_REASON)

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
    void renameFile(QString oldFileName, QString newFileName);
    void deleteFile(QString fileName);
    void uploadFile(QFileInfo fileInfo);
    void downloadFile(MyFileInfo fileInfo);

signals:
    void connectedToSystemSignal(bool connected, QList<MyFileInfo> *userFiles);
    void disconnectedSignal(DISCONNECT_REASON disconnectReason);
    void refreshedSignal(bool connected, QList<MyFileInfo> *userFiles);
    void renamedFileSignal(bool connected, QString oldFileName, QString newFileName);
    void deletedFileSignal(bool connected, QString fileName);
    void gotUploadACKSignal(bool connected, MyFileInfo fileInfo, qint64 currentSize);
    void gotUploadAcceptSignal(bool connected, MyFileInfo fileInfo);
    void gotDownloadACKSignal(bool connected, MyFileInfo fileInfo, qint64 currentSize);
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
    QString newFileName;
    qint64 currentSize = 0;
    DISCONNECT_REASON disconnectReason = NORMAL;

    inline bool isConnected() { return socket->state() == QTcpSocket::ConnectedState; }
    void getResp();
    void gotLoggingResponse();
    void gotRefreshResponse();
    void gotDeleteResponse();
    void gotRenamedResponse();
    void gotFileChunk();
    void gotFilesListResponse();
    void gotErrorResponse();
    void sendUploadChunks();
private slots:
    void connected();
    void disconnected();
    void gotError(QAbstractSocket::SocketError error);
    //void gotResp();
};

#endif // TCPWORKER_H
