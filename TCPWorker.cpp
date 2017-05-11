#include "TCPWorker.h"

    TCPWorker::TCPWorker(QObject *parent) : QObject(parent)
{
    receivedData = new QByteArray();
    userFiles = new QList<MyFileInfo>;
    //userFiles = getFilesFromSystem();

}
TCPWorker::~TCPWorker()
{
    delete userFiles;
    delete receivedData;
}

//TIN
void TCPWorker::connectToSystem(QString login, QString password, QString address)
{
    this->login = login;
    this->password = password;
    socket = new QTcpSocket();

    connect(socket, SIGNAL(connected()), this, SLOT(connected()));
    connect(socket, SIGNAL(disconnected()),this, SLOT(disconnected()));
    connect(socket, SIGNAL(readyRead()), this, SLOT(gotResp()));

    // this is not blocking call
    socket->connectToHost(address, 11000); //11000 2666

    if(!socket->waitForConnected(5000))
        emit connectedToSystemSignal(isConnected, userFiles);
}

//TIN
void TCPWorker::disconnect()
{
    socket->disconnectFromHost();
}

//TIN
void TCPWorker::refresh()
{
    // so far
    Command command(44);
    while(!stop)
    {
        socket->write(command.getCode());
        socket->flush();
    }
    qDebug() << "Po przesyłaniu";
    stop = false;
    emit refreshedSignal(isConnected, userFiles);
}

//PAIN
void TCPWorker::deleteFile(QString fileName)
{
    actionId = 2;
    if(isConnected)
    {
        currentFileName = fileName;
        MyFileInfo file;
        for(int i = 0; i < userFiles->size(); ++i)
        {
            file = userFiles->at(i);
            if(file.getFileName() == currentFileName)
            {
                userFiles->removeAt(i);
                break;
            }
        }
    }
}

void TCPWorker::cancel()
{
    stop = true;
}

//PAIN
void TCPWorker::uploadFile(QString fileName, qlonglong size, QDateTime lastModified)
{
    actionId = 3;
    if(isConnected)
    {
        currentFile = MyFileInfo(fileName, size, lastModified);
        MyFileInfo tmpFile;
        int i;
        for(i = 0; i < userFiles->size(); ++i)
        {
            tmpFile = userFiles->at(i);
            if(tmpFile.getFileName() == fileName)
            {
                userFiles->replace(i, currentFile);
                break;
            }
        }
        if(i == userFiles->size())
            userFiles->append(currentFile);
    }
}

//PAIN
void TCPWorker::downloadFile(QString fileName)
{
    actionId = 4;
    if(isConnected)
    {
        int i;
        for(i = 0; i < userFiles->size(); ++i)
        {
            MyFileInfo tmpFile = userFiles->at(i);
            if(tmpFile.getFileName() == fileName)
            {
                currentFile = tmpFile;
                break;
            }
        }
    }
}

// PAIN
void TCPWorker::gotResponse()
{
    switch(actionId)
    {
        case 0:
            emit connectedToSystemSignal(isConnected, userFiles);
            break;
        case 1:
            emit refreshedSignal(isConnected, userFiles);
            break;
        case 2:
            emit deletedFileSignal(isConnected, currentFileName);
            break;
        case 3:
            emit gotUploadACKSignal(isConnected, currentFile.getFileName(), currentFile.getFileSize(), currentFile.getFileLastModified());
            break;
           case 4:
            emit gotDownloadACKSignal(isConnected, currentFile.getFileName());
            break;
    }
}

//TIN
void TCPWorker::gotResp()
{
    qDebug() << "Something received";

    char ch;
    socket->read(&ch, sizeof(ch));

    //std::string chString = ch + "\0";
    //qDebug() << chString.c_str();

    int chInt = ch;
    switch(chInt)
    {
        case 8:
            if(!isConnected)
            {
                qDebug() << "Got accept";
                quint8 code = 2;
                QString loginDataString = login + ":" + password;
                quint32 size = loginDataString.size();
                Command command(code, size, loginDataString);
                if(socket->state() == QAbstractSocket::ConnectedState)
                {
                    socket->write(command.getCode());
                    socket->flush();
                    socket->write(command.getSize());
                    socket->flush();
                    socket->write(command.getData());
                    socket->flush();
                }
             }
            break;
        case 69:
            qDebug() << "Got 0x69 - logging ACK";
            isConnected = true;
            emit connectedToSystemSignal(isConnected, userFiles);
        default:
            break;
    }
}

//TIN
void TCPWorker::connected()
{
    qDebug() << "Connected to " << socket->peerAddress().toString();
    QString secret = "zyrafywchodzadoszafy";
    quint8 code = 1;
    quint32 secretSize = secret.size();
    Command command(code, secretSize, secret);
    if(socket->state() == QAbstractSocket::ConnectedState)
    {
        //flush writes as much as possible from the internal write buffer to the underlying network socket, without blocking.
        socket->write(command.getCode());
        socket->flush();
        socket->write(command.getSize());
        socket->flush();
        socket->write(command.getData());
        socket->flush();
    }
}

//TIN
void TCPWorker::disconnected()
{
    qDebug() << "Disconnected";
    socket->close();
    isConnected = false;
    emit disconnectedSignal();
}


//PAIN
QList<MyFileInfo>* TCPWorker::getFilesFromSystem() const
{
    QList<MyFileInfo>* list = new QList<MyFileInfo>();
    for(int i = 0; i < 5; ++i)
    {
        MyFileInfo file(QString("File") + QString::number(i), (qlonglong)i*1024 , QDateTime::currentDateTime());
        list->append(file);
    }
    return list;
}
