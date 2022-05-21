//
// Created by lisongqian on 2022/5/20.
//

#include <QFile>
#include "ClickLabel.h"
#include "log/log.h"


ClickLabel::ClickLabel(int index, QWidget *parent) : QLabel(parent), m_index(index) {
    init();
}

ClickLabel::ClickLabel(const QString &text, int index, QWidget *parent, Qt::WindowFlags f) : QLabel(text, parent, f),
                                                                                             m_index(index) {
    init();
}

void ClickLabel::init() {
    if (QFile qss(":/components.qss");qss.open(QFile::ReadOnly)) {
        this->setStyleSheet(qss.readAll());
        qss.close();
    }
}

void ClickLabel::mousePressEvent(QMouseEvent *event) {
    QLabel::mouseReleaseEvent(event);
    emit clicked(m_index);
}

