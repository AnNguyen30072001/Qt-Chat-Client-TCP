#include "chatclient.h"

#include <QApplication>
#include <QThread>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    ChatClient chatClient;
    chatClient.setChatClientPointer(&chatClient);
    chatClient.show();
    return a.exec();
}
