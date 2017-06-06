#include "TransportLayer.h"

TransportLayer::TransportLayer(QObject *parent) : QObject(parent)
{
}

TransportLayer::TransportLayer(QTcpSocket *socket, QObject *parent) : QObject(parent)
{
    this->socket = socket;
}

Command TransportLayer::getCmd(int timeout)
{
    qDebug() << "GetCmd started.";
    QByteArray code, sizeArray, data;
    Command cmd;
    code = getBytes(timeout, (int)sizeof(quint8));
    qDebug() << "CODE: " << code.toHex();
    if(code.size() > 0)
    {
        cmd.setCode(code);
    }
    else
    {
        return Command();
    }
    if(cmd.getState() == WAIT_FOR_SIZE)
    {
        sizeArray = getBytes(timeout, (int)sizeof(quint32));
        qDebug() << "SIZE: " << sizeArray.toHex();
        if(sizeArray.size() > 0)
        {
            cmd.setSize(sizeArray);
        }
        else
            return Command();
        QDataStream tmpStream(&sizeArray, QIODevice::ReadOnly);
        int requiredBytes;
        tmpStream >> requiredBytes;
        if(requiredBytes >= maxSizeData)
            return Command();
        data = getBytes(timeout, requiredBytes);
        qDebug() << "DATA: " << data;
        if(data.size() == requiredBytes)
        {
            cmd.setData(data);
        }
        else
            return Command();
    }
    return cmd;
}

// for chunks from DOWNLOAD
Command TransportLayer::getCmd(int timeout1, int timeout2, int timeout3)
{
    qDebug() << "GetCmd started.";
    Command cmd;
    if(code.size() == 0)
        code = getBytes(timeout1, (int)sizeof(quint8));
    qDebug() << "CODE: " << code.toHex();
    if(code.size() > 0)
    {
        cmd.setCode(code);
    }
    else
    {
        return Command();
    }
    if(cmd.getState() == WAIT_FOR_SIZE)
    {
        if(sizeArray.size() == 0)
            sizeArray = getBytes(timeout2, (int)sizeof(quint32));
        qDebug() << "SIZE: " << sizeArray.toHex();
        if(sizeArray.size() > 0)
        {
            cmd.setSize(sizeArray);
        }
        else
            return Command();
        QDataStream tmpStream(&sizeArray, QIODevice::ReadOnly);
        int requiredBytes;
        tmpStream >> requiredBytes;
        if(requiredBytes >= maxSizeData)
        {
            clear();
            return Command();
        }
        data = getBytes(timeout3, requiredBytes);
        qDebug() << "DATA: " << data;
        if(data.size() > 0)
        {
            cmd.setData(data);
        }
        else
            return Command();
    }
    return cmd;
}

QByteArray TransportLayer::getBytes(int timeout, int requiredBytes)
{
    QByteArray bytes;
    bool onReadyRead = true;
    if(socket->bytesAvailable() < requiredBytes)
    {
        onReadyRead = socket->waitForReadyRead(timeout);
    }
    if(!onReadyRead)
        return QByteArray();
    if(socket->bytesAvailable() >= requiredBytes)
    {
        bytes = socket->read(requiredBytes);
        if(bytes.size() != requiredBytes)
        {
            return QByteArray();
        }
    }
    return bytes;
}

bool TransportLayer::sendCmd(Command& cmd)
{
    if(sendData(cmd.getCode()))
    {
        if(sendData(cmd.getSize()))
        {
            return sendData(cmd.getData());
        }
    }
    return false;
}

bool TransportLayer::sendData(QByteArray data)
{
    qint64 bytesCount = socket->write(data);
    socket->waitForBytesWritten(); //socket->flush()
    return bytesCount == data.size();
}


void TransportLayer::clear()
{
    code.clear();
    sizeArray.clear();
    data.clear();
}
