#include "mainwindow.h"
#include "ui_mainwindow.h"

static const char blankString[] = QT_TRANSLATE_NOOP("SettingsDialog", "N/A");

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    MySerialPort = new QSerialPort;
    QString portName;
    QString description;
    QString manufacturer;
    QString serialNumber;

    // 获取可以用的串口
    QList<QSerialPortInfo> serialPortInfos = QSerialPortInfo::availablePorts();
    // 输出当前系统可以使用的串口个数
    qDebug() << "Total numbers of ports: " << serialPortInfos.count();

    // 将所有可以使用的串口设备添加到SerialPortName中
    for (const QSerialPortInfo &serialPortInfo : serialPortInfos)
    {
        QStringList list;
        portName = serialPortInfo.portName();
        description = serialPortInfo.description();
        manufacturer = serialPortInfo.manufacturer();
        serialNumber = serialPortInfo.serialNumber();
        list << serialPortInfo.portName()
             << (!description.isEmpty() ? description : blankString)
             << (!manufacturer.isEmpty() ? manufacturer : blankString)
             << (!serialNumber.isEmpty() ? serialNumber : blankString)
             << serialPortInfo.systemLocation()
             << (serialPortInfo.vendorIdentifier() ? QString::number(serialPortInfo.vendorIdentifier(), 16) : blankString)
             << (serialPortInfo.productIdentifier() ? QString::number(serialPortInfo.productIdentifier(), 16) : blankString);
        // 获取串口的详细信息
#if 0
        QString portDetails = QString("描述：%1，制造商：%2，序列号：%3")
                                  .arg(list.at(1))
                                  .arg(list.at(2))
                                  .arg(list.at(3));
#else
        QString portDetails = QString("%1")
                                  .arg(list.at(1));
#endif
        // 将串口的详细信息追加到串口名字后面
        QString portNameWithDetails = list.first() + " (" + portDetails + ")";

        // 将串口名字和详细信息作为ItemData存储
        QVariant itemData = QVariant::fromValue(list);

        // 添加Item到SerialPortName
        ui->SerialPortName->addItem(portNameWithDetails, itemData);
    }
    // 设置波特率
    ui->SerialPortRate->addItem(QStringLiteral("1200"), QSerialPort::Baud1200);
    ui->SerialPortRate->addItem(QStringLiteral("2400"), QSerialPort::Baud2400);
    ui->SerialPortRate->addItem(QStringLiteral("4800"), QSerialPort::Baud4800);
    ui->SerialPortRate->addItem(QStringLiteral("9600"), QSerialPort::Baud9600);
    ui->SerialPortRate->addItem(QStringLiteral("19200"), QSerialPort::Baud19200);
    ui->SerialPortRate->addItem(QStringLiteral("57600"), QSerialPort::Baud57600);
    ui->SerialPortRate->addItem(QStringLiteral("115200"), QSerialPort::Baud115200);
    ui->SerialPortRate->setCurrentIndex(3);
    // 设置数据位
    ui->SerialPortData->addItem(QStringLiteral("5"), QSerialPort::Data5);
    ui->SerialPortData->addItem(QStringLiteral("6"), QSerialPort::Data6);
    ui->SerialPortData->addItem(QStringLiteral("7"), QSerialPort::Data7);
    ui->SerialPortData->addItem(QStringLiteral("8"), QSerialPort::Data8);
    ui->SerialPortData->setCurrentIndex(3);
    // 设置奇偶校验位
    ui->SerialPortParity->addItem(QStringLiteral("None"), QSerialPort::NoParity);
    ui->SerialPortParity->addItem(QStringLiteral("Even"), QSerialPort::EvenParity);
    ui->SerialPortParity->addItem(QStringLiteral("Odd"), QSerialPort::OddParity);
    ui->SerialPortParity->addItem(QStringLiteral("Mark"), QSerialPort::MarkParity);
    ui->SerialPortParity->addItem(QStringLiteral("Space"), QSerialPort::SpaceParity);
    // 设置停止位
    ui->SerialPortStop->addItem(QStringLiteral("1"), QSerialPort::OneStop);
    ui->SerialPortStop->addItem(QStringLiteral("2"), QSerialPort::TwoStop);
    ui->SerialPortStop->addItem(QStringLiteral("3"), QSerialPort::OneAndHalfStop);
    // 添加流控
    ui->SerialPortFlow->addItem(QStringLiteral("None"), QSerialPort::NoFlowControl);
    ui->SerialPortFlow->addItem(QStringLiteral("RTS/CTS"), QSerialPort::HardwareControl);
    ui->SerialPortFlow->addItem(QStringLiteral("XON/XOFF"), QSerialPort::SoftwareControl);

    // 禁用发送按钮
    ui->SendButton->setEnabled(false);

    // 程序关闭按钮
    connect(ui->ClosedExe, &QPushButton::clicked, this, &MainWindow::close);

    // 清空接收区
    connect(ui->ReceiveClean, &QPushButton::clicked, this, &MainWindow::ReceiveClean);

    // 打开串口按钮
    connect(ui->SerialPortSwitch, &QPushButton::clicked, this, &MainWindow::SerialPortSwitch);

    // 定时发送-定时器
    SendTimer = new QTimer(this);
    connect(SendTimer, &QTimer::timeout, this, &MainWindow::sendData);
}

