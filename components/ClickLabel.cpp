//
// Created by lisongqian on 2022/5/20.
//

#include <QFile>
#include <QStyleOption>
#include <QPainter>
#include "ClickLabel.h"
#include "log/log.h"


ClickLabel::ClickLabel(int index, QWidget *parent) : QLabel(parent), m_index(index), m_dir_id(0) {
    init();
}

ClickLabel::ClickLabel(const QString &text, int index, QWidget *parent, Qt::WindowFlags f) : QLabel(text, parent, f),
                                                                                             m_index(index),
                                                                                             m_dir_id(0) {
    init();
}

void ClickLabel::init() {
    setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    if (QFile qss(":/components.qss");qss.open(QFile::ReadOnly)) {
        this->setStyleSheet(qss.readAll());
        qss.close();
    }
    setProperty("level", "normal");
}

void ClickLabel::mousePressEvent(QMouseEvent *event) {
    QLabel::mouseReleaseEvent(event);
    emit clicked(m_index);
}


