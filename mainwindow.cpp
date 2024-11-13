#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QSizePolicy>
#include <QMessageBox>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QSettings>
#include <QSizeGrip>
#include <QDir>
#include <QKeyEvent>

const QString MainWindow::appName = "cliProV1";
const QString MainWindow::appAuthor = "Andreas Getzin";
const QString MainWindow::settingsFile = "settings/cliProSettings.ini";
const QString MainWindow::settingsGroupGeneral = "app";
const QString MainWindow::settingsValWindowWidth = "window_width";
const QString MainWindow::settingsValWindowHeight = "window_height";

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    this->ui->setupUi(this);
    this->setWindowTitle(this->appName);
    this->setFocusPolicy(Qt::StrongFocus); //without this, arrow keys do not work
    this->setMinimumSize(this->minWindowSize_w, this->minWindowSize_h);
    this->loadAppSettings();

    //CREATE dynamic +/- button, BEFORE(!) loadButtonsFromJson
    this->dynBtn = new dynAddRmButton(dynAddRmButton::btnModeADD);

    //this will load all existing profiles into a list of the class
    this->profMenu = new profileMenu(this);
    this->profMenu->setModal(true);

    this->contEdit = new buttonEdit(this);
    this->contEdit->setModal(true);

    connect(this->ui->buttonProfile, SIGNAL(clicked()), this, SLOT(profileButtonClicked()));
    connect(this->profMenu, SIGNAL(selProfileHasChanged(QString)), this, SLOT(updateButtonsForProfileChange(QString)));
    connect(this->profMenu, SIGNAL(newProfileCreated(QString)), this, SLOT(createDefaultJsonForNewProfile(QString)));

    this->currSelectedProfileName = profMenu->getCurrSelProfileName();
    qDebug() << "currSelectedProfileName: " << currSelectedProfileName;

    if(this->currSelectedProfileName == nullptr || this->currSelectedProfileName == ""){
        this->pathToFileForSelectedProfile = "";
        qDebug() << "Profile name is invalid. Load default app settings instead.";
    }else{
        this->pathToFileForSelectedProfile = profileMenu::constructFilePathForProfileJson(this->currSelectedProfileName);
        qDebug() << "pathToFileForSelectedProfile: " << pathToFileForSelectedProfile;

        bool jsonFileExists = QFile::exists(pathToFileForSelectedProfile);
        qDebug() << "File exists? -> " << jsonFileExists;

        //check if file even exists
        if(!jsonFileExists){
            qDebug() << "jsonFile of path " << pathToFileForSelectedProfile + "does not exist";

            //create default file instead
            this->saveDefaultJsonForProfile(pathToFileForSelectedProfile);

            //ToDo init default state of App or something...
            //for now just open the profiles Menu, which forces creation of a profile
            //profMenu->show(); //WARNING does not appear in center of main window, since that is not shown yet (OK for now)
            //loadDefaultAppState();
        }else{
            //loads all existing buttons before ADDING(!) the dynamic +/- button
            this->loadButtonsFromJson();
            qDebug() << "All buttons loaded.";
        }
        this->setDisplayedProfileName(this->currSelectedProfileName);
    }

    qDebug() << "dynBtn exists?" << (this->dynBtn == nullptr ? true : false);

    this->addDynBtnAtEndOfContentButtons();

    connect(this->dynBtn, SIGNAL(released()), this, SLOT(processDynBtnMainAction()));
    connect(this->dynBtn, SIGNAL(keyPressOnDynBtn(int)), this, SLOT(processDynBtnKeyPress(int)));
    connect(this->dynBtn, SIGNAL(mainWindowButtonsNeedSwitch(dynAddRmButton::btnMode)), this, SLOT(adjustButtons(dynAddRmButton::btnMode)));

    this->setUpUnmarkAllBtn();

    connect(this->ui->buttonAdd, SIGNAL(clicked()), this, SLOT(processActionForAddButton()));
    connect(this->ui->buttonSearch, SIGNAL(clicked()), this, SLOT(processActionForSearchButton()));
    connect(this->ui->textInputField, SIGNAL(textChanged(QString)), this, SLOT(processTextFieldChange(QString)));

    this->ui->buttonAdd->setDisabled(true);
    this->ui->buttonSearch->setDisabled(true);

    this->fixTabOrder();
}

MainWindow::~MainWindow()
{
    this->saveAppSettings();
    this->saveCurrentButtonsAsJson();
    delete this->unmarkAllBtn;
    delete this->dynBtn;
    delete this->profMenu;
    delete this->contEdit;
    delete this->ui;
}

