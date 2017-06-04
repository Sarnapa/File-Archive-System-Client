#include "TCPWorker.h"

TCPWorker::TCPWorker(QObject *parent) : QObject(parent)
{}

TCPWorker::~TCPWorker()
{
    delete socket;
}

void TCPWorker::connectToSystem(QString login, QString password, QString address)
{
    this->login = login;
    this->password = password;

    qRegisterMetaType<MyFileInfo>();
    socket = new QTcpSocket;
    socket->setParent(0);
    socketStream.setDevice(socket);
    socketStream.setVersion(QDataStream::Qt_5_9);
    transportLayer = new TransportLayer(socket, this);
    fileService = new FileService(this);
    disconnectReason = NORMAL;
    //receivedCommand = Command();

    connect(socket, SIGNAL(connected()), this, SLOT(connected()));
    connect(socket, SIGNAL(disconnected()),this, SLOT(disconnected()), Qt::ConnectionType::DirectConnection);
    connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(gotError(QAbstractSocket::SocketError)), Qt::ConnectionType::DirectConnection);
    //connect(socket, SIGNAL(readyRead()), this, SLOT(gotResp()));

    //to abort previous connection. On an unconnected socket, this function does nothing.
    socket->abort();
    // this is not blocking call
    int pos = address.indexOf(':');
    QString ip = address.mid(0, pos);
    QString port = address.mid(pos + 1, address.length() - pos - 1);
    socket->connectToHost(ip, port.toInt());

    if(isConnected())
        return;

    if(!socket->waitForConnected(5000))
        emit connectedToSystemSignal(false, NULL);
}

void TCPWorker::disconnect()
{
    socket->disconnectFromHost();
}

void TCPWorker::refresh()
{
    currentStatus = REFRESH_ACTION;
    SerializationLayer listRequest(LIST);
    listRequest.serializeData(login);
    Command cmd(listRequest.getCodeBytes(), listRequest.getSizeBytes(), listRequest.getDataBytes());
    if(!transportLayer->sendCmd(cmd))
    {
        disconnectReason = SEND_ERROR;
        emit disconnect();
    }
    else
    {
        qDebug() << "sended.";
        getResp();
    }
}

void TCPWorker::gotRefreshResponse()
{
    cmdSerial.deserialize(receivedCommand, true);
    qRegisterMetaType<MyFileInfo>();
    currentStatus = LOGGED;
    emit refreshedSignal(isConnected(), cmdSerial.getFilesList());
}

void TCPWorker::renameFile(QString oldFileName, QString newFileName)
{
    currentStatus = RENAME_ACTION;
    SerializationLayer renameRequest(RENAME);
    currentFileName = oldFileName;
    this->newFileName = newFileName;
    renameRequest.serializeData(oldFileName, newFileName, login);
    Command cmd(renameRequest.getCodeBytes(), renameRequest.getSizeBytes(), renameRequest.getDataBytes());
    if(!transportLayer->sendCmd(cmd))
    {
        disconnectReason = SEND_ERROR;
        emit disconnect();
    }
    else
    {
        qDebug() << "sended.";
        getResp();
    }
}

void TCPWorker::gotRenamedResponse()
{
    currentStatus = LOGGED;
    emit renamedFileSignal(isConnected(), currentFileName, newFileName);
    currentFileName = QString();
    newFileName = QString();
}

void TCPWorker::deleteFile(QString fileName)
{
    currentStatus = DELETE_FILE_ACTION;
    currentFileName = fileName;
    SerializationLayer listRequest(DELETE);
    listRequest.serializeData(currentFileName, login);
    Command cmd(listRequest.getCodeBytes(), listRequest.getSizeBytes(), listRequest.getDataBytes());
    if(!transportLayer->sendCmd(cmd))
    {
        disconnectReason = SEND_ERROR;
        emit disconnect();
    }
    else
    {
        qDebug() << "sended.";
        getResp();
    }
}

void TCPWorker::gotDeleteResponse()
{
    currentStatus = LOGGED;
    emit deleteFile(currentFileName);
    currentFileName = QString();
}

