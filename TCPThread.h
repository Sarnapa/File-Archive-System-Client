#ifndef TCPTHREAD_H
#define TCPTHREAD_H

#include <QThread>

class TCPThread : public QThread
{
    Q_OBJECT
public:
    TCPThread();
    ~TCPThread();   
protected:
    void run();

};

#endif // TCPTHREAD_H
