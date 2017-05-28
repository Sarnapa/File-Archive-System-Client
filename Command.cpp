#include "Command.h"

Command::Command()
{
    this->state = WAIT_FOR_CODE;
}

//to send
Command::Command(QByteArray &code, QByteArray &size, QByteArray &data)
{
    this->code = code;
    this->size = size;
    this->data = data;
}

Command::~Command()
{}

// to send
/*Command::Command(CMD code, QObject *parent) : QObject(parent)
{
    this->code = Converter::intToArray((quint8)code);
}*/

// to send
/*Command::Command(CMD code, QString data, QObject *parent) : QObject(parent)
{
    this->code = Converter::intToArray((quint8)code);
    //this->size = intToArray((quint32)data.length());
    this->data = data.toLocal8Bit();
    this->size = Converter::intToArray((quint32)data.size());
}*/

/*Command::Command(CMD code, QByteArray &data, QObject *parent) : QObject(parent)
{
    this->code = Converter::intToArray((quint8)code);
    this->data = data;
    this->size = Converter::intToArray((quint32)data.size());
}*/

Command& Command::operator=(const Command &cmd)
{
    this->code = cmd.code;
    //this->codeInt = cmd.codeInt;
    this->size = cmd.size;
    //this->sizeInt = cmd.sizeInt;
    this->data = cmd.data;
    //this->dataString = cmd.dataString;
    this->state = cmd.state;
    return *this;
}

/*CMD Command::getCodeInt()
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
}*/

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

void Command::setCode(QByteArray &code)
{
    this->code = code;
    needMoreData();
}

void Command::setSize(QByteArray &size)
{
    this->size = size;
    if(this->size.size() > 0)
        this->state = WAIT_FOR_DATA;
    else
        this->state = WRONG_CMD;
}

void Command::setData(QByteArray &data)
{
    this->data = data;
    if(data.size() > 0)
        this->state = GOT_DATA;
    else
        this->state = WRONG_CMD;
}

/*void Command::setState(STATE state)
{
    this->state = state;
}*/

/*void Command::getCmdCode(QTcpSocket *socket, QDataStream &socketStream)
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
    if(sendCmdCode(socket))
    {
        if(sendCmdSize(socket))
        {
            if(sendCmdData(socket))
                qDebug() << "sended";
            else
                qDebug() << "not send data";
        }
        else qDebug("not send size");
    }
    else qDebug() << "not send code";
    sendCmdCode(socket);
    sendCmdSize(socket);
    sendCmdData(socket);
}*/

/*bool Command::sendCmdCode(QTcpSocket *socket)
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
}*/

/*QByteArray getReversedSize()
{
    QByteArray reversedSize = size;
    std::reverse(reversedSize.begin(), reversedSize.end());
    return reversedSize;
}*/

void Command::needMoreData()
{
    if(code.size() == 0)
    {
        this->state = WRONG_CMD;
        return;
    }
    QDataStream stream(&code, QIODevice::ReadOnly);
    quint8 codeInt;
    stream >> codeInt;
    quint8 needMoreDataCmds [] = {0x01, 0x02, 0x05, 0x06, 0x07, 0x09, 0x0a, 0x0b, 0x33};
    for(unsigned int i = 0; i < 9; ++i)
    {
        if(needMoreDataCmds[i] == codeInt)
        {
            this->state = WAIT_FOR_SIZE;
        }
    }
    this->state = NO_DATA;
}

