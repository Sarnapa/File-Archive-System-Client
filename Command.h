#ifndef COMMAND_H
#define COMMAND_H

#include <QDebug>
#include <QByteArray>
#include <QString>
#include "iostream"

/*
INTRODUCE = 0x01,
LOGIN = 0x02,
LIST = 0x05,
DOWNLOAD = 0x06,
UPLOAD = 0x07,
ACCEPT = 0x08,
CHUNK = 0x09,
DELETE = 0x0a,
RENAME = 0x0b,
COMMIT = 0x0c,
ROLLBACK = 0x0d,
COMMITRDY = 0x0e,
COMMITACK = 0x0f,
ERROR = 0x33,
EXIT = 0xff
*/

class Command
{
public:
    Command()
    {}

    Command(quint8 code)
    {
        this->code = intToArray(code);
    }

    Command(quint8 code, quint32 sizeInt, QString data)
    {
        this->code = intToArray(code);
        this->size = intToArray(sizeInt);
        /*for(unsigned int i = 0; i < 4; ++i)
        {
            this->size[i] = (sizeInt >> 8 * (3 - i)) & 0xFF;
        }*/
        this->data = data.toLocal8Bit();
    }

    QByteArray getCode()
    {
        return code;
    }

    QByteArray getSize()
    {
        return size;
    }

    QByteArray getData()
    {
        return data;
    }

    static QByteArray intToArray(quint8 source)
    {
        QByteArray temp;
        QDataStream data(&temp, QIODevice::ReadWrite);
        data << source;
        return temp;
    }

    static QByteArray intToArray(quint32 source)
    {
        QByteArray temp;
        QDataStream data(&temp, QIODevice::ReadWrite);
        data << source;
        return temp;
    }

    QByteArray getReversedSize()
    {
        QByteArray reversedSize = size;
        std::reverse(reversedSize.begin(), reversedSize.end());
        return reversedSize;
    }

private:
    QByteArray code;
    QByteArray size;
    QByteArray data;
};

#endif // COMMAND_H