void MainWindow::fixTabOrder(){
    qDebug() << "start: Fix Tab Order";
    QWidget::setTabOrder(this->ui->buttonProfile, this->ui->textInputField);
    QWidget::setTabOrder(this->ui->textInputField, this->ui->buttonSearch);
    QWidget::setTabOrder(this->ui->buttonSearch, this->ui->buttonAdd);
    QWidget::setTabOrder(this->ui->buttonAdd, this->unmarkAllBtn);

    if(this->contentBtnList.count() > 0){
        qDebug() << "count() > 0";
        QWidget::setTabOrder(this->unmarkAllBtn, this->contentBtnList.at(0));
        for(qsizetype i = 0; i < this->contentBtnList.count() - 1; ++i){
            qDebug() << "i : " << i;
            QWidget::setTabOrder(this->contentBtnList.at(i), this->contentBtnList.at(i+1));
        }
        QWidget::setTabOrder(this->contentBtnList.last(), this->dynBtn);
    }else{
        QWidget::setTabOrder(this->unmarkAllBtn, this->dynBtn);
    }
    qDebug() << "end: Fix Tab order";
}

void MainWindow::loadAppSettings(){
    QSettings settings(this->settingsFile, QSettings::IniFormat);
    settings.beginGroup(this->settingsGroupGeneral);
    bool ok;
    int width = settings.value(this->settingsValWindowWidth).toInt(&ok);
    if(!ok){
        qDebug() << "read width not OK. Set default value.";
        width = this->defaultWindowSize_w;
    }
    int height = settings.value(this->settingsValWindowHeight).toInt(&ok);
    if(!ok){
        qDebug() << "read height not OK. Set default value.";
        height = this->defaultWindowSize_h;
    }
    this->resize(width, height);
    settings.endGroup();
}

void MainWindow::saveAppSettings(){
    QSettings settings(this->settingsFile, QSettings::IniFormat);
    settings.beginGroup(this->settingsGroupGeneral);
    settings.setValue(this->settingsValWindowWidth, QString::number(this->width()));
    settings.setValue(this->settingsValWindowHeight, QString::number(this->height()));
    settings.endGroup();
}

void MainWindow::setUpUnmarkAllBtn(){
    qDebug() << "start: setUpUnmarkAllBtn";

    //not visible at first
    this->unmarkAllBtn = new QPushButton();
    this->unmarkAllBtn->setText("Cancel: Do not delete selected buttons");
    this->unmarkAllBtn->setStyleSheet("color: darkgreen; border: 1px solid lightgreen; border-radius: 30%; font-weight: bold;"
                                      "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #f6ffed, stop:1 #edfff4)");
    this->unmarkAllBtn->hide();
    this->ui->layoutAtTop->addWidget(this->unmarkAllBtn);

    //the stretch of "26" is a combination of the stretch of the other elements + a tiny bit more to make it look nice
    this->ui->layoutAtTop->setStretch(ui->layoutAtTop->indexOf(this->unmarkAllBtn), 26);
    connect(this->unmarkAllBtn, SIGNAL(clicked()), this, SLOT(unmarkAllContentButtons()));
}

void MainWindow::addDynBtnAtEndOfContentButtons(){

    qDebug() << "start: addDynBtnAtEndOfContentButtons";

    //ToDo check if this part is actually needed (might be an artifact of older code)
    if(this->unmarkAllBtn){
        //ToDo.. takeAt from widget?
        delete this->unmarkAllBtn;
        this->setUpUnmarkAllBtn();
    }

    if(this->dynBtn == nullptr){
        qDebug() << "dynBtn is nullptr, exit!";
        exit(1);
    }else{
        qDebug() << "dnyBtn is OK";
    }
    qsizetype currItemCount = this->contentBtnList.count();
    qDebug() << "currItemCount: " << currItemCount;

    qsizetype row4dynBtn = (currItemCount) / this->maxItemsPerRow;
    qsizetype col4dynBtn = (currItemCount) % this->maxItemsPerRow;
    qDebug() << "row/col: " << row4dynBtn << col4dynBtn;

    this->ui->scrollGrid->addWidget(this->dynBtn, row4dynBtn, col4dynBtn);
    qDebug() << "dynBtn added to widget";

    qDebug() << "count: " << ui->scrollGrid->count()
             << ui->scrollGrid->rowCount() << ","
             << ui->scrollGrid->columnCount() << "\n";

    if(contentBtnCount::getMarkedForDelCnt() > 0){
        qDebug() << "MODE >> RM!";
        this->dynBtn->setMode(dynAddRmButton::btnModeRM);
    }else{
        qDebug() << "MODE >> ADD!";
        this->dynBtn->setMode(dynAddRmButton::btnModeADD);
    }
}

