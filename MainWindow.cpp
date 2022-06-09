//
// Created by lisongqian on 2022/5/9.
//

#include <QFile>
#include "MainWindow.h"
#include "log/log.h"
#include "request/HTTPRequest.h"
#include "config.h"

extern Config g_config;

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(std::make_shared<Ui::MainWindow>()) {
    ui->setupUi(this);
    if (QFile qss(":/main.qss");qss.open(QFile::ReadOnly)) {
        this->setStyleSheet(qss.readAll());
        qss.close();
    }
    ui->statusbar->hide();
    ui->leftWidget->setAttribute(Qt::WidgetAttribute::WA_StyledBackground);
    setWindowFlags(windowFlags() & ~Qt::WindowMaximizeButtonHint);
    setFixedSize(this->width(), this->height());
    int icon_size = 25;
    ui->listWidget->setIconSize(QSize(icon_size, icon_size));
    ui->listWidget->setCurrentRow(0);
    connect(ui->listWidget, &QListWidget::currentRowChanged, this, &MainWindow::changeTab);

    auto label = std::make_shared<ClickLabel>(tr("文件"), 0, ui->navigation);
    label->adjustSize();
    label->setGeometry(QRect(0, 0, label->width() + 8, ui->navigation->height()));
    connect(label.get(), &ClickLabel::clicked, this, &MainWindow::navigationClick);
    label->setCursor(QCursor(Qt::PointingHandCursor));
    label->setProperty("level", "last");
    label->style()->polish(label.get());
    m_navigation.push_back(std::move(label));
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
    m_navigation[index * 2]->setProperty("level", "last");
    m_navigation[index * 2]->style()->polish(m_navigation[index * 2].get());
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

void MainWindow::addNavigation(std::string_view name) {
    QPoint point = m_navigation[m_navigation.size() - 1]->pos();
    auto tag = std::make_shared<ClickLabel>(-1, ui->navigation);
    QIcon icon = QIcon(":/images/caret-right.svg");
    QPixmap m_pic = icon.pixmap(icon.actualSize(QSize(10, 10)));//size自行调整
    tag->setPixmap(m_pic);
    tag->adjustSize();
    tag->setGeometry(QRect(point.x() + m_navigation[m_navigation.size() - 1]->width(), 0, tag->width(),
                           ui->navigation->height()));
    auto label = std::make_shared<ClickLabel>(tr(name.data()), (m_navigation.size() + 1) / 2, ui->navigation);
    label->adjustSize();
    label->setGeometry(
            QRect(point.x() + m_navigation[m_navigation.size() - 1]->width() + tag->width(), 0, label->width() + 8,
                  ui->navigation->height()));
    connect(label.get(), &ClickLabel::clicked, this, &MainWindow::navigationClick);
    label->setCursor(QCursor(Qt::PointingHandCursor));

    m_navigation[m_navigation.size() - 1]->setProperty("level", "normal");
    m_navigation[m_navigation.size() - 1]->style()->polish(m_navigation[m_navigation.size() - 1].get());
    label->setProperty("level", "last");
    label->style()->polish(label.get());
    m_navigation.push_back(std::move(tag));
    m_navigation.push_back(std::move(label));

}

void MainWindow::updateFileList() {
    string res;
    map<string, string> data;
    data["dir"] = "0";
    auto req = std::make_shared<HTTPRequest>(g_config.ip.data(), g_config.port);
    req->init();
    bool flag = req->post("/filelist", data, res);
    req->close_socket();
}
