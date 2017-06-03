#ifndef FILESERVICE_H
#define FILESERVICE_H

#include <QObject>
#include <QThread>
#include <QFileInfo>
#include <QFile>
#include <QByteArray>
#include <QDataStream>
#include <QDebug>
#include "Converter.h"
#include "MyFileInfo.h"

class FileService : public QObject
{
    Q_OBJECT
public:
    explicit FileService(QObject *parent = 0);
    explicit FileService(QFileInfo fileInfo, QObject *parent = 0);
    ~FileService();

    FileService& operator=(const FileService& fileService);

    bool isFileOpen();
    quint64 getFileSize();
    QString getFileName();
    QByteArray getFileBlock(qint64 blockSize = 4096);
    QFileInfo getFileInfo();
    void setFileInfo(QFileInfo fileInfo);
    void fileClose();
private:
    QFileInfo fileInfo;
    QFile *file;
};

#endif // FILESERVICE_H