void MainWindow::loadButtonsFromJson(){

    qDebug() << "start: loadButtonsFromJson";
    qDebug() << "Open file: " << this->pathToFileForSelectedProfile;

    QFile file(this->pathToFileForSelectedProfile);

    //ReadOnly -> does not create File if it does not exist
    if(!file.open(QIODevice::ReadOnly)) {
        qDebug("ReadOnly unsuccessful.");
        QMessageBox::information(0, "error", file.errorString());
        exit(EXIT_FAILURE);
    }else{
        qDebug("ReadOnly successful.");
    }

    QTextStream inStream(&file);

    QString fileAsString;
    fileAsString = inStream.readAll();
    file.close();

    qDebug() << "File as string: " << fileAsString << "\n\n";

    QByteArray jsonBA = fileAsString.toLocal8Bit(); //BA = ByteArray
    QJsonDocument jsonDoc =QJsonDocument::fromJson(jsonBA);

    if(jsonDoc.isNull()){
        qDebug() << "Failed to create JSON document.";
        exit(2);
    }
    if(!jsonDoc.isObject()){
        qDebug() << "JSON File @ " << pathToFileForSelectedProfile << " is not a proper JSON Document.";
        exit(3);
    }

    QJsonObject jsonObjAllButtons = jsonDoc.object();

    if(jsonObjAllButtons.isEmpty()){
        qDebug()<<"JSON object is empty.";
        exit(4);
    }

    qDebug() << "Everything good.";

    qDebug() << "jsonObjAllButtons.count(): " << jsonObjAllButtons.count();
    qDebug() << "jsonObjAllButtons.value(): " << jsonObjAllButtons.value("contentButtons");

    //ToDo: combine lines
    QJsonValue contentButtonsArrAsVal = jsonObjAllButtons.value("contentButtons");
    QJsonArray contentButtonsArr = contentButtonsArrAsVal.toArray();

    qDebug() << "contentButtonsArr: " << contentButtonsArr <<
        ".count: " << contentButtonsArr.count() << "\n\n\n";

    for(qsizetype i = 0; i < contentButtonsArr.count(); ++i){

        qDebug() << "i (" << i << ")" << contentButtonsArr.at(i);

        QJsonObject contentButtonObj = contentButtonsArr.at(i).toObject();
        QStringList keyStrList = contentButtonObj.keys();

        qDebug() << "keyStrList: " << keyStrList;

        QJsonValue contentButtonValID = contentButtonObj.value("id");
        QJsonValue contentButtonValTitle = contentButtonObj.value("title");
        QJsonValue contentButtonValContent = contentButtonObj.value("content");

        qDebug() << "contentButtonValID: " << contentButtonValID;
        qDebug() << "contentButtonValContent: " << contentButtonValContent << "\n";

        QString contentButtonValTitleAsStr = contentButtonValTitle.toString();
        QString contentButtonValContentAsStr = contentButtonValContent.toString();

        qsizetype row = i / this->maxItemsPerRow;
        qsizetype col = i % this->maxItemsPerRow;

        qDebug() << "(pre) count: " << this->ui->scrollGrid->count() << " ("
                 << this->ui->scrollGrid->rowCount() << ","
                 << this->ui->scrollGrid->columnCount() << ")";

        qDebug() << "(~~~) insert at: " << " (" << row << "," << col << ")";

        //create and append the new content button to list
        contentButton *newContentBtn = new contentButton(this);

        //ToDo handle title or content empty?

        newContentBtn->setTitle(contentButtonValTitleAsStr);
        newContentBtn->setContent(contentButtonValContentAsStr);

        qDebug() << "contentButton added -> next: add widget to scrollGrid";
        this->ui->scrollGrid->addWidget(newContentBtn, row, col);

        qDebug() << "Widget added -> next: append it to list of cntBtns";
        this->contentBtnList.append(newContentBtn);
        this->updateIndexOfAllButtons();


        qDebug() << "Added to list";
        connect(newContentBtn, SIGNAL(dynBtnSetMode(dynAddRmButton::btnMode)), this->dynBtn, SLOT(setMode(dynAddRmButton::btnMode)));
        connect(newContentBtn, SIGNAL(keyWasPressed(int,qsizetype)), this, SLOT(processContentButtonKeyPress(int,qsizetype)));
        connect(newContentBtn, SIGNAL(startContentButtonEdit(qsizetype)), this, SLOT(startButtonEdit(qsizetype)));
        connect(newContentBtn, SIGNAL(deleteButton(qsizetype)), this, SLOT(processSingleButtonDeletion(qsizetype)));
        qDebug() << "everything has been connected!";
    }
}

void MainWindow::saveCurrentButtonsAsJson(){
    this->saveButtonsAsJson(this->pathToFileForSelectedProfile, this->contentBtnList);
}

