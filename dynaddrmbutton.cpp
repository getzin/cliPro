#include "dynaddrmbutton.h"

#include <QKeyEvent>


dynAddRmButton::dynAddRmButton(btnMode mode) {
    // QFont font(QString("Noto Sans"), 30, 5, false);
    QFont font("Monospace");
    font.setStyleHint(QFont::TypeWriter, QFont::PreferBitmap);
    font.setWeight(QFont::Bold);
    font.setPointSize(30);

    font.setBold(true);
    this->setFont(font);
    this->setSizePolicy(QSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored));
    this->setMode(mode);
}

dynAddRmButton::~dynAddRmButton() {
    qDebug() << "    ~dynAddRmButton --- DTOR!!!";
}

void dynAddRmButton::setStyleSheetAdd(){
    qDebug() << "  & setStyleSheetAdd";

    this->setStyleSheet("dynAddRmButton { color: green; border: 1px solid green; border-radius: 10%;"
                        "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #ecf5eb, stop:1 #e0f5df) }");
}

void dynAddRmButton::setStyleSheetRm(){
    qDebug() << "  & setStyleSheetRm";
    // this->setPalette(QPalette(QColor(200,200,200)));
    this->setStyleSheet("dynAddRmButton { color: darkred; border: 2px solid red; border-radius: 10%;"
                        "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #ffa490, stop:1 #ff8166) }");
}


void dynAddRmButton::setMode(dynAddRmButton::btnMode mode){
    qDebug() << "setMode (mode: " << mode << ")";
    if(mode == btnModeADD){
        this->setText("+");
        this->setStyleSheetAdd();
        currBtnMode = btnModeADD;
    }else{
        this->setText("-");
        this->setStyleSheetRm();
        currBtnMode = btnModeRM;
    }

    qDebug() << "      !! EMIT for mainWindow !!";
    emit mainWindowButtonsNeedSwitch(mode);

    qDebug() << "/setMode";
}

// void dynAddRmButton::setAddMode(){
//     qDebug() << "    setAddMode (start)";
//     this->setMode(btnModeADD);
//     qDebug() << "    setAddMode (end)";
// }
// void dynAddRmButton::setRmMode(){
//     qDebug() << "    setRmMode (start)";
//     this->setMode(btnModeRM);
//     qDebug() << "    setRmMode (end)";
// }

void dynAddRmButton::switchMode(){
    qDebug() << "    Switch mode (start)";
    this->setMode(currBtnMode == btnModeADD ? btnModeRM : btnModeADD);
    qDebug() << "    Switch mode (end)";
}

dynAddRmButton::btnMode dynAddRmButton::getCurrBtnMode()
{
    return currBtnMode;
}

void dynAddRmButton::keyPressEvent(QKeyEvent *event){
    qDebug() << "Key press event! (dynAddRmButton)";
    int key = event->key();
    if(key == Qt::Key_Return || key == Qt::Key_Enter){
        qDebug() << "(dynAddRmButton) Enter pressed.";
        // if(currBtnMode == btnModeADD){
        //     emit dynBtnEnterKey();
        // }else if(currBtnMode == btnModeADD){
        //     emit dynBtnEnterKey();
        // }

        emit dynBtnEnterKey();
    }
    qDebug() << "/ Key press event! (dynAddRmButton)";
}
