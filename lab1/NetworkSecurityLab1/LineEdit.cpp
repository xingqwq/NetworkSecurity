#include "LineEdit.h"

LineEdit::LineEdit(QWidget* parent)
    :QLineEdit(parent)
{
    this->setMaxLength(3);
    this->setAlignment(Qt::AlignCenter);
    m_next = nullptr;
}

void LineEdit::setValidCheck(){
    QValidator *validator = new QIntValidator(0, 255, this);
    this->setValidator(validator);
}

void LineEdit::keyPressEvent(QKeyEvent * event)
{
    if(event->key() == Qt::Key_Period || event->key() == Qt::Key_Space){
        if (m_next){
            m_next->setFocus();
        }
    }else{
        QLineEdit::keyPressEvent(event);
    }
}

void LineEdit::focusInEvent(QFocusEvent *event)
{
    this->setFocus();
    QLineEdit::focusInEvent(event);
}

