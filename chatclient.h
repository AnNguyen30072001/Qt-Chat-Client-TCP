#ifndef CHATCLIENT_H
#define CHATCLIENT_H

#include <QMainWindow>
#include <QtSql>
#include <QDebug>
#include <QFileInfo>
#include "chatclient.h"
#include <thread>
#include <string>
#include <WinSock2.h>
#include <stdint.h>
#include <QThread>


QT_BEGIN_NAMESPACE
namespace Ui { class ChatClient; }
QT_END_NAMESPACE

class ChatClient : public QMainWindow
{
    Q_OBJECT

public:
    ChatClient(QWidget *parent = nullptr);
    ~ChatClient();
    Ui::ChatClient* getUi();
    void setChatClientPointer(ChatClient* client);

signals:
    void dataRecv(QString data);
    void infoMsgSignal();
    void warningMsgSignal();

public slots:
    void updateGui(QString data);
    void showInfoMessage();
    void showWarningMessage();

private slots:
    void on_connectButton_clicked();

    void on_lineEdit_returnPressed();

    void on_pushButton_clicked();

    void on_lineEdit_username_returnPressed();

    void on_lineEdit_port_returnPressed();

    void on_lineEdit_ip_returnPressed();

    void on_actionExit_triggered();

    void on_actionLogout_triggered();

private:
    Ui::ChatClient *ui;
};

void recv_handle();


#endif // CHATCLIENT_H
