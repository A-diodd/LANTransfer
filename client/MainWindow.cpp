#include "MainWindow.h"
#include "NetworkManager.h"
#include "TransferManager.h"

#include <QComboBox>
#include <QFileDialog>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QProgressBar>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QWidget>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      networkManager(new NetworkManager(this)),
      transferManager(new TransferManager(networkManager, this))
{
	
    setupUi();
    setupConnections();
}

void MainWindow::setupUi()
{
    auto *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    setWindowTitle("LANTransfer");
    resize(1000, 700);

    auto *mainLayout = new QVBoxLayout(centralWidget);

    //device
    auto *deviceGroup = new QGroupBox("Devices", centralWidget);
    auto *deviceLayout = new QVBoxLayout(deviceGroup);

    deviceList = new QListWidget(deviceGroup);

    deviceList->addItem("Device-A    192.168.1.10    Online");
    deviceList->addItem("Device-B    192.168.1.11    Online");
    deviceList->addItem("Device-C    192.168.1.12    Offline");

    deviceLayout->addWidget(deviceList);

    mainLayout->addWidget(deviceGroup);

    //file
    auto *fileGroup = new QGroupBox("File", centralWidget);
    auto *fileLayout = new QHBoxLayout(fileGroup);

    filePathEdit = new QLineEdit(fileGroup);
    filePathEdit->setPlaceholderText("Select a file...");

    selectFileButton =
        new QPushButton("Select File", fileGroup);

    fileLayout->addWidget(filePathEdit);
    fileLayout->addWidget(selectFileButton);

    mainLayout->addWidget(fileGroup);

    //target
    auto *targetGroup = new QGroupBox("Target", centralWidget);
    auto *targetLayout = new QHBoxLayout(targetGroup);

    targetComboBox = new QComboBox(targetGroup);

    targetComboBox->addItem("Device-A");
    targetComboBox->addItem("Device-B");
    targetComboBox->addItem("Device-C");

    targetLayout->addWidget(targetComboBox);

    mainLayout->addWidget(targetGroup);

    //progress
    auto *progressGroup =
        new QGroupBox("Transfer Progress", centralWidget);

    auto *progressLayout =
        new QVBoxLayout(progressGroup);

    progressBar = new QProgressBar(progressGroup);
    progressBar->setRange(0, 100);
    progressBar->setValue(50);

    transferInfoLabel =
        new QLabel("50 MB / 100 MB", progressGroup);

    speedLabel =
        new QLabel("0 MB/s", progressGroup);

    progressLayout->addWidget(progressBar);
    progressLayout->addWidget(transferInfoLabel);
    progressLayout->addWidget(speedLabel);

    mainLayout->addWidget(progressGroup);

    //control
    auto *controlLayout = new QHBoxLayout();

    sendButton = new QPushButton("Send", centralWidget);
    pauseButton = new QPushButton("Pause", centralWidget);
    resumeButton = new QPushButton("Resume", centralWidget);
    cancelButton = new QPushButton("Cancel", centralWidget);

    controlLayout->addWidget(sendButton);
    controlLayout->addWidget(pauseButton);
    controlLayout->addWidget(resumeButton);
    controlLayout->addWidget(cancelButton);

    mainLayout->addLayout(controlLayout);

    //log
    auto *logGroup =
        new QGroupBox("Logs", centralWidget);

    auto *logLayout =
        new QVBoxLayout(logGroup);

    logEdit = new QTextEdit(logGroup);
    logEdit->setReadOnly(true);

    logEdit->append("[INFO] LANTransfer started.");

    logLayout->addWidget(logEdit);

    mainLayout->addWidget(logGroup);


    mainLayout->setStretch(0, 1);
    mainLayout->setStretch(1, 1);
    mainLayout->setStretch(2, 1);
    mainLayout->setStretch(3, 2);
    mainLayout->setStretch(4, 1);
}

void MainWindow::setupConnections()
{
    connect(selectFileButton,
            &QPushButton::clicked,
            this,
            &MainWindow::onSelectFileClicked);

    connect(sendButton,
            &QPushButton::clicked,
            this,
            &MainWindow::onSendClicked);

    connect(pauseButton,
            &QPushButton::clicked,
            this,
            &MainWindow::onPauseClicked);

    connect(resumeButton,
            &QPushButton::clicked,
            this,
            &MainWindow::onResumeClicked);

    connect(cancelButton,
            &QPushButton::clicked,
            this,
            &MainWindow::onCancelClicked);

    //mainwindow 连接 NetworkManager的信号
    connect(networkManager,
            &NetworkManager::connected,
            this,
            [this]()
            {
                logEdit->append(
                    "[INFO] Connected to server.");
            });

    connect(networkManager,
            &NetworkManager::disconnected,
            this,
            [this]()
            {
                logEdit->append(
                    "[INFO] Disconnected from server.");
            });

    connect(networkManager,
            &NetworkManager::errorOccurred,
            this,
            [this](const QString &message)
            {
                logEdit->append(
                    "[ERROR] " + message);
            });

    connect(networkManager,
            &NetworkManager::messageReceived,
            this,
            [this](Protocol::MessageType type,
                   const QByteArray &payload)
            {

                if(type == Protocol::MessageType::HelloAck)
                {
                    logEdit->append(
                        "[RECV] HelloAck");
                }


                if(type == Protocol::MessageType::FileAccept)
                {
                    logEdit->append(
                        "[RECV] File accepted");
                }

            });

    connect(transferManager,
            &TransferManager::logMessage,
            this,
            [this](const QString &message)
            {
                logEdit->append(message);
            });

    connect(transferManager,
            &TransferManager::transferFailed,
            this,
            [this](const QString &message)
            {
                logEdit->append(
                    "[ERROR] " + message);
            });
                
}

void MainWindow::onSelectFileClicked()
{
    QString filePath =
        QFileDialog::getOpenFileName(
            this,
            "Select File");

    if (filePath.isEmpty())
        return;

    filePathEdit->setText(filePath);

    logEdit->append(
        "[INFO] Selected file: " + filePath);
}

void MainWindow::onSendClicked()
{
	QString filePath =filePathEdit->text();
	if(filePath.isEmpty())
	{
		logEdit->append(
			"[WARN] Please select a file.");
			
		return;	
	}
	transferManager->startTransfer(filePath);
}

void MainWindow::onPauseClicked()
{
    logEdit->append("[INFO] Pause requested.");
}

void MainWindow::onResumeClicked()
{
    logEdit->append("[INFO] Resume requested.");
}

void MainWindow::onCancelClicked()
{
    logEdit->append("[INFO] Cancel requested.");
}