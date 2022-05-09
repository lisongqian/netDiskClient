#include <iostream>
#include <map>
#include <QApplication>
#include <QMainWindow>
#include <QTextCodec>

#ifdef UNIX
#include <pthread.h>
#else

#include <WinSock2.h>

#pragma comment(lib, "ws2_32.lib")  //加载 ws2_32.dll
#endif

#include "log/log.h"
#include "common.h"
#include "config.h"
#include "MainWindow.h"

using std::string;
using std::cout;
using std::map;
using std::endl;

int main(int argc, char **argv) {
    // 1. 接收参数
    Config config;
    config.ParseArg(argc, argv);
    Log::instance()->Init(config.buff_size);
    LOG_INFO("HELLO WINDOWS")
    map<string, string> m;
    m["data"] = "123";
    LOG_INFO(Map2String(m).c_str())
    QTextCodec::setCodecForLocale( QTextCodec :: codecForName("UTF-8"));
    QApplication app(argc,argv);
    MainWindow mainWindow;
    mainWindow.show();
    QApplication::exec(); //应用程序运行
    return 0;
}
