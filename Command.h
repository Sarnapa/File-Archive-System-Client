#ifndef COMMAND_H
#define COMMAND_H

#include <QObject>
#include <QByteArray>
#include <QDataStream>

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

class Command
{
public:
    Command();
    Command(QByteArray &code, QByteArray &size, QByteArray &data);
     ~Command();

    Command& operator=(const Command& cmd);

    QByteArray getCode();
    QByteArray getSize();
    QByteArray getData();
    //STATE getState();
    void setCode(QByteArray &code);
    void setSize(QByteArray &size);
    void setData(QByteArray &data);
    //void setState(STATE state);

    bool needMoreData();
private:

    QByteArray code;
    QByteArray size;
    QByteArray data;
    //STATE state;
};

#endif // COMMAND_H
