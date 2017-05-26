#ifndef FILESERVICE_H
#define FILESERVICE_H

#include <QObject>
#include <QFileInfo>
#include <QFile>
#include <QByteArray>
#include <QDataStream>
#include <QDebug>
#include "Converter.h"

class FileService : public QObject
{
    Q_OBJECT
public:
    explicit FileService(QObject *parent = 0);
    explicit FileService(QFileInfo &fileInfo, QObject *parent = 0);
    ~FileService();

    bool isFileOpen();
    QByteArray getFileBlock();
private:
    QFileInfo fileInfo;
    QFile *file;
};

#endif // FILESERVICE_H
