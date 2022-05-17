//
// Created by lisongqian on 2022/5/9.
//

#ifndef NETDISK_MAINWINDOW_H
#define NETDISK_MAINWINDOW_H

#include <QMainWindow>
#include "ui_mainWindow.h"
#include "LoginDialog.h"


class MainWindow : public QMainWindow {
Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    void add_connection(LoginDialog *dialog) const;
    ~MainWindow();
public slots:
    void show_myself();
private:
    std::shared_ptr<Ui::MainWindow> ui;
};


#endif //NETDISK_MAINWINDOW_H
