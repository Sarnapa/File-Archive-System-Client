#include "RemoteListModel.h"

RemoteListModel::RemoteListModel(QObject *parent)
    : QAbstractItemModel(parent)
{
    iconProvider = new QFileIconProvider();
    filesList = new QList<QFileInfo>();
    worker = new TCPWorker;
    workerThread = new TCPThread;
    worker->moveToThread(workerThread);
    connect(worker, SIGNAL(connectedToSystemSignal(bool,QList<QFileInfo>*)), this, SLOT(connectedToSystem(bool,QList<QFileInfo>*)));
    connect(worker, SIGNAL(disconnectedSignal()), this, SLOT(disconnected()));
    connect(worker, SIGNAL(refreshedSignal(bool,QList<QFileInfo>*)), this, SLOT(refreshed(bool,QList<QFileInfo>*)));
    connect(worker, SIGNAL(deletedFileSignal(bool,QString)), this, SLOT(deletedFile(bool,QString)));
    connect(worker, SIGNAL(gotUploadACKSignal(bool,QFileInfo,qint64)), this, SLOT(gotUploadACK(bool,QFileInfo,qint64)));
    connect(worker, SIGNAL(gotDownloadACKSignal(bool,QString)), this, SLOT(gotDownloadACK(bool,QString)));

    connect(this, SIGNAL(connectToSystemSignal(QString,QString,QString)), worker, SLOT(connectToSystem(QString,QString,QString)));
    connect(this, SIGNAL(disconnectSignal()), worker, SLOT(disconnect()));
    connect(this, SIGNAL(cancelSignal()), worker, SLOT(cancel()), Qt::ConnectionType::DirectConnection);
    connect(this, SIGNAL(refreshSignal()), worker, SLOT(refresh()));
    connect(this, SIGNAL(deleteFileSignal(QString)), worker, SLOT(deleteFile(QString)));
    connect(this, SIGNAL(uploadFileSignal(QFileInfo)), worker, SLOT(uploadFile(QFileInfo)));
    connect(this, SIGNAL(downloadFileSignal(QString)), worker, SLOT(downloadFile(QString)));
}

RemoteListModel::~RemoteListModel()
{
    delete iconProvider;
    delete filesList;
}

QVariant RemoteListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    // DisplayRole - The key data to be rendered in the form of text. (QString)
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
    {
       switch (section)
       {
           case 0: return "Name";
           case 1: return "Size";
           case 2: return "Date Modified";
       }
    }
    return QVariant();
}

int RemoteListModel::rowCount(const QModelIndex &parent) const
{
    // For list models only the root node (an invalid parent) should return the list's size. For all
    // other (valid) parents, rowCount() should return 0 so that it does not become a tree model.
    if (!connected() || parent.isValid())
        return 0;
    return filesList->size();
}

int RemoteListModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return COLUMNS_COUNT;
}


//Returns the data stored under the given role for the item referred to by the index.
QVariant RemoteListModel::data(const QModelIndex &index, int role) const
{
    if (!connected() || !index.isValid())
        return QVariant();

    switch (role)
    {
       case Qt::DisplayRole:
           switch (index.column())
           {
               case 0: return fileName(index);
               case 1: return fileSize(index);
               case 2: return lastModifiedDate(index);
           }
           break;
       case Qt::EditRole:
           switch (index.column())
           {
               case 0: return fileName(index);
               case 1: return fileSize(index);
               case 2: return lastModifiedDate(index);
           }
           break;
       case Qt::DecorationRole:
           if (index.column() == 0)
               return fileIcon(index);
           break;
       }

    return QVariant();
}

QModelIndex RemoteListModel::index(int row, int column, const QModelIndex &parent) const
{
    if (parent.isValid()) return QModelIndex();

    if(row >= filesList->size() || column >= COLUMNS_COUNT || column < 0 || row < 0)
        return QModelIndex();

    return createIndex(row, column, nullptr);
}

QModelIndex RemoteListModel::parent(const QModelIndex &index) const
{
    return QModelIndex();
}


bool RemoteListModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if(index.isValid() && role == Qt::EditRole)
    {
        if (data(index, role) != value)
        {
            QFileInfo file = filesList->at(index.row());
            switch (index.column())
            {
                case 0:
                    /*file.setFileName(value.toString());
                    filesList->replace(index.row(), file);*/
                    break;
                case 1:
                    /*file.setFileSize(value.toLongLong());
                    filesList->replace(index.row(), file);*/
                    break;
                case 2:
                    /*file.setFileLastModified(value.toDateTime());
                    filesList->replace(index.row(), file);*/
                    break;
            }
            emit dataChanged(index, index);
            return true;
        }
    }
    return false;
}

// Returns the item flags for the given index.
// Item flags desribe the properties of an item (can be selected, edited, dragged etc)
// Flag value is power of two.
// The ItemFlags type is a typedef for QFlags<ItemFlag>. It stores an OR combination of ItemFlag values.
Qt::ItemFlags RemoteListModel::flags(const QModelIndex &index) const
{
    if (!connected() || !index.isValid())
        return Qt::NoItemFlags;

    // The default implementation from QAbstractTableModel returns Qt::ItemIsSelectable | Qt::ItemIsEnabled.
    Qt::ItemFlags flags = QAbstractItemModel::flags(index);
    //flags |= Qt::ItemIsDragEnabled;
    if (index.column() == 0)
    {
       flags |= Qt::ItemIsEditable;
    }
    return flags;
}

// Calling beginInsertRows() and endInsertRows() to notify other components that the model has changed.
bool RemoteListModel::insertRows(QList<QFileInfo> *newFiles, int count)
{
    int filesCount = filesList->size();
    if(!connected() || count < 1)
        return false;

    beginInsertRows(QModelIndex(), filesCount, filesCount + count - 1);
    for(int i = 0; i < newFiles->size(); ++i)
    {
        const QFileInfo file = newFiles->at(i);
        int idx = findFile(file.fileName());
        if(idx != -1)
            filesList->replace(idx, file);
        else
            filesList->append(file);
    }
    endInsertRows();
    return true;
}

bool RemoteListModel::removeRow(QString fileName)
{
    int filesCount = filesList->size();

    if(!connected() || filesCount < 1)
        return false;

    int idx = findFile(fileName);

    beginRemoveRows(QModelIndex(), idx, idx);
    filesList->removeAt(idx);
    endRemoveRows();

    return true;
}

bool RemoteListModel::removeAllRows()
{
    int filesCount = filesList->size();

    if(!connected() || filesCount < 1)
        return false;
    beginRemoveRows(QModelIndex(), 0, filesCount - 1);
    filesList->clear();
    endRemoveRows();

    return true;
}

int RemoteListModel::findFile(QString fileName) const
{
    for(int i = 0; i < filesList->size(); ++i)
    {
        if(filesList->at(i).fileName() == fileName)
            return i;
    }
    return -1;
}

QString RemoteListModel::fileName(const QModelIndex &index) const
{
    if (!connected())
        return QString();
    return filesList->at(index.row()).fileName();
}

QIcon RemoteListModel::fileIcon(const QModelIndex &index) const
{
    if (!connected() || !index.isValid())
        return QIcon();
    return iconProvider->icon(QFileIconProvider::File);
}

QString RemoteListModel::fileSize(const QModelIndex &index) const
{
    if (!connected())
        return QString();

    const QFileInfo &file = filesList->at(index.row());
    qlonglong bytes = file.size();
    if (bytes >= 1000000000)
        return QLocale().toString(bytes / 1000000000) + QString(" GB");
    if (bytes >= 1000000)
        return QLocale().toString(bytes / 1000000) + QString(" MB");
    if (bytes >= 1000)
        return QLocale().toString(bytes / 1000) + QString(" KB");
    return QLocale().toString(bytes) + QString(" bytes");
}

QString RemoteListModel::lastModifiedDate(const QModelIndex &index) const
{
    if (!connected())
        return QString();

    const QFileInfo &file = filesList->at(index.row());

    return file.lastModified().toString(Qt::LocalDate);
}

QString RemoteListModel::userLogin() const
{
    return login;
}

QString RemoteListModel::userPasswd() const
{
    return passwd;
}

QString RemoteListModel::systemAddress() const
{
    return address;
}

// Connection Service

void RemoteListModel::connectToSystem(QString &login, QString &password, QString &address)
{
    this->login = login;
    this->passwd = password;
    this->address = address;
    workerThread->start();
    //worker->connectToSystem(login, password, address);
    emit connectToSystemSignal(login, password, address);
}

void RemoteListModel::disconnect()
{
    //worker->disconnect();
    emit disconnectSignal();
}

void RemoteListModel::refresh()
{
    //worker->refresh();
    emit refreshSignal();
}

void RemoteListModel::deleteFile(int row)
{
    QString fileName = filesList->at(row).fileName();
    //worker->deleteFile(fileName);
    emit deleteFileSignal(fileName);
}

void RemoteListModel::cancel()
{
    qDebug() << "Cancel w modelu";
    //worker->cancel();
    emit cancelSignal();
}

void RemoteListModel::uploadFile(QFileInfo fileInfo)
{
    //qDebug() << QThread::currentThreadId();
    //worker->uploadFile(fileName, size, lastModified);
    emit uploadFileSignal(fileInfo);
}

void RemoteListModel::downloadFile(QString fileName)
{
    //worker->downloadFile(fileName);
    emit downloadFileSignal(fileName);
}

void RemoteListModel::connectedToSystem(bool connected, QList<QFileInfo>* userFiles)
{
    isConnected = connected;
    if(isConnected)
        insertRows(userFiles, userFiles->size());
    else
    {
        workerThread->exit();
        clearUserData();
    }
    emit connectedToSystemSignal(isConnected);
}

void RemoteListModel::disconnected()
{
    isConnected = false;
    workerThread->exit();
    clearUserData();
    removeAllRows();
    emit disconnectedSignal();
}

void RemoteListModel::refreshed(bool connected, QList<QFileInfo> *userFiles)
{
    isConnected = connected;
    if(isConnected)
        insertRows(userFiles, userFiles->size());
    else
    {
        workerThread->exit();
        clearUserData();
        removeAllRows();
    }
    emit refreshedSignal(connected);
}

void RemoteListModel::deletedFile(bool connected, QString fileName)
{
    isConnected = connected;
    if(isConnected)
        removeRow(fileName);
    else
    {
        workerThread->exit();
        clearUserData();
        removeAllRows();
    }
    emit deletedFileSignal(isConnected);
}

void RemoteListModel::gotUploadACK(bool connected, QFileInfo fileInfo, qint64 currentSize)
{
    isConnected = connected;
    int value = 0;
    if(isConnected)
    {
        value = (currentSize * 100) / fileInfo.size();
        if(value >= 100)
        {
            value = 100;
            QList<QFileInfo> *newFiles = new QList<QFileInfo>;
            newFiles->append(fileInfo);
            insertRows(newFiles, 1);
        }
    }
    else
    {
        workerThread->exit();
        clearUserData();
        removeAllRows();
    }
    emit gotUploadACKSignal(isConnected, value);
}

void RemoteListModel::gotDownloadACK(bool connected, QString fileName)
{
    isConnected = connected;
    int value = 0;
    if(isConnected)
    {

    }
    else
    {
        workerThread->exit();
        clearUserData();
        removeAllRows();
    }
    emit gotDownloadACKSignal(isConnected, value, fileName);
}
