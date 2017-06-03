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
    QByteArray code, sizeArray, data;
    Command cmd;
    code = getBytes(timeout, (int)sizeof(quint8));
    if(code.size() > 0)
    {
        qDebug() << "CODE: " << code.toHex() << " " << code.size();
        cmd.setCode(code);
    }
    else
        return Command();
    if(cmd.getState() == WAIT_FOR_SIZE)
    {
        sizeArray = getBytes(timeout, (int)sizeof(quint32));
        if(sizeArray.size() > 0)
        {
            qDebug() << "SIZE: " << sizeArray.toHex() <<  " " << sizeArray.size();
            cmd.setSize(sizeArray);
        }
        else
            return Command();
        QDataStream tmpStream(&sizeArray, QIODevice::ReadOnly);
        int requiredBytes;
        tmpStream >> requiredBytes;
        data = getBytes(timeout, requiredBytes);
        if(data.size() > 0)
        {
            qDebug() << "DATA: " << data.toHex() << " " << " " << data.size();
            cmd.setData(data);
        }
        else
            return Command();
    /*char *tmp = new char;
    int requiredBytes = (int)sizeof(quint8);
    bool onReadyRead = socket->waitForReadyRead(timeout);
    if(!onReadyRead)
        return Command();
    if(socket->bytesAvailable() >= requiredBytes)
    {
        qint64 readedBytes = socket->read(tmp, requiredBytes);
        if(readedBytes == requiredBytes)
        {
            code = QByteArray(tmp);
            cmd.setCode(code);
        }
        else
        {
            return Command();
        }
    }
    requiredBytes = (int)sizeof(quint32);
    onReadyRead = socket->waitForReadyRead(timeout);
    if(!onReadyRead)
        return Command();
    if(socket->bytesAvailable() >= (requiredBytes))
    {
        qint64 readedBytes = socket->read(tmp, requiredBytes);
        if(readedBytes == requiredBytes)
        {
            size = QByteArray(tmp);
            cmd.setSize(size);1
        }
        else
        {
            return Command();
        }
    }
    QDataStream tmpStream(&size, QIODevice::ReadOnly);
    tmpStream >> requiredBytes;
    onReadyRead = socket->waitForReadyRead(timeout);
    if(!onReadyRead)
        return Command();
    if(socket->bytesAvailable() >= (requiredBytes))
    {
        qint64 readedBytes = socket->read(tmp, requiredBytes);
        if(readedBytes == requiredBytes)
        {
            data = QByteArray(tmp);
            cmd.setData(data);
        }
        else
        {
            return Command();
        }
    }*/
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

QByteArray TransportLayer::getCmdCode()
{
    //QDataStream socketStream;
    //socketStream.setDevice(socket);
    int requiredBytesCount = (int)sizeof(quint8);
    char *tmp = new char[requiredBytesCount];
    qint64 bytesAvailable = socket->bytesAvailable();
    qDebug() << "BytesAvailable: " << bytesAvailable;
    if(socket->bytesAvailable() < requiredBytesCount)
    {
        qDebug() << "Required one byte for command code. ";
        return QByteArray();
    }
    //socketStream.startTransaction();

    //socketStream >> tmp;

    //socketStream.commitTransaction();
    //socket->read(tmp, requiredBytesCount);
    //QByteArray code(tmp, requiredBytesCount);
    QByteArray code = socket->readAll();
    return code;
}

QByteArray TransportLayer::getCmdSize()
{
    //char *tmp = new char;
    int requiredBytesCount = (int)sizeof(quint32);
    qint64 bytesAvailable = socket->bytesAvailable();
    qDebug() << "BytesAvailable: " << bytesAvailable;
    if(socket->bytesAvailable() < requiredBytesCount)
    {
        qDebug() << "Required 4 bytes for command code. " << bytesAvailable;
        return QByteArray();
    }
    QByteArray size = socket->readAll();
    //socket->read(tmp, requiredBytesCount);
    //QByteArray size(tmp, requiredBytesCount);
    return size;
}

QByteArray TransportLayer::getCmdData(int size)
{
    //char *tmp = new char;
    qint64 bytesAvailable = socket->bytesAvailable();
    qDebug() << "BytesAvailable: " << bytesAvailable;
    if(socket->bytesAvailable() < size)
    {
        qDebug() << "Expected " + QString(size) + " bytes.";
        return QByteArray();
    }
    QByteArray data = socket->readAll();
    //socket->read(tmp, size);
    //QByteArray data(tmp, size);
    return data;
}

void TransportLayer::sendCmd(Command& cmd)
{
    /*qDebug() << "W transportLayer:";
    qDebug() << cmd.getCode().toHex();
    qDebug() << cmd.getSize().toHex();
    qDebug() << QString().fromStdString(cmd.getData().toStdString());*/
    if(sendData(cmd.getCode()))
    {
        if(sendData(cmd.getSize()))
        {
            if(sendData(cmd.getData()))
                qDebug() << "sended";
            else
                qDebug() << "not send data";
        }
        else qDebug("not send size");
    }
    else qDebug() << "not send code";
}


/*bool TransportLayer::sendCmdCode(QByteArray code)
{
    //QDataStream out(&data, QIODevice::WriteOnly);
    //out.setByteOrder(QDataStream::BigEndian);
    socket->write(code);
    return socket->waitForBytesWritten(); //socket->flush()
}

bool TransportLayer::sendCmdSize(QByteArray size)
{
    //QDataStream out(&data, QIODevice::WriteOnly);
    //out.setByteOrder(QDataStream::BigEndian);
    socket->write(size);
    return socket->waitForBytesWritten(); //socket->flush()
}

bool TransportLayer::sendCmdData(QByteArray data)
{
    //QDataStream out(&data, QIODevice::WriteOnly);
    //out.setByteOrder(QDataStream::BigEndian);
    socket->write(data);
    return socket->waitForBytesWritten(); //socket->flush()
}*/

bool TransportLayer::sendData(QByteArray data)
{
    //QDataStream out(&data, QIODevice::WriteOnly);
    //out.setByteOrder(QDataStream::BigEndian);
    socket->write(data);
    return socket->waitForBytesWritten(); //socket->flush()
}
