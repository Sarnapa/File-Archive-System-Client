#include "SerializationLayer.h"

SerializationLayer::SerializationLayer(QObject *parent) : QObject(parent)
{
    filesList = new QList<MyFileInfo>;
}

SerializationLayer::SerializationLayer(OBJECT_CODE code, QObject *parent) : QObject(parent)
{
    filesList = new QList<MyFileInfo>;
    this->code = code;
    serializeCode();
}

SerializationLayer::~SerializationLayer()
{
    delete filesList;
}

QByteArray SerializationLayer::getCodeBytes()
{
    return codeBytes;
}

QByteArray SerializationLayer::getSizeBytes()
{
    return sizeBytes;
}

QByteArray SerializationLayer::getDataBytes()
{
    return dataBytes;
}

OBJECT_CODE SerializationLayer::getCode()
{
    return code;
}

quint32 SerializationLayer::getSize()
{
    return size;
}

quint32 SerializationLayer::getErrorCode()
{
    return errorCode;
}

QList<MyFileInfo>* SerializationLayer::getFilesList()
{
    return filesList;
}

void SerializationLayer::serializeCode()
{
    codeBytes = Converter::intToArray((quint8)code);
}

void SerializationLayer::serializeSize()
{
    quint32 size = quint32(dataBytes.size());
    sizeBytes = Converter::intToArray(size);
}


void SerializationLayer::serializeData(QString s1, QString s2)
{
    QString s = s1 + ":" + s2;
    dataBytes = s.toUtf8();
    serializeSize();
}

void SerializationLayer::serializeData(QString login)
{
    dataBytes = login.toUtf8();
    serializeSize();
}

void SerializationLayer::serializeData(quint32 begin, quint32 end, QString fileName, QString login)
{
    QByteArray beginBytes = Converter::intToArray(begin);
    QByteArray endBytes = Converter::intToArray(end);
    QByteArray fileNameBytes = fileName.toUtf8();
    QByteArray loginBytes = login.toUtf8();
    dataBytes.append(beginBytes);
    dataBytes.append(':');
    dataBytes.append(endBytes);
    dataBytes.append(':');
    dataBytes.append(fileNameBytes);
    dataBytes.append(':');
    dataBytes.append(loginBytes);
    serializeSize();
}

void SerializationLayer::serializeData(quint64 fileSize, QString fileName, QString login)
{
    QByteArray fileNameBytes = fileName.toUtf8();
    QByteArray fileSizeBytes = Converter::intToArray(fileSize);
    QByteArray loginBytes = login.toUtf8();
    dataBytes.append(fileSizeBytes);
    dataBytes.append(fileNameBytes);
    dataBytes.append(':');
    dataBytes.append(loginBytes);
    serializeSize();
}

void SerializationLayer::serializeData(QByteArray fileData)
{
    dataBytes.append(fileData);
    serializeSize();
}

void SerializationLayer::serializeData(QString fileName, QString newFileName, QString login)
{
    QString s = fileName + ":" + newFileName + ":" + login;
    dataBytes = s.toUtf8();
    serializeSize();
}

void SerializationLayer::deserialize(Command& cmd, bool isFilesChunk)
{
    QByteArray codeBytes = cmd.getCode();
    if(codeBytes.size() > 0)
    {
        if(code != ACCEPT)
        {
            deserializeSize(cmd.getSize());
            if(size != 0)
            {
                if(code == CHUNK)
                {
                    deserializeChunkCmd(cmd.getData(), isFilesChunk);
                }
                else if(code == ERROR)
                    deserializeErrorCmd(cmd.getData());
            }
        }
    }
}

void SerializationLayer::deserializeCode(QByteArray code)
{
    QDataStream stream(&code, QIODevice::ReadOnly);
    quint8 codeInt;
    stream >> codeInt;
    this->code = (OBJECT_CODE)codeInt;
}

void SerializationLayer::deserializeSize(QByteArray size)
{
    QDataStream stream(&size, QIODevice::ReadOnly);
    stream >> this->size;
}

void SerializationLayer::deserializeChunkCmd(QByteArray data, bool isFilesChunk)
{
    if(isFilesChunk)
    {
        QString dataString = QTextCodec::codecForMib(106)->toUnicode(data);
        quint64 fileSize;
        QString fileName;
        unsigned int filesCount = dataString.count(';');
        int startPos = 0;
        int endPos;
        for(unsigned int i = 0; i < filesCount; ++i)
        {
           endPos = dataString.indexOf(':', startPos);
           fileName = dataString.mid(startPos, endPos - startPos);
           startPos = endPos + 1;
           endPos = dataString.indexOf(';', startPos);
           fileSize = (quint64)dataString.mid(startPos, endPos - startPos).toInt();
           startPos = endPos + 1;
           MyFileInfo fileInfo(fileName, fileSize);
           filesList->append(fileInfo);
        }
    }
    else
    {

    }
}

void SerializationLayer::deserializeErrorCmd(QByteArray data)
{
    QDataStream stream(&data, QIODevice::ReadOnly);
    stream >> this->errorCode;
}
