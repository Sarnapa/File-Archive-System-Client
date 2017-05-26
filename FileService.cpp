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

}

//for reading
bool FileService::isFileOpen()
{
    return file->open(QIODevice::ReadOnly);
}

//QByteArray FileService::prepareCmdData()
//{
   // QString fileName = fileInfo.fileName();
    //QByteArray fileSize = Converter::intToBinary((quint64)fileInfo.size());
    //return fileSize + ":" + fileName + ":";
    //QByteArray block;
    //QDataStream out(&block, QIODevice::WriteOnly);
    /*out.setVersion(QDataStream::Qt_5_9);
    out.device()->seek(0);*/
    //block.append(fileSize);
    //out << (quint64)fileInfo.size(); //<< ':' << fileName << ':' << login << '\0';
    //block.append(":" + fileName + ":" + login);
    /*qDebug() << fileSize.size();
    qDebug() << fileInfo.size();
    qDebug() << block;
    qDebug() << sizeof(quint64) << " " << fileName.size() << " " << login.size();
    qDebug() << block.size();
    return block;
}*/

QByteArray FileService::getFileBlock()
{
    return file->readAll();
}
