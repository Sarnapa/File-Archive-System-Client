#include "Converter.h"

Converter::Converter(QObject *parent) : QObject(parent)
{}


QByteArray Converter::intToBinary(quint8 source)
{
    QByteArray byteArray;
    byteArray.append((char)(source & 0xFF));
    return byteArray;
}

QByteArray Converter::intToBinary(quint32 source)
{
    QByteArray byteArray;
    for(unsigned int i = 0; i != sizeof(source); ++i)
    {
        byteArray.append((char)((source & (0xFF << (i*8))) >> (i*8)));
    }
    return byteArray;
}

QByteArray Converter::intToBinary(quint64 source)
{
    QByteArray byteArray;
    for(unsigned int i = 0; i != sizeof(source); ++i)
    {
        byteArray.append((char)((source & (0xFF << (i*8))) >> (i*8)));
    }
    return byteArray;

}

QByteArray Converter::intToArray(quint8 source)
{
    QByteArray temp;
    QDataStream data(&temp, QIODevice::ReadWrite);
    data << source;
    return temp;
}

QByteArray Converter::intToArray(quint32 source)
{
    QByteArray temp;
    QDataStream data(&temp, QIODevice::ReadWrite);
    data << source;
    return temp;
}

QByteArray Converter::intToArray(quint64 source)
{
    QByteArray temp;
    QDataStream data(&temp, QIODevice::ReadWrite);
    data << source;
    return temp;
}
