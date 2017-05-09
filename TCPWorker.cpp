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
    this->address = address;
    socket = new QTcpSocket();

    connect(socket, SIGNAL(connected()), this, SLOT(connected()));
    connect(socket, SIGNAL(disconnected()),this, SLOT(disconnected()));
    connect(socket, SIGNAL(readyRead()), this, SLOT(gotResp()));

    // this is not blocking call
    socket->connectToHost(address, 11000); //11000 2666

    if(!socket->waitForConnected(5000))
    {
        isConnected = false;
        emit connectedToSystemSignal(isConnected, userFiles);
    }
}

//TIN
void TCPWorker::disconnect()
{
    isConnected = false;
    socket->disconnectFromHost();
}

void TCPWorker::refresh()
{
    // so far
    actionId = 1;
    quint8 number;
    Command command(number);
    for(unsigned int i = 0; i < 10; ++i)
    {
        number = 1;
        qDebug() << command.getCode().toInt();
        socket->write(command.getCode());
        //socket->waitForBytesWritten(1000);
        //socket->flush();
        qDebug() << "po petli numer: " << i;
    }
    qDebug() << "po wszytkich petlach";
    number = 255;
    Command lastCommand(number);
    socket->write(lastCommand.getCode());
    //socket->waitForBytesWritten(1000);
    //socket->flush();
    qDebug() << "po koncowym 255";
    gotResponse(); // tylko dla interfejsu - malo wazne
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
                qDebug() << "got accept";
                quint8 code = 2;
                std::string tmpLogin = "user1:pass1";
                quint32 size = tmpLogin.size();
                Command command(code, size, tmpLogin);
                socket->write(command.getCode());
                socket->write(command.getSize());
                socket->write(command.getData());
                socket->flush();
             }
            break;
        case 69:
            qDebug() << "got 0x69";
            isConnected = true;
            emit connectedToSystemSignal(isConnected, userFiles);
        default:
            break;
    }
}

//TIN
void TCPWorker::connected()
{
    qDebug() << "connected to " + address << " . " + socket->peerAddress().toString();
    std::string secret = "zyrafywchodzadoszafy";
    qint8 code = 1;
    qint32 secretSize = secret.size();
    Command command(code, secretSize, secret);
    socket->write(command.getCode());
    socket->write(command.getSize());
    socket->write(command.getData());
    socket->flush();
}

//TIN
void TCPWorker::disconnected()
{
    qDebug() << "disconnected";
    if(socket->ConnectedState )
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
