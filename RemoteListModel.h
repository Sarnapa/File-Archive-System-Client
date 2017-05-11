#ifndef REMOTELISTMODEL_H
#define REMOTELISTMODEL_H

#include <QAbstractListModel>
#include <QFileIconProvider>
#include <QTimer>
#include <QFuture>
#include <QtConcurrent/QtConcurrent>
#include "TCPWorker.h"

#define COLUMNS_COUNT 3

class RemoteListModel : public QAbstractItemModel
{
    Q_OBJECT
    Q_PROPERTY(QString userLogin READ userLogin)
    Q_PROPERTY(QString userPasswd READ userPasswd)
    Q_PROPERTY(QString systemAddress READ systemAddress)

public:
    explicit RemoteListModel(QObject *parent = 0);
    ~RemoteListModel();

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index = QModelIndex()) const override;

    // Editable:
    bool setData(const QModelIndex &index, const QVariant &value,
                 int role = Qt::EditRole) override;

    Qt::ItemFlags flags(const QModelIndex& index) const override;

    // Add data:
    bool insertRows(QList<MyFileInfo> *newFiles, int count);

    // Remove data:
    bool removeRow(QString fileName);
    bool removeAllRows();

    inline bool connected() const
    {
      return isConnected;
    }

    QString fileName(const QModelIndex &index) const;
    QIcon fileIcon(const QModelIndex &index) const;
    QString fileSize(const QModelIndex &index) const;
    QString lastModifiedDate(const QModelIndex &index) const;

    QString userLogin() const;
    QString userPasswd() const;
    QString systemAddress() const;

    void connectToSystem(QString &login, QString &password, QString &address);
    void disconnect();
    void refresh();
    void deleteFile(int row);
    void cancel();
    void uploadFile(QString fileName, qlonglong size, QDateTime lastModified);
    void downloadFile(QString fileName);

    //for PAIN
    int getFilePartNumber()
    {
        return filePartNumber;
    }

signals:
    void connectToSystemSignal(QString login, QString password, QString address);

    void connectedToSystemSignal(bool connected);
    void disconnectedSignal();
    void refreshedSignal(bool connected);
    void deletedFileSignal(bool connected);
    void gotUploadACKSignal(bool connected, int progressBarValue);
    void gotDownloadACKSignal(bool connected, int progressBarValue, QString fileName);
private:
    QList<MyFileInfo> *filesList;
    QFileIconProvider *iconProvider;
    QString login = "";
    QString passwd = "";
    QString address = "";
    bool isConnected = false;
    TCPWorker *worker;
    //for PAIN
    QTimer *timer;
    int filePartNumber = 0;

    int findFile(QString fileName) const;
    inline void clearUserData()
    {
        this->login = "";
        this->passwd = "";
        this->address = "";
    }

private slots:
    void connectedToSystem(bool connected, QList<MyFileInfo> *userFiles);
    void disconnected();
    void refreshed(bool connected, QList<MyFileInfo> *userList);
    void deletedFile(bool connected, QString fileName);
    void gotUploadACK(bool connected, QString fileName, qlonglong size, QDateTime lastModified);
    void gotDownloadACK(bool connected, QString fileName);
};

#endif // REMOTELISTMODEL_H
