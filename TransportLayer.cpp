#include "TransportLayer.h"

TransportLayer::TransportLayer(QObject *parent) : QObject(parent)
{
}

TransportLayer::TransportLayer(QTcpSocket *socket, QObject *parent) : QObject(parent)
{
    this->socket = socket;
}

/*Command TransportLayer::getCmd()
{
    Command cmd;
    QByteArray code = getCmdCode();
    if(code.size() == 0)
    {
        return Command();
    }
    cmd.setCode(getCmdCode());
    if(cmd.needMoreData())
    {
        QByteArray sizeArray = getCmdSize();
        if(sizeArray.size() == 0)
            return Command();
        cmd.setSize(sizeArray);
        QByteArray data = getCmdData(sizeArray.toInt());
        if(data.size() == 0)
            return Command();
        cmd.setData(data);
    }
    return cmd;
}*/

QByteArray TransportLayer::getCmdCode()
{
    QByteArray code;
    if(socket->bytesAvailable() != (int)sizeof(quint8))
    {
        qDebug() << "Required one byte for command code.";
        return QByteArray();
    }
    code = socket->readAll();
    return code;
}

QByteArray TransportLayer::getCmdSize()
{
    QByteArray size;
    if(socket->bytesAvailable() != (int)sizeof(quint32))
    {
        qDebug() << "Required 4 bytes for command code.";
        return QByteArray();
    }
    size = socket->readAll();
    return size;
}

QByteArray TransportLayer::getCmdData(int size)
{
    QByteArray data;
    if(socket->bytesAvailable() != size)
    {
        qDebug() << "Expected " + QString(size) + " bytes.";
        return QByteArray();
    }
    data = socket->readAll();
    return data;
}

void TransportLayer::sendCmd(Command& cmd)
{
    qDebug() << "W transportLayer:";
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
