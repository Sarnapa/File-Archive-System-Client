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

    Command getCmd(int timeout = 30000);
    QByteArray getCmdCode();
    QByteArray getCmdSize();
    QByteArray getCmdData(int size);

    void sendCmd(Command& cmd);
private:
    QTcpSocket *socket;
    QDataStream socketStream;
    Command receivedCmd;
    QList<Command> commandList;

    QByteArray getBytes(int timeout, int requiredBytes);
    bool sendData(QByteArray data);
};

#endif // TRANSPORTLAYER_H
