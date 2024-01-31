#include "chatclient.h"
#include "./ui_chatclient.h"
#include <iostream>
#include <WinSock2.h>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QThread>
#include <QAction>
#include <QMenuBar>
#include <QMenu>
#include <stdlib.h>
#include <thread>
#include <string>
#include <stdbool.h>
#include <QApplication>
#include <QTime>
#include <QHBoxLayout>


Ui::ChatClient* ui_user;
ChatClient* chatClient_ptr;
SOCKET user_soc;

u_short port_value;
const char* username_str;
const char* ip_str;
QString username;

bool logout_flag = false;

Ui::ChatClient* ChatClient::getUi()
{
    return ui;
}

void ChatClient::setChatClientPointer(ChatClient* client) {
    chatClient_ptr = client;
}


ChatClient::ChatClient(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::ChatClient)
{
    ui->setupUi(this);
    ui->connectLabel->setText("Connected to Server. You can now chat with everyone.");
    ui->connectLabel->hide();
    ui->lineEdit->hide();
    ui->textBrowser->hide();
    ui->pushButton->hide();
    ui_user = getUi();

    connect(this, &ChatClient::dataRecv, this, &ChatClient::updateGui);
    connect(this, &ChatClient::infoMsgSignal, this, &ChatClient::showInfoMessage);
    connect(this, &ChatClient::warningMsgSignal, this, &ChatClient::showWarningMessage);


}

ChatClient::~ChatClient()
{
    delete ui;
}

void ChatClient::showInfoMessage() {
    QMessageBox::information(this, "Server closed", "Connection with the server has been shut down");
}

void ChatClient::showWarningMessage() {
    QMessageBox::warning(this, "ERROR", "Receive message failed");
}

void recv_handle() {
    while(true)
    {
    char recvbuf[512];
    ZeroMemory(recvbuf, 512);
    auto iRecv = recv(user_soc, recvbuf, 512, 0);
    if (iRecv > 0)
    {
        emit chatClient_ptr->dataRecv(QString(recvbuf));
    }
    else if (iRecv == 0) {
        emit chatClient_ptr->infoMsgSignal();
        closesocket(user_soc);
        WSACleanup();
        break;
    }
    else {
        if(logout_flag == true) {
            break;
        }
        emit chatClient_ptr->warningMsgSignal();
        closesocket(user_soc);
        WSACleanup();
        break;
        }
    }
}

void ChatClient::on_connectButton_clicked()
{
    QSqlDatabase mydb = QSqlDatabase::addDatabase("QSQLITE");
    mydb.setDatabaseName("C:/C++_practice/chat_history.db");
    if(!mydb.open()) {
        QMessageBox::critical(this, "ERROR", "Cannot access database!");
        QApplication::quit();
    }
    username = ui->lineEdit_username->text();
    QString port = ui->lineEdit_port->text();
    QString ip = ui->lineEdit_ip->text();

    if(username == "") {
        QMessageBox::warning(this, "Connect to TCP server", "Username cannot be blank!");
    }
    else if(port == ""){
        QMessageBox::warning(this, "Connect to TCP server", "Port cannot be blank!");
    }
    else if(ip == ""){
        QMessageBox::warning(this, "Connect to TCP server", "IP address cannot be blank!");
    }
    else{
        bool ok;
        port_value = port.toUShort(&ok);
        QByteArray usernameByteArray = username.toUtf8();
        username_str = usernameByteArray.constData();
        QByteArray ipByteArray = ip.toUtf8();
        ip_str = ipByteArray.constData();

        WSADATA wsa;
        if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
        {
            QMessageBox::warning(this, "Connect to TCP server", "WSAStartup failed!");
        }
        auto clientSoc = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

        user_soc = clientSoc;

        if (clientSoc == INVALID_SOCKET)
        {
            QMessageBox::warning(this, "Connect to TCP server", "Socket creation failed!");
            closesocket(clientSoc);
            WSACleanup();
        }

        // fill server address
        struct sockaddr_in TCPServerAdd;
        TCPServerAdd.sin_family = AF_INET;
        TCPServerAdd.sin_port = htons(port_value);
        TCPServerAdd.sin_addr.S_un.S_addr = inet_addr(ip_str);

        // device ipv4: 192.168.1.6; or use loopback addr: 127.0.0.1
        // 13.250.60.143

        // connect to server
        auto iConnect = WSAAPI::connect(clientSoc, (SOCKADDR*)& TCPServerAdd, sizeof(TCPServerAdd));
        auto iSend = send(clientSoc, username_str, strlen(username_str), 0);
        if (iConnect == SOCKET_ERROR)
        {
            QMessageBox::warning(this, "Connect to TCP server", "Connection failed. Wrong IP address or Port");
            closesocket(clientSoc);
            WSACleanup();
        }
        else if (iSend == SOCKET_ERROR)
        {
            QMessageBox::warning(this, "Connect to TCP server", "Connection failed. Cannot notify username");
            closesocket(clientSoc);
            WSACleanup();
        }
        else{
            ui->groupBox->hide();
            ui->connectButton->hide();
            ui->connectLabel->show();
            ui->lineEdit->show();
            ui->textBrowser->show();
            ui->pushButton->show();

            std::thread recverThread = std::thread(recv_handle);
            recverThread.detach();
        }
    }
}

