//
// Created by lisongqian on 2022/5/13.
//

//#include <QAbstractItemView>
#include <QMessageBox>
#include <fstream>
#include <memory>
#include "LoginDialog.h"
#include "request/HTTPRequest.h"
#include "rapidjson/document.h"
#include "config.h"
#include "log/log.h"

using namespace rapidjson;
using std::string;
using std::map;


LoginDialog::LoginDialog(QWidget *parent) : QDialog(parent), ui(std::make_shared<Ui::LoginDialog>()) {
    ui->setupUi(this);
    connect(ui->loginButton, &QPushButton::clicked, this, &LoginDialog::slot_login);
}

void LoginDialog::slot_login() {
    string username = ui->userName->text().toStdString();
    string password = ui->password->text().toStdString();
    if (!username.empty() && !password.empty()) {
        map<string, string> data;
        string res;
        data["username"] = username;
        data["password"] = password;
        bool flag = login(username, password, true);
        if (flag) {
            emit open_main_window();
            this->close();
        }
    }
    else {
        QMessageBox::information(this, "错误", "请输入用户名和密码");
    }
}

bool LoginDialog::login(const string &username, const string &password, bool messagebox) {
    map<string, string> data;
    string res;
    data["username"] = username;
    data["password"] = password;
//    auto req = std::make_shared<HTTPRequest>("127.0.0.1", 23450);
    auto req = std::make_shared<HTTPRequest>("192.168.229.129", 23450);
    req->init();
    bool flag = req->post("/login", data, res);
    req->close_socket();
    if (flag) {
        try {
            Document document;
            document.Parse(res.c_str());
            assert(document.IsObject());
            assert(document.HasMember("status"));
            assert(document.HasMember("code"));
            if (document["status"].GetInt() == 200) {
                if (document["code"].GetInt() == 1) {
                    extern Config g_config;
                    std::ofstream out(g_config.login_cache_path);
                    if (out.is_open()) {
                        out << username << std::endl;
                        out << password << std::endl;
                        out.close();
                        g_config.token = document["token"].GetString();
                        return true;
                    }
                }
                if (messagebox) {
                    if (document.HasMember("msg")) {
                        QMessageBox::information(this, "错误", document["msg"].GetString());

                    }
                    else {
                        QMessageBox::information(this, "错误", "登录仅本次有效");
                    }
                }
            }
            else {
                QMessageBox::information(this, "错误", "网络错误");
            }
        }
        catch (std::exception &e) {
            LOG_ERROR("exception:%s", e.what())
        }
    }
    else {
        if (messagebox)
            QMessageBox::information(this, "错误", "服务器或网络错误，请重试");
    }
    return false;
}