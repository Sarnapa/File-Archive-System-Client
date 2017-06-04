#ifndef COMMAND_H
#define COMMAND_H

#include <QObject>
#include <QByteArray>
#include <QDataStream>

enum OBJECT_CODE
{
    NOOP = 0x00,
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
    EXIT = 0xff,
};

enum STATE
{
    WAIT_FOR_CODE = 0,
    NO_DATA = 1,
    WAIT_FOR_SIZE = 2,
    WAIT_FOR_DATA = 3,
    GOT_DATA = 4,
    WRONG_CMD = 5
};

enum ERROR_CODE
{
    MISSING_FILE = 0x01,
    LOGGING_ERROR = 0x192
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
    STATE getState();
    void setCode(QByteArray &code);
    void setSize(QByteArray &size);
    void setData(QByteArray &data);
    void setState(STATE state);
private:

    QByteArray code;
    QByteArray size;
    QByteArray data;
    STATE state;

    void needMoreData();
};

#endif // COMMAND_H
