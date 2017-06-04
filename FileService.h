#ifndef FILESERVICE_H
#define FILESERVICE_H

#include <QObject>
#include <QThread>
#include <QFileInfo>
#include <QFile>
#include <QByteArray>
#include <QDataStream>
#include <QDebug>
#include <QDir>
#include "Converter.h"
#include "MyFileInfo.h"

class FileService : public QObject
{
    Q_OBJECT
public:
    explicit FileService(QObject *parent = 0);
    explicit FileService(QFileInfo fileInfo, QObject *parent = 0);
    explicit FileService(MyFileInfo myFileInfo, QObject *parent = 0);
    ~FileService();

    FileService& operator=(const FileService& fileService);

    bool isFileOpen(bool forReading);
    quint64 getFileSize();
    QString getFileName();
    QByteArray getFileBlock(qint64 blockSize = 4096);
    void writeFileBlock(QByteArray block);
    QFileInfo getFileInfo();
    MyFileInfo getMyFileInfo();
    void setFileInfo(QFileInfo fileInfo);
    void setMyFileInfo(MyFileInfo myFileInfo);
    void fileClose();
private:
    QFileInfo fileInfo;
    MyFileInfo myFileInfo;
    QFile *file;
};

#endif // FILESERVICE_H
