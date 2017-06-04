#include "FileService.h"

FileService::FileService(QObject *parent): QObject(parent)
{

}

FileService::FileService(QFileInfo fileInfo, QObject *parent) : QObject(parent)
{
    this->fileInfo = fileInfo;
}

FileService::FileService(MyFileInfo myFileInfo, QObject *parent) : QObject(parent)
{
    this->myFileInfo = myFileInfo;
}

FileService::~FileService()
{
    delete file;
}

FileService &FileService::operator=(const FileService &fileService)
{
    this->fileInfo = fileService.fileInfo;
    return *this;
}

//for reading
bool FileService::isFileOpen(bool forReading)
{
    if(forReading)
    {
        this->file = new QFile(fileInfo.absoluteFilePath());
        return file->open(QIODevice::ReadOnly);
    }
    else
    {
        QString path = QDir::currentPath() + QString(QDir::separator()) + "local" + QString(QDir::separator())  + myFileInfo.getFileName(); //"C:/Qt/QtProjects/FileArchiveSystemClient/local/File3";
        this->file = new QFile(path);
        return file->open(QIODevice::WriteOnly);
    }
}

quint64 FileService::getFileSize()
{
    return (quint64)file->size();
}

QString FileService::getFileName()
{
    return fileInfo.fileName();
}

QByteArray FileService::getFileBlock(qint64 blockSize)
{
    QByteArray dataBlock = file->read(blockSize);
    return dataBlock;
}

void FileService::writeFileBlock(QByteArray block)
{
    //qDebug() << "BYTES_COUNT1: " << block.size();
    qint64 bytesCount = file->write(block);
    file->flush();
    //qDebug() << "BYTES_COUNT2: " << bytesCount << " " << file->size();
}

QFileInfo FileService::getFileInfo()
{
    return fileInfo;
}

MyFileInfo FileService::getMyFileInfo()
{
    return myFileInfo;
}

void FileService::setFileInfo(QFileInfo fileInfo)
{
    this->fileInfo = fileInfo;
}

void FileService::setMyFileInfo(MyFileInfo myFileInfo)
{
    this->myFileInfo = myFileInfo;
}

void FileService::fileClose()
{
    file->close();
    qDebug() << "File closed";
}
