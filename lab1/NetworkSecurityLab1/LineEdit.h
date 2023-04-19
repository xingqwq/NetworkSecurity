#ifndef LINEEDIT_H
#define LINEEDIT_H

#include <iostream>
#include <QMainWindow>
#include <QLineEdit>
#include <QKeyEvent>
#include <QWidget>
#include <QDebug>
#include <QIntValidator>

class LineEdit:public QLineEdit{
    Q_OBJECT;
public:
    explicit LineEdit(QWidget* parent = 0);
    void setValidCheck();
    void keyPressEvent(QKeyEvent * event) override;
    void focusInEvent(QFocusEvent *event) override;
    void setNext(LineEdit* next){
        this->m_next = next;
    }

private:
    QLineEdit* m_next;
};

#endif // LINEEDIT_H
