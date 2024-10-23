#include "contentbutton.h"

#include <QSizePolicy>
#include <QDebug>
#include <QMouseEvent>
#include <QClipboard>


int contentBtnCount::totalContentBtnCount = 0;
int contentBtnCount::markedContentBtnCount = 0;
contentButton* contentButton::currSelectedBtn = nullptr;

// contentButton::contentButton(dynAddRmButton *dynBtnPtr){
contentButton::contentButton(){
    this->setSizePolicy(QSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored));
    this->setStyleButtonDefault();
    // this->setFocusPolicy(Qt::NoFocus);
    // dynBtn = dynBtnPtr;
    indexInGrid = getTotalCnt() - 1; //-1, as the number is already increased (see ctor of contentbtncount.h)
}

contentButton::~contentButton(){
    if(isMarked){
        this->decrMarkedCnt();
    }
}

void contentButton::setStyleSheetWrap(QString str){
    this->setStyleSheet(str);
}

void contentButton::setStyleButtonIsMarked(){
    qDebug() << "  style (is marked)";
    // this->setStyleSheetWrap("background-color: #FFB0B0; outline: 1px dashed #ff0000; outline-offset: 0px; border: 1px solid silver; border-radius: 10%;");
    this->setStyleSheetWrap("contentButton { color: black; border: 1px solid lightcoral; border-radius: 10%;"
                            "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #ffe8e3, stop:1 #fad4cd) }");
}

void contentButton::setStyleButtonIsSelected(){
    qDebug() << "  style (is selected)";
    this->setStyleSheetWrap("contentButton { color: black; border: 1px solid grey; border-radius: 10%;"
                            "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #fafacd, stop:1 #f7f7ba) }");
}

void contentButton::setStyleButtonDefault(){
    qDebug() << "  style (default)";
    this->setStyleSheetWrap("contentButton { background-color: white; border: 1px solid silver; border-radius: 10%; }");
}

bool contentButton::getIsMarked(){
    return this->isMarked;
}

bool contentButton::getIsSelected(){
    return this->isSelected;
}

void contentButton::setAsSelectedButton(){
    qDebug() << ">> setAsSelectedButton";
    if(currSelectedBtn == nullptr){
        qDebug() << "no button is currently selected. This button now is! (Index: " << this->indexInGrid << ")";
        this->isSelected = true;
        this->setStyleButtonIsSelected();
        currSelectedBtn = this;
    }else{
        qDebug() << "Another button is already selected.";
    }
    qDebug() << ">> /setAsSelectedButton";
}

void contentButton::unsetAsSelectedButton(){
    qDebug() << ">> unsetAsSelectedButton";
    if(currSelectedBtn == this){
        qDebug() << "This button is selected, will now become unselected!";
        this->isSelected = false;
        this->setStyleButtonDefault();
        currSelectedBtn = nullptr;
    }else{
        qDebug() << "Another button is the selected button. Do nothing.";
    }
    qDebug() << ">> /unsetAsSelectedButton";
}

void contentButton::setMarked(){
    this->isMarked = true;
    this->incrMarkedCnt();
    this->setStyleButtonIsMarked();
}

void contentButton::setUnmarked(){
    qDebug() << "setUnmarked";
    this->isMarked = false;
    this->decrMarkedCnt();
    this->setStyleButtonDefault();
    qDebug() << "/setUnmarked";
}

void contentButton::switchMarking(){
    if(this->isMarked){
        this->setUnmarked();
    }else{
        this->setMarked();
    }
}

void contentButton::keyPressEvent(QKeyEvent *event){
    qDebug() << "==================== Key press event! (contentButton)";

    if(this->currSelectedBtn){
        qDebug() << "There somehow is a selected button... ID: " << currSelectedBtn->indexInGrid;
    }

    //https://doc.qt.io/qt-6/qclipboard.html
    //https://doc.qt.io/qt-6/qt.html#Key-enum

    int key = event->key();

    if(key == Qt::Key_Return || key == Qt::Key_Enter){
        qDebug() << "Enter pressed.";

        //ToDo this code segment is incomplete

        Qt::KeyboardModifiers mod = event->modifiers();
        if(mod == Qt::CTRL || mod == Qt::SHIFT){
            if(getMarkedCnt() == 0){
                emit dynBtnNeedsSwitch();
            }

            this->unsetAsSelectedButton();
            this->setMarked();

        }else{

            if(getMarkedCnt() > 0){
                this->setMarked();
            }else{
                emit startContentButtonEdit(this->indexInGrid);
            }
        }
    }else if(this->isSelected){
        qDebug() << "............ is selected";

        // if(event->modifiers() == Qt::CTRL){
            if(key == Qt::Key_C){
                qDebug() << "ctrl+c was pressed";
                QGuiApplication::clipboard()->setText(this->text());
            }else if(key == Qt::Key_V){
                qDebug() << "ctrl+v was pressed";
                this->setText(QGuiApplication::clipboard()->text());
            // }
        // }else if(key == Qt::Key_Left){
        //     qDebug() << "left key";
        // }else if(key == Qt::Key_Right){
        //     qDebug() << "right key";
        // }else if(key == Qt::Key_Up){
        //     qDebug() << "up key";
        // }else if(key == Qt::Key_Down){
        //     qDebug() << "down key";
        // }
        }else if(key == Qt::Key_Left || key == Qt::Key_Right || key == Qt::Key_Up || key == Qt::Key_Down
                   || key == Qt::Key_Delete || key == Qt::Key_Backspace){
            qDebug() << "+++ indexInGrid: " << this->indexInGrid;
            emit keyWasPressed(key, this->indexInGrid);
        }
    }else{
        qDebug() << ".. not selected";
    }
    qDebug() << "==================== / Key press event! (contentButton)";
}

