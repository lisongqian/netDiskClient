//
// Created by lisongqian on 2022/5/9.
//

#ifndef NETDISK_MAINWINDOW_H
#define NETDISK_MAINWINDOW_H

#include <QMainWindow>
#include "ui_mainWindow.h"


class MainWindow : public QMainWindow {
Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override = default;

private:
    Ui::MainWindow *ui;
};


#endif //NETDISK_MAINWINDOW_H