void MainWindow::saveDefaultJsonForProfile(QString pathToFile){
    QVector<contentButton*> dummy; //empty list stays empty
    this->saveButtonsAsJson(pathToFile, dummy);
}

void MainWindow::saveButtonsAsJson(QString pathToFile, QVector<contentButton*> listOfBtns){

    //making sure not to safe into an empty string, this case should technically never happen,
    //unless somehow no profile is selected which also should never happen... but there is a chance
    //that it could happen by manually editing the .ini file while the app is already opened (or when it is opened twice)
    if(pathToFile.length() <= 0){
        return;
    }

    //QFile file(path);
    QFile file(pathToFile);
    if(!file.open(QIODevice::WriteOnly)) {
        qDebug("Write unsuccessful.");
        QMessageBox::information(0, "error", file.errorString());
        return;
    }else{
        qDebug("Write successful.");
    }

    QTextStream outStream(&file);
    QJsonDocument jsonDoc{};
    QJsonObject jsonObjAllButtons{};
    QJsonArray jsonArr{};

    for(qsizetype i = 0; i < listOfBtns.count(); ++i){
        qDebug() << "contentButtons to JSON for loop, i: " << i;
        QJsonObject jsonObjBtnTmp{};
        jsonObjBtnTmp.insert("id", i);
        jsonObjBtnTmp.insert("title", listOfBtns.at(i)->getTitle());
        jsonObjBtnTmp.insert("content", listOfBtns.at(i)->getContent());
        jsonArr.append(jsonObjBtnTmp);
    }

    jsonObjAllButtons.insert("contentButtons", jsonArr);
    jsonDoc.setObject(jsonObjAllButtons);
    outStream << jsonDoc.toJson();
    file.close();
}

void MainWindow::unmarkAllContentButtons(){
    for(qsizetype i = 0; i < contentBtnCount::getTotalCnt(); ++i){
        this->contentBtnList.at(i)->unsetMarkedForDeletion();
    }
    this->dynBtn->setMode(dynAddRmButton::btnModeADD);
}

void MainWindow::doDefaultFocus(){
    if(this->contentBtnList.empty()){
        //if the contentBtnList is empty, set the focus on the dynBtn
        this->dynBtn->setFocus();
    }else{
        //if list is not empty, just set the focus on the first button
        this->contentBtnList.at(0)->gainFocus();
    }
}

void MainWindow::processArrowKeyPress(int key, qsizetype indexOfSender){
    qDebug() << "start: processArrowKeyPress (index of sender: " << indexOfSender << ")";

    qsizetype sizeForCalc = contentBtnList.count();
    if(indexOfSender == contentBtnList.count()){
        //expl: if we were called through dynBtn (see processDynBtnKeyPress), then the calculation
        //should include the dynBtn as if it was another contentButton. Otherwise ignore it
        ++sizeForCalc;
    }

    qsizetype newIndex = -1;

    if(key == Qt::Key_Left){
        qDebug() << "left key";
        newIndex = (((indexOfSender - 1) + sizeForCalc) % sizeForCalc);

    }else if(key == Qt::Key_Right){
        qDebug() << "right key";
        newIndex = ((indexOfSender + 1) % sizeForCalc);

    }else if(key == Qt::Key_Down){
        qDebug() << "down key";
        newIndex = indexOfSender + this->maxItemsPerRow;
        qDebug() << "newIndex(tmp): " << newIndex << " (count: " << sizeForCalc << ")";

        //newIndex is out of bounds --> loop around & newIndex will now land on the first row
        if(newIndex >= sizeForCalc){
            qDebug() << "out of bounds!";
            newIndex = newIndex - ((newIndex / this->maxItemsPerRow) * this->maxItemsPerRow);
        }

    }else if(key == Qt::Key_Up){
        qDebug() << "up key";
        newIndex = indexOfSender - this->maxItemsPerRow;
        qDebug() << "newIndex(tmp): " << newIndex << " (count: " << sizeForCalc << ")";

        //newIndex is out of bounds --> loop around & newIndex will now land on the last row
        if(newIndex < 0){
            qDebug() << "out of bounds!";
            newIndex = newIndex + ((sizeForCalc / this->maxItemsPerRow) + 1) * this->maxItemsPerRow;
        }

        //last row might not be filled completely, if newIndex landed on an unoccupied tile, shift it up a single row again
        if(newIndex >= sizeForCalc){
            qDebug() << "still out of bounds!";
            newIndex -= this->maxItemsPerRow;
        }
    }

    qDebug() << "newIndex: " << newIndex;
    if(newIndex >= 0 && newIndex < this->contentBtnList.count()){
        //contentBtnList.at(newIndex)->setAsSelectedButton();
        //contentBtnList.at(newIndex)->setFocus(); //this worked..
        this->contentBtnList.at(newIndex)->gainFocus();
    }else{
        //if index is invalid / out of bounds
        this->doDefaultFocus();
    }

    qDebug() << "end: processArrowKeyPress";
}