void TCPWorker::cancel()
{
    switch(currentStatus)
    {
        case START_UPLOAD_FILE_ACTION:
        {
            fileService->fileClose();
            isStopped = true;
            emit gotUploadACKSignal(isConnected(), MyFileInfo(fileService->getFileInfo()), -1);
            break;
        }
        case WAIT_FOR_UPDATE_ACCEPT:
        {
            isStopped = true;
            emit gotUploadACKSignal(isConnected(), MyFileInfo(fileService->getFileInfo()), -1);
            break;
        }
        default:
        {
            isStopped = true;
            break;
        }
    }
}

void TCPWorker::uploadFile(QFileInfo fileInfo)
{
    fileService->setFileInfo(fileInfo);
    if(fileService->isFileOpen(true))
    {
        currentStatus = START_UPLOAD_FILE_ACTION;
        qint64 fileSize = fileService->getFileSize();
        SerializationLayer uploadCmdSerial(UPLOAD);
        uploadCmdSerial.serializeData((quint64)fileSize, fileService->getFileName(), login);
        Command cmd(uploadCmdSerial.getCodeBytes(), uploadCmdSerial.getSizeBytes(), uploadCmdSerial.getDataBytes());
        if(!transportLayer->sendCmd(cmd))
        {
            fileService->fileClose();
            disconnectReason = SEND_ERROR;
            emit disconnect();
        }
        else
        {
            qDebug() << "sended.";
            getResp();
        }
    }
    else
    {
        qDebug() << "Cannot open file.";
        emit gotUploadACKSignal(isConnected(), MyFileInfo(fileService->getFileInfo()), -2);
    }
}


void TCPWorker::downloadFile(MyFileInfo fileInfo)
{
    fileService->setMyFileInfo(fileInfo);
    if(fileService->isFileOpen(false))
    {
        currentStatus = DOWNLOAD_FILE_ACTION;
        SerializationLayer downloadCmdSerial(DOWNLOAD);
        downloadCmdSerial.serializeData(0,(quint32)fileInfo.getFileSize(), fileInfo.getFileName(), login);
        Command cmd(downloadCmdSerial.getCodeBytes(), downloadCmdSerial.getSizeBytes(), downloadCmdSerial.getDataBytes());
        qDebug() << "DF CODE: " << cmd.getCode().toHex();
        qDebug() << "DF SIZE: " << cmd.getSize().toHex();
        qDebug() << "DF DATA: " << cmd.getData();
        if(!transportLayer->sendCmd(cmd))
        {
            fileService->fileClose();
            disconnectReason = SEND_ERROR;
            emit disconnect();
        }
        else
        {
            qDebug() << "sended.";
            getResp();
        }
    }
    else
    {
        qDebug() << "Cannot open file.";
        emit gotDownloadACKSignal(isConnected(), fileInfo, -2);
    }
}

void TCPWorker::gotFileChunk()
{
    QByteArray fileBlock;
    qint64 fileSize = fileService->getFileSize();
    MyFileInfo fileInfo = fileService->getMyFileInfo();
    if(!isStopped)
    {
        if(isConnected())
        {
            cmdSerial.deserialize(receivedCommand, false);
            fileBlock = cmdSerial.getFileChunk();
            qDebug() << "FILE_BLOCK_SIZE" << fileBlock.size();
            qDebug() << "FILE_BLOCK: " << fileBlock;
            fileService->writeFileBlock(fileBlock);
            currentSize += strlen(fileBlock);
            qDebug() << "CURRENT_SIZE: " << currentSize;
            if(currentSize == fileSize)
            {
                qDebug() << "File saved.";
                fileService->fileClose();
                emit gotDownloadACKSignal(isConnected(), fileInfo, currentSize);
                currentStatus = LOGGED;
                currentSize = 0;
                isStopped = false;
            }
            else
            {
                emit gotDownloadACKSignal(isConnected(), fileInfo, currentSize);
            }
        }
        else
        {
            qDebug() << "File not saved.";
            fileService->fileClose();
            currentStatus = DISCONNECTED;
            currentSize = 0;
            isStopped = false;
            emit gotDownloadACKSignal(isConnected(), fileInfo, -1);
        }
    }
    else
    {
        qDebug() << "File not saved.";
        fileService->fileClose();
        currentStatus = LOGGED;
        currentSize = 0;
        isStopped = false;
        emit gotDownloadACKSignal(isConnected(), fileInfo, -1);
    }
}

