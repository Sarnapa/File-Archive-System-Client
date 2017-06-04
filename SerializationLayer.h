#ifndef SERIALIZATIONLAYER_H
#define SERIALIZATIONLAYER_H

#include <QFileInfo>
#include <MyFileInfo.h>
#include <QTextCodec>
#include "Converter.h"
#include "Command.h"

class SerializationLayer : public QObject
{
    Q_OBJECT
public:
    explicit SerializationLayer(QObject *parent = 0);
    explicit SerializationLayer(OBJECT_CODE code, QObject *parent = 0);
    ~SerializationLayer();

    QByteArray getCodeBytes();
    QByteArray getSizeBytes();
    QByteArray getDataBytes();

    OBJECT_CODE getCode();
    quint32 getSize();
    ERROR_CODE getErrorCode();
    QByteArray getFileChunk();
    QList<MyFileInfo>* getFilesList();

    void serializeData(QString s1, QString s2); // LOGIN or DELETE
    void serializeData(QString login); // LIST
    void serializeData(quint32 begin, quint32 end, QString fileName, QString login); // DOWNLOAD
    void serializeData(quint64 fileSize, QString fileName, QString login); //UPLOAD
    void serializeData(QByteArray fileData); // CHUNK
    void serializeData(QString fileName, QString newFileName, QString login); //RENAME

    void deserializeCode(QByteArray code);
    void deserialize(Command& cmd, bool isFilesChunk);
private:
    QByteArray codeBytes;
    QByteArray sizeBytes;
    QByteArray dataBytes;

    OBJECT_CODE code;
    quint32 size;
    //for various command
    bool isFileChunk = false;
    quint32 errorCode;
    QByteArray fileChunk;
    QList<MyFileInfo>* filesList;

    void serializeCode();
    void serializeSize();
    void deserializeSize(QByteArray size);
    void deserializeChunkCmd(QByteArray data,bool isFilesChunk);
    void deserializeErrorCmd(QByteArray data);
};

#endif // SERIALIZATIONLAYER_H