MainWindow::~MainWindow()
{
    delete ui;
}

// 串口开关
void MainWindow::SerialPortSwitch()
{
    // 打开串口
    if (ui->SerialPortSwitch->text() == tr("打开串口"))
    {
        // 获取选中的串口信息
        QVariant itemData = ui->SerialPortName->currentData();
        QStringList list = itemData.value<QStringList>();
        QString portName = list.at(0);

        // 设置串口名字
        MySerialPort->setPortName(portName);
        // 设置波特率
        MySerialPort->setBaudRate(ui->SerialPortRate->currentData().toInt());
        // 设置数据位
        MySerialPort->setDataBits(static_cast<QSerialPort::DataBits>(ui->SerialPortData->currentData().toInt()));
        // 设置奇偶校验位
        MySerialPort->setParity(static_cast<QSerialPort::Parity>(ui->SerialPortParity->currentData().toInt()));
        // 设置停止位
        MySerialPort->setStopBits(static_cast<QSerialPort::StopBits>(ui->SerialPortStop->currentData().toInt()));
        // 设置流控
        MySerialPort->setFlowControl(static_cast<QSerialPort::FlowControl>(ui->SerialPortFlow->currentData().toInt()));

        // 打开串口
        if (MySerialPort->open(QIODevice::ReadWrite))
        {
            ui->SerialPortName->setEnabled(false);
            ui->SerialPortRate->setEnabled(false);
            ui->SerialPortData->setEnabled(false);
            ui->SerialPortParity->setEnabled(false);
            ui->SerialPortStop->setEnabled(false);
            ui->SerialPortFlow->setEnabled(false);
            ui->SendButton->setEnabled(true);
            ui->SerialPortSwitch->setText(QStringLiteral("关闭串口"));
            // 信号与槽函数关联
            connect(MySerialPort, &QSerialPort::readyRead, this, &MainWindow::readData);

            // 检查是否选择了自动重发
            if (ui->AutoResend->isChecked())
            {
                // 启动自动重发定时器
                SendTimer->start(ui->AutoResendTimeMs->value());
                ui->SendButton->setText(QStringLiteral("自动发送中"));
            }
        }
        else
        {
            QMessageBox::critical(this, "错误提示", "串口打开失败!!! \r\n该串口可能被占用\r\n请选择正确的串口");
        }
    }
    else
    {
        // 停止发送数据
        SendTimer->stop();

        // 关闭串口读取数据
        disconnect(MySerialPort, &QSerialPort::readyRead, this, &MainWindow::readData);

        // 关闭串口
        MySerialPort->clear();
        MySerialPort->close();
        // 恢复设置功能
        ui->SerialPortName->setEnabled(true);
        ui->SerialPortRate->setEnabled(true);
        ui->SerialPortData->setEnabled(true);
        ui->SerialPortParity->setEnabled(true);
        ui->SerialPortStop->setEnabled(true);
        ui->SerialPortFlow->setEnabled(true);
        ui->SendButton->setEnabled(false);
        ui->SerialPortSwitch->setText(QStringLiteral("打开串口"));
    }
}


