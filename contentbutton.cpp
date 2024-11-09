#include "contentbutton.h"

#include <QSizePolicy>
#include <QDebug>
#include <QMouseEvent>
#include <QClipboard>
#include <QMessageBox>
#include <QPainter>
#include <QTextDocument>
#include <QInputDialog>



const QString contentButton::textForNewTitleAct = "Add title";
const QString contentButton::textForEditTitleAct = "Edit title";
const QString contentButton::textForRemoveTitleAct = "Remove title";
const QString contentButton::textForMarkDeletionAct = "Mark for deletion";
const QString contentButton::textForUnmarkDeletionAct = "Unmark from deletion";
const QString contentButton::textForDeleteButton = "Delete button";

int contentBtnCount::totalContentBtnCount = 0;
int contentBtnCount::markedContentBtnCount = 0;
contentButton* contentButton::focusedButton = nullptr;

// contentButton::contentButton(dynAddRmButton *dynBtnPtr){
contentButton::contentButton(QWidget *parent)
    : QPushButton(parent),
    newEditTitleAction(textForNewTitleAct, this),
    removeTitleAction(textForRemoveTitleAct, this),
    markForDeleteAction(textForMarkDeletionAct, this),
    deleteButtonAction(textForDeleteButton, this)
{
    this->setMinimumSize(this->minButtonSize_w, this->minButtonSize_h);
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
    optionsMenu.addAction(&newEditTitleAction);
    titleActionSeparator = optionsMenu.addSeparator();
    titleActionSeparator->setVisible(false);
    optionsMenu.addAction(&removeTitleAction);
    removeTitleAction.setVisible(false);
    optionsMenu.addSeparator();
    optionsMenu.addAction(&markForDeleteAction);
    optionsMenu.addSeparator();
    optionsMenu.addAction(&deleteButtonAction);
    connect(&newEditTitleAction, SIGNAL(triggered()), this, SLOT(titleAdjust()));
    connect(&removeTitleAction, SIGNAL(triggered()), this, SLOT(removeTitle()));
    connect(&markForDeleteAction, SIGNAL(triggered()), this, SLOT(markForDeletion()));
    connect(&deleteButtonAction, SIGNAL(triggered()), this, SLOT(deleteThisButton()));
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
        this->markForDeleteAction.setText(this->textForUnmarkDeletionAct);
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
        this->markForDeleteAction.setText(this->textForMarkDeletionAct);
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

void contentButton::gainFocus(){
    contentButton::clearFocusedButton();
    this->setAsFocusedButton();
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
        // QGuiApplication::clipboard()->setText(this->text());
        QGuiApplication::clipboard()->setText(this->content);
    }else if(key == Qt::Key_V){

        bool save = true;
        if(this->getContent().length() > 0){
        // if(this->text().length() > 0){

            qDebug() << "There already is text!";
            QMessageBox::StandardButton reply;

            reply = QMessageBox::question(this, "Override Confirmation",
                                          "Do you really want to override the existing content for this button?"
                                          "<br><br><i>(WARNING: This is irreversible)</i>",
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
            // this->setText(QGuiApplication::clipboard()->text());
            this->setContent(QGuiApplication::clipboard()->text());
            this->repaint();
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

void contentButton::titleAdjust(){
    qDebug() << "start: titleAdjust";
    //ToDo
    bool inputOK;

    QString userInput;
    if(this->hasTitle()){
        userInput = QInputDialog::getText(this, "Edit title", "Change title to: ",
                                                  QLineEdit::Normal, this->title, &inputOK);
    }else{
        userInput = QInputDialog::getText(this, "New title", "New title: ",
                                                  QLineEdit::Normal, "", &inputOK);
    }

    if (inputOK){
        qDebug() << "Input OK";
        //ToDo
        this->setTitle(userInput);
    }

    qDebug() << "/end: titleAdjust";
}

void contentButton::removeTitle(){
    this->setTitle("");
}

void contentButton::markForDeletion(){
    qDebug() << "markForDeletion";
    this->switchMarking();
}

void contentButton::deleteThisButton(){
    emit deleteButton(this->indexInGrid);
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
    // buttonEdit *btnEdit = new buttonEdit();

    // buttonEdit *btnEdit = new buttonEdit(this, this->text());
    // btnEdit->setModal(true);

    // btnEdit->setWindowFlags(Qt::CustomizeWindowHint);
    // btnEdit->setWindowFlags(Qt::CustomizeWindowHint);
    // btnEdit->setWindowFlags(Qt::FramelessWindowHint);
    // btnEdit->setWindowFlags(Qt::CustomizeWindowHint);
    // btnEdit->setWindowFlags(Qt::WindowTitleHint);
    // btnEdit->
    // btnEdit->setStyleSheet("background-color: white; border: 1px solid silver; border-radius: 10%;");

    // btnEdit->setStyle(QPushButton::style());

    // btnEdit->getCancelBtn()->setStyle(QPushButton::style());
    // btnEdit->getSaveBtn()->setStyle(QPushButton::style());



    // buttonEdit *btnEdit = new buttonEdit(this, this->text());
    // btnEdit->setModal(true);
    // btnEdit->show();


    // delete btnEdit;

    // btnEdit->setParent(this);

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

QString contentButton::getTitle(){
    return this->title;
}

QString contentButton::getContent(){
    return this->content;
}

void contentButton::setTitle(QString title){
    qDebug() << "start: setButtonTitle";

    if(title.length() == 0){
        this->title.clear();
        this->newEditTitleAction.setText(textForNewTitleAct);
        titleActionSeparator->setVisible(false);
        removeTitleAction.setVisible(false);
    }else{
        this->title = title;
        this->newEditTitleAction.setText(textForEditTitleAct);
        titleActionSeparator->setVisible(true);
        removeTitleAction.setVisible(true);
    }

    // QPaintEvent* ev = new QPaintEvent(this->rect());
    // this->paintEvent(ev);
    // this->repaint();

    qDebug() << "end: setButtonTitle";
}

void contentButton::setContent(QString content){
    qDebug() << "start: setButtonContent";
    this->content = content;
    // QPaintEvent* ev = new QPaintEvent(this->rect());
    // this->paintEvent(ev);
    // this->repaint();

    //ToDo check this part.. maybe create "updateText" function?
    //this->setText(this->buttonTitle);
    qDebug() << "end: setButtonContent";
}

bool contentButton::hasTitle(){
    return (this->title.length() > 0);
}

// bool contentButton::hasContent(){
//     return (this->content.length() > 0);
// }

void contentButton::paintEvent(QPaintEvent *event){

    // qDebug() << "    (start) PAINT EVENT";

    QPushButton::paintEvent(event);

    QPainter combinedImage(this);

    // int cutAtPercent = 100;
    // int totalPercent = 500;
    // int dividingHeight = this->height()*cutAtPercent/totalPercent;


    QTextDocument docTitle(this->title);
    QTextOption optTitle(Qt::AlignHCenter);
    docTitle.setDefaultFont(QFont("SansSerif", 20, QFont::Medium));
    docTitle.setDefaultTextOption(optTitle);
    docTitle.adjustSize();

    if(docTitle.size().width() < this->width()){
        docTitle.setTextWidth(this->width());

        // qDebug() << "(adjusted!) docTitle.size().width(): " << docTitle.size().width();
        // qDebug() << "(adjusted!) docTitle.size().height(): " << docTitle.size().height();
    }

    // int dividingHeight = qMin(this->height()/5, 30);
    // int dividingHeight = docTitle.size().height();
    // int remainingHeight = qMax(this->height() - dividingHeight, 30);

    //ToDo check int vs. double
    int dividingHeight = this->title.length() > 0 ? docTitle.size().height() : 0;
    int remainingHeight = this->height() - dividingHeight;


    // qDebug() << "this->width(): " << this->width();
    // qDebug() << "this->height(): " << this->height();
    // qDebug() << "---";

    // qDebug() << "dividingHeight: " << dividingHeight;
    // qDebug() << "remainingHeight: " << remainingHeight;
    // qDebug() << "---";

    QPoint pointTitle(0,0);
    QSize sizeTitle(this->width() - 1, dividingHeight);
    QRect rectTitle(pointTitle, sizeTitle);

    // qDebug() << "docTitle.textWidth(): " << docTitle.textWidth(); //always "-1"
    // qDebug() << "docTitle.size().width(): " << docTitle.size().width();
    // qDebug() << "docTitle.size().height(): " << docTitle.size().height();

    QPixmap pixmapTitle(docTitle.size().width(), docTitle.size().height());
    pixmapTitle.fill(Qt::transparent);

    // qDebug() << "pixmapTitle.width(): " << pixmapTitle.width();
    // qDebug() << "pixmapTitle.height(): " << pixmapTitle.height();


    QPainter paintTitle(&pixmapTitle);
    paintTitle.setBrush(Qt::black);
    // paintTitle.setFont(QFont("Times", 100, QFont::Medium));
    docTitle.drawContents(&paintTitle, pixmapTitle.rect());
    // QPixmap scaledPixmapTitle = pixmapTitle.scaledToWidth(this->rect().width(), Qt::SmoothTransformation);
    // QPixmap scaledPixmapTitle = pixmapTitle.scaled(sizeTitle, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    QPixmap scaledPixmapTitle(pixmapTitle);
    /*
     * problem: when setting the scaledPixmap as icon (see below), any text
     *          that resides in the center of the pixmap will be what is visible,
     *          but we want to see as much of the "left text" as possible
     * solution: crop the pixmap to size rect, effectively throwing away anything
     *           that doesn't fit into the rect (== that was overhanging on the right)
     */
    QPixmap croppedPixmapTitle = scaledPixmapTitle.copy(pixmapTitle.rect());

    // combinedImage.setBrush(Qt::gray);
    // combinedImage.drawRect(rectTitle);

    combinedImage.drawPixmap(rectTitle,croppedPixmapTitle);

    // qDebug() << "---";
    //---------------------------------------------------------------------------------


    QPoint pointContent(0, dividingHeight);
    // QSize sizeContent(this->width() - 1, this->height() - dividingHeight - 1);
    // QSize sizeContent(this->width() - 1, remainingHeight - 1); //ToDo check the "-1" of height
    QSize sizeContent(this->width() - 1, remainingHeight); //ToDo check the "-1" of height
    // QRect rectContent(pointContent, sizeContent);

    QTextDocument docContent(this->content);
    // QTextDocument docContent("OK this is just a test ignore what I say here have a nice day");
    // QTextOption optContent(Qt::AlignHCenter);
    // docContent.setDefaultTextOption(optContent);
    docContent.setDefaultFont(QFont("Times", 12, QFont::Medium));
    docTitle.adjustSize();

    // qDebug() << "docContent.textWidth(): " << docContent.textWidth();
    // qDebug() << "docContent.size().width(): " << docContent.size().width();
    // qDebug() << "docContent.size().height(): " << docContent.size().height();

    // docContent.setTextWidth(this->width()); //Forces Line Breaks.. ToDo maybe create toggle in right-click menu?
    // docContent.setTextWidth(docContent.size().width());

    // QTextDocument docContent;
    // QString contentHtml = "<p style=\"text-align: center; white-space: pre-line\">" + readTxt + "</p>";
    // docContent.setHtml(contentHtml);

    // QTextDocument docContent(readTxt);
    // docContent.setProperty()

    // int heightForPixmap = qMax(remainingHeight, int(docContent.size().height()));
    // QPixmap pixmapContent(docContent.size().width(), heightForPixmap);

    QPixmap pixmapContent(docContent.size().width(), docContent.size().height());

    pixmapContent.fill(Qt::transparent);
    QPainter paintContent(&pixmapContent);
    paintContent.setBrush(Qt::black);
    // paintContent.setFont(QFont("Serif", 10, QFont::Medium));

    // qDebug() << "pixmapContent.rect().width(): " << pixmapContent.rect().width();
    // qDebug() << "pixmapContent.rect().height(): " << pixmapContent.rect().height();

    docContent.drawContents(&paintContent, pixmapContent.rect());
    // QPixmap scaledPixmapContent = pixmapContent.scaledToWidth(this->rect().width(), Qt::SmoothTransformation);

    // qDebug() << "(2) remainingHeight: " << remainingHeight;

    if(pixmapContent.rect().height() > remainingHeight){
        // qDebug() << "   IF (pixmapContent.rect().height() > remainingHeight == true)";
        QPixmap scaledPixmapContent = pixmapContent.scaledToHeight(remainingHeight, Qt::SmoothTransformation);

        // QPixmap scaledPixmapContent = (pixmapContent.rect().height() > remainingHeight)
        //                                   ? pixmapContent.scaledToHeight(remainingHeight, Qt::SmoothTransformation)
        //                                   : pixmapContent;

        // QPixmap scaledPixmapContent = pixmapContent.scaled(sizeContent, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        //see comment above, same applies now

        // qDebug() << "scaledPixmapContent.width(): " << scaledPixmapContent.width();
        // qDebug() << "scaledPixmapContent.height(): " << scaledPixmapContent.height();

        // qDebug() << "---";

        // qDebug() << "rectContent.x(): " << rectContent.x();
        // qDebug() << "rectContent.y(): " << rectContent.y();
        // qDebug() << "rectContent.width(): " << rectContent.width();
        // qDebug() << "rectContent.height(): " << rectContent.height();

        // QRect rectForContentCrop(rectContent);
        // rectForContentCrop.setHeight(rectContent.height() + dividingHeight*2);

        // qDebug() << "rectForContentCrop.x(): " << rectForContentCrop.x();
        // qDebug() << "rectForContentCrop.y(): " << rectForContentCrop.y();
        // qDebug() << "rectForContentCrop.width(): " << rectForContentCrop.width();
        // qDebug() << "rectForContentCrop.height(): " << rectForContentCrop.height();

        // QPixmap croppedPixmapContent = scaledPixmapContent.copy(rectForContentCrop);

        //    inline QPixmap copy(int x, int y, int width, int height) const;

        QRect rectForContentCrop(0, 0, this->width() - 1, remainingHeight);

        QPixmap croppedPixmapContent = scaledPixmapContent.copy(rectForContentCrop);

        // qDebug() << "---";

        // qDebug() << "croppedPixmapTitle.width(): " << croppedPixmapTitle.width();
        // qDebug() << "croppedPixmapTitle.height(): " << croppedPixmapTitle.height();

        // qDebug() << "croppedPixmapContent.width(): " << croppedPixmapContent.width();
        // qDebug() << "croppedPixmapContent.height(): " << croppedPixmapContent.height();

        combinedImage.drawPixmap(pointContent, croppedPixmapContent);
    }else{
        // qDebug() << "   ELSE (pixmapContent.rect().height() <= remainingHeight)";
        QRect rectForUnscaledUncropped(0, dividingHeight, pixmapContent.rect().width(), pixmapContent.rect().height());

        combinedImage.drawPixmap(rectForUnscaledUncropped, pixmapContent);
    }

    // qDebug() << "    (/end) PAINT EVENT";
}
