#include "Command.h"

Command::Command(QObject *parent) : QObject(parent)
{

}

Command::~Command()
{

}


// to send
Command::Command(CMD code, QObject *parent) : QObject(parent)
{
    this->code = intToArray((quint8)code);
}

// to send
Command::Command(CMD code, QString data, QObject *parent) : QObject(parent)
{
    this->code = intToArray((quint8)code);
    this->size = intToArray((quint32)data.length());
    this->data = data.toLocal8Bit();
}

Command& Command::operator=(const Command &cmd)
{
    this->code = cmd.code;
    this->codeInt = cmd.codeInt;
    this->size = cmd.size;
    this->sizeInt = cmd.sizeInt;
    this->data = cmd.data;
    this->dataString = cmd.dataString;
    return *this;
}

CMD Command::getCodeInt()
{
    return (CMD)codeInt;
}

quint32 Command::getSizeInt()
{
    return sizeInt;
}

QString Command::getDataString()
{
    return dataString;
}

QByteArray Command::getCode()
{
    return code;
}

QByteArray Command::getSize()
{
    return size;
}

QByteArray Command::getData()
{
    return data;
}

STATE Command::getState()
{
    return state;
}

void Command::getCmdCode(QTcpSocket *socket, QDataStream &socketStream)
{
    socketStream.startTransaction();
    if(socket->bytesAvailable() < (int)sizeof(quint8))
    {
        qDebug() << "Got less than one byte.";
        return;
    }
    quint8 code;
    socketStream >> code;

    if (!socketStream.commitTransaction())
    {
        qDebug() << "Transaction failed.";
        return;
    }

    this->codeInt = code;
    if(needMoreData())
        this->state = WAIT_FOR_SIZE;
    else
        this->state = NO_DATA;
}

void Command::getCmdSize(QTcpSocket *socket, QDataStream &socketStream)
{
    //socketStream.device()->seek(0);
    socketStream.startTransaction();
    if(socket->bytesAvailable() < (int)sizeof(quint32))
    {
        qDebug() << "Got less than 4 bytes.";
        return;
    }
    quint32 size;
    socketStream >> size;
    if (!socketStream.commitTransaction())
    {
        qDebug() << "Transaction failed.";
        return;
    }

    this->sizeInt = qFromBigEndian(size);
    this->state = WAIT_FOR_DATA;
}

void Command::getCmdData(QTcpSocket *socket, QDataStream &socketStream)
{
    //socketStream.device()->seek(0);
    socketStream.startTransaction();
    socketStream.setByteOrder(QDataStream::LittleEndian);
    if(socket->bytesAvailable() < size.toInt())
    {
        qDebug() << "Got less than expected bytes.";
        return;
    }
    QString data;
    socketStream >> data;

    if (!socketStream.commitTransaction())
    {
        qDebug() << "Transaction failed.";
        return;
    }

    this->dataString = data;
    this->state = GOT_DATA;
}

void Command::sendCmd(QTcpSocket *socket)
{
    sendCmdCode(socket);
    sendCmdSize(socket);
    sendCmdData(socket);
}

bool Command::sendCmdCode(QTcpSocket *socket)
{
    QDataStream out(&code, QIODevice::WriteOnly);
    out.setByteOrder(QDataStream::BigEndian);
    socket->write(code);
    return socket->waitForBytesWritten(); //socket->flush()
}

bool Command::sendCmdSize(QTcpSocket *socket)
{
    QDataStream out(&size, QIODevice::WriteOnly);
    out.setByteOrder(QDataStream::BigEndian);
    socket->write(size);
    return socket->waitForBytesWritten(); //socket->flush()
}

bool Command::sendCmdData(QTcpSocket *socket)
{
    QDataStream out(&data, QIODevice::WriteOnly);
    out.setByteOrder(QDataStream::BigEndian);
    socket->write(data);
    return socket->waitForBytesWritten(); //socket->flush()
}

/*QByteArray getReversedSize()
{
    QByteArray reversedSize = size;
    std::reverse(reversedSize.begin(), reversedSize.end());
    return reversedSize;
}*/

bool Command::needMoreData()
{
    quint8 needMoreDataCmds [] = {0x01, 0x02, 0x05, 0x06, 0x07, 0x09, 0x0a, 0x0b, 0x33};
    for(unsigned int i = 0; i < 9; ++i)
    {
        if(needMoreDataCmds[i] == codeInt)
            return true;
    }
    return false;
}

QByteArray Command::intToArray(quint8 source)
{
    QByteArray temp;
    QDataStream data(&temp, QIODevice::ReadWrite);
    data << source;
    return temp;
}

QByteArray Command::intToArray(quint32 source)
{
    QByteArray temp;
    QDataStream data(&temp, QIODevice::ReadWrite);
    data << source;
    return temp;
}
