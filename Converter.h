#ifndef CONVERTER_H
#define CONVERTER_H

#include <QObject>
#include <QByteArray>
#include <QDataStream>
#include <QtEndian>
#include <QDebug>

class Converter : public QObject
{
    Q_OBJECT
public:
    explicit Converter(QObject *parent = 0);

    static QByteArray intToBinary(quint8 source);
    static QByteArray intToBinary(quint32 source);
    static QByteArray intToBinary(quint64 source);

    static QByteArray intToArray(quint8 source);
    static QByteArray intToArray(quint32 source);
    static QByteArray intToArray(quint64 source);

};

#endif // CONVERTER_H
