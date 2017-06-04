#include "MainWindow.h"
#include "ui_MainWindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    localModel = new QFileSystemModel(this);
    localModel->setRootPath("C:/");
    ui->localView->setModel(localModel);
    remoteModel = new RemoteListModel(this);
    ui->remoteView->setModel(remoteModel);

    connect(remoteModel, SIGNAL(connectedToSystemSignal(bool)), this, SLOT(connectedToSystem(bool)));
    connect(remoteModel, SIGNAL(disconnectedSignal()), this, SLOT(disconnected()));
    connect(remoteModel, SIGNAL(refreshedSignal(bool)), this, SLOT(refreshed(bool)));
    connect(remoteModel, SIGNAL(deletedFileSignal(bool)), this, SLOT(deletedFile(bool)));
    connect(ui->uploadButton, SIGNAL(clicked(bool)), this, SLOT(uploadFile()));
    connect(remoteModel, SIGNAL(gotUploadACKSignal(bool,QString,int)), this, SLOT(gotUploadACK(bool,QString,int)));
    connect(remoteModel, SIGNAL(gotUploadAcceptSignal(bool,QString)), this, SLOT(gotUploadAccept(bool,QString)));
    connect(ui->downloadButton, SIGNAL(clicked(bool)), this, SLOT(downloadFile()));
    connect(remoteModel, SIGNAL(gotDownloadACKSignal(bool,int,QString)), this, SLOT(gotDownloadACK(bool,int,QString)));
    connect(this, SIGNAL(progressBarValueChanged(int)), ui->progressBar, SLOT(setValue(int)));
}

MainWindow::~MainWindow()
{
    delete localModel;
    delete remoteModel;
    delete ui;
}

void MainWindow::updateWindow()
{
    setEnabled(true);
    QList<QAction *> &actionsList = ui->mainToolBar->actions();
    for(int i = 0; i < actionsList.size(); ++i)
    {
        if(actionsList.at(i)->objectName() == "actionConnect")
            actionsList.at(i)->setEnabled(!connectionStatus);
        else
            actionsList.at(i)->setEnabled(connectionStatus);
    }
    ui->downloadButton->setEnabled(connectionStatus);
    ui->uploadButton->setEnabled(connectionStatus);
    if(connectionStatus)
    {
        statusLabel = new QLabel();
        statusLabel->setText("Connected as: " + remoteModel->userLogin());
        ui->statusBar->addPermanentWidget(statusLabel);
    }
    else
    {
        if(statusLabel != nullptr)
        {
            ui->statusBar->removeWidget(statusLabel);
        }
    }
    ui->progressBar->setValue(0);
    ui->remoteView->repaint();
}

void MainWindow::on_actionConnect_triggered()
{
    logForm = new LoggingForm(this);
    connect(logForm, SIGNAL(connectSignal(QString&,QString&,QString&)), this, SLOT(connectToSystem(QString&,QString&,QString&)));
    logForm->show();
}

void MainWindow::on_actionDisconnect_triggered()
{
    if(!actionStatus)
        remoteModel->disconnect();
}

void MainWindow::on_actionRefresh_triggered()
{
    if(!actionStatus)
    {
        setEnabled(false);
        remoteModel->refresh();
    }
}

void MainWindow::on_actionRename_triggered()
{
    // not implemented
}

void MainWindow::on_actionDelete_triggered()
{
    if(!actionStatus)
    {
        QModelIndex idx = ui->remoteView->currentIndex();
        if(idx.isValid())
        {
            setEnabled(false);
            remoteModel->deleteFile(idx.row());
        }
    }
}

void MainWindow::on_actionCancel_triggered()
{
    qDebug() << "Cancel w MainWindow";
    if(actionStatus)
    {
        remoteModel->cancel();
    }
}

void MainWindow::connectToSystem(QString &login, QString &password, QString &address)
{
    remoteModel->connectToSystem(login, password, address);
}

