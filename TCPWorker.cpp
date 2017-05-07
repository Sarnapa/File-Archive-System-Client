#include "TCPWorker.h"

TCPWorker::TCPWorker(QObject *parent) : QObject(parent)
{
    //receivedData = new QByteArray();
    userFiles = getFilesFromSystem();

}
TCPWorker::~TCPWorker()
{
    //delete receivedData;
    delete userFiles;
}


void TCPWorker::connectToSystem(QString login, QString password, QString address)
{
    // so far
    actionId = 0;
    if(login == adminLogin && password == adminPassword)
        isConnected = true;
}

void TCPWorker::disconnect()
{
    isConnected = false;
    emit disconnectedSignal();
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
