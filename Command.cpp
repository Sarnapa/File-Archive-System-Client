#include "Command.h"

Command::Command()
{
    this->state = WRONG_CMD;
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

Command& Command::operator=(const Command &cmd)
{
    this->code = cmd.code;
    this->size = cmd.size;
    this->data = cmd.data;
    this->state = cmd.state;
    return *this;
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

void Command::setCode(QByteArray &code)
{
    this->code = code;
    needMoreData();
}

void Command::setSize(QByteArray &size)
{
    this->size = size;
    if(this->size.size() == (int)sizeof(quint32))
        this->state = WAIT_FOR_DATA;
    else
        this->state = WRONG_CMD;
}

void Command::setData(QByteArray &data)
{
    this->data = data;
    QDataStream tmpStream(&size, QIODevice::ReadOnly);
    int requiredBytes;
    tmpStream >> requiredBytes;
    if(data.size() == requiredBytes)
        this->state = GOT_DATA;
    else
        this->state = WRONG_CMD;
}

void Command::needMoreData()
{
    if(code.size() != (int)sizeof(quint8))
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
            return;
        }
    }
    this->state = NO_DATA;
}