void MainWindow::connectedToSystem(bool connected)
{
    connectionStatus = connected;
    if(connectionStatus)
    {
        updateWindow();
        logForm->close();
    }
    else
    {
        logForm->setEnabled(true);
        QMessageBox::warning(logForm, "No connection", "Cannot connect to archive system.");
    }
}

void MainWindow::disconnected()
{
    actionStatus = false;
    if(connectionStatus)
    {
        connectionStatus = false;
        updateWindow();
    }
    else
    {
        logForm->setEnabled(true);
        QMessageBox::warning(logForm, "Logging Error", "Login and/or password wrong.");
    }
}

void MainWindow::refreshed(bool connected)
{
    connectionStatus = connected;
    if(connectionStatus == false)
    {
        QMessageBox::warning(this, "Error", "Lost connection to system.");
        updateWindow();
    }
    setEnabled(true);
}

void MainWindow::deletedFile(bool connected)
{
    connectionStatus = connected;
    if(connectionStatus == false)
    {
        QMessageBox::warning(this, "Error", "Lost connection to system.");
        updateWindow();
    }
    setEnabled(true);
}

/*void MainWindow::renamedFIle(bool connected)
{
    connectionStatus = connected;
    if(connectionStatus == false)
    {
        QMessageBox::warning(this, "Error", "Lost connection to system.");
        updateWindow();
    }
    setEnabled(true);
}*/

void MainWindow::uploadFile()
{
    if(!actionStatus)
    {
        ui->progressBar->setValue(0);
        QModelIndex idx = ui->localView->currentIndex();
        if(idx.isValid())
        {
            QFileInfo fileInfo = localModel->fileInfo(idx);
            if(fileInfo.isFile() && !fileInfo.isExecutable())
            {
                actionStatus = true;
                remoteModel->uploadFile(fileInfo);
            }
        }
    }
}

void MainWindow::gotUploadACK(bool connected, QString fileName ,int progressBarValue)
{
    connectionStatus = connected;
    if(connectionStatus == false)
    {
        QMessageBox::warning(this, "Error", "Lost connection to system.");
        updateWindow();
    }

    if(progressBarValue == -1) // stop
    {
        QMessageBox::information(this, "Information", "Stop uploading file " + fileName + ".");
        progressBarValue = 0;
        actionStatus = false;
    }
    else if(progressBarValue == -2) // cannot open file
    {
        QMessageBox::information(this, "Information", "Cannot open file " + fileName + " to read.");
        progressBarValue = 0;
        actionStatus = false;
    }
    emit progressBarValueChanged(progressBarValue);
}

void MainWindow::gotUploadAccept(bool connected, QString fileName)
{
    connectionStatus = connected;
    if(connectionStatus == false)
    {
        QMessageBox::warning(this, "Error", "File " + fileName + " refused.");
        updateWindow();
    }
    else
    {
        QMessageBox::information(this, "Information", "File " + fileName + " saved.");
    }
    actionStatus = false;
}

void MainWindow::downloadFile()
{
    if(!actionStatus)
    {
        QModelIndex idx = ui->remoteView->currentIndex();
        if(idx.isValid())
        {
            actionStatus = true;
            remoteModel->downloadFile(idx);
        }
    }
}

void MainWindow::gotDownloadACK(bool connected, int progressBarValue, QString fileName)
{
    connectionStatus = connected;
    if(connectionStatus == false)
    {
        QMessageBox::warning(this, "Error", "Lost connection to system.");
        updateWindow();
    }
    if(progressBarValue == 100)
    {
        actionStatus = false;
        /*QFile newFile(path + fileName);
        if(newFile.open(QIODevice::ReadWrite))
        {
            newFile.write("Test");
        }
        newFile.close();*/
    }
    else if(progressBarValue == 0)
        actionStatus = false;
    emit progressBarValueChanged(progressBarValue);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if(connectionStatus)
    {
        qDebug() << "Catch close application event.";
        remoteModel->disconnect();
    }
}
