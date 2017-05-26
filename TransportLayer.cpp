#include "TransportLayer.h"

TransportLayer::TransportLayer(QObject *parent) : QObject(parent)
{
}

TransportLayer::TransportLayer(QTcpSocket *socket, QObject *parent) : QObject(parent)
{
    this->socket = socket;
}

Command TransportLayer::getCmd()
{
    Command cmd;
    cmd.setCode(getCmdCode());
    if(cmd.needMoreData())
    {
        QByteArray size = getCmdSize();
        cmd.setSize(size);
        cmd.setData(getCmdData(size.toInt()));
    }
    return cmd;
}

QByteArray TransportLayer::getCmdCode()
{
    QByteArray code;
    if(socket->bytesAvailable() < (int)sizeof(quint8))
    {
        qDebug() << "Got less than one byte.";
        return NULL;
    }
    code = socket->readAll();
    return code;
}

QByteArray TransportLayer::getCmdSize()
{
    QByteArray size;
    if(socket->bytesAvailable() < (int)sizeof(quint32))
    {
        qDebug() << "Got less than 4 bytes.";
        return NULL;
    }
    size = socket->readAll();
    return size;
}

QByteArray TransportLayer::getCmdData(int size)
{
    QByteArray data;
    if(socket->bytesAvailable() < size)
    {
        qDebug() << "Got less than expected bytes.";
        return NULL;
    }
    data = socket->readAll();
    return data;
}

void TransportLayer::sendCmd(Command& cmd)
{
    qDebug() << cmd.getCode().toHex();
    qDebug() << cmd.getSize().toHex();
    qDebug() << QString().fromStdString(cmd.getData().toStdString());
    if(sendCmdCode(cmd.getCode()))
    {
        if(sendCmdSize(cmd.getSize()))
        {
            if(sendCmdData(cmd.getData()))
                qDebug() << "sended";
            else
                qDebug() << "not send data";
        }
        else qDebug("not send size");
    }
    else qDebug() << "not send code";
}

bool TransportLayer::sendCmdCode(QByteArray code)
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
}
