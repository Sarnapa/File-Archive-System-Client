#ifndef COMMAND_H
#define COMMAND_H

#include <QObject>
#include <QDebug>
#include <QByteArray>
#include <QString>
#include <QDataStream>
#include <QTcpSocket>
#include <QtEndian>
#include <iostream>

enum CMD
{
    NOOP = 0x00,
    INTRODUCE = 0x01,
    LOGIN = 0x02,
    LIS = 0x05,
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
    EXIT = 0xff,
    TEST1 = 0x46,
    TEST2 = 0x47
};

enum STATE
{
    NO_DATA = 0,
    WAIT_FOR_SIZE = 1,
    WAIT_FOR_DATA = 2,
    GOT_DATA = 3,
};

enum SENDER_OBJECT
{
    CODE = 0,
    SIZE = 1,
    DATA = 2
};

class Command : public QObject
{
    Q_OBJECT
public:
    explicit Command(QObject *parent = 0);

    // to send
    explicit Command(CMD code, QObject *parent = 0);
    explicit Command(CMD code, QString data, QObject *parent = 0);
     ~Command();

    CMD getCodeInt();
    quint32 getSizeInt();
    QString getDataString();
    STATE getState();

public slots:
    void getCmdCode(QTcpSocket *socket, QDataStream &socketStream);
    void getCmdSize(QTcpSocket *socket, QDataStream &socketStream);
    void getCmdData(QTcpSocket *socket, QDataStream &socketStream);

    void sendCmd(QTcpSocket *socket);
    bool sendCmdCode(QTcpSocket *socket);
    bool sendCmdSize(QTcpSocket *socket);
    bool sendCmdData(QTcpSocket *socket);

signals:
    void sendCmdSignal(QTcpSocket *socket);

private:
    //To receive
    quint8 codeInt;
    quint32 sizeInt;
    QString dataString;

    //To send
    QByteArray code;
    QByteArray size;
    QByteArray data;

    STATE state;

    bool needMoreData();
    static QByteArray intToArray(quint8 source);
    static QByteArray intToArray(quint32 source);
};

#endif // COMMAND_H
