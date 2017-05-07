#ifndef MYFILEINFO_H
#define MYFILEINFO_H

#include <QLocale>
#include <QDateTime>
#include <QMessageBox>

class MyFileInfo
{
public:
    MyFileInfo()
    {}
    MyFileInfo(QString name, qlonglong size, QDateTime lastModified): fileName(name), fileSize(size), fileLastModified(lastModified)
    {}
    ~MyFileInfo()
    {}

    MyFileInfo& operator =(const MyFileInfo& file)
    {
        this->fileName = file.getFileName();
        this->fileSize = file.getFileSize();
        this->fileLastModified = file.getFileLastModified();
        return *this;
    }

    QString getFileName() const
    {
        return fileName;
    }

    //QVariant supports qlonglong and qulonglong. As the documentation says, these are the same as qint64 and quint64.
    qlonglong getFileSize() const
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
    qlonglong fileSize;
    QDateTime fileLastModified;
};

#endif // MYFILEINFO_H