void TCPWorker::sendUploadChunks()
{
    currentStatus = UPLOAD_FILE_ACTION;
    QByteArray fileBlock;
    bool sended = false;
    qint64 fileSize = fileService->getFileSize();
    QFileInfo fileInfo = fileService->getFileInfo();
    while(!isStopped && currentSize < fileSize)
    {
        if(isConnected())
        {
            SerializationLayer chunkCmdSerial(CHUNK);
            fileBlock = fileService->getFileBlock();
            chunkCmdSerial.serializeData(fileBlock);
            Command cmd(chunkCmdSerial.getCodeBytes(), chunkCmdSerial.getSizeBytes(), chunkCmdSerial.getDataBytes());
            while(!isStopped)
            {
                if(transportLayer->sendCmd(cmd))
                {
                    sended = true;
                    break;
                }
            }
            if(!sended)
            {
                fileService->fileClose();
                disconnectReason = SEND_ERROR;
                currentSize = 0;
                isStopped = false;
                emit disconnect();
                return;
            }
            else
            {
                currentSize += strlen(fileBlock);
                qDebug() << "CURRENT_SIZE: " << currentSize;
            }
        }
        else
            isStopped = true;
        emit gotUploadACKSignal(isConnected(), MyFileInfo(fileInfo), currentSize);
    }
    fileService->fileClose();
    if(!isStopped && currentSize >= fileSize)
    {
        qDebug() << "File sended to system.";
        currentStatus = WAIT_FOR_UPDATE_ACCEPT;
        currentSize = 0;
        isStopped = false;
        qDebug() << "sended.";
        getResp();
    }
    else
    {
        currentStatus = LOGGED;
        currentSize = 0;
        isStopped = false;
        emit gotUploadACKSignal(isConnected(), MyFileInfo(fileInfo), -1);
    }
}

void TCPWorker::getResp()
{
    switch(currentStatus)
    {
        case START_UPLOAD_FILE_ACTION:
        case WAIT_FOR_UPDATE_ACCEPT:
        {
            while(!isStopped && receivedCommand.getState() == WRONG_CMD)
            {
                qDebug() << "WRONG_CMD1";
                receivedCommand = transportLayer->getCmd(1000);
            }
            break;
        }
        case DOWNLOAD_FILE_ACTION:
        {
            while(!isStopped && receivedCommand.getState() == WRONG_CMD)
            {
                unsigned int i = 0;
                qDebug() << "WRONG_CMD2";
                receivedCommand = transportLayer->getCmd(20000);
                ++i;
                if(i == 5)
                    break;
            }
            break;
        }
        default:
        {
            receivedCommand = transportLayer->getCmd();
            break;
        }
    }
    if(receivedCommand.getState() != WRONG_CMD)
    {
        cmdSerial.deserializeCode(receivedCommand.getCode());
        qDebug() << "RECEIVED: " << cmdSerial.getCode();
        switch(cmdSerial.getCode())
        {
            case ACCEPT:
            {
                switch(currentStatus)
                {
                    case CONNECTED:
                    {
                        gotLoggingResponse();
                        break;
                    }
                    case RENAME_ACTION:
                    {
                        gotRenamedResponse();
                        break;
                    }
                    case DELETE_FILE_ACTION:
                    {
                        gotDeleteResponse();
                        break;
                    }
                    case START_UPLOAD_FILE_ACTION:
                    {
                        qDebug() << "START_FOR_UPDATE_ACCEPT";
                        sendUploadChunks();
                        break;
                    }
                    case WAIT_FOR_UPDATE_ACCEPT:
                    {
                        qDebug() << "WAIT_FOR_UPDATE_ACCEPT";
                        currentStatus = LOGGED;
                        qRegisterMetaType<MyFileInfo>();
                        emit gotUploadAcceptSignal(isConnected(), MyFileInfo(fileService->getFileInfo()));
                        break;
                    }
                }
                break;
            }
            case CHUNK:
            {
                switch(currentStatus)
                {
                    case LOGGED:
                    {
                        gotFilesListResponse();
                        break;
                    }
                    case REFRESH_ACTION:
                    {
                        gotRefreshResponse();
                        break;
                    }
                    case DOWNLOAD_FILE_ACTION:
                    {
                        gotFileChunk();
                        break;
                    }
                }
                break;
            }
            case ERROR:
            {
                gotErrorResponse();
                break;
            }
            default:
            {
                disconnectReason = WRONG_CMD_ERROR;
                emit disconnect();
                break;
            }
        }
    }
    else
    {
        switch(currentStatus)
        {
            case CONNECTED:
            {
                emit disconnect();
                break;
            }
            case LOGGED:
            case REFRESH_ACTION:
            {
                disconnectReason = NOT_RECEIVE_FILES_LIST;
                emit disconnect();
                break;
            }
            case DELETE_FILE_ACTION:
            {
                disconnectReason = WRONG_CMD_ERROR;
                currentFileName = QString();
                emit disconnect();
                break;
            }
            case RENAME_ACTION:
            {
                disconnectReason = WRONG_CMD_ERROR;
                currentFileName = QString();
                newFileName = QString();
                emit disconnect();
                break;
            }
            case DOWNLOAD_FILE_ACTION:
            case START_UPLOAD_FILE_ACTION:
            case WAIT_FOR_UPDATE_ACCEPT:
            {
                currentStatus = LOGGED;
                break;
            }
        }
    }
    isStopped = false;
    receivedCommand = Command();
}