void MainWindow::processSingleButtonDeletion(qsizetype indexOfSender){
    qDebug() << "start: processDeleteKeyPress (index of sender: " << indexOfSender << ")";

    QMessageBox::StandardButton reply;

    //ToDo, once buttons have names/labels/titles, include it in the confirmation question's text
    reply = QMessageBox::question(this, "Delete Confirmation", "Do you really want to delete this button?",
                                  QMessageBox::Reset|QMessageBox::No|QMessageBox::Yes, QMessageBox::No);

    if(reply == QMessageBox::Yes){
        qDebug() << "Yes clicked";

        this->removeSelectedButton(indexOfSender);

        //give focus to the button that now takes is in the removed button's place (or dynBtn)
        if(indexOfSender < contentBtnList.count()){
            this->contentBtnList.at(indexOfSender)->gainFocus();
        }else{
            this->dynBtn->setFocus();
        }

    }else if(reply == QMessageBox::No){
        qDebug() << "No clicked";
        ; //do nothing (?)
    }else{
        qDebug() << "Reset clicked";

        contentButton *cntBtnLstItem = this->contentBtnList.at(indexOfSender);
        if(cntBtnLstItem && cntBtnLstItem->isFocused()){
            qDebug() << "Item does exist and is selected right now";
            //clear it from being the selected button
            cntBtnLstItem->unsetAsFocusedButton();
        }
    }
    qDebug() << "end: processDeleteKeyPress";
}

void MainWindow::processMinusKey(){
    if(contentButton::getMarkedForDelCnt() > 0){
        this->processRemoveAllMarkedButtons();
    }else{
        qDebug() << "no buttons marked for deletion. Do nothing.";
    }
}

void MainWindow::processEscapeKey(){
    contentButton::clearFocusedButton();
    this->unmarkAllContentButtons();
}

void MainWindow::processRemainingKeys(int key){
    if(key == Qt::Key_Plus){
        this->processAddANewButton("");
    }else if(key == Qt::Key_Minus){
        this->processMinusKey();
    }else if(key == Qt::Key_Escape){
        this->processEscapeKey();
    }else{
        qDebug() << "No action for pressed key. (" << key << ")";
    }
}

void MainWindow::processDynBtnMainAction(){
    if(dynBtn->getCurrBtnMode() == dynAddRmButton::btnModeADD){
        this->processAddANewButton("");
    }else if(dynBtn->getCurrBtnMode() == dynAddRmButton::btnModeRM){
        this->processRemoveAllMarkedButtons();
    }else{
        qDebug() << "current button mode is invalid.";
    }
}

void MainWindow::processDynBtnKeyPress(int key){
    qDebug() << "start: processDynBtnKeyPress";
    if(key == Qt::Key_Left || key == Qt::Key_Right
        || key == Qt::Key_Up || key == Qt::Key_Down){
        //expl: The dynBtn is always at the very end of all buttons
        //so that's the index we will be working with
        this->processArrowKeyPress(key, this->contentBtnList.count());
    }else if(key == Qt::Key_Return || key == Qt::Key_Enter){
        processDynBtnMainAction();
    }else{
        this->processRemainingKeys(key);
    }
    qDebug() << "end: processDynBtnKeyPress";
}

void MainWindow::processContentButtonKeyPress(int key, qsizetype indexOfSender){
    qDebug() << "start: processKeyPress (index of sender: " << indexOfSender << ")";
    if(indexOfSender >= 0 && indexOfSender < this->contentBtnList.length()){
        if(key == Qt::Key_Left || key == Qt::Key_Right
            || key == Qt::Key_Up || key == Qt::Key_Down){
            this->processArrowKeyPress(key, indexOfSender);
        }else if(key == Qt::Key_Delete || key == Qt::Key_Backspace){
            this->processSingleButtonDeletion(indexOfSender);
        }else{
            this->processRemainingKeys(key);
        }
    }else{
        qDebug() << "index is out of bounds.";
    }
    qDebug() << "end: processKeyPress";
}

void MainWindow::keyPressEvent(QKeyEvent *event){
    qDebug() << "start: key press event! (MainWindow)";

    int key = event->key();
    if(key == Qt::Key_Left || key == Qt::Key_Right
               || key == Qt::Key_Up || key == Qt::Key_Down){
        qDebug() << "Arrow Key!";
        if(!(this->contentBtnList.empty())){
            this->contentBtnList.at(0)->gainFocus();
        }
    }else if(key == Qt::Key_Return || key == Qt::Key_Enter){
        qDebug() << "Enter pressed on MainWindow.";
        if(this->ui->textInputField->hasFocus() || this->ui->buttonAdd->hasFocus()){
            qDebug() << "Text field or add button has focus!";
            this->processActionForAddButton();
        }
    }else{
        this->processRemainingKeys(key);
    }
    qDebug() << "end: Key press event! (MainWindow)";
}

