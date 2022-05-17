//
// Created by lisongqian on 2022/5/13.
//

#ifndef NETDISK_LOGINDIALOG_H
#define NETDISK_LOGINDIALOG_H


#include <QDialog>
#include "ui_login.h"

using std::string;


class LoginDialog : public QDialog {
Q_OBJECT
public:
    explicit LoginDialog(QWidget *parent = nullptr);
    bool login(const string& username,const string& password,bool messagebox = false);
    ~LoginDialog() = default;
signals:
    void open_main_window();

private:
    Ui::LoginDialog *ui;
private slots:
    void slot_login();
};


#endif //NETDISK_LOGINDIALOG_H
