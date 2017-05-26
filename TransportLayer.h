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

    Command getCmd();

    void sendCmd(Command& cmd);
private:
    QTcpSocket *socket;
    QDataStream socketStream;

    QByteArray getCmdCode();
    QByteArray getCmdSize();
    QByteArray getCmdData(int size);

    bool sendCmdCode(QByteArray code);
    bool sendCmdSize(QByteArray size);
    bool sendCmdData(QByteArray data);
};

#endif // TRANSPORTLAYER_H
