#include "TCPWorker.h"

TCPWorker::TCPWorker(QObject *parent) : QObject(parent)
{
    userFiles = new QList<MyFileInfo>;
}

TCPWorker::~TCPWorker()
{
    delete userFiles;
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
    receivedCommand = Command();

    connect(socket, SIGNAL(connected()), this, SLOT(connected()));
    connect(socket, SIGNAL(disconnected()),this, SLOT(disconnected()));
    connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(gotError(QAbstractSocket::SocketError)), Qt::ConnectionType::DirectConnection);
    connect(socket, SIGNAL(readyRead()), this, SLOT(gotResp()));

    //to abort previous connection. On an unconnected socket, this function does nothing.
    socket->abort();
    // this is not blocking call
    socket->connectToHost(address, 11000); //11000 2666

    if(!socket->waitForConnected(5000))
        emit connectedToSystemSignal(false, userFiles);
}

void TCPWorker::disconnect()
{
    socket->disconnectFromHost();
}

void TCPWorker::refresh()
{
    qRegisterMetaType<MyFileInfo>();
    emit refreshedSignal(isConnected(), userFiles);
}

void TCPWorker::deleteFile(QString fileName)
{

}

void TCPWorker::cancel()
{
    qDebug("Cancel");
    switch(currentStatus)
    {
        case START_UPLOAD_FILE_ACTION:
        {
            fileService->fileClose();
            currentStatus = LOGGED;
            emit gotUploadACKSignal(isConnected(), MyFileInfo(fileService->getFileInfo()), -1);
            break;
        }
        case WAIT_FOR_UPDATE_ACCEPT:
        {
            currentStatus = LOGGED;
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
    if(fileService->isFileOpen())
    {
        currentStatus = START_UPLOAD_FILE_ACTION;
        qint64 fileSize = fileService->getFileSize();
        SerializationLayer uploadCmdSerial(UPLOAD);
        uploadCmdSerial.serializeData((quint64)fileSize, fileService->getFileName(), login);
        Command cmd(uploadCmdSerial.getCodeBytes(), uploadCmdSerial.getSizeBytes(), uploadCmdSerial.getDataBytes());
        transportLayer->sendCmd(cmd);
    }
    else
    {
        qDebug() << "Cannot open file.";
    }
}

void TCPWorker::downloadFile(QString fileName)
{

}

void TCPWorker::sendUploadChunks()
{
    qDebug() << "START UPLOAD";
    currentStatus = UPLOAD_FILE_ACTION;
    QByteArray fileBlock;
    qint64 fileSize = fileService->getFileSize();
    QFileInfo fileInfo = fileService->getFileInfo();
    unsigned int i = 0;
    while(!isStopped && currentSize < fileSize)
    {
        if(isConnected())
        {
            SerializationLayer chunkCmdSerial(CHUNK);
            qDebug() << "UPLOAD: " << i;
            fileBlock = fileService->getFileBlock();
            //qDebug() << "FILEBLOCK: " << fileBlock;
            chunkCmdSerial.serializeData(fileBlock);
            Command cmd(chunkCmdSerial.getCodeBytes(), chunkCmdSerial.getSizeBytes(), chunkCmdSerial.getDataBytes());
            //qDebug() << "CMD: " << chunkCmdSerial.getCodeBytes().toHex() << " " << chunkCmdSerial.getSizeBytes().toHex() << " " << chunkCmdSerial.getDataBytes();
            transportLayer->sendCmd(cmd);
            currentSize += strlen(fileBlock);
            qDebug() << "CURRENT_SIZE: " << currentSize;
        }
        else
            isStopped = true;
        ++i;
        emit gotUploadACKSignal(isConnected(), MyFileInfo(fileInfo), currentSize);
    }
    fileService->fileClose();
    if(!isStopped && currentSize == fileSize)
    {
        qDebug() << "File sended to system.";
        currentStatus = WAIT_FOR_UPDATE_ACCEPT;
    }
    else
    {
        currentStatus = LOGGED;
        emit gotUploadACKSignal(isConnected(), MyFileInfo(fileInfo), -1);
    }
    currentSize = 0;
    isStopped = false;
}


void TCPWorker::gotResp()
{
    STATE stateCmd = receivedCommand.getState();
    switch(stateCmd)
    {
        case WAIT_FOR_CODE:
        {
            receivedCommand.setCode(transportLayer->getCmdCode());
            cmdSerial.deserializeCode(receivedCommand.getCode());
            qDebug() << "Received: " << cmdSerial.getCode();
            switch(cmdSerial.getCode())
            {
                case ACCEPT:
                {
                    qDebug() << "Got ACCEPT";
                    switch(currentStatus)
                    {
                        case CONNECTED:
                        {
                            qRegisterMetaType<MyFileInfo>();
                            currentStatus = LOGGED;
                            receivedCommand = Command();
                            emit connectedToSystemSignal(true, userFiles);
                            break;
                        }
                        case START_UPLOAD_FILE_ACTION:
                        {
                            sendUploadChunks();
                            break;
                        }
                        case WAIT_FOR_UPDATE_ACCEPT:
                        {
                            currentStatus = LOGGED;
                            receivedCommand = Command();
                            emit gotUploadAcceptSignal(isConnected(), MyFileInfo(fileService->getFileInfo()));
                            break;
                        }
                    }
                    break;
                }
            }
            break;
        }
        case WAIT_FOR_SIZE:
        {
            receivedCommand.setSize(transportLayer->getCmdSize());
            break;
        }
        case WAIT_FOR_DATA:
        {
            int size = receivedCommand.getSize().toInt();
            receivedCommand.setData(transportLayer->getCmdData(size));
            break;
        }
        case GOT_DATA:
        {
            switch(cmdSerial.getCode())
            {
                case CHUNK:
                {
                    switch(currentStatus)
                    {
                        // files list
                        case CONNECTED:
                            qRegisterMetaType<MyFileInfo>();
                            cmdSerial.deserialize(receivedCommand, true);
                            currentStatus = LOGGED;
                            receivedCommand = Command();
                            emit connectedToSystemSignal(true, cmdSerial.getFilesList());
                            break;
                        case DOWNLOAD_FILE_ACTION:
                            cmdSerial.deserialize(receivedCommand, false);
                            break;
                    }
                    break;
                }
                case ERROR:
                {
                    cmdSerial.deserialize(receivedCommand, false);
                    break;
                }
                default:
                {
                    emit disconnect();
                    break;
                }
            }
            receivedCommand = Command();
            break;
        }
        case WRONG_CMD:
        {
           receivedCommand = Command();
           emit disconnect();
           break;
        }
    }
}

void TCPWorker::connected()
{
    qDebug() << "Connected to " << socket->peerAddress().toString();
    currentStatus = CONNECTED;
    SerializationLayer loginCmdSerial(LOGIN);
    loginCmdSerial.serializeData(login, password);
    Command cmd(loginCmdSerial.getCodeBytes(), loginCmdSerial.getSizeBytes(), loginCmdSerial.getDataBytes());
    transportLayer->sendCmd(cmd);
}

void TCPWorker::disconnected()
{
    qDebug() << "Disconnected";
    isStopped = false;
    currentSize = 0;
    currentStatus = DISCONNECTED;
    socket->close();
    emit disconnectedSignal();
}

void TCPWorker::gotError(QAbstractSocket::SocketError error)
{
    switch(error)
    {
        case QAbstractSocket::RemoteHostClosedError:
        {
            qDebug() << "RemoteHostClosedError";
            emit disconnect();
            break;
        }
        case QAbstractSocket::NetworkError:
        {
            qDebug() << "NetworkError";
            emit disconnect();
            break;
        }
    }
}
