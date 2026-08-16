#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class QListWidget;
class QLineEdit;
class QPushButton;
class QComboBox;
class QProgressBar;
class QLabel;
class QTextEdit;
class NetworkManager;
class TransferManager;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private slots:
    void onSelectFileClicked();
    void onSendClicked();
    void onPauseClicked();
    void onResumeClicked();
    void onCancelClicked();

private:
    void setupUi();
    void setupConnections();

private:
    // Device
    QListWidget *deviceList;

    // File
    QLineEdit *filePathEdit;
    QPushButton *selectFileButton;

    // Target
    QComboBox *targetComboBox;

    // Progress
    QProgressBar *progressBar;
    QLabel *transferInfoLabel;
    QLabel *speedLabel;

    // Control
    QPushButton *sendButton;
    QPushButton *pauseButton;
    QPushButton *resumeButton;
    QPushButton *cancelButton;

    // Log
    QTextEdit *logEdit;

    // Network
    NetworkManager *networkManager;
    TransferManager *transferManager;
};

#endif // MAINWINDOW_H