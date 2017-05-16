#include "TCPWorker.h"

TCPWorker::TCPWorker(QObject *parent) : QObject(parent)
{
    userFiles = new QList<QFileInfo>;
}

TCPWorker::~TCPWorker()
{
    delete userFiles;
    //delete socket;
}

void TCPWorker::connectToSystem(QString login, QString password, QString address)
{
    this->login = login;
    this->password = password;

    socket = new QTcpSocket;
    //workerThread = new TCPThread;
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
    // so far
    Command command(TEST1);
    //command.moveToThread(workerThread);
    //workerThread->start();
    //connect(this, SIGNAL(sendCmdSignal(QTcpSocket*)), &command, SLOT(sendCmd(QTcpSocket*)));
    for(unsigned int i = 0; i < 10000; ++i)
    //while(!stop)
    {
        command.sendCmdCode(socket);
    }

    Command command2(TEST2);
    command2.sendCmdCode(socket);
    //qDebug() << "Po przesyłaniu";
    stop = false;
    emit refreshedSignal(isConnected(), userFiles);
}

void TCPWorker::deleteFile(QString fileName)
{

}

void TCPWorker::cancel()
{
    qDebug() << "no elo elo";
    stop = true;
    //workerThread->exit();
}

void TCPWorker::uploadFile(QString fileName, qlonglong size, QDateTime lastModified)
{
}

void TCPWorker::downloadFile(QString fileName)
{

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
    /*char ch;
    socket->read(&ch, sizeof(ch));

    //std::string chString = ch + "\0";
    //qDebug() << chString.c_str();

    int chInt = ch;
    switch(chInt)
    {
        case 8:
            qDebug() << "Got accept";
            quint8 code = 2;
            QString loginDataString = login + ":" + password;
            quint32 size = loginDataString.size();
            Command command(code, size, loginDataString);
            if(isConnected())
            {
                socket->write(command.getCode());
                socket->flush();
                socket->write(command.getSize());
                socket->flush();
                socket->write(command.getData());
                socket->flush();
            }
            break;
        case 69:
            qDebug() << "Got 0x69 - logging ACK";
            emit connectedToSystemSignal(isConnected(), userFiles);
        default:
            break;
    }*/
}

void TCPWorker::connected()
{
    qDebug() << "Connected to " << socket->peerAddress().toString();
    currentStatus = CONNECTED;
    QString loginDataString = login + ":" + password;
    Command command(LOGIN, loginDataString);
    if(command.sendCmdCode(socket))
        if(command.sendCmdSize(socket))
            if(command.sendCmdData(socket))
                qDebug() << "Data for logging were sended.";
    /*QString secret = "zyrafywchodzadoszafy";
    Command command(INTRODUCE, secret);
    if(command.sendCmdCode(socket))
        if(command.sendCmdSize(socket))
            if(command.sendCmdData(socket))
                qDebug() << "Secret was sended.";*/
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
