#include "contentbutton.h"

#include <QSizePolicy>
#include <QDebug>
#include <QMouseEvent>
#include <QClipboard>
#include <QMessageBox>
#include <QPainter>
#include <QInputDialog>
#include <QApplication>

#include "apputils.h"

qsizetype contentBtnCount::totalContentBtnCount = 0;
qsizetype contentBtnCount::markedForDeletionCount = 0;
contentButton* contentButton::focusedButton = nullptr;
contentButton* contentButton::lastUnfocusedButton = nullptr;

QString const contentButton::textForNewTitleAct = "Add title";
QString const contentButton::textForEditTitleAct = "Edit title";
QString const contentButton::textForRemoveTitleAct = "Remove title";
QString const contentButton::textForCopyContentAct = "Copy content";
QString const contentButton::textForPasteContentAct = "Paste content";
QString const contentButton::textForCutContentAct = "Cut content";
QString const contentButton::textForRemoveContentAct = "Remove content";
QString const contentButton::textForMoveButtonAct = "Change position";
QString const contentButton::textForMarkDeletionAct = "Mark for deletion";
QString const contentButton::textForUnmarkDeletionAct = "Unmark from deletion";
QString const contentButton::textForDeleteAllMarkedAct = "Delete ALL marked";
QString const contentButton::textForDeleteButton = "Delete this button";

contentButton::~contentButton(){
    if(this->isMarkedForDeletion()){
        this->decrMarkedForDelCnt();
        if(contentButton::getMarkedForDelCnt() == 0){
            emit this->dynBtnSetMode(dynButton::btnModeADD);
        }
    }
    this->unsetAsFocusedButton(); //the called func already includes a check on whether this button "has focus" or not
    this->unsetAsLastUnfocusedButton();
}

contentButton::contentButton(QWidget *const parent)
    : QPushButton(parent),
      addOrEditTitleAction(this->textForNewTitleAct, this),
      removeTitleAction(this->textForRemoveTitleAct, this),
      copyContentAction(this->textForCopyContentAct, this),
      pasteContentAction(this->textForPasteContentAct, this),
      cutContentAction(this->textForCutContentAct, this),
      removeContentAction(this->textForRemoveContentAct, this),
      moveButtonAction(this->textForMoveButtonAct, this),
      markForDeleteAction(this->textForMarkDeletionAct, this),
      deleteAllMarkedAction(this->textForDeleteAllMarkedAct, this),
      deleteButtonAction(this->textForDeleteButton, this)
{
    this->initButtonSettings();
    this->setUpContextMenu();
    this->connectAllActions();
}

void contentButton::initButtonSettings(){
    this->setMinimumSize(this->minButtonSize_w, this->minButtonSize_h);
    this->setSizePolicy(QSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored));
    this->setStyleDefault();
    this->setFocusPolicy(Qt::StrongFocus); //tab+click focus
    this->setAttribute(Qt::WA_MacShowFocusRect);

    //-1 here as the number is already increased upon creation (see ctor of contentbtncount.h)
    this->setIndexInList(contentBtnCount::getTotalCnt() - 1);

    //settings for title & content
    QTextOption optTitle(Qt::AlignHCenter);
    this->titleDoc.setDefaultTextOption(optTitle);
    this->titleDoc.setDefaultFont(QFont("SansSerif", 20, QFont::Medium));
    this->titleDoc.setDefaultStyleSheet("body { color : black; }");
    this->contentDoc.setDefaultFont(QFont("Times", 12, QFont::Medium));
    this->contentDoc.setDefaultStyleSheet("body { color : black; }");
}

void contentButton::setUpContextMenu(){
    //add/edit title
    this->optionsMenu.addAction(&(this->addOrEditTitleAction));

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

    //cut content
    this->cutContentActionSeparator = this->optionsMenu.addSeparator();
    this->cutContentActionSeparator->setVisible(false);
    this->optionsMenu.addAction(&(this->cutContentAction));
    this->cutContentAction.setVisible(false);

    //remove content
    this->removeContentActionSeparator = this->optionsMenu.addSeparator();
    this->removeContentActionSeparator->setVisible(false);
    this->optionsMenu.addAction(&(this->removeContentAction));
    this->removeContentAction.setVisible(false);

    //move button (visible by default!)
    this->moveButtonActionSeparator = this->optionsMenu.addSeparator();
    this->optionsMenu.addAction(&(this->moveButtonAction));

    //mark for deletion
    this->optionsMenu.addSeparator();
    this->optionsMenu.addAction(&(this->markForDeleteAction));

    //delete all marked buttons
    this->deleteAllMarkedActionSeparator = this->optionsMenu.addSeparator();
    this->deleteAllMarkedActionSeparator->setVisible(false);
    this->optionsMenu.addAction(&(this->deleteAllMarkedAction));
    this->deleteAllMarkedAction.setVisible(false);

    //delete this button
    this->optionsMenu.addSeparator();
    this->optionsMenu.addAction(&(this->deleteButtonAction));
}

void contentButton::connectAllActions(){
    connect(&(this->addOrEditTitleAction), SIGNAL(triggered()), this, SLOT(titleAdjust()));
    connect(&(this->removeTitleAction), SIGNAL(triggered()), this, SLOT(removeTitle()));
    connect(&(this->copyContentAction), SIGNAL(triggered()), this, SLOT(copyContentToClipboard()));
    connect(&(this->pasteContentAction), SIGNAL(triggered()), this, SLOT(pasteContentFromClipboard()));
    connect(&(this->cutContentAction), SIGNAL(triggered()), this, SLOT(cutContentToClipboard()));
    connect(&(this->removeContentAction), SIGNAL(triggered()), this, SLOT(removeContent()));
    connect(&(this->moveButtonAction), SIGNAL(triggered()), this, SLOT(emitIndexForMoveButton()));
    connect(&(this->markForDeleteAction), SIGNAL(triggered()), this, SLOT(switchMarkedForDeletion()));
    connect(&(this->deleteAllMarkedAction), SIGNAL(triggered()), this, SLOT(emitDeleteAllSignal()));
    connect(&(this->deleteButtonAction), SIGNAL(triggered()), this, SLOT(deleteThisButton()));
}

void contentButton::setStyleDefault(){
    this->setStyleSheet("contentButton { color: black; border: 1px solid silver; border-radius: 10%; "
                        "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #F0F0F0, stop:1 #eaeaea) }");
}

void contentButton::setStyleFocused(){
    this->setStyleSheet("contentButton { color: black; border: 1px solid grey; border-radius: 10%;"
                        "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #fbfbd5, stop:1 #fdfde2) }"
                        "contentButton:focus { color: black; border: 2px solid grey; border-radius: 10%; outline: none;"
                        "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #fafacd, stop:1 #f7f7ba) }");
}

void contentButton::setStyleMarkedForDeletion(){
    this->setStyleSheet("contentButton { color: black; border: 1px solid lightcoral; border-radius: 10%;"
                        "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #ffe8e3, stop:1 #fad4cd) }");
}

void contentButton::setStyleMarkedForDelAndFocus(){
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

//slot
void contentButton::switchMarkedForDeletion(){
    if(this->markedForDeletion){
        this->unsetMarkedForDeletion();
    }else{
        this->setMarkedForDeletion();
    }
    this->checkForDynBtnSwitch();
}

void contentButton::checkForDynBtnSwitch(){
       /** case 1: button has been demarked and
         *           we now have 0 marked buttons
         *           --> switch state of dynFuncBtn to ADD
         * case 2: button is now marked (was not
         *           before) and our count now is 1
         *           --> switch state of dynFuncBtn to RM
         */
    qsizetype markedForDelCount = contentButton::getMarkedForDelCnt();
    if(markedForDelCount == 0 && this->isNotMarkedForDeletion()){
        emit this->dynBtnSetMode(dynButton::btnModeADD);
    }else if(markedForDelCount == 1 && this->isMarkedForDeletion()){
        emit this->dynBtnSetMode(dynButton::btnModeRM);
    }
}

//slot
void contentButton::emitDeleteAllSignal(){
    emit this->deleteAllMarkedButtons();
}

//slot
void contentButton::emitIndexForMoveButton(){
    emit this->moveButton(this->indexInList);
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

void contentButton::keyPressEvent(QKeyEvent * const event){
    qDebug() << "start: keyPressEvent (contentButton)";
    int key = event->key();
    if(key == Qt::Key_Return || key == Qt::Key_Enter){
        qDebug() << "Enter key pressed.";
        Qt::KeyboardModifiers mod = event->modifiers();
        if(mod == Qt::CTRL || mod == Qt::SHIFT){
            this->openOptionsMenu(this->mapToGlobal(this->rect().center()));
        }else{
            emit this->startContentButtonEdit(this->getIndexInList());
        }
    }else if(key == Qt::Key_Backspace){
        this->removeContent();
    }else if(key == Qt::Key_X){
        this->cutContentToClipboard();
        this->repaint();
    }else if(key == Qt::Key_C){
        this->copyContentToClipboard();
    }else if(key == Qt::Key_V){
        this->pasteContentFromClipboard();
    }else if(key == Qt::Key_Left || key == Qt::Key_Right
               || key == Qt::Key_Up || key == Qt::Key_Down
               || key == Qt::Key_Escape || key == Qt::Key_Plus
               || key == Qt::Key_P || key == Qt::Key_S){
        emit this->keyWasPressed(key, this->getIndexInList());

    }else if(key == Qt::Key_M){
        this->emitIndexForMoveButton();
    }else if(key == Qt::Key_Delete){
        Qt::KeyboardModifiers mod = event->modifiers();
        if(mod == Qt::CTRL || mod == Qt::SHIFT){
            this->switchMarkedForDeletion();
        }else{
            emit this->keyWasPressed(key, this->getIndexInList());
        }
    }else if(key == Qt::Key_Minus || key == Qt::Key_Underscore){
        //underscore is Shift+MinusKey(-) on most keyboard layouts
        Qt::KeyboardModifiers mod = event->modifiers();
        if(mod == Qt::CTRL || mod == Qt::SHIFT){
            this->switchMarkedForDeletion();
        }else{
            emit this->keyWasPressed(key, this->getIndexInList());
        }
    }
    qDebug() << "end: keyPressEvent (contentButton)";
}

//slot
void contentButton::titleAdjust(){
    QString userInput;
    bool pressedOK;
    if(this->hasTitle()){
        userInput = QInputDialog::getText(this, "Edit title", "Change title to: ", QLineEdit::Normal, this->title, &pressedOK);
    }else{
        userInput = QInputDialog::getText(this, "New title", "New title: ", QLineEdit::Normal, "", &pressedOK);
    }

    if(pressedOK){
        this->setTitle(userInput.replace("\n",""));
        this->saveJSON();
    }
}

//slot
void contentButton::copyContentToClipboard(){
    if(this->content.length() > 0){
        QGuiApplication::clipboard()->setText(this->content);
    }else{
        timedPopUp(this, defaultQuickPopUpTimer, "No content", "Button content is empty.<br>Will not copy to clipboard.");
    }
}

//slot
void contentButton::pasteContentFromClipboard(){
    bool save = true;
    QString clipboardText = QGuiApplication::clipboard()->text();
    if(clipboardText.length() > 0){
        if(this->getContent().length() > 0){
            qDebug() << "There already is text!";
            QMessageBox::StandardButton reply;
            reply = QMessageBox::question(this, "Override confirmation",
                                          "Do you really want to override the existing content for this button?"
                                          "<br><br><i><b><u>WARNING:</u></b> This action is <b>irreversible!</b></i>",
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
        if(this->getContent().length() > 0){
            timedPopUp(this, defaultPopUpTimer, "Invalid clipboard data", "Clipboard does not have valid text data."
                                                                          "<br>Button content won't be overwritten.");
        }else{
            timedPopUp(this, defaultQuickPopUpTimer, "Invalid clipboard data", "Clipboard does not have valid text data.");
        }
    }
}

void contentButton::clearContent(){
    this->content.clear();
    this->contentDisplayed.clear();
    this->disableCopyCutRemoveContent();
}

//slot
void contentButton::cutContentToClipboard(){
    if(this->content.length() > 0){
        QGuiApplication::clipboard()->setText(this->content);
        this->clearContent();
    }else{
        timedPopUp(this, defaultQuickPopUpTimer, "No content", "Button content is empty.<br>Will not cut to clipboard.");
    }
}

//slot
void contentButton::removeContent(){
    if(this->content.length() > 0){
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, "Remove content confirmation",
                                      "Do you really want to remove <b>the content</b> of this button?"
                                      "<br><br><i><b><u>WARNING:</u></b> This action is <b>irreversible!</b></i>",
                                      QMessageBox::No|QMessageBox::Yes, QMessageBox::No);
        if(reply == QMessageBox::Yes){
            this->clearContent();
        }
    }else{
        qDebug() << "button content is already empty, got nothing to delete.";
    }
}

void contentButton::enableCopyCutRemoveContent(){
    this->copyContentAction.setVisible(true);
    if(this->copyContentActionSeparator){
        this->copyContentActionSeparator->setVisible(true);
    }
    this->cutContentAction.setVisible(true);
    if(this->cutContentActionSeparator){
        this->cutContentActionSeparator->setVisible(true);
    }
    this->removeContentAction.setVisible(true);
    if(this->removeContentActionSeparator){
        this->removeContentActionSeparator->setVisible(true);
    }
}

void contentButton::disableCopyCutRemoveContent(){
    this->copyContentAction.setVisible(false);
    if(this->copyContentActionSeparator){
        this->copyContentActionSeparator->setVisible(false);
    }
    this->cutContentAction.setVisible(false);
    if(this->cutContentActionSeparator){
        this->cutContentActionSeparator->setVisible(false);
    }
    this->removeContentAction.setVisible(false);
    if(this->removeContentActionSeparator){
        this->removeContentActionSeparator->setVisible(false);
    }
}

void contentButton::enablePasteContent(){
    this->pasteContentAction.setVisible(true);
    if(this->pasteContentActionSeparator){
        this->pasteContentActionSeparator->setVisible(true);
    }
}

void contentButton::disablePasteContent(){
    this->pasteContentAction.setVisible(false);
    if(this->pasteContentActionSeparator){
        this->pasteContentActionSeparator->setVisible(false);
    }
}

void contentButton::enableMoveButton(){
    this->moveButtonAction.setVisible(true);
    if(this->moveButtonActionSeparator){
        this->moveButtonActionSeparator->setVisible(true);
    }
}

void contentButton::disableMoveButton(){
    this->moveButtonAction.setVisible(false);
    if(this->moveButtonActionSeparator){
        this->moveButtonActionSeparator->setVisible(false);
    }
}

void contentButton::enableDeleteAllMarked(){
    this->deleteAllMarkedAction.setVisible(true);
    if(this->deleteAllMarkedActionSeparator){
        this->deleteAllMarkedActionSeparator->setVisible(true);
    }
}

void contentButton::disableDeleteAllMarked(){
    this->deleteAllMarkedAction.setVisible(false);
    if(this->deleteAllMarkedActionSeparator){
        this->deleteAllMarkedActionSeparator->setVisible(false);
    }
}

//slot
void contentButton::removeTitle(){
    this->setTitle("");
    this->saveJSON();
}

//slot
void contentButton::deleteThisButton(){
    emit this->deleteButton(this->getIndexInList());
}

void contentButton::openOptionsMenu(QPoint const &p){
    this->optionsMenu.popup(p);
}

void contentButton::setIndexInList(qsizetype const index){
    this->indexInList = index;
};

qsizetype contentButton::getIndexInList() const{
    return this->indexInList;
};

void contentButton::saveJSON(){
    emit this->saveButtonChangesIntoJSON();
}

QString contentButton::getTitle() const{
    return this->title;
}

bool contentButton::setTitle(QString const &newTitle){
    if(this->title != newTitle){
        if(newTitle.length() > 0){
            if(newTitle.length() <= this->maxTitleLengthGeneral){
                this->title = newTitle;

                QString titleWithHtml;
                titleWithHtml.append("<body>");
                //we clip very long titles
                if(this->title.length() < this->maxTitleLengthForDisplaying){
                    titleWithHtml.append(this->title);
                }else{
                    titleWithHtml.append(this->title.first(this->maxTitleLengthForDisplaying).append("..."));
                }
                titleWithHtml.replace("\n",""); //remove linebreaks
                titleWithHtml.append("</body>");
                this->titleDisplayed = titleWithHtml;

                this->addOrEditTitleAction.setText(this->textForEditTitleAct);
                this->removeTitleAction.setVisible(true);
                if(this->removeTitleActionSeparator){
                    this->removeTitleActionSeparator->setVisible(true);
                }
            }else{
                QString message = "The maximum amount of characters for titles is "
                                  + QString::number(this->maxTitleLengthGeneral)
                                  + "<br>Current invalid length: "
                                  + QString::number(newTitle.length());
                timedPopUp(this, defaultPopUpTimer, "Too many characters", message);
                return false;
            }
        }else{
            this->title.clear();
            this->titleDisplayed.clear();
            this->addOrEditTitleAction.setText(this->textForNewTitleAct);
            this->removeTitleAction.setVisible(false);
            if(this->removeTitleActionSeparator){
                this->removeTitleActionSeparator->setVisible(false);
            }
        }
        this->titleDoc.setHtml(this->titleDisplayed);
        this->originalTitleWidth = this->titleDoc.size().width();
    }else{
        qDebug() << "Title has not changed.";
    }
    return true;
}

bool contentButton::hasTitle() const{
    return (this->title.length() > 0);
}

QString contentButton::getContent() const{
    return this->content;
}

bool contentButton::setContent(QString const &newContent){
    if(newContent.length() > 0){
        if(newContent.length() <= this->maxContentLengthGeneral){
            this->content = newContent;
            this->enableCopyCutRemoveContent();

            QString contentWithHtml;
            contentWithHtml.append("<body>");
            //for very long content (many characters or many lines), display "..." instead
            if(this->content.count('\n') > this->maxContentLinesForDisplaying
                || this->content.length() > this->maxContentLengthForDisplaying){
                contentWithHtml.append(this->content.first(this->maxContentRemainingDisplayedChars).append("..."));
            }else{
                contentWithHtml.append(this->content);
            }
            contentWithHtml.replace("\n","<br>"); //replace regular linebreaks with html style linebreaks
            contentWithHtml.append("</body>");
            this->contentDisplayed = contentWithHtml;
            this->contentDoc.setHtml(this->contentDisplayed);
        }else{
            QString message = "The maximum amount of characters for content is "
                              + QString::number(this->maxContentLengthGeneral)
                              + "<br>Current invalid length: "
                              + QString::number(newContent.length());
            timedPopUp(this, defaultPopUpTimer, "Too many characters", message);
            return false;
        }
    }else{
        this->removeContent();
    }
    return true;
}

void contentButton::checkIfSearchIsMatched(QString const &searchString){
    if(this->title.contains(searchString)){
        // qDebug() << "Title contains search string: " << searchString;
        this->buttonMatchesSearch = searchStatusMatched;
    }else if(this->content.contains(searchString)){
        // qDebug() << "Content contains search string: " << searchString;
        this->buttonMatchesSearch = searchStatusMatched;
    }else{
        this->buttonMatchesSearch = searchStatusNoMatch;
    }
}

void contentButton::resetSearchStatus(){
    this->buttonMatchesSearch = searchStatusDefault;
}

contentButton::searchStatus contentButton::getSearchStatus() const{
    return this->buttonMatchesSearch;
}

void contentButton::mouseLeftClick(){
    QFocusEvent* focus = new QFocusEvent(QEvent::MouseButtonPress, Qt::MouseFocusReason);
    this->focusInEvent(focus);
    delete focus;
}

void contentButton::mouseRightClick(QMouseEvent const * const event){
    this->openOptionsMenu(event->globalPosition().toPoint());
}

void contentButton::mousePressEvent(QMouseEvent * const event){
    Qt::KeyboardModifiers mod = event->modifiers();
    if(mod == Qt::CTRL || mod == Qt::SHIFT){
        this->switchMarkedForDeletion();
    }else{
        if(event->button() == Qt::RightButton){
            this->mouseRightClick(event);
        }else{
            this->mouseLeftClick();
        }
    }
}

void contentButton::mouseDoubleClickEvent(QMouseEvent *const event){
    Qt::KeyboardModifiers mod = event->modifiers();
    if(mod == Qt::CTRL || mod == Qt::SHIFT){
        this->switchMarkedForDeletion();
    }else{
        if(event->button() == Qt::RightButton){
            this->mouseRightClick(event);
        }else{
            emit this->startContentButtonEdit(this->getIndexInList());
        }
    }
}

void contentButton::focusInEvent(QFocusEvent * const event){
    QWidget::focusInEvent(event);
    Qt::FocusReason r = event->reason();
    // qDebug() << "focusInEvent ; index: " << this->getIndexInList() << " ; reason: " << r;
    if(r == Qt::MouseFocusReason || r == Qt::TabFocusReason
        || r == Qt::BacktabFocusReason || r == Qt::OtherFocusReason){
        this->setAsFocusedButton();
    }
}

void contentButton::focusOutEvent(QFocusEvent * const event){
    QWidget::focusOutEvent(event);
    Qt::FocusReason r = event->reason();
    // qDebug() << "focusOutEvent ; index: " << this->getIndexInList() << " ; reason: " << r;
    if(r == Qt::MouseFocusReason || r == Qt::TabFocusReason
        || r == Qt::BacktabFocusReason || r == Qt::OtherFocusReason){
        this->unsetAsFocusedButton();
    }
}

void contentButton::paintEvent(QPaintEvent * const event){

    QPushButton::paintEvent(event);
    QPainter combinedImage(this);

    if(this->originalTitleWidth < this->width()){
        this->titleDoc.setTextWidth(this->width());
    }else{
        this->titleDoc.setTextWidth(this->originalTitleWidth);
    }

    int dividingHeight = this->title.length() > 0 ? this->titleDoc.size().height() : 0;
    int remainingHeight = this->height() - dividingHeight;

    QPoint pointTitle(0,0);
    QSize sizeTitle(this->width() - 1, dividingHeight);
    QRect rectTitle(pointTitle, sizeTitle);

    QPixmap pixmapTitle(this->titleDoc.size().width(), this->titleDoc.size().height());
    pixmapTitle.fill(Qt::transparent);

    QPainter paintTitle(&pixmapTitle);
    paintTitle.setBrush(Qt::black);
    this->titleDoc.drawContents(&paintTitle, pixmapTitle.rect());
    QPixmap scaledPixmapTitle(pixmapTitle);
    /*
     * problem: when drawing the scaledPixmap as is, it will be
     *          centered, but we want to see it starting from the left
     * solution: crop the scaled pixmap to rectTitle, effectively throwing away anything
     *           that doesn't fit into the rect (== that was overhanging on the right)
     */
    QPixmap croppedPixmapTitle = scaledPixmapTitle.copy(pixmapTitle.rect());
    combinedImage.drawPixmap(rectTitle,croppedPixmapTitle);

    //----------------END OF TITLE PAINTING---------------------
    //----------------------------------------------------------
    //----------------------------------------------------------
    //--------------START OF CONTENT PAINTING-------------------

    QPoint pointContent(0, dividingHeight);

    QPixmap pixmapContent(this->contentDoc.size().width(), this->contentDoc.size().height());
    pixmapContent.fill(Qt::transparent);
    QPainter paintContent(&pixmapContent);
    paintContent.setBrush(Qt::black);
    this->contentDoc.drawContents(&paintContent, pixmapContent.rect());

    if(pixmapContent.rect().height() > remainingHeight){
        QPixmap scaledPixmapContent = pixmapContent.scaledToHeight(remainingHeight, Qt::SmoothTransformation);
        QRect rectForContentCrop(0, 0, this->width() - 1, remainingHeight);
        QPixmap croppedPixmapContent = scaledPixmapContent.copy(rectForContentCrop);
        combinedImage.drawPixmap(pointContent, croppedPixmapContent);
    }else{
        QRect rectForUnscaledUncropped(0, dividingHeight, pixmapContent.rect().width(), pixmapContent.rect().height());
        combinedImage.drawPixmap(rectForUnscaledUncropped, pixmapContent);
    }
}
