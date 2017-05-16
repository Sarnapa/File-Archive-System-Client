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

    connect(socket, SIGNAL(connected()), this, SLOT(connected()));
    connect(socket, SIGNAL(disconnected()),this, SLOT(disconnected()));
    //connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(gotError(QAbstractSocket::SocketError)));
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
    while(!isStopped)
    {
        if(!isConnected())
        {
            isStopped = true;
            return;
        }
        else
        {
            sendCmd(TEST1, NULL);
        }
    }
    isStopped = false;
    sendCmd(TEST2, NULL);
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

void TCPWorker::uploadFile(QString fileName, qlonglong size, QDateTime lastModified)
{
}

void TCPWorker::downloadFile(QString fileName)
{

}

void TCPWorker::sendCmd(CMD code, QString data)
{
    Command cmd;
    if(data != NULL)
    {
        cmd = Command(code, data);
        cmd.sendCmd(socket);
    }
    else
    {
        cmd = Command(code);
        cmd.sendCmdCode(socket);
    }
}

void TCPWorker::gotResp()
{
    qDebug() << "Something received";
    Command command;
    command.getCmdCode(socket, socketStream);
    if(command.getState() == NO_DATA)
    {
        switch(command.getCodeInt())
        {
            case ACCEPT:
            {
                qDebug() << "Got ACCEPT";
                currentStatus = LOGGED;
                emit connectedToSystemSignal(true, userFiles);
            }
            default:
            {
                break;
            }
        }
    }
}

void TCPWorker::connected()
{
    qDebug() << "Connected to " << socket->peerAddress().toString();
    currentStatus = CONNECTED;
    QString loginDataString = login + ":" + password;
    sendCmd(LOGIN, loginDataString);
}

void TCPWorker::disconnected()
{
    qDebug() << "Disconnected";
    currentStatus = DISCONNECTED;
    socket->close();
    emit disconnectedSignal();
}

/*void TCPWorker::gotError(QAbstractSocket::SocketError error)
{

}*/
