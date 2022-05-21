//
// Created by lisongqian on 2022/5/9.
//

#include <QFile>
#include "MainWindow.h"
#include "log/log.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(std::make_shared<Ui::MainWindow>()) {
    ui->setupUi(this);
    if (QFile qss(":/main.qss");qss.open(QFile::ReadOnly)) {
        this->setStyleSheet(qss.readAll());
        qss.close();
    }
    ui->listWidget->setCurrentRow(0);
    connect(ui->listWidget, &QListWidget::currentRowChanged, this, &MainWindow::changeTab);

    int x = 0;
    for (int i = 0; i < 3; i++) {
        if (i) {
            auto tag = std::make_shared<ClickLabel>(-1, ui->navigation);
            QIcon icon = QIcon(":/images/caret-right.svg");
            QPixmap m_pic = icon.pixmap(icon.actualSize(QSize(10, 10)));//size自行调整
            tag->setPixmap(m_pic);
            tag->adjustSize();
            tag->setGeometry(QRect(x, 0, tag->width(), ui->navigation->height()));
            x += tag->width();
            m_navigation.push_back(std::move(tag));
        }
        auto label = std::make_shared<ClickLabel>(tr(" 文件 "), i, ui->navigation);
        label->adjustSize();
        label->setGeometry(QRect(x, 0, label->width(), ui->navigation->height()));
        x += label->width();
        connect(label.get(), &ClickLabel::clicked, this, &MainWindow::navigationClick);
        m_navigation.push_back(std::move(label));
    }
    m_navigation[m_navigation.size() - 1]->setStyleSheet("color: #000000;font-size: 20px;");
    showFileNavigation();
}

/**
 * 显示窗口
 */
void MainWindow::show_myself() {
    this->show();
}

/**
 * 连接信号，用于触发内部函数
 * @param dialog
 */
void MainWindow::addConnection(LoginDialog *dialog) const {
    connect(dialog, &LoginDialog::open_main_window, this, &MainWindow::show_myself);
}

/**
 * 切换标签页
 * @param currentRow
 */
void MainWindow::changeTab(int currentRow) {
    if (currentRow >= 0) {
        LOG_INFO("changeTab:%d", currentRow);
        ui->tabWidget->setCurrentIndex(currentRow);
    }
}

/**
 * 点击导航栏
 * @param index
 */
void MainWindow::navigationClick(int index) {
    LOG_INFO("click index:%d", index)
    /**
     * index 为负数为 ">"，点击无效果
     */
    if (index < 0) return;
    while (m_navigation.size() > index * 2 + 1) {
        m_navigation.pop_back();
    }
    m_navigation[index * 2]->setStyleSheet("color: #000000;font-size: 20px;");
}

/**
 * 显示文件导航栏
 */
void MainWindow::showFileNavigation(bool isShow) {
    if (isShow) {
        for (const auto &item: m_navigation) {
            item->show();
        }
    }
    else {
        for (const auto &item: m_navigation) {
            item->hide();
        }
    }
}
