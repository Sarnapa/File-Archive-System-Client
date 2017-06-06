#ifndef TRANSPORTLAYER_H
#define TRANSPORTLAYER_H

#include <QTcpSocket>
#include "Command.h"

class TransportLayer : public QObject
{
    Q_OBJECT
public:
    explicit TransportLayer(QObject *parent = 0);
    explicit TransportLayer(QTcpSocket *socket, QObject *parent = 0);

    Command getCmd(int timeout = 10000);
    Command getCmd(int timeout1, int timeout2, int timeout3);
    QByteArray getCmdCode();
    QByteArray getCmdSize();
    QByteArray getCmdData(int size);

    bool sendCmd(Command& cmd);
    void clear();
private:
    QTcpSocket *socket;
    QDataStream socketStream;
    Command receivedCmd;
    QList<Command> commandList;
    const int maxSizeData = 10000000;
    QByteArray code, sizeArray, data;

    QByteArray getBytes(int timeout, int requiredBytes);
    bool sendData(QByteArray data);
};

#endif // TRANSPORTLAYER_H
