#include "FileService.h"

FileService::FileService(QObject *parent)
{

}

FileService::FileService(QFileInfo &fileInfo, QObject *parent) : QObject(parent)
{
    this->fileInfo = fileInfo;
    this->file = new QFile(fileInfo.absoluteFilePath());
}

FileService::~FileService()
{
    delete file;
}

//for reading
bool FileService::isFileOpen()
{
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
    char *dataBlock = new char;
    file->read(dataBlock, blockSize);
    //qDebug() << "getFileBlock: " << QString(dataBlock) << " " << dataBlock << " " << bytesCount;
    return dataBlock;
}

void FileService::fileClose()
{
    file->close();
    qDebug() << "File closed";
}
