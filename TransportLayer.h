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

    QByteArray getCmdCode();
    QByteArray getCmdSize();
    QByteArray getCmdData(int size);

    void sendCmd(Command& cmd);
private:
    QTcpSocket *socket;
    QDataStream socketStream;

    /*bool sendCmdCode(QByteArray code);
    bool sendCmdSize(QByteArray size);
    bool sendCmdData(QByteArray data);*/
    bool sendData(QByteArray data);
};

#endif // TRANSPORTLAYER_H
