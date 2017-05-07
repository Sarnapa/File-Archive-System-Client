#include "TCPWorker.h"

TCPWorker::TCPWorker(QObject *parent) : QObject(parent)
{
    receivedData = new QByteArray();
    userFiles = new QList<MyFileInfo>;
    //userFiles = getFilesFromSystem();

}
TCPWorker::~TCPWorker()
{
    delete receivedData;
    delete userFiles;
}


void TCPWorker::connectToSystem(QString login, QString password, QString address)
{
    // so far
    /*actionId = 0;
    if(login == adminLogin && password == adminPassword)
        isConnected = true;*/
    this->login = login;
    this->password = password;
    this->address = address;
    socket = new QTcpSocket;

    connect(socket, SIGNAL(connected()), this, SLOT(connected()));
    connect(socket, SIGNAL(disconnected()),this, SLOT(disconnected()));

    // this is not blocking call
    socket->connectToHost(address, 11000);

    if(!socket->waitForConnected(5000))
    {
        isConnected = false;
        emit connectedToSystemSignal(isConnected, userFiles);
    }
}

void TCPWorker::disconnect()
{
    isConnected = false;
    socket->disconnectFromHost();
}

void TCPWorker::refresh()
{
    // so far
    actionId = 1;
}

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

void TCPWorker::gotResponse(QByteArray *data)
{

}

void TCPWorker::connected()
{
    // 80.54.182.42
    qDebug() << "connected to " + address << " . " + socket->peerAddress().toString();
    isConnected = true;
    char secret[] = "zyrafywchodzadoszafy";
    int secretSize = sizeof(secret)/sizeof(char);
    Command command("0x01", secretSize, secret);
    socket->write(command.getCode());
    socket->write(command.getSize());
    socket->write(command.getData());
    //emit connectedToSystemSignal(isConnected, userFiles);
}

void TCPWorker::disconnected()
{
    qDebug() << "disconnected";
    socket->close();
    isConnected = false;
    emit disconnectedSignal();
}

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