void TCPWorker::gotErrorResponse()
{
    cmdSerial.deserialize(receivedCommand, false);
    switch(cmdSerial.getErrorCode())
    {
        case MISSING_FILE:
        {
            if(currentStatus == DOWNLOAD_FILE_ACTION)
            {
                qDebug() << "MISSING_FILE";
                fileService->fileClose();
                currentSize = 0;
                currentStatus = LOGGED;
                qRegisterMetaType<MyFileInfo>();
                emit gotDownloadACKSignal(isConnected(), fileService->getMyFileInfo(), -3);
            }
            break;
        }
        case LOGGING_ERROR:
        {
            if(currentStatus == CONNECTED)
            {
                qDebug() << "LOGGING_ERROR";
                disconnectReason = ERROR_LOGGING;
                emit disconnect();
            }
            break;
        }
    }
}

void TCPWorker::connected()
{
    qDebug() << "Connected to " << socket->peerAddress().toString() << " " << socket->peerPort();
    currentStatus = CONNECTED;
    SerializationLayer loginCmdSerial(LOGIN);
    loginCmdSerial.serializeData(login, password);
    Command cmd(loginCmdSerial.getCodeBytes(), loginCmdSerial.getSizeBytes(), loginCmdSerial.getDataBytes());
    if(!transportLayer->sendCmd(cmd))
    {
        disconnectReason = SEND_ERROR;
        emit disconnect();
    }
    else
    {
        qDebug() << "sended.";
        getResp();
    }
}

void TCPWorker::gotLoggingResponse()
{
    currentStatus = LOGGED;
    SerializationLayer listRequest(LIST);
    listRequest.serializeData(login);
    Command cmd(listRequest.getCodeBytes(), listRequest.getSizeBytes(), listRequest.getDataBytes());
    if(!transportLayer->sendCmd(cmd))
    {
        disconnectReason = SEND_ERROR;
        emit disconnect();
    }
    else
    {
        qDebug() << "sended.";
        getResp();
    }
}

void TCPWorker::gotFilesListResponse()
{
    qRegisterMetaType<MyFileInfo>();
    cmdSerial.deserialize(receivedCommand, true);
    emit connectedToSystemSignal(true, cmdSerial.getFilesList());
}

void TCPWorker::disconnected()
{
    qDebug() << "Disconnected";
    isStopped = false;
    currentSize = 0;
    currentStatus = DISCONNECTED;
    socket->close();
    qRegisterMetaType<DISCONNECT_REASON>();
    emit disconnectedSignal(disconnectReason);
}

void TCPWorker::gotError(QAbstractSocket::SocketError error)
{
    switch(error)
    {
        case QAbstractSocket::RemoteHostClosedError:
        {
            qDebug() << "RemoteHostClosedError";
            if(currentStatus != CONNECTED)
            {
                isStopped = true;
                disconnectReason = REMOTE_HOST_CLOSED_ERROR;
                emit disconnect();
            }
            break;
        }
        case QAbstractSocket::NetworkError:
        {
            qDebug() << "NetworkError";
            disconnectReason = NETWORK_ERROR;
            emit disconnect();
            break;
        }
    }
}
