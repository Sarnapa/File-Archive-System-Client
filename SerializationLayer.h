#ifndef SERIALIZATIONLAYER_H
#define SERIALIZATIONLAYER_H

#include <QFileInfo>
#include "Converter.h"
#include "Command.h"

enum OBJECT_CODE
{
    NOOP = 0x00,
    INTRODUCE = 0x01,
    LOGIN = 0x02,
    LIS = 0x05,
    DOWNLOAD = 0x06,
    UPLOAD = 0x07,
    ACCEPT = 0x08,
    CHUNK = 0x09,
    DELETE = 0x0a,
    RENAME = 0x0b,
    COMMIT = 0x0c,
    ROLLBACK = 0x0d,
    COMMITRDY = 0x0e,
    COMMITACK = 0x0f,
    ERROR = 0x33,
    EXIT = 0xff,
    TEST1 = 0x46,
    TEST2 = 0x47
};

class SerializationLayer : public QObject
{
    Q_OBJECT
public:
    explicit SerializationLayer(QObject *parent = 0);
    explicit SerializationLayer(OBJECT_CODE code, QObject *parent = 0);

    QByteArray getCodeBytes();
    QByteArray getSizeBytes();
    QByteArray getDataBytes();

    OBJECT_CODE getCode();
    quint32 getSize();
    quint32 getErrorCode();
    QList<QFileInfo> getFilesList();

    void serializeData(QString s1, QString s2); // LOGIN or DELETE
    void serializeData(QString login); // LIST
    void serializeData(quint32 begin, quint32 end, QString fileName, QString login); // DOWNLOAD
    void serializeData(QString fileName, quint64 fileSize, QString login); //UPLOAD
    void serializeData(QByteArray fileData); // CHUNK
    void serializeData(QString fileName, QString newFileName, QString login); //RENAME

    void deserialize(Command& cmd);
private:
    QByteArray codeBytes;
    QByteArray sizeBytes;
    QByteArray dataBytes;

    OBJECT_CODE code;
    quint32 size;
    //for various command
    quint32 errorCode;
    QList<QFileInfo> filesList;

    void serializeCode();
    void serializeSize();
    void deserializeCode(QByteArray code);
    void deserializeSize(QByteArray size);
    void deserializeChunkCmd(QByteArray data);
    void deserializeErrorCmd(QByteArray data);
};

#endif // SERIALIZATIONLAYER_H
