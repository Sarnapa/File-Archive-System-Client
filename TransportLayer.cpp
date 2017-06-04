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
        cmd.setCode(code);
    }
    else
        return Command();
    if(cmd.getState() == WAIT_FOR_SIZE)
    {
        sizeArray = getBytes(timeout, (int)sizeof(quint32));
        if(sizeArray.size() > 0)
        {
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
            {

            }
            else
                qDebug() << "not send data";
        }
        else qDebug("not send size");
    }
    else qDebug() << "not send code";
}

bool TransportLayer::sendData(QByteArray data)
{
    //QDataStream out(&data, QIODevice::WriteOnly);
    //out.setByteOrder(QDataStream::BigEndian);
    socket->write(data);
    return socket->waitForBytesWritten(); //socket->flush()
}