// 发送数据
void MainWindow::on_SendButton_clicked()
{
    sendData();
}

// 实际发送数据
void MainWindow::sendData()
{
    QDateTime current_date_time =QDateTime::currentDateTime();
    QString current_date =current_date_time.toString("[hh:mm:ss] : ");
    QString sendMsg = ui->SerialPortSendText->toPlainText();

    if (sendMsg.isEmpty())
        return;

    sendMsg.remove(' ');

    if(ui->AutoWrap->checkState() == 2 && ui->ShowTime->checkState() == 2 && ui->SendHEX->isChecked() == true) {
        // 自动换行 显示时间戳 发送HEX
        ui->SerialPortReceiveText->insertPlainText("\n");
        ui->SerialPortReceiveText->insertPlainText(current_date);
        MySerialPort->write(sendMsg.toUtf8().toHex());
    } else if (ui->AutoWrap->checkState() == 2 && ui->ShowTime->checkState() == 2 && ui->SendHEX->isChecked() == false) {
        // 自动换行 显示时间戳 发送ASCII
        ui->SerialPortReceiveText->insertPlainText("\n");
        ui->SerialPortReceiveText->insertPlainText(current_date);
        MySerialPort->write(sendMsg.toUtf8());
    } else if (ui->AutoWrap->checkState() == 2 && ui->ShowTime->checkState() == 0 && ui->SendHEX->isChecked() == true) {
        // 自动换行 不显示时间戳 发送HEX
        ui->SerialPortReceiveText->insertPlainText("\n");
        MySerialPort->write(sendMsg.toUtf8().toHex());
    } else if (ui->AutoWrap->checkState() == 2 && ui->ShowTime->checkState() == 0 && ui->SendHEX->isChecked() == false) {
        // 自动换行 不显示时间戳 发送ASCII
        ui->SerialPortReceiveText->insertPlainText("\n");
        MySerialPort->write(sendMsg.toUtf8());
    } else if (ui->AutoWrap->checkState() == 0 && ui->ShowTime->checkState() == 2 && ui->SendHEX->isChecked() == true) {
        // 不自动换行 显示时间戳 发送HEX
        ui->SerialPortReceiveText->insertPlainText(current_date);
        MySerialPort->write(sendMsg.toUtf8().toHex());
    } else if (ui->AutoWrap->checkState() == 0 && ui->ShowTime->checkState() == 2 && ui->SendHEX->isChecked() == false) {
        // 不自动换行 显示时间戳 发送ASCII
        ui->SerialPortReceiveText->insertPlainText(current_date);
        MySerialPort->write(sendMsg.toUtf8());
    } else if (ui->AutoWrap->checkState() == 0 && ui->ShowTime->checkState() == 0 && ui->SendHEX->isChecked() == true) {
        // 不自动换行 不显示时间戳 发送HEX
        MySerialPort->write(sendMsg.toUtf8().toHex());
    } else {
        // 不自动换行 不显示时间戳 发送ASCII
        MySerialPort->write(sendMsg.toUtf8());
    }
}

// 接收数据
void MainWindow::readData()
{
    QByteArray receivebuf = MySerialPort->readAll();
    if (!receivebuf.isEmpty())
    {
        QString str = QString::fromUtf8(receivebuf);
        ui->SerialPortReceiveText->insertPlainText(str);
        ui->SerialPortReceiveText->moveCursor(QTextCursor::End);
    }
}

// 清空接收区
void MainWindow::ReceiveClean()
{
    ui->SerialPortReceiveText->clear();
}

void MainWindow::on_AutoResend_stateChanged(int state)
{
    if (state)
    {
        ui->AutoResendTimeMs->setEnabled(false);
        SendTimer->start(ui->AutoResendTimeMs->value());
        ui->SendButton->setText(QStringLiteral("自动发送中"));
        ui->SendButton->setEnabled(false); // 禁用发送按钮
    }
    else
    {
        SendTimer->stop();
        ui->SendButton->setText(QStringLiteral("手动发送"));
        ui->AutoResendTimeMs->setEnabled(true);
        ui->SendButton->setEnabled(true); // 启用发送按钮
    }
}
