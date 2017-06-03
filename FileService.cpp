#include "FileService.h"

FileService::FileService(QObject *parent): QObject(parent)
{

}

FileService::FileService(QFileInfo fileInfo, QObject *parent) : QObject(parent)
{
    this->fileInfo = fileInfo;
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
bool FileService::isFileOpen()
{
    this->file = new QFile(fileInfo.absoluteFilePath());
    return file->open(QIODevice::ReadOnly | QIODevice::Text);
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