void MainWindow::processActionForAddButton(){
    qDebug() << "Add button clicked.";
    QString currInputFieldText = this->ui->textInputField->text();
    if(!currInputFieldText.isEmpty()){
        qDebug() << "There is text.";
        this->processAddANewButton(currInputFieldText);
        this->ui->textInputField->setText("");
    }else{
        qDebug() << "No text in field.";
    }
    //give focus to the newly added button (will be the last in list)
    this->contentBtnList.last()->gainFocus();
}

void MainWindow::processActionForSearchButton(){
    qDebug() << "Search button clicked";
    QString currText = this->ui->textInputField->text();
    if(!currText.isEmpty()){
        qDebug() << "There is text.";

        //ToDo actually do stuff (implementation details yet TBD)

        this->ui->textInputField->setText("");
    }else{
        qDebug() << "No text in field.";
    }
    this->ui->textInputField->setFocus();
}

void MainWindow::processTextFieldChange(QString text){
    if(!text.isEmpty()){
        qDebug() << "processTextFieldChange (text: " << text << ")";
        this->ui->buttonAdd->setEnabled(true);
        this->ui->buttonSearch->setEnabled(true);
    }else{
        qDebug() << "processTextFieldChange, empty text.";
        this->ui->buttonAdd->setDisabled(true);
        this->ui->buttonSearch->setDisabled(true);
    }
}

void MainWindow::processAddANewButton(QString defaultText){

    //ToDo check if a profile has even been selected before...
    //      or initialize app with "default profile" or something?

    qDebug() << "Add new button";

    qsizetype currItemCount = this->ui->scrollGrid->count();

    qsizetype row = (currItemCount - 1) / this->maxItemsPerRow;
    qsizetype col = (currItemCount - 1) % this->maxItemsPerRow;

    qsizetype row4dynBtn = (currItemCount) / this->maxItemsPerRow;
    qsizetype col4dynBtn = (currItemCount) % this->maxItemsPerRow;

    qDebug() << "(pre) count: " << this->ui->scrollGrid->count() << " ("
             << this->ui->scrollGrid->rowCount() << ","
             << this->ui->scrollGrid->columnCount() << ")";

    qDebug() << "(~~~) insert at: " << " (" << row << "," << col << ")";

    //take the dynBtn, we will re-insert it later
    QLayoutItem *dynButton = this->ui->scrollGrid->takeAt(currItemCount-1);

    //CHECK IF NEEDED
    // if (i.widget())
    //     delete i.widget();
    // if (i.layout())
    //     delete i.layout();

    //create and append the new content button to list
    contentButton *newContentBtn = new contentButton(this);

    newContentBtn->setTitle("");
    if(!defaultText.isEmpty()){
        newContentBtn->setContent(defaultText);
    }else{
        //QString newBtnTxt = QString(QString::number(row) + "," + QString::number(col));
        newContentBtn->setContent("");
    }

    this->ui->scrollGrid->addWidget(newContentBtn, row, col);
    this->contentBtnList.append(newContentBtn);
    this->updateIndexOfAllButtons();

    connect(newContentBtn, SIGNAL(dynBtnSetMode(dynAddRmButton::btnMode)), this->dynBtn, SLOT(setMode(dynAddRmButton::btnMode)));
    connect(newContentBtn, SIGNAL(keyWasPressed(int,qsizetype)), this, SLOT(processContentButtonKeyPress(int,qsizetype)));
    connect(newContentBtn, SIGNAL(startContentButtonEdit(qsizetype)), this, SLOT(startButtonEdit(qsizetype)));
    connect(newContentBtn, SIGNAL(deleteButton(qsizetype)), this, SLOT(processSingleButtonDeletion(qsizetype)));

    //re-insert dynBtn
    this->ui->scrollGrid->addWidget(dynButton->widget(), row4dynBtn, col4dynBtn);

    qDebug() << "(post) count: " << ui->scrollGrid->count() << " ("
             << ui->scrollGrid->rowCount() << ","
             << ui->scrollGrid->columnCount() << ")\n\n";

    this->fixTabOrder();
}

void MainWindow::updateIndexOfAllButtons(){
    for(qsizetype i = 0; i < this->contentBtnList.count(); ++i){
        this->contentBtnList.at(i)->indexInGrid = i; //ToDo "setIndexInGrid(i)"
    }
}