void contentButton::mouseReleaseLeftBtn(){
    qDebug() << "Left Button!";

    // if(!this->isMarked){
    //     this->setStyleSheetWrap("contentButton { background-color: yellow; }");
    // }else{
    //     ; //for now: do nothing here..
    // }

    //SIGNAL: unmark all + switch button
    emit cntButtonWasLeftClicked();

    if(!(this->isMarked)){

        qDebug() << "Button is not marked";

        if(currSelectedBtn == nullptr){
            qDebug() << "^ currSelectedBtn is nullptr";
            //currSelectedBtn = this;
            this->setAsSelectedButton();
            qDebug() << "^ currSelectedBtn no longer is nullptr";
        }else{
            qDebug() << "^ currSelectedBtn is NOT nullptr";

            if(currSelectedBtn == this){
                qDebug() << "^ currSelectedBtn == this  (clear ptr)";
                this->unsetAsSelectedButton();
                //currSelectedBtn = nullptr;
            }else{
                currSelectedBtn->unsetAsSelectedButton();
                //currSelectedBtn = this;
                this->setAsSelectedButton();
                qDebug() << "^ currSelectedBtn != this";
            }
        }

    }else{
        ; //for now: do nothing here..
    }
}


//static func (!)
void contentButton::clearButtonSelection(){
    if(currSelectedBtn != nullptr){
        currSelectedBtn->unsetAsSelectedButton();
        currSelectedBtn = nullptr;
    }
}


void contentButton::mouseReleaseRightBtn(){
    qDebug() << "Right Button!";

    //if there is a selected button, unselect it now
    contentButton::clearButtonSelection();

    int markCnt = this->getMarkedCnt();

    qDebug() << " (before (un)marking) getMarkedCnt() --> " << markCnt;

    // isMarked ^= 1; //switches 'isMarked'
    if(!isMarked){
        this->setMarked();
    }else{
        this->setUnmarked();
    }

    markCnt = this->getMarkedCnt();

    qDebug() << " (after (un)marking) getMarkedCnt() --> " << markCnt;


    /*   * case 1: button has been demarked and
         *           we now have 0 marked buttons
         *           --> switch state of dynFuncBtn to ADD
         * case 2: button is now marked (was not
         *           before) and our count now is 1
         *           --> switch state of dynFuncBtn to RM */
    if(( !(this->isMarked) && markCnt == 0)
        ||  (this->isMarked  && markCnt == 1)){
        qDebug() << "Switch mode";
        // dynBtn->switchMode();

        qDebug() << "EMIT!";
        emit dynBtnNeedsSwitch();
    }

    qDebug() << "end of mouseReleaseRightBtn";
}

// void contentButton::mousePressEvent(QMouseEvent *event){
//     qDebug() << "mousePressEvent";
//     this->setFocus();
//     ; //do nothing (this override serves the purpose of blocking the style change that normally happens)
//     qDebug() << "/mousePressEvent";
// }

void contentButton::mouseReleaseEvent(QMouseEvent *event){
    qDebug() << "mouseReleaseEvent";
    // this->setFocus();
    if(event->button() == Qt::LeftButton){
        this->mouseReleaseLeftBtn();
    }else if(event->button() == Qt::RightButton){
        this->mouseReleaseRightBtn();
    }
    qDebug() << "/mouseReleaseEvent";
}

// void contentButton::obtainFocus(){
//     this->setFocus();
// }

//ToDo
void contentButton::mouseDoubleClickEvent(QMouseEvent *event){
    //https://www.youtube.com/watch?v=Yg1FBrbfwNM
    qDebug() << "Double click!";

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
        this->setAsSelectedButton();
        emit startContentButtonEdit(this->indexInGrid);
    }
}


void contentButton::focusOutEvent(QFocusEvent *event){
    QWidget::focusOutEvent(event);

    qDebug() << "  focusOutEvent ---- this->getMarkedCnt(): " << this->getMarkedCnt();

    if(this->getMarkedCnt() == 0){
        qDebug() << "focusOutEvent ; index: " << this->indexInGrid;
        qDebug() << ">Reason: " << event->reason();

        Qt::FocusReason r = event->reason();
        if(r == Qt::MouseFocusReason || r == Qt::TabFocusReason || r == Qt::BacktabFocusReason){
            this->unsetAsSelectedButton();
        }
    }
}

void contentButton::focusInEvent(QFocusEvent *event){
    QWidget::focusInEvent(event);

    qDebug() << "  focusInEvent ---- this->getMarkedCnt(): " << this->getMarkedCnt();

    if(this->getMarkedCnt() == 0){
        qDebug() << "focusInEvent ; index: " << this->indexInGrid;
        qDebug() << ">Reason: " << event->reason();

        Qt::FocusReason r = event->reason();
        if(r == Qt::MouseFocusReason || r == Qt::TabFocusReason || r == Qt::BacktabFocusReason){
            this->setAsSelectedButton();
        }
    }
}
