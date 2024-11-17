#include "dynaddrmbutton.h"

#include <QKeyEvent>

dynAddRmButton::dynAddRmButton(QWidget *parent)
    : QPushButton(parent)
{
    qDebug() << "dynAddRmButton --- ctor";
    // QFont font(QString("Noto Sans"), 30, 5, false);
    QFont font("Monospace");
    font.setStyleHint(QFont::TypeWriter, QFont::PreferBitmap);
    font.setWeight(QFont::Bold);
    font.setPointSize(30);
    font.setBold(true);

    this->setFont(font);
    this->setSizePolicy(QSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored));
    this->setMode(btnModeADD);
}

dynAddRmButton::~dynAddRmButton() {
    ; //nothing, we have no allocations/pointers to memory
    qDebug() << "~dynAddRmButton --- dtor";
}

void dynAddRmButton::setStyleSheetAdd(){
    qDebug() << "start: setStyleSheetAdd";

    this->setStyleSheet("dynAddRmButton { color: green; border: 1px solid green; border-radius: 10%;"
                        "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #ecf5eb, stop:1 #e0f5df) }"
                        "dynAddRmButton:focus { color: green; border: 2px solid green; border-radius: 10%; outline: none;"
                        "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #dee9dd, stop:1 #d0e4cf) }");
}

void dynAddRmButton::setStyleSheetRm(){
    qDebug() << "start: setStyleSheetRm";
    // this->setPalette(QPalette(QColor(200,200,200)));
    this->setStyleSheet("dynAddRmButton { color: darkred; border: 1px solid red; border-radius: 10%;"
                        "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #ffb9a9, stop:1 #ff9d88) }"
                        "dynAddRmButton:focus { color: darkred; border: 2px solid red; border-radius: 10%; outline: none;"
                        "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #ffa490, stop:1 #ff8166) }");
}


void dynAddRmButton::setMode(dynAddRmButton::btnMode mode){
    qDebug() << "start: setMode (mode: " << mode << ")";
    if(mode == btnModeADD){
        this->setText("+");
        this->setStyleSheetAdd();
        this->currBtnMode = btnModeADD;
    }else{
        this->setText("-");
        this->setStyleSheetRm();
        this->currBtnMode = btnModeRM;
    }
    qDebug() << "emit: mainWindowButtonsNeedSwitch";
    emit this->mainWindowButtonsNeedSwitch(mode);
    qDebug() << "end: setMode";
}

void dynAddRmButton::switchMode(){
    qDebug() << "start: switch mode";
    this->setMode(currBtnMode == btnModeADD ? btnModeRM : btnModeADD);
    qDebug() << "end: switch mode";
}

dynAddRmButton::btnMode dynAddRmButton::getCurrBtnMode() const{
    return this->currBtnMode;
}

void dynAddRmButton::keyPressEvent(QKeyEvent *event){
    qDebug() << "start: Key press event! (dynAddRmButton)";
    emit this->keyPressOnDynBtn(event->key());
    qDebug() << "end: Key press event! (dynAddRmButton)";
}
