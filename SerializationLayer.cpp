#include "SerializationLayer.h"

SerializationLayer::SerializationLayer(QObject *parent) : QObject(parent)
{}

SerializationLayer::SerializationLayer(OBJECT_CODE code, QObject *parent) : QObject(parent)
{
    this->code = code;
    serializeCode();
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
    codeBytes = Converter::intToArray((quint8)code); // Converter::intToArray((quint8)code);
    //qDebug() << "1: " <<  (quint8)code << " " << codeBytes.toHex();
}

void SerializationLayer::serializeSize()
{
    quint32 size = quint32(dataBytes.size());
    sizeBytes = Converter::intToArray(size); // Converter::intToArray(size);
    //qDebug() << "2: " << size << " " << sizeBytes.toHex();
}


void SerializationLayer::serializeData(QString s1, QString s2)
{
    QString s = s1 + ":" + s2;
    dataBytes = s.toUtf8();
    /*QByteArray s1Bytes = s1.toUtf8();
    QByteArray s2Bytes = s2.toUtf8();
    dataBytes.append(s1Bytes);
    dataBytes.append(':');
    dataBytes.append(s2Bytes);*/
    serializeSize();
}

void SerializationLayer::serializeData(QString login)
{
    dataBytes = login.toUtf8();
    serializeSize();
}

void SerializationLayer::serializeData(quint32 begin, quint32 end, QString fileName, QString login)
{
    QByteArray beginBytes = Converter::intToArray(begin); // Converter::intToArray(begin);
    QByteArray endBytes = Converter::intToArray(end); // Converter::intToArray(end);
    //QString s = fileName + ":" + login;
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
    QByteArray fileSizeBytes = Converter::intToArray(fileSize); // Converter::intToArray(fileSize);
    QByteArray loginBytes = login.toUtf8();
    //QDataStream stream(&dataBytes, QIODevice::WriteOnly);
    //stream << fileSize;
    //dataBytes = fileSizeBytes;
    dataBytes.append(fileSizeBytes);
    //dataBytes.append(':');
    dataBytes.append(fileNameBytes);
    dataBytes.append(':');
    dataBytes.append(loginBytes);
    //qDebug() << fileSizeBytes.toHex();
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
    //QByteArray codeBytes = cmd.getCode();
    if(codeBytes.size() > 0)
    {
        deserializeCode(codeBytes);
        if(code != ACCEPT)
        {
            deserializeSize(cmd.getSize());
            if(size != 0)
            {
                if(code == CHUNK)
                    deserializeChunkCmd(cmd.getData(), isFilesChunk);
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
    //so far
    if(isFilesChunk)
    {
        QDataStream stream(&data, QIODevice::ReadOnly);
        QString dataString;
        stream >> dataString;
        qint64 fileSize;
        QString fileName;
        unsigned int filesCount = dataString.count(';');
        int startPos = 0;
        int endPos;
        for(unsigned int i = 0; i < filesCount; ++i)
        {
           endPos = dataString.indexOf(':', i);
           fileName = dataString.mid(startPos, endPos - startPos);
           startPos = endPos + 1;
           endPos = dataString.indexOf(';', i);
           fileSize = (qint64)dataString.mid(startPos, endPos - startPos).toInt();
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
