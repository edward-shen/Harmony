#pragma once
#include <qapplication.h>

class QCustomData {
public:
    QCustomData() : m_ptr(nullptr) {}
    QCustomData(const QCustomData &other) : m_ptr(other.m_ptr) {};
    ~QCustomData() {};

    QCustomData(void* ptr) : m_ptr(ptr) {};

    void* ptr() const {
        return m_ptr;
    }
private:
    void* m_ptr;
};

Q_DECLARE_METATYPE(QCustomData);