#include "contentbutton.h"
#include "apputils.h"

#include <QSizePolicy>
#include <QDebug>
#include <QMouseEvent>
#include <QClipboard>
#include <QMessageBox>
#include <QPainter>
#include <QTextDocument>
#include <QInputDialog>

qsizetype contentBtnCount::totalContentBtnCount = 0;
qsizetype contentBtnCount::markedForDeletionCount = 0;
contentButton* contentButton::focusedButton = nullptr;
contentButton* contentButton::lastUnfocusedButton = nullptr;

QString const contentButton::textForNewTitleAct = "Add title";
QString const contentButton::textForEditTitleAct = "Edit title";
QString const contentButton::textForRemoveTitleAct = "Remove title";
QString const contentButton::textForCopyContentAct = "Copy content";
QString const contentButton::textForPasteContentAct = "Paste content";
QString const contentButton::textForMarkDeletionAct = "Mark for deletion";
QString const contentButton::textForUnmarkDeletionAct = "Unmark from deletion";
QString const contentButton::textForDeleteButton = "Delete button";

contentButton::contentButton(QWidget *parent)
    : QPushButton(parent),
    newEditTitleAction(this->textForNewTitleAct, this),
    removeTitleAction(this->textForRemoveTitleAct, this),
    copyContentAction(this->textForCopyContentAct, this),
    pasteContentAction(this->textForPasteContentAct, this),
    markForDeleteAction(this->textForMarkDeletionAct, this),
    deleteButtonAction(this->textForDeleteButton, this)
{
    this->setMinimumSize(this->minButtonSize_w, this->minButtonSize_h);
    this->setSizePolicy(QSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored));
    this->setStyleDefault();
    this->setFocusPolicy(Qt::StrongFocus); //tab+click focus
    this->setAttribute(Qt::WA_MacShowFocusRect);

    //-1 here as the number is already increased upon creation (see ctor of contentbtncount.h)
    this->setIndexInGrid(contentBtnCount::getTotalCnt() - 1);

    //---MENU---
    //add/edit title
    this->optionsMenu.addAction(&(this->newEditTitleAction));

    //remove title
    this->removeTitleActionSeparator = this->optionsMenu.addSeparator();
    this->removeTitleActionSeparator->setVisible(false);
    this->optionsMenu.addAction(&(this->removeTitleAction));
    this->removeTitleAction.setVisible(false);

    //copy content
    this->copyContentActionSeparator = this->optionsMenu.addSeparator();
    this->copyContentActionSeparator->setVisible(false);
    this->optionsMenu.addAction(&(this->copyContentAction));
    this->copyContentAction.setVisible(false);

    //paste content
    this->pasteContentActionSeparator = this->optionsMenu.addSeparator();
    this->pasteContentActionSeparator->setVisible(false);
    this->optionsMenu.addAction(&(this->pasteContentAction));
    this->pasteContentAction.setVisible(false);

    //mark for deletion
    this->optionsMenu.addSeparator();
    this->optionsMenu.addAction(&(this->markForDeleteAction));

    //delete button
    this->optionsMenu.addSeparator();
    this->optionsMenu.addAction(&(this->deleteButtonAction));

    //---qt connects---
    connect(&(this->newEditTitleAction), SIGNAL(triggered()), this, SLOT(titleAdjust()));
    connect(&(this->copyContentAction), SIGNAL(triggered()), this, SLOT(copyContentToClipboard()));
    connect(&(this->pasteContentAction), SIGNAL(triggered()), this, SLOT(pasteContentFromClipboard()));
    connect(&(this->removeTitleAction), SIGNAL(triggered()), this, SLOT(removeTitle()));
    connect(&(this->markForDeleteAction), SIGNAL(triggered()), this, SLOT(switchMarkedForDeletion()));
    connect(&(this->deleteButtonAction), SIGNAL(triggered()), this, SLOT(deleteThisButton()));
}

contentButton::~contentButton(){
    if(this->isMarkedForDeletion()){
        this->decrMarkedForDelCnt();
        if(contentButton::getMarkedForDelCnt() == 0){
            emit this->dynBtnSetMode(dynAddRmButton::btnModeADD);
        }
    }
    this->unsetAsFocusedButton(); //called func already includes check on whether this button "is focused" or not
    this->unsetAsLastUnfocusedButton();
}

void contentButton::setStyleDefault(){
    qDebug() << "style (default)";
    this->setStyleSheet("contentButton { color: black; border: 1px solid silver; border-radius: 10%; "
                            "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #F0F0F0, stop:1 #eaeaea) }");
}

void contentButton::setStyleFocused(){
    qDebug() << "style (focus)";
    this->setStyleSheet("contentButton { color: black; border: 1px solid grey; border-radius: 10%;"
                        "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #fbfbd5, stop:1 #fdfde2) }"
                        "contentButton:focus { color: black; border: 2px solid grey; border-radius: 10%; outline: none;"
                        "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #fafacd, stop:1 #f7f7ba) }");
}

void contentButton::setStyleMarkedForDeletion(){
    qDebug() << "style (marked)";
    this->setStyleSheet("contentButton { color: black; border: 1px solid lightcoral; border-radius: 10%;"
                            "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #ffe8e3, stop:1 #fad4cd) }");
}

void contentButton::setStyleMarkedForDelAndFocus(){
    qDebug() << "style (marked & focus)";
    this->setStyleSheet("contentButton { color: black; border: 1px solid lightcoral; border-radius: 10%;"
                            "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #ffe8e3, stop:1 #fad4cd) }"
                            "contentButton:focus { color: black; border: 2px solid lightcoral; border-radius: 10%; outline: none;"
                            "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #ffdabd, stop:1 #ffcabd) }");
}

bool contentButton::isMarkedForDeletion() const{
    return this->markedForDeletion;
}

bool contentButton::isNotMarkedForDeletion() const{
    return !(this->isMarkedForDeletion());
}

void contentButton::setMarkedForDeletion(){
    if(this->isNotMarkedForDeletion()){
        this->incrMarkedForDelCnt();
        if(this->isFocused()){
            this->setStyleMarkedForDelAndFocus();
        }else{
            this->setStyleMarkedForDeletion();
        }
        this->markedForDeletion = true;
        this->markForDeleteAction.setText(this->textForUnmarkDeletionAct);
    }
}

void contentButton::unsetMarkedForDeletion(){
    if(this->isMarkedForDeletion()){
        this->decrMarkedForDelCnt();
        if(this->isFocused()){
            this->setStyleFocused();
        }else{
            this->setStyleDefault();
        }
        this->markedForDeletion = false;
        this->markForDeleteAction.setText(this->textForMarkDeletionAct);
    }
}

void contentButton::switchMarkedForDeletion(){
    if(this->markedForDeletion){
        this->unsetMarkedForDeletion();
    }else{
        this->setMarkedForDeletion();
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
    qsizetype markedForDelCount = contentButton::getMarkedForDelCnt();
    if(markedForDelCount == 0 && this->isNotMarkedForDeletion()){
        qDebug() << "emit set mode to add.";
        emit this->dynBtnSetMode(dynAddRmButton::btnModeADD);
    }else if(markedForDelCount == 1 && this->isMarkedForDeletion()){
        qDebug() << "emit set mode to rm";
        emit this->dynBtnSetMode(dynAddRmButton::btnModeRM);
    }
}

bool contentButton::isFocused() const{
    return (contentButton::focusedButton == this);
}

void contentButton::setAsFocusedButton(){
    if(this->isNotFocused()){
        this->setFocus();
        if(this->isMarkedForDeletion()){
            this->setStyleMarkedForDelAndFocus();
        }else{
            this->setStyleFocused();
        }
        contentButton::focusedButton = this;
    }
}

void contentButton::unsetAsFocusedButton(){
    if(this->isFocused()){
        this->clearFocus();
        if(this->isMarkedForDeletion()){
            this->setStyleMarkedForDeletion();
        }else{
            this->setStyleDefault();
        }
        contentButton::lastUnfocusedButton = this;
        contentButton::focusedButton = nullptr;
    }
}

void contentButton::unsetAsLastUnfocusedButton(){
    if(contentButton::lastUnfocusedButton == this){
        contentButton::clearLastUnfocusedButton();
    }
}

void contentButton::gainFocus(){
    contentButton::clearFocusedButton();
    this->setAsFocusedButton();
}

bool contentButton::isNotFocused() const{
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

//static
void contentButton::restoreLastUnfocusedButtonToFocusedButton(){
    if(contentButton::lastUnfocusedButton != nullptr){
        contentButton::lastUnfocusedButton->gainFocus();
    }
}

//static
void contentButton::clearLastUnfocusedButton(){
    contentButton::lastUnfocusedButton = nullptr;
}

void contentButton::keyPressEvent(QKeyEvent *event){
    qDebug() << "start: Key press event! (contentButton)";

    int key = event->key();
    if(key == Qt::Key_Return || key == Qt::Key_Enter){
        qDebug() << "Enter pressed.";
        Qt::KeyboardModifiers mod = event->modifiers();
        if(mod == Qt::CTRL || mod == Qt::SHIFT){
            this->openOptionsMenu(this->mapToGlobal(this->rect().center()));
        }else{
            emit this->startContentButtonEdit(this->getIndexInGrid());
        }
    }else if(key == Qt::Key_C){
        this->copyContentToClipboard();
    }else if(key == Qt::Key_V){
        this->pasteContentFromClipboard();
    }else if(key == Qt::Key_Left || key == Qt::Key_Right
               || key == Qt::Key_Up || key == Qt::Key_Down
               || key == Qt::Key_Delete || key == Qt::Key_Backspace
               || key == Qt::Key_Escape || key == Qt::Key_Plus
               || key == Qt::Key_P){
        emit this->keyWasPressed(key, this->getIndexInGrid());
    }else if(key == Qt::Key_Minus || key == Qt::Key_Underscore){
        //underscore is Shift+MinusKey(-) on most keyboard layouts
        Qt::KeyboardModifiers mod = event->modifiers();
        if(mod == Qt::CTRL || mod == Qt::SHIFT){
            this->switchMarkedForDeletion();
        }else{
            emit this->keyWasPressed(key, this->getIndexInGrid());
        }
    }
    qDebug() << "end: Key press event! (contentButton)";
}

void contentButton::mouseLeftClick(){
    qDebug() << "Left Button on a contentButton!";
    QFocusEvent* focus = new QFocusEvent(QEvent::MouseButtonPress, Qt::MouseFocusReason);
    this->focusInEvent(focus);
    delete focus;
}

void contentButton::titleAdjust(){
    qDebug() << "start: titleAdjust";

    QString userInput;
    bool pressedOK;
    if(this->hasTitle()){
        userInput = QInputDialog::getText(this, "Edit title", "Change title to: ", QLineEdit::Normal, this->title, &pressedOK);
    }else{
        userInput = QInputDialog::getText(this, "New title", "New title: ", QLineEdit::Normal, "", &pressedOK);
    }

    if(pressedOK){
        qDebug() << "Input OK";
        this->setTitle(userInput);
        this->saveJSON(); //ToDo optimize
    }

    qDebug() << "end: titleAdjust";
}

void contentButton::copyContentToClipboard(){
    if(this->content.length() > 0){
        QGuiApplication::clipboard()->setText(this->content);
    }else{
        timedPopUp(this,defaultShortPopUpTimer,"Button content is empty.<br>Will not copy to clipboard.");
    }
}

void contentButton::pasteContentFromClipboard(){
    bool save = true;
    QString textToPaste = QGuiApplication::clipboard()->text();
    if(textToPaste.length() > 0){
        if(this->getContent().length() > 0){
            qDebug() << "There already is text!";
            QMessageBox::StandardButton reply;

            reply = QMessageBox::question(this, "Override Confirmation",
                                          "Do you really want to override the existing content for this button?"
                                          "<br><br><i>(WARNING: This is irreversible)</i>",
                                          QMessageBox::No|QMessageBox::Yes, QMessageBox::No);
            if(reply == QMessageBox::No){
                save = false;
            }
        }
        if(save){
            this->setContent(QGuiApplication::clipboard()->text());
            this->saveJSON();
            this->repaint();
        }
    }else{
        timedPopUp(this,defaultShortPopUpTimer,"Clipboard does not have any text data.<br>Button content won't be overwritten.");
    }
}

void contentButton::enableCopyContent(){
    qDebug() << "start: Enable text copying.";
    this->copyContentAction.setVisible(true);
    if(copyContentActionSeparator){
        this->copyContentActionSeparator->setVisible(true);
    }
}

void contentButton::disableCopyContent(){
    qDebug() << "start: Disable text copying.";
    this->copyContentAction.setVisible(false);
    if(copyContentActionSeparator){
        this->copyContentActionSeparator->setVisible(false);
    }
}

void contentButton::enablePasteContent(){
    qDebug() << "start: Enable text pasting.";
    this->pasteContentAction.setVisible(true);
    if(pasteContentActionSeparator){
        this->pasteContentActionSeparator->setVisible(true);
    }
}

void contentButton::disablePasteContent(){
    qDebug() << "start: Disable text pasting.";
    this->pasteContentAction.setVisible(false);
    if(pasteContentActionSeparator){
        this->pasteContentActionSeparator->setVisible(false);
    }
}

void contentButton::removeTitle(){
    this->setTitle("");
    this->saveJSON(); //ToDo optimize
}

void contentButton::deleteThisButton(){
    emit this->deleteButton(this->getIndexInGrid());
}

void contentButton::openOptionsMenu(QPoint p){
    qDebug() << "start: openOptionsMenu (x: " << p.x() << ", y: " << p.y() << ")";
    optionsMenu.popup(p);
}

void contentButton::setIndexInGrid(qsizetype index){
    this->indexInGrid = index;
};

qsizetype contentButton::getIndexInGrid() const{
    return this->indexInGrid;
};

void contentButton::saveJSON(){
    emit this->saveButtonChangesIntoJSON(); //ToDo (optimization) saveJSONforSingleButton (or similar) + decouple saving title & saving content
}

QString contentButton::getTitle() const{
    return this->title;
}

void contentButton::setTitle(QString newTitle){
    qDebug() << "start: setButtonTitle";
    if(this->title != newTitle){
        if(newTitle.length() > 0){
            this->title = newTitle;
            this->newEditTitleAction.setText(this->textForEditTitleAct);
            this->removeTitleAction.setVisible(true);
            if(removeTitleActionSeparator){
                this->removeTitleActionSeparator->setVisible(true);
            }
        }else{
            this->title.clear();
            this->newEditTitleAction.setText(this->textForNewTitleAct);
            this->removeTitleAction.setVisible(false);
            if(removeTitleActionSeparator){
                this->removeTitleActionSeparator->setVisible(false);
            }
        }
    }else{
        qDebug() << "Title has not changed.";
    }
    qDebug() << "end: setButtonTitle";
}

bool contentButton::hasTitle() const{
    return (this->title.length() > 0);
}

QString contentButton::getContent() const{
    return this->content;
}

void contentButton::setContent(QString newContent){
    qDebug() << "start: setButtonContent";

    this->content = newContent;

    if(this->content.length() > 0){
        this->enableCopyContent();
    }else{
        this->disableCopyContent();
    }

    //ToDo check this part.. maybe create "updateText" function? Maybe use repaint? Think about this again if paintEvent is changed
    //this->setText(this->buttonTitle);
    //this->repaint()

    qDebug() << "end: setButtonContent";
}

void contentButton::mouseRightClick(QMouseEvent *event){
    qDebug() << "start: mouseRightClick";
    this->openOptionsMenu(event->globalPosition().toPoint());
    qDebug() << "end: mouseRightClick";
}

void contentButton::mousePressEvent(QMouseEvent *event){
    qDebug() << "start: mousePressEvent";
    this->setAsFocusedButton();
    if(event->button() == Qt::LeftButton){
        this->mouseLeftClick();
    }else if(event->button() == Qt::RightButton){
        this->mouseRightClick(event);
    }
    qDebug() << "end: mousePressEvent";
}

void contentButton::mouseDoubleClickEvent(QMouseEvent *event){
    //https://www.youtube.com/watch?v=Yg1FBrbfwNM
    qDebug() << "Double click!";
    if(event->button() == Qt::LeftButton){
        emit this->startContentButtonEdit(this->getIndexInGrid());
    }else if(event->button() == Qt::RightButton){
        this->mouseRightClick(event);
    }
}

void contentButton::focusInEvent(QFocusEvent *event){

    qDebug() << "focusInEvent ---- this->getMarkedForDelCnt(): " << this->getMarkedForDelCnt();
    qDebug() << "focusInEvent ; index: " << this->getIndexInGrid();

    QWidget::focusInEvent(event);
    qDebug() << "Reason: " << event->reason();

    Qt::FocusReason r = event->reason();
    if(r == Qt::MouseFocusReason || r == Qt::TabFocusReason
        || r == Qt::BacktabFocusReason || r == Qt::OtherFocusReason){
        this->setAsFocusedButton();
    }
}

void contentButton::focusOutEvent(QFocusEvent *event){

    qDebug() << "focusOutEvent ---- this->getMarkedForDelCnt(): " << this->getMarkedForDelCnt();
    qDebug() << "focusOutEvent ; index: " << this->getIndexInGrid();

    QWidget::focusOutEvent(event);
    qDebug() << "Reason: " << event->reason();

    Qt::FocusReason r = event->reason();
    if(r == Qt::MouseFocusReason || r == Qt::TabFocusReason
        || r == Qt::BacktabFocusReason || r == Qt::OtherFocusReason){
        this->unsetAsFocusedButton();
    }
}

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
