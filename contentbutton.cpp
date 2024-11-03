#include "contentbutton.h"

#include <QSizePolicy>
#include <QDebug>
#include <QMouseEvent>
#include <QClipboard>
#include <QMessageBox>



const QString contentButton::textForTitleAddition = "Add title";
const QString contentButton::textForTitleEditing = "Edit title";
const QString contentButton::textForMarkDeletion = "Mark for deletion";
const QString contentButton::textForUnmarkDeletion = "Unmark from deletion";

int contentBtnCount::totalContentBtnCount = 0;
int contentBtnCount::markedContentBtnCount = 0;
contentButton* contentButton::focusedButton = nullptr;

// contentButton::contentButton(dynAddRmButton *dynBtnPtr){
contentButton::contentButton(QWidget *parent)
    : QPushButton(parent),
      adjustTitleAction(textForTitleAddition, this),
      markForDeleteAction(textForMarkDeletion, this)
{
    this->setSizePolicy(QSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored));
    this->setStyleDefault();
    // this->setFocusPolicy(Qt::ClickFocus); //disables tab focus
    this->setFocusPolicy(Qt::StrongFocus); //tab+click focus
    this->setAttribute(Qt::WA_MacShowFocusRect);
    // this->setStyleSheet("contentButton:focus { border: none; outline: none; }");
    // this->setStyleSheetWrap("contentButton:focus { color: black; border: 1px solid grey; border-radius: 10%; outline: none;"
    //                         "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #fafacd, stop:1 #f7f7ba) }");
    // dynBtn = dynBtnPtr;
    this->indexInGrid = getTotalCnt() - 1; //-1 here, as the number is already increased (see ctor of contentbtncount.h)

    //---MENU---
    optionsMenu.addAction(&adjustTitleAction);
    optionsMenu.addSeparator();
    optionsMenu.addAction(&markForDeleteAction);
    connect(&adjustTitleAction, SIGNAL(triggered()), this, SLOT(handleTitleAdjust()));
    connect(&markForDeleteAction, SIGNAL(triggered()), this, SLOT(markForDeletion()));
}


contentButton::~contentButton(){
    if(this->isMarked()){
        this->decrMarkedCnt();
        if(contentButton::getMarkedCnt() == 0){
            emit dynBtnSetMode(dynAddRmButton::btnModeADD);
        }
    }
    this->unsetAsFocusedButton(); //func already includes check on whether even "is focused" or not
}

void contentButton::setStyleDefault(){
    qDebug() << "  style (default)";
    this->setStyleSheet("contentButton { color: black; border: 1px solid silver; border-radius: 10%; "
                            "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #F0F0F0, stop:1 #eaeaea) }");
}

void contentButton::setStyleMarked(){
    qDebug() << "  style (marked)";
    // this->setStyleSheetWrap("background-color: #FFB0B0; outline: 1px dashed #ff0000; outline-offset: 0px; border: 1px solid silver; border-radius: 10%;");
    this->setStyleSheet("contentButton { color: black; border: 1px solid lightcoral; border-radius: 10%;"
                            "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #ffe8e3, stop:1 #fad4cd) }");
}

void contentButton::setStyleFocused(){
    qDebug() << "  style (focus)";
    this->setStyleSheet("contentButton { color: black; border: 1px solid grey; border-radius: 10%;"
                            "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #fbfbd5, stop:1 #fdfde2) }"
                            "contentButton:focus { color: black; border: 2px solid grey; border-radius: 10%; outline: none;"
                            "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #fafacd, stop:1 #f7f7ba) }");
}

void contentButton::setStyleMarkedAndFocus(){
    qDebug() << "  style (marked & focus)";
    this->setStyleSheet("contentButton { color: black; border: 1px solid lightcoral; border-radius: 10%;"
                            "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #ffe8e3, stop:1 #fad4cd) }"
                            "contentButton:focus { color: black; border: 2px solid lightcoral; border-radius: 10%; outline: none;"
                            "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #ffdabd, stop:1 #ffcabd) }");
}

bool contentButton::isMarked(){
    return this->marked;
}

bool contentButton::notMarked(){
    return !(this->isMarked());
}

void contentButton::setMarked(){
    if(this->notMarked()){
        this->incrMarkedCnt();
        if(this->isFocused()){
            this->setStyleMarkedAndFocus();
        }else{
            this->setStyleMarked();
        }
        this->marked = true;
        this->markForDeleteAction.setText(this->textForUnmarkDeletion);
    }
}