void MainWindow::removeSelectedButton(qsizetype index){

    qDebug() << "start: removeSelectedButton (index: " << index;

    contentButton *cntBtnLstItem = contentBtnList.at(index);

    if(cntBtnLstItem && cntBtnLstItem->isFocused()){
        qDebug() << "Item does exist and is selected right now";

        //delete from list
        this->contentBtnList.erase(contentBtnList.cbegin() + index);
        delete cntBtnLstItem;

        //ToDo ***CHECK IF THIS CODE IS NEEDED***
        // cntBtnLstItem = NULL; //invalidate pointer (for logic that follows)
        //delete from grid
        // QLayoutItem *takenItem = ui->scrollGrid->takeAt(i);
        // delete takenItem;
    }
    this->updateIndexOfAllButtons();
    this->rebuildGrid();
}

void MainWindow::removeAllButtonsThatAreMarkedForDel(){
    qsizetype totalBtnCnt = contentBtnCount::getTotalCnt();

    qsizetype i = 0;
    //i gets incremented in loop body if item(i) is NOT marked for deletion
    //totalBtnCnt gets decremented instead, if item(i) WAS marked for deletion
    while(i < totalBtnCnt){

        qDebug() << "i: " << i;
        contentButton *cntBtnLstItem = contentBtnList.at(i);

        //check if item exists + is marked
        //  1) if yes -> delete the item
        //  2) if no  -> do nothing except i++
        if(cntBtnLstItem && cntBtnLstItem->isMarkedForDeletion()){
            qDebug() << "Item does exist (i: " << i << "totalCnt: " << totalBtnCnt << ")";
            qDebug() << "Item is marked --> DELETE!";

            //delete from list
            this->contentBtnList.erase(contentBtnList.cbegin() + i);
            delete cntBtnLstItem;

            //ToDo ***CHECK IF THIS CODE IS NEEDED***
            // cntBtnLstItem = NULL; //invalidate pointer (for logic that follows)
            //delete from grid
            // QLayoutItem *takenItem = ui->scrollGrid->takeAt(i);
            // delete takenItem;

            --totalBtnCnt;
        }else{
            ++i;
        }
    }
    this->updateIndexOfAllButtons();
    this->rebuildGrid();

    //in case the focused button was among those that got deleted
    if(!(contentButton::isAnyButtonInFocus())){
        this->doDefaultFocus();
    }
}

void MainWindow::deleteAllItemsFromGrid(){
    //REBUILD step 1) delete all existing items (including dynAddRmButton)
    while(this->ui->scrollGrid->count() > 0){
        //ToDo check what this part actually does and if it's needed..... doesn't seem like it
        // QWidget* widget = ui->scrollGrid->itemAt(0)->widget();
        // if(widget){
        //     ui->scrollGrid->removeWidget(widget);
        //     //delete widget;
        // }
        QLayoutItem *item = ui->scrollGrid->takeAt(0);
        if(item){
            // ui->scrollGrid->takeAt(0);
            delete item;
        }
    }
}

void MainWindow::buildGridFromContentList(){
    qDebug() << "ui->scrollGrid->count(): " << this->ui->scrollGrid->count();
    //REBUILD step 2) build the grid anew with all remaining content buttons
    for(qsizetype k = 0; k < contentBtnCount::getTotalCnt(); ++k){
        qsizetype row = k / this->maxItemsPerRow;
        qsizetype col = k % this->maxItemsPerRow;
        qDebug() << "insert at: " << " (" << row << "," << col << ") [z]";
        contentButton *contentBtnToBeAdded = this->contentBtnList.at(k);
        // QString newBtnTxt = QString(QString::number(row) + "," + QString::number(col));
        this->ui->scrollGrid->addWidget(contentBtnToBeAdded, row, col);
        qDebug() << "contentButton added [z]";
    }
}

void MainWindow::rebuildGrid(){
    //REBUILD gridLayout happens in 3 steps:
    //  1) delete all existing items (including dynAddRmButton)
    this->deleteAllItemsFromGrid();
    //  2) build the list anew with all remaining content buttons
    this->buildGridFromContentList();
    //  3) insert the dynAddRmButton back in
    this->addDynBtnAtEndOfContentButtons();
    //  4) tab order needs fix
    this->fixTabOrder();
}


void MainWindow::processRemoveAllMarkedButtons(){
    qDebug() << "start: processRemoveAllMarkedButtons";

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Delete Confirmation", "Do you really want to delete all marked items?",
                                  QMessageBox::Reset|QMessageBox::No|QMessageBox::Yes, QMessageBox::No);

    if(reply == QMessageBox::Yes){
        qDebug() << "No clicked";

        this->removeAllButtonsThatAreMarkedForDel();
        // this->dynBtn->switchMode();

    }else if(reply == QMessageBox::No){
        qDebug() << "No clicked";
        ; //do nothing (?)
    }else{
        qDebug() << "Reset clicked";
        this->unmarkAllContentButtons();
    }
}

