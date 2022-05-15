//
// Created by lisongqian on 2022/5/9.
//

#include "MainWindow.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);
}

void MainWindow::show_myself() {
    this->show();
}

void MainWindow::add_connection(LoginDialog *dialog) const {
    connect(dialog, &LoginDialog::open_main_window, this, &MainWindow::show_myself);
}
