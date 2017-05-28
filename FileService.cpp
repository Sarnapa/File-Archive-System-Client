#include "FileService.h"

FileService::FileService(QObject *parent): QObject(parent)
{

}

FileService::FileService(QFileInfo fileInfo, QObject *parent) : QObject(parent)
{
    //qDebug() << "FILESERVICE: " << QThread::currentThreadId();
    this->fileInfo = fileInfo;
}

FileService::~FileService()
{
    delete file;
}

FileService &FileService::operator=(const FileService &fileService)
{
    //qDebug() << "PRZYPISANKO: " << QThread::currentThreadId();
    this->fileInfo = fileService.fileInfo;
    return *this;
}

//for reading
bool FileService::isFileOpen()
{
    this->file = new QFile(fileInfo.absoluteFilePath());
    return file->open(QIODevice::ReadOnly);
}

quint64 FileService::getFileSize()
{
    return (quint64)file->size();
}

QString FileService::getFileName()
{
    return fileInfo.fileName();
}

char* FileService::getFileBlock(qint64 blockSize)
{
    char *tmp = new char;
    QDataStream stream(file);
    qint64 bytesCount = file->read(tmp, blockSize);
    char *dataBlock = new char[bytesCount];
    datablock = tmp;
    return dataBlock;
}

QFileInfo FileService::getFileInfo()
{
    return fileInfo;
}

void FileService::setFileInfo(QFileInfo fileInfo)
{
    this->fileInfo = fileInfo;
}

void FileService::fileClose()
{
    file->close();
    qDebug() << "File closed";
}