//slot
void MainWindow::profileButtonClicked(){
    qDebug() << "start: profile button clicked";
    // profilePicker* profiles = new profilePicker(this);
    // profiles->show();

    //clear button selection before opening the menu
    contentButton::clearFocusedButton();

    if(profMenu){
        qDebug() << "profiles exist";
        this->profMenu->show();
    }else{
        //ToDo this case is probably wrong / unneeded?
        qDebug() << "no profiles..";
    }
    qDebug() << "profiles view should now be active...";
}

void MainWindow::clearContentButtonList(){
    qDebug() << "~~~ (PRE) ~~~ count of contentBtnList: " << this->contentBtnList.count();
    while(this->contentBtnList.count()){
        contentButton* tmpContentButton = this->contentBtnList.takeFirst();
        delete tmpContentButton;
        qDebug() << "~~~ (PRE) ~~~ count of contentBtnList: " << this->contentBtnList.count();
    }
    qDebug() << "~~~ ( PRE ) ~~~ count of scrollGrid items: " << this->ui->scrollGrid->count();
}

void MainWindow::changeProfileName(QString newName){
    // delete currSelectedProfileName;
    // this->currSelectedProfileName = new QString(newName);
    this->currSelectedProfileName = newName;
    this->pathToFileForSelectedProfile = profileMenu::constructFilePathForProfileJson(currSelectedProfileName);
}

void MainWindow::setDisplayedProfileName(QString name){
    this->ui->buttonProfile->setText(name);
}

//ToDo check if profileName is even needed... remove if not
void MainWindow::updateButtonsForProfileChange(QString profileName){
    qDebug() << "start: updateButtonsForProfileChange";

    qDebug() << "received name: " << profileName;

    if(profileName == ""){
        qDebug() << "invalid profileName.. load default.";
        //ToDo load default stuff
    }else{
        if(profileName == this->currSelectedProfileName){
            qDebug() << "Profile is already the current selected one, no change, do nothing.";
        }else{

            this->saveCurrentButtonsAsJson();
            this->clearContentButtonList();
            this->changeProfileName(profileName);

            //ToDo what do when this part fails?
            this->loadButtonsFromJson();
            this->rebuildGrid();

            qDebug() << "~~~ (POST) ~~~ count of contentBtnList: " << contentBtnList.count();
            qDebug() << "~~~ (POST) ~~~ count of scrollGrid items: " << this->ui->scrollGrid->count();

            this->dynBtn->setMode(dynAddRmButton::btnModeADD);
            this->setDisplayedProfileName(this->currSelectedProfileName);
        }
    }
    qDebug() << "end: updateButtonsForProfileChange";
}

void MainWindow::createDefaultJsonForNewProfile(QString profileName){
    qDebug() << "start: createDefaultJsonForNewProfile (profileName: " << profileName << ")";
    this->saveDefaultJsonForProfile(profileMenu::constructFilePathForProfileJson(profileName));
    qDebug() << "end: createDefaultJsonForNewProfile";
}

void MainWindow::startButtonEdit(qsizetype indexOfSender){
    qDebug() << "start: startButtonEdit";
    if(indexOfSender >= 0 && indexOfSender < contentBtnList.count()){
        this->contEdit->editButton(this->contentBtnList.at(indexOfSender));
    }else{
        qDebug() << "Invalid index.";
    }
    qDebug() << "end: startButtonEdit";
}

void MainWindow::mousePressEvent(QMouseEvent *event){
    qDebug() << "start: mousePressEvent";
    contentButton::clearFocusedButton();
    if(this->contentBtnList.empty()){
        this->dynBtn->setFocus();
    }else{
        this->ui->centralwidget->setFocus();
    }
    qDebug() << "end: mousePressEvent";
}

void MainWindow::adjustButtons(dynAddRmButton::btnMode mode){
    qDebug() << "start: adjustButtons";
    if(mode == dynAddRmButton::btnModeADD){
        qDebug() << "ADD";
        this->ui->textInputField->show();
        this->ui->buttonSearch->show();
        this->ui->buttonAdd->show();
        this->unmarkAllBtn->hide();
    }else if(mode == dynAddRmButton::btnModeRM){
        qDebug() << "RM";
        this->ui->textInputField->hide();
        this->ui->buttonSearch->hide();
        this->ui->buttonAdd->hide();
        this->unmarkAllBtn->show();
    }else{
        qDebug() << "mode: " << mode << " is not a valid mode..";
    }
    qDebug() << "end: adjustButtons";
}