void contentButton::unsetMarked(){
    if(this->isMarked()){
        this->decrMarkedCnt();
        if(this->isFocused()){
            this->setStyleFocused();
        }else{
            this->setStyleDefault();
        }
        this->marked = false;
        this->markForDeleteAction.setText(this->textForMarkDeletion);
    }
}

void contentButton::switchMarking(){
    if(this->marked){
        this->unsetMarked();
    }else{
        this->setMarked();
    }
    this->checkForDynBtnSwitch();
}

void contentButton::checkForDynBtnSwitch(){
    /*   * case 1: button has been demarked and
         *           we now have 0 marked buttons
         *           --> switch state of dynFuncBtn to ADD
         * case 2: button is now marked (was not
         *           before) and our count now is 1
         *           --> switch state of dynFuncBtn to RM */
    int markedBtnCount = contentButton::getMarkedCnt();
    if(markedBtnCount == 0 && this->notMarked()){
        qDebug() << "MODE == ADD?!?";
        emit dynBtnSetMode(dynAddRmButton::btnModeADD);
    }else if(markedBtnCount == 1 && this->isMarked()){
        qDebug() << "MODE == RM?!?";
        emit dynBtnSetMode(dynAddRmButton::btnModeRM);
    }
}

bool contentButton::isFocused(){
    return (contentButton::focusedButton == this);
}

void contentButton::setAsFocusedButton(){
    if(this->notFocused()){
        this->setFocus();
        if(this->isMarked()){
            this->setStyleMarkedAndFocus();
        }else{
            this->setStyleFocused();
        }
        contentButton::focusedButton = this;
    }
}

void contentButton::unsetAsFocusedButton(){
    if(this->isFocused()){
        this->clearFocus();
        if(this->isMarked()){
            this->setStyleMarked();
        }else{
            this->setStyleDefault();
        }
        contentButton::focusedButton = nullptr;
    }
}

bool contentButton::notFocused(){
    return !(this->isFocused());
}

//static
void contentButton::clearFocusedButton(){
    if(contentButton::isAnyButtonInFocus()){
        contentButton::focusedButton->unsetAsFocusedButton();
    }
}

//static
bool contentButton::isAnyButtonInFocus(){
    return (contentButton::focusedButton != nullptr);
}

void contentButton::keyPressEvent(QKeyEvent *event){
    qDebug() << "==================== Key press event! (contentButton)";

    int key = event->key();

    if(key == Qt::Key_Return || key == Qt::Key_Enter){
        qDebug() << "Enter pressed.";

        Qt::KeyboardModifiers mod = event->modifiers();
        if(mod == Qt::CTRL || mod == Qt::SHIFT){
            this->openMenu(this->mapToGlobal(this->rect().center()));
        }else{
            emit startContentButtonEdit(this->indexInGrid);
        }
    }

    if(key == Qt::Key_C){
        QGuiApplication::clipboard()->setText(this->text());
    }else if(key == Qt::Key_V){

        bool save = true;
        if(this->text().length() > 0){

            qDebug() << "There already is text!";
            QMessageBox::StandardButton reply;

            reply = QMessageBox::question(this, "Override Confirmation", "Do you really want to override the existing content for this button?",
                                          QMessageBox::Reset|QMessageBox::No|QMessageBox::Yes, QMessageBox::No);

            if(reply == QMessageBox::Yes){
                qDebug() << "Yes clicked";
                // this->removeSelectedButton(indexOfSender);
            }else if(reply == QMessageBox::No){
                qDebug() << "No clicked";
                save = false;
            }
        }

        if(save){
            this->setText(QGuiApplication::clipboard()->text());
        }

    }else if(key == Qt::Key_Left || key == Qt::Key_Right
               || key == Qt::Key_Up || key == Qt::Key_Down
               || key == Qt::Key_Delete || key == Qt::Key_Backspace
               || key == Qt::Key_Escape
               || key == Qt::Key_Plus || key == Qt::Key_Minus){
        emit keyWasPressed(key, this->indexInGrid);
    }else if(key == Qt::Key_Tab){
        qDebug() << "TAAAAAAAAAAAAAAAAAAB"; //doesn't work?
    }
    qDebug() << "==================== / Key press event! (contentButton)";
}

void contentButton::mouseLeftClick(){
    qDebug() << "Left Button!";

    // QKeyEvent keyPressed(QKeyEvent::KeyPress, Qt::Key_Tab, Qt::NoModifier);
    // this->keyPressEvent(&keyPressed);

    // this->setFocus();
    // this->focus

    QFocusEvent* focus = new QFocusEvent(QEvent::MouseButtonPress, Qt::MouseFocusReason);
    this->focusInEvent(focus);
    delete focus;
}



void contentButton::handleTitleAdjust(){
    qDebug() << "handleTitleAdjust";
    //ToDo
}

