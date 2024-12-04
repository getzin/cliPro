#include "dynbutton.h"

#include <QKeyEvent>

dynButton::~dynButton() {
    ; //nothing, we have no allocations/pointers to memory
    qDebug() << "~dynButton --- dtor";
}

dynButton::dynButton(QWidget * const parent)
    : QPushButton(parent)
{
    qDebug() << "dynButton --- ctor";
    this->init();
}

void dynButton::init(){
    // QFont font(QString("Noto Sans"), 30, 5, false);
    QFont font("Monospace");
    font.setStyleHint(QFont::TypeWriter, QFont::PreferBitmap);
    font.setWeight(QFont::Bold);
    font.setPointSize(30);
    font.setBold(true);

    this->setFont(font);
    this->setSizePolicy(QSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored));
    this->setMode(btnModeDISABLED);
}

void dynButton::setStyleSheetAdd(){
    this->setStyleSheet("dynButton { color: green; border: 1px solid green; border-radius: 10%;"
                        "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #ecf5eb, stop:1 #e0f5df) }"
                        "dynButton:focus { color: green; border: 2px solid green; border-radius: 10%; outline: none;"
                        "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #dee9dd, stop:1 #d0e4cf) }");
}

void dynButton::setStyleSheetRm(){
    this->setStyleSheet("dynButton { color: darkred; border: 1px solid red; border-radius: 10%;"
                        "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #ffb9a9, stop:1 #ff9d88) }"
                        "dynButton:focus { color: darkred; border: 2px solid red; border-radius: 10%; outline: none;"
                        "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #ffa490, stop:1 #ff8166) }");
}

void dynButton::setStyleSheetDisabled(){
    this->setStyleSheet("dynButton { color: black; border: 1px solid darkgray; border-radius: 10%;"
                        "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #e2e2e2, stop:1 #cbcbcb) }"
                        "dynButton:focus { color: black; border: 2px solid darkgray; border-radius: 10%; outline: none;"
                        "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #e2e2e2, stop:1 #cbcbcb) }");
}

//slot
void dynButton::setMode(btnMode const mode){
    qDebug() << "start: setMode (mode: " << mode << ")";
    if(mode == btnModeADD){
        this->setText("+");
        this->setStyleSheetAdd();
        this->currBtnMode = btnModeADD;
    }else if(mode == btnModeRM){
        this->setText("-");
        this->setStyleSheetRm();
        this->currBtnMode = btnModeRM;
    }else if(mode == btnModeDISABLED){
        this->setText("?");
        this->setStyleSheetDisabled();
        this->currBtnMode = btnModeDISABLED;
    }else{
        qDebug() << "invalid mode.";
    }
    emit this->mainWindowButtonsNeedSwitch(this->currBtnMode);
}

dynButton::btnMode dynButton::getCurrBtnMode() const{
    return this->currBtnMode;
}

void dynButton::keyPressEvent(QKeyEvent *const event){
    emit this->keyPressOnDynBtn(event->key());
}
