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
    /*FileService fileService(fileInfo, login);
    if(fileService.isFileOpen())
    {
        QByteArray data = fileService.prepareCmdData();
        sendCmd(UPLOAD, data);
        QByteArray fileBlock = fileService.getFileBlock();
        while(!isStopped)
        {

        }
    }
    else
    {

    }*/
    //for test
    emit refreshedSignal(isConnected(), userFiles);
}

void TCPWorker::downloadFile(QString fileName)
{

}


void TCPWorker::gotResp()
{
    qDebug() << "Something received";
    Command cmd = transportLayer->getCmd();
    SerializationLayer cmdSerial;
    cmdSerial.deserialize(cmd);
    switch(cmdSerial.getCode())
    {
        case ACCEPT:
        {
            qDebug() << "Got ACCEPT";
            currentStatus = LOGGED;
            emit connectedToSystemSignal(true, userFiles);
            break;
        }
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
            isStopped = false;
            emit disconnect();
            break;
        }
        case QAbstractSocket::NetworkError:
        {
            qDebug() << "NetworkError";
            isStopped = false;
            emit disconnect();
            break;
        }
    }
}