void contentButton::markForDeletion(){
    qDebug() << "markForDeletion";
    this->switchMarking();
}

void contentButton::openMenu(QPoint p){

    qDebug() << "openMenu (x: "  << p.x() << ", y: " << p.y() << ")";

    // if(!this->rightClickMenu.isVisible()){
        // ToDo open Menu..
        optionsMenu.popup(p);
        // rightClickMenu.setFocus();
    // }
}


void contentButton::mouseRightClick(QMouseEvent *event){
    qDebug() << "start: mouseRightClick";

    this->openMenu(event->globalPosition().toPoint());

    qDebug() << "end: mouseRightClick";
}

// void contentButton::mousePressEvent(QMouseEvent *event){
//     qDebug() << "mousePressEvent";
//     this->setFocus();
//     ; //do nothing (this override serves the purpose of blocking the style change that normally happens)
//     qDebug() << "/mousePressEvent";
// }

void contentButton::mousePressEvent(QMouseEvent *event){
// void contentButton::mouseReleaseEvent(QMouseEvent *event){
    qDebug() << "mousePressEvent";
    // this->setFocus();
    // this->setFocus(Qt::TabFocusReason);
    this->setAsFocusedButton();

    // QFocusEvent* focus = new QFocusEvent(QEvent::MouseButtonPress, Qt::MouseFocusReason);
    // this->focusInEvent(focus);
    // delete focus;

    if(event->button() == Qt::LeftButton){
        this->mouseLeftClick();
    }else if(event->button() == Qt::RightButton){
        this->mouseRightClick(event);
    }
    qDebug() << "/mousePressEvent";
}

// void contentButton::obtainFocus(){
//     this->setFocus();
// }


//ToDo currently does not seem to always trigger for right clicks (something to do with the focus logic, maybe race condition)
void contentButton::mouseDoubleClickEvent(QMouseEvent *event){
    //https://www.youtube.com/watch?v=Yg1FBrbfwNM
    qDebug() << "Double click!!!!!!!!!!!!!!!!!!!!!!!!!!!!";

    //ToDo open edit
    // contentEdit *contEdit = new contentEdit();

    // contentEdit *contEdit = new contentEdit(this, this->text());
    // contEdit->setModal(true);

    // contEdit->setWindowFlags(Qt::CustomizeWindowHint);
    // contEdit->setWindowFlags(Qt::CustomizeWindowHint);
    // contEdit->setWindowFlags(Qt::FramelessWindowHint);
    // contEdit->setWindowFlags(Qt::CustomizeWindowHint);
    // contEdit->setWindowFlags(Qt::WindowTitleHint);
    // contEdit->
    // contEdit->setStyleSheet("background-color: white; border: 1px solid silver; border-radius: 10%;");

    // contEdit->setStyle(QPushButton::style());

    // contEdit->getCancelBtn()->setStyle(QPushButton::style());
    // contEdit->getSaveBtn()->setStyle(QPushButton::style());



    // contentEdit *contEdit = new contentEdit(this, this->text());
    // contEdit->setModal(true);
    // contEdit->show();


    // delete contEdit;

    // contEdit->setParent(this);

    if(event->button() == Qt::LeftButton){
        // this->setAsSelectedButton();
        emit startContentButtonEdit(this->indexInGrid);
    }else if(event->button() == Qt::RightButton){
        this->mouseRightClick(event);
    }
}


void contentButton::focusOutEvent(QFocusEvent *event){

    qDebug() << "  focusOutEvent ---- this->getMarkedCnt(): " << this->getMarkedCnt();

    QWidget::focusOutEvent(event);
    qDebug() << "focusOutEvent ; index: " << this->indexInGrid;
    qDebug() << ">Reason: " << event->reason();

    Qt::FocusReason r = event->reason();
    if(r == Qt::MouseFocusReason || r == Qt::TabFocusReason
        || r == Qt::BacktabFocusReason || r == Qt::OtherFocusReason){
        this->unsetAsFocusedButton();
    }
}

void contentButton::focusInEvent(QFocusEvent *event){

    qDebug() << "  focusInEvent ---- this->getMarkedCnt(): " << this->getMarkedCnt();

    QWidget::focusInEvent(event);
    qDebug() << "focusInEvent ; index: " << this->indexInGrid;
    qDebug() << ">Reason: " << event->reason();

    Qt::FocusReason r = event->reason();
    if(r == Qt::MouseFocusReason || r == Qt::TabFocusReason
        || r == Qt::BacktabFocusReason || r == Qt::OtherFocusReason){
        this->setAsFocusedButton();
    }
}