void ChatClient::updateGui(QString data) {
    ui_user->textBrowser->insertPlainText(data);
    //    ui_user->textBrowser->insertPlainText("\n");
}

void ChatClient::on_lineEdit_returnPressed()
{
    QString sendUiText = ui->lineEdit->text();
    std::string dataToSend = sendUiText.toStdString();
    std::string usernameToSend = username.toStdString();
    std::string packet;
    if(sendUiText != "") {
        QDateTime date = QDateTime::currentDateTime();
        QString formattedTime = date.toString("[dd.MM.yyyy] [hh:mm:ss]");
        packet = formattedTime.toStdString() + " " + usernameToSend + ": " + dataToSend + "\n";
        if (send(user_soc, packet.c_str(), strlen(packet.c_str()), 0) == SOCKET_ERROR)
        {
            QMessageBox::warning(this, "Error", "Packet failed to send");
            closesocket(user_soc);
            WSACleanup();
        }
        else{
//            ui->textBrowser->insertPlainText(formattedTime);
//            ui->textBrowser->insertPlainText(" ");
//            ui->textBrowser->insertPlainText(username);
//            ui->textBrowser->insertPlainText(": ");
//            ui->textBrowser->insertPlainText(sendUiText);
//            ui->textBrowser->insertPlainText("\n");
            ui->lineEdit->clear();
        }
    }
}

void ChatClient::on_pushButton_clicked()
{
    QString sendUiText = ui->lineEdit->text();
    std::string dataToSend = sendUiText.toStdString();
    std::string usernameToSend = username.toStdString();
    std::string packet;
    if(sendUiText != "") {
        QDateTime date = QDateTime::currentDateTime();
        QString formattedTime = date.toString("[dd.MM.yyyy] [hh:mm:ss]");
        packet = formattedTime.toStdString() + " " + usernameToSend + ": " + dataToSend + "\n";
        if (send(user_soc, packet.c_str(), strlen(packet.c_str()), 0) == SOCKET_ERROR)
        {
            QMessageBox::warning(this, "Error", "Packet failed to send");
            closesocket(user_soc);
            WSACleanup();
        }
        else{
//            ui->textBrowser->insertPlainText(formattedTime);
//            ui->textBrowser->insertPlainText(" ");
//            ui->textBrowser->insertPlainText(username);
//            ui->textBrowser->insertPlainText(": ");
//            ui->textBrowser->insertPlainText(sendUiText);
//            ui->textBrowser->insertPlainText("\n");
            ui->lineEdit->clear();
        }
    }
}


void ChatClient::on_lineEdit_username_returnPressed()
{
    chatClient_ptr->on_connectButton_clicked();
}


void ChatClient::on_lineEdit_port_returnPressed()
{
    chatClient_ptr->on_connectButton_clicked();
}


void ChatClient::on_lineEdit_ip_returnPressed()
{
    chatClient_ptr->on_connectButton_clicked();
}

void ChatClient::on_actionLogout_triggered()
{
    logout_flag = true;
    closesocket(user_soc);
    WSACleanup();
    ui->groupBox->show();
    ui->connectButton->show();
    ui->connectLabel->hide();
    ui->lineEdit->clear();
    ui->lineEdit->hide();
    ui->textBrowser->clear();
    ui->textBrowser->hide();
    ui->pushButton->hide();
}

void ChatClient::on_actionExit_triggered()
{
    QApplication::quit();
}




