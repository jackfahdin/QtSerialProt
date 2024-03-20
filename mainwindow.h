#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

// QT6 serial port 头文件
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QMessageBox>
#include <QList>
#include <QDebug>
#include <QDateTime>
#include <QTimer>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void SerialPortSwitch();
    void sendData();
    void readData();
    void ReceiveClean();

    void on_AutoResend_stateChanged(int state);

    void on_SendButton_clicked();

private:
    Ui::MainWindow *ui;
    QSerialPort *MySerialPort;
    QTimer *SendTimer;
};
#endif // MAINWINDOW_H
