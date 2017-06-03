#ifndef MYFILEINFO_H
#define MYFILEINFO_H

#include <QLocale>
#include <QDateTime>
#include <QMessageBox>
#include <QFileInfo>

class MyFileInfo
{
public:
    MyFileInfo()
    {}
    MyFileInfo(QString name, qlonglong size): fileName(name), fileSize(size)
    {}
    MyFileInfo(QString name, qlonglong size, QDateTime lastModified): fileName(name), fileSize(size), fileLastModified(lastModified)
    {}
    MyFileInfo(const MyFileInfo& fileInfo): fileName(fileInfo.fileName), fileSize((qulonglong)fileInfo.fileSize), fileLastModified(fileInfo.fileLastModified)
    {}
    MyFileInfo(const QFileInfo& fileInfo): fileName(fileInfo.fileName()), fileSize((qulonglong)fileInfo.size()), fileLastModified(fileInfo.lastModified())
    {}
    ~MyFileInfo()
    {}

    MyFileInfo& operator =(const MyFileInfo& file)
    {
        this->fileName = file.fileName;
        this->fileSize = file.fileSize;
        this->fileLastModified = file.fileLastModified;
        return *this;
    }

    QString getFileName() const
    {
        return fileName;
    }

    //QVariant supports qlonglong and qulonglong. As the documentation says, these are the same as qint64 and quint64.
    qulonglong getFileSize() const
    {
        return fileSize;
    }

    QDateTime getFileLastModified() const
    {
        return fileLastModified;
    }

    void setFileName(QString fileName)
    {
        this->fileName = fileName;
    }

    void setFileSize(qlonglong fileSize)
    {
        this->fileSize = fileSize;
    }

    void setFileLastModified(QDateTime fileLastModified)
    {
        this->fileLastModified = fileLastModified;
    }

private:
    QString fileName;
    qulonglong fileSize;
    QDateTime fileLastModified;
};

Q_DECLARE_METATYPE(MyFileInfo)


#endif // MYFILEINFO_H
