#include "TCPWorker.h"

TCPWorker::TCPWorker(QObject *parent) : QObject(parent)
{
    userFiles = new QList<QFileInfo>;
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

    socket = new QTcpSocket;
    socket->setParent(0);
    socketStream.setDevice(socket);
    socketStream.setVersion(QDataStream::Qt_5_9);
    transportLayer = new TransportLayer(socket, this);
    fileService = new FileService(this);
    //cmdSerial.setParent(this);

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
    emit refreshedSignal(isConnected(), userFiles);
}

void TCPWorker::deleteFile(QString fileName)
{

}

void TCPWorker::cancel()
{
    qDebug("Cancel");
    isStopped = true;
}

void TCPWorker::uploadFile(QFileInfo fileInfo)//(QString fileName, qlonglong size, QDateTime lastModified)
{
    fileService->setFileInfo(fileInfo);
    if(fileService->isFileOpen())
    {
        currentStatus = UPLOAD_FILE_ACTION;
        qint64 fileSize = fileService->getFileSize();
        SerializationLayer uploadCmdSerial(UPLOAD);
        uploadCmdSerial.serializeData((quint64)fileSize, fileService->getFileName(), login);
        Command cmd(uploadCmdSerial.getCodeBytes(), uploadCmdSerial.getSizeBytes(), uploadCmdSerial.getDataBytes());
        transportLayer->sendCmd(cmd);
        sendUploadChunks();
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
    char* fileBlock = NULL;
    SerializationLayer chunkCmdSerial(CHUNK);
    qint64 fileSize = fileService->getFileSize();
    QFileInfo fileInfo = fileService->getFileInfo();
    unsigned int i = 0;
    while(!isStopped && currentSize < fileSize)
    {
        if(isConnected())
        {
            qDebug() << "UPLOAD: " << i;
            fileBlock = fileService->getFileBlock();
            qDebug() << "FILEBLOCK: " << QString(fileBlock);
            chunkCmdSerial.serializeData(fileBlock);
            Command cmd(chunkCmdSerial.getCodeBytes(), chunkCmdSerial.getSizeBytes(), chunkCmdSerial.getDataBytes());
            qDebug() << "CMD: " << chunkCmdSerial.getCodeBytes().toHex() << " " << chunkCmdSerial.getSizeBytes().toHex() << " " << chunkCmdSerial.getDataBytes();
            transportLayer->sendCmd(cmd);
            currentSize += strlen(fileBlock);
            qDebug() << "CURRENT_SIZE: " << currentSize;
        }
        else
            isStopped = true;
        ++i;
        emit gotUploadACKSignal(isConnected(), fileInfo, currentSize);
    }
    fileService->fileClose();
    currentStatus = LOGGED; // according to doc, should be UPLOADED_FILE - wait for accept
    currentSize = 0;
    isStopped = false;
}


void TCPWorker::gotResp()
{
    qDebug() << "Something received";
    STATE stateCmd = receivedCommand.getState();
    switch(stateCmd)
    {
        case WAIT_FOR_CODE:
        {
            receivedCommand.setCode(transportLayer->getCmdCode());
            cmdSerial.deserializeCode(receivedCommand.getCode());
            switch(cmdSerial.getCode())
            {
                case ACCEPT:
                {
                    qDebug() << "Got ACCEPT";
                    switch(currentStatus)
                    {
                        case CONNECTED:
                        {
                            currentStatus = LOGGED;
                            receivedCommand = Command();
                            emit connectedToSystemSignal(true, userFiles);
                            break;
                        }
                        case UPLOAD_FILE_ACTION:
                        {
                            sendUploadChunks();
                            break;
                        }
                        case UPLOADED_FILE:
                        {
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
            cmdSerial.deserialize(receivedCommand);
            switch(cmdSerial.getCode())
            {
                case CHUNK:
                {
                    switch(currentStatus)
                    {
                        // files list
                        case LOGGED:
                            break;
                        case DOWNLOAD_FILE_ACTION:
                            break;
                    }
                    break;
                }
                case ERROR:
                {
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
