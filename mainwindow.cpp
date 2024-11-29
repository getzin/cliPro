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
#include <QKeyEvent>
#include <QClipboard>
#include <QMimeData>

#include "apputils.h"
#include "movebutton.h"

const QClipboard *MainWindow::clipboard = nullptr;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow),
      dynBtn(this),
      profMenu(this),
      btnEdit(this),
      moveBtnMenu(this),
      unmarkAllBtn(this)
{
    this->ui->setupUi(this);
    this->setWindowTitle(appSettings::appName);
    this->setFocusPolicy(Qt::StrongFocus); //without this, arrow keys do not work
    this->setMinimumSize(this->minWindowSize_w, this->minWindowSize_h);

    profileMenu::createProfilesFolderIfNotExist();

    this->loadAppSettings();
    this->currSelectedProfileName = this->profMenu.getCurrSelProfileName();

    this->loadJsonOrCreateDefault();
    this->setUpUnmarkAllBtn();
    this->addDynBtnAtEndOfContentButtons();
    this->updateProfileSettingsValidity();
    this->fixTabOrder();

    if(this->dynBtn.getCurrBtnMode() == dynAddRmButton::btnModeDISABLED){
        qDebug() << "CURR MODE >> DISABLED! (next: adjust buttons)";
        adjustButtons(dynAddRmButton::btnModeDISABLED);
    }

    this->ui->hSpace1->changeSize(10,0);
    this->ui->hSpace2->changeSize(5,0);
    this->ui->buttonInfo->setToolTipDuration(INT_MAX);
    this->ui->buttonInfo->setCheckable(false);
    this->ui->buttonInfo->setFocusPolicy(Qt::NoFocus);
    this->ui->profileLabel->setFocusPolicy(Qt::NoFocus);
    this->ui->centralwidget->setFocusPolicy(Qt::ClickFocus);
    this->ui->buttonSearch->setCheckable(true);

    //will become "enabled" as soon as there is input in the input field
    this->ui->buttonAdd->setDisabled(true);
    this->ui->buttonSearch->setDisabled(true);

    //unfortunately does not get rid of the "squished together buttons" problem
    //when shrinking mainWindow past a certain size (not sure what is causing it or how to prevent it)
    this->ui->scrollGrid->setHorizontalSpacing(scrollGridSpacing_h);
    this->ui->scrollGrid->setVerticalSpacing(scrollGridSpacing_v);

    connect(&(this->dynBtn), SIGNAL(released()), this, SLOT(processDynBtnMainAction()));
    connect(&(this->dynBtn), SIGNAL(keyPressOnDynBtn(int)), this, SLOT(processDynBtnKeyPress(int)));
    connect(&(this->dynBtn), SIGNAL(mainWindowButtonsNeedSwitch(dynAddRmButton::btnMode)), this, SLOT(adjustButtons(dynAddRmButton::btnMode)));

    connect(&(this->profMenu), SIGNAL(selProfileHasChanged(QString,bool)), this, SLOT(updateButtonsForProfileChange(QString,bool)));
    connect(&(this->profMenu), SIGNAL(newProfileCreated(QString)), this, SLOT(createDefaultJsonForNewProfile(QString)));
    connect(&(this->profMenu), SIGNAL(profMenuRejected()), this, SLOT(restoreLastUnfocused()));

    connect(&(this->moveBtnMenu), SIGNAL(updateButtonPosition(qsizetype,qsizetype)), this, SLOT(moveButtonInList(qsizetype,qsizetype)));

    connect(this->ui->buttonProfile, SIGNAL(clicked()), this, SLOT(profileButtonClicked()));
    connect(this->ui->buttonAdd, SIGNAL(clicked()), this, SLOT(processActionForAddButton()));
    connect(this->ui->buttonSearch, SIGNAL(clicked()), this, SLOT(processActionForSearchButton()));
    connect(this->ui->textInputField, SIGNAL(textChanged(QString)), this, SLOT(processTextFieldChange(QString)));

    if(!clipboard){ clipboard = QApplication::clipboard(); }
    connect(clipboard, SIGNAL(dataChanged()), this, SLOT(processClipBoard()));
    this->processClipBoard(); //process initial clipboard
}

MainWindow::~MainWindow()
{
    this->saveAppSettings();
    this->saveCurrentButtonsAsJson();
    delete this->ui;
}

void MainWindow::updatePlaceholderProfileNameText(){
    if(this->profMenu.getProfilesCount() > 0){
        this->setDisplayedProfileName("<select here>");
    }else{
        this->setDisplayedProfileName("<create here>");
    }
}

void MainWindow::loadJsonOrCreateDefault(){
    qDebug() << "start: loadJsonOrCreateDefault";
    qDebug() << "currSelectedProfileName: " << this->currSelectedProfileName;
    if(this->currSelectedProfileName == nullptr || this->currSelectedProfileName == ""){
        this->pathToFileForSelectedProfile = "";
        this->updatePlaceholderProfileNameText();
    }else{
        this->pathToFileForSelectedProfile = profileMenu::constructFilePathForProfileJson(this->currSelectedProfileName);
        qDebug() << "pathToFileForSelectedProfile: " << this->pathToFileForSelectedProfile;

        bool jsonFileExists = QFile::exists(this->pathToFileForSelectedProfile);
        qDebug() << "File exists? -> " << jsonFileExists;

        //check if file even exists
        if(!jsonFileExists){
            qDebug() << "jsonFile of path " << this->pathToFileForSelectedProfile + "does not exist";

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
}

void MainWindow::fixTabOrder(){
    qDebug() << "start: Fix Tab Order";
    QWidget::setTabOrder(this->ui->buttonProfile, this->ui->textInputField);
    QWidget::setTabOrder(this->ui->textInputField, this->ui->buttonSearch);
    QWidget::setTabOrder(this->ui->buttonSearch, this->ui->buttonAdd);
    QWidget::setTabOrder(this->ui->buttonAdd, &(this->unmarkAllBtn));

    if(this->contentBtnList.count() > 0){
        qDebug() << "count() > 0";
        QWidget::setTabOrder(&(this->unmarkAllBtn), this->contentBtnList.at(0));
        for(qsizetype i = 0; i < this->contentBtnList.count() - 1; ++i){
            qDebug() << "i : " << i;
            QWidget::setTabOrder(this->contentBtnList.at(i), this->contentBtnList.at(i+1));
        }
        QWidget::setTabOrder(this->contentBtnList.last(), &(this->dynBtn));
    }else{
        QWidget::setTabOrder(&(this->unmarkAllBtn), &(this->dynBtn));
    }
    qDebug() << "end: Fix Tab order";
}

void MainWindow::loadAppSettings(){
    QSettings settings(appSettings::settingsFile, QSettings::IniFormat);
    settings.beginGroup(appSettings::settingsGroupMainWindow);
    bool pos_x_ok = false;
    bool pos_y_ok = false;
    bool width_ok = false;
    bool height_ok = false;
    int pos_x = settings.value(appSettings::settingsValMWPosX).toInt(&pos_x_ok);
    int pos_y = settings.value(appSettings::settingsValMWPosY).toInt(&pos_y_ok);
    int width = settings.value(appSettings::settingsValMWWidth).toInt(&width_ok);
    int height = settings.value(appSettings::settingsValMWHeight).toInt(&height_ok);

    if(!pos_x_ok || !pos_y_ok){
        qDebug() << "Read positions values not OK. Move window to (0,0).";
        if(QGuiApplication::screens().count() == 1){
            //place window in center, if either value is not OK
            QSize primaryScreen = QGuiApplication::primaryScreen()->size();
            int screenWidth = primaryScreen.width();
            int screenHeight = primaryScreen.height();
            pos_x = (screenWidth-width)/2;
            pos_y = (screenHeight-height)/2;
        }else{
            //the code above does not create expected results for multi monitor setups
            //--> so just move it to coordinate (0,0) if there are more than 1 screens
            pos_x = 0;
            pos_y = 0;
        }
    }
    this->move(pos_x, pos_y);

    if(!width_ok || width < this->minWindowSize_w){
        qDebug() << "read width not OK. Set default value.";
        width = this->defaultWindowSize_w;
    }
    if(!height_ok || height < this->minWindowSize_h){
        qDebug() << "read height not OK. Set default value.";
        height = this->defaultWindowSize_h;
    }
    this->resize(width, height);
    settings.endGroup();
}

void MainWindow::saveAppSettings(){
    QSettings settings(appSettings::settingsFile, QSettings::IniFormat);
    settings.beginGroup(appSettings::settingsGroupMainWindow);
    settings.setValue(appSettings::settingsValMWWidth, QString::number(this->width()));
    settings.setValue(appSettings::settingsValMWHeight, QString::number(this->height()));
    settings.setValue(appSettings::settingsValMWPosX, QString::number(this->pos().x()));
    settings.setValue(appSettings::settingsValMWPosY, QString::number(this->pos().y()));
    settings.endGroup();
}

void MainWindow::setUpUnmarkAllBtn(){
    qDebug() << "start: setUpUnmarkAllBtn";

    this->unmarkAllBtn.setText("Cancel: Do not delete selected buttons");
    this->unmarkAllBtn.setStyleSheet("color: darkgreen; border: 1px solid lightgreen; border-radius: 30%; font-weight: bold;"
                                      "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #f6ffed, stop:1 #edfff4)");

    //not visible at first
    this->unmarkAllBtn.hide();
    this->ui->layoutAtTop->addWidget(&(this->unmarkAllBtn));

    this->ui->layoutAtTop->setStretch(this->ui->layoutAtTop->indexOf(&(this->unmarkAllBtn)), this->stretchOfUnmarkAllBtn);
    connect(&(this->unmarkAllBtn), SIGNAL(clicked()), this, SLOT(unmarkAllContentButtons()));
}

void MainWindow::addDynBtnAtEndOfContentButtons(){
    qDebug() << "start: addDynBtnAtEndOfContentButtons";

    qsizetype currScrollGridCount = this->ui->scrollGrid->count();
    qDebug() << "currScrollGridCount: " << currScrollGridCount;

    qsizetype row = (currScrollGridCount) / this->maxItemsPerRow;
    qsizetype col = (currScrollGridCount) % this->maxItemsPerRow;
    qDebug() << "dynBtn row: " << row << ", " << col;

    this->ui->scrollGrid->addWidget(&(this->dynBtn), row, col);
    qDebug() << "dynBtn added to widget";

    qDebug() << "(after adding dynBtn) scroll grid count: "
             << this->ui->scrollGrid->count() << " (row count: "
             << this->ui->scrollGrid->rowCount() << ", column count: "
             << this->ui->scrollGrid->columnCount() << ")";
}

void MainWindow::createAndAddNewButton(qsizetype row, qsizetype col, QString title, QString content){
    qDebug() << "(create and add new button) scroll grid count: "
             << this->ui->scrollGrid->count() << " (row count: "
             << this->ui->scrollGrid->rowCount() << ", column count: "
             << this->ui->scrollGrid->columnCount() << ")";

    qDebug() << "insert at: " << " (" << row << "," << col << ")";
    contentButton *newContentBtn = new contentButton(this);
    newContentBtn->setTitle(title.remove('\n'));
    newContentBtn->setContent(content);
    if(this->clipboard->text().length() > 0){
        newContentBtn->enablePasteContent();
    }
    if(this->searchActive){
        newContentBtn->disableMoveButton();
    }
    qDebug() << "contentButton added -> next: add newContentBtn to scrollGrid";

    this->ui->scrollGrid->addWidget(newContentBtn, row, col);
    qDebug() << "newContentBtn added -> next: append it to contentBtnList";

    this->contentBtnList.append(newContentBtn);
    qDebug() << "Added to contentBtnList";

    connect(newContentBtn, SIGNAL(dynBtnSetMode(dynAddRmButton::btnMode)), &(this->dynBtn), SLOT(setMode(dynAddRmButton::btnMode)));
    connect(newContentBtn, SIGNAL(keyWasPressed(int,qsizetype)), this, SLOT(processContentButtonKeyPress(int,qsizetype)));
    connect(newContentBtn, SIGNAL(startContentButtonEdit(qsizetype)), this, SLOT(startButtonEdit(qsizetype)));
    connect(newContentBtn, SIGNAL(deleteButton(qsizetype)), this, SLOT(processSingleButtonDeletion(qsizetype)));
    connect(newContentBtn, SIGNAL(saveButtonChangesIntoJSON()), this, SLOT(saveCurrentButtonsAsJson()));
    connect(newContentBtn, SIGNAL(deleteAllMarkedButtons()), this, SLOT(processRemoveAllMarkedButtons()));
    connect(newContentBtn, SIGNAL(moveButton(qsizetype)), this, SLOT(openMoveButtonMenu(qsizetype)));
    qDebug() << "everything has been connected!";
}

void MainWindow::loadButtonsFromJson(){

    qDebug() << "start: loadButtonsFromJson";

    qDebug() << "currSelectedProfileName: " << this->currSelectedProfileName;
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

    qDebug() << "File as string: " << fileAsString;

    QByteArray jsonBA = fileAsString.toLocal8Bit(); //BA = ByteArray
    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonBA);

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

    QJsonValue contentButtonsArrAsVal = jsonObjAllButtons.value("contentButtons");
    QJsonArray contentButtonsArr = contentButtonsArrAsVal.toArray();

    qDebug() << "contentButtonsArr: " << contentButtonsArr <<
        ".count: " << contentButtonsArr.count();

    for(qsizetype i = 0; i < contentButtonsArr.count(); ++i){

        qDebug() << "i (" << i << ")" << contentButtonsArr.at(i);

        QJsonObject contentButtonObj = contentButtonsArr.at(i).toObject();
        QJsonValue contentButtonValTitle = contentButtonObj.value("title");
        QJsonValue contentButtonValContent = contentButtonObj.value("content");
        qDebug() << "contentButtonValContent: " << contentButtonValContent;

        QString contentButtonValTitleAsStr = contentButtonValTitle.toString();
        QString contentButtonValContentAsStr = contentButtonValContent.toString();

        qsizetype row = i / this->maxItemsPerRow;
        qsizetype col = i % this->maxItemsPerRow;
        this->createAndAddNewButton(row,col,contentButtonValTitleAsStr,contentButtonValContentAsStr);
    }
    this->updateIndexOfAllButtons();
}

void MainWindow::processAddANewButton(QString defaultText){
    qDebug() << "start: processAddANewButton";
    this->ui->scrollGrid->removeWidget(&(this->dynBtn));
    qsizetype row = this->ui->scrollGrid->count() / this->maxItemsPerRow;
    qsizetype col = this->ui->scrollGrid->count() % this->maxItemsPerRow;
    this->createAndAddNewButton(row,col,"",defaultText);
    this->addDynBtnAtEndOfContentButtons();

    qDebug() << "(post) count: "
             << this->ui->scrollGrid->count() << " (row: "
             << this->ui->scrollGrid->rowCount() << ", column:"
             << this->ui->scrollGrid->columnCount();

    this->updateIndexOfAllButtons();
    this->fixTabOrder();
    this->saveCurrentButtonsAsJson(); //ToDo optimize "saveJSONforSingleButton" (or similar)
}

void MainWindow::saveCurrentButtonsAsJson(){
    qDebug() << "start: saveCurrentButtonsAsJson";
    this->saveButtonsAsJson(this->pathToFileForSelectedProfile, this->contentBtnList);
}

void MainWindow::saveDefaultJsonForProfile(QString pathToFile){
    qDebug() << "start: saveDefaultJsonForProfile";
    QVector<contentButton*> dummy; //empty list stays empty
    this->saveButtonsAsJson(pathToFile, dummy);
}

void MainWindow::saveButtonsAsJson(QString pathToFile, QVector<contentButton*> listOfBtns){
    qDebug() << "start: saveButtonsAsJson (pathToFile: " << pathToFile << ")";
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
    qDebug() << "end: saveButtonsAsJson";
}

void MainWindow::unmarkAllContentButtons(){
    for(qsizetype i = 0; i < this->contentBtnList.count(); ++i){
        this->contentBtnList.at(i)->unsetMarkedForDeletion();
    }
    if(this->dynBtn.getCurrBtnMode() == dynAddRmButton::btnModeRM){
        this->dynBtn.setMode(dynAddRmButton::btnModeADD);
        contentButton::restoreLastUnfocusedButtonToFocusedButton();
    }
}

void MainWindow::focusContentButton(contentButton *btn){
    qDebug() << "start: focusContentButton";
    btn->gainFocus();
    this->ui->scrollArea->ensureWidgetVisible(btn);
}

void MainWindow::focusDynBtn(){
    qDebug() << "start: focusDynBtn";
    this->dynBtn.setFocus();
    this->ui->scrollArea->ensureWidgetVisible(&(this->dynBtn));
}

void MainWindow::focusHackSolution(){
    //after adding a new content button, focusing directly on the last content button will not
    //scroll down the screen, but instead always scrolls to the very top. Reason unknown...
    //this hack mostly fixes the issue
    this->contentBtnList.last()->gainFocus();
    this->ui->scrollArea->ensureWidgetVisible(&(this->dynBtn));
}

void MainWindow::doDefaultFocus(){
    qDebug() << "start: doDefaultFocus";
    bool focusHasBeenSet = false;
    if(this->ui->scrollGrid->count() > 0){
        qDebug() << "scrollGrid is NOT empty.";
        //if list is not empty, just set the focus on the first button
        qsizetype index = this->getSearchAdjustedIndexIfSearchIsActive(0);
        if(indexIsInBounds(index,this->contentBtnList.count())){
            this->focusContentButton(this->contentBtnList.at(index));
            focusHasBeenSet = true;
        }
    }

    if(!focusHasBeenSet){
        qDebug() << "scrollGrid is empty. Give focus to dynBtn.";
        this->focusDynBtn();

    }
}

qsizetype MainWindow::getHiddenButtonCount(){
    qsizetype hiddenButtonCount = 0;
    for(qsizetype i = 0; i < this->contentBtnList.count(); ++i){
        if(this->contentBtnList.at(i)->isHidden()){
            ++hiddenButtonCount;
        }
    }
    return hiddenButtonCount;
}

qsizetype MainWindow::getAdjustedIndexOfSenderForSearch(qsizetype indexOfSender){
    //this logic works, but is a bit hard to understand. There might be a simpler way to do this
    qsizetype visibleCount = 0;
    for(qsizetype i = 0; i < this->contentBtnList.count() && visibleCount <= indexOfSender; ++i){
        if(this->contentBtnList.at(i)->isHidden()){
            --indexOfSender;
        }else{
            ++visibleCount;
        }
    }
    return indexOfSender;
}

qsizetype MainWindow::getAdjustedNewIndexForSearch(qsizetype newIndex){
    qDebug() << "start: getAdjustedNewIndexForSearch (newIndex: " << newIndex;
    for(qsizetype i = 0; i < this->contentBtnList.count() && i <= newIndex; i++){
        if(this->contentBtnList.at(i)->isHidden()){
            newIndex++;
        }
    }
    qDebug() << "adjusted index: " << newIndex;
    return newIndex;
}

qsizetype MainWindow::getSearchAdjustedIndexIfSearchIsActive(qsizetype index){
    qDebug() << "start: getSearchAdjustedIndexIfSearchIsActive";
    if(this->searchActive){
        index = this->getAdjustedNewIndexForSearch(index);
    }
    return index;
}

void MainWindow::processArrowKeyPress(int key, qsizetype indexOfSender){
    qDebug() << "start: processArrowKeyPress (index of sender: " << indexOfSender << ")";

    qsizetype sizeForCalc = this->contentBtnList.count();
    if(indexOfSender == this->contentBtnList.count()){
        //expl: if we were called through dynBtn (see processDynBtnKeyPress), then the calculation
        //should include the dynBtn as if it was another contentButton. Otherwise ignore it
        ++sizeForCalc;
    }

    qDebug() << "(before adj.) sizeForCalc: " << sizeForCalc;
    qDebug() << "(before adj.) indexOfSender: " << indexOfSender;
    if(this->searchActive){
        qDebug() << "search is active! adjust size!";
        sizeForCalc -= this->getHiddenButtonCount();
        indexOfSender = this->getAdjustedIndexOfSenderForSearch(indexOfSender);
    }
    qDebug() << "(after adj.) sizeForCalc: " << sizeForCalc;
    qDebug() << "(after adj.) indexOfSender: " << indexOfSender;

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

    qDebug() << "(before adj.) newIndex: " << newIndex;
    if(this->searchActive){
        qDebug() << "search is active! adjust index!";
        newIndex = this->getAdjustedNewIndexForSearch(newIndex);
    }
    qDebug() << "(after adj.) newIndex: " << newIndex;

    if(indexIsInBounds(newIndex, this->contentBtnList.count())){
        this->focusContentButton(this->contentBtnList.at(newIndex));
    }else{
        this->doDefaultFocus();
    }

    qDebug() << "end: processArrowKeyPress";
}

void MainWindow::processSingleButtonDeletion(qsizetype indexOfSender){
    qDebug() << "start: processDeleteKeyPress (index of sender: " << indexOfSender << ")";

    QMessageBox::StandardButton reply;

    //ToDo, once buttons have names/labels/titles, include it in the confirmation question's text
    reply = QMessageBox::question(this, "Delete button confirmation",
                                  "Do you really want to delete <b>this</b> button?"
                                  "<br><br><i><b><u>WARNING:</u></b> This action is <b>irreversible!</b></i>",
                                  QMessageBox::Reset|QMessageBox::No|QMessageBox::Yes, QMessageBox::No);

    if(reply == QMessageBox::Yes){
        qDebug() << "Yes clicked";

        this->removeSelectedButton(indexOfSender);
        this->saveCurrentButtonsAsJson();

        //give focus to the button that now resides in the removed button's place (or dynBtn)
        qDebug() << "give focus to the button that now resides in the removed button's place (or dynBtn)";
        qsizetype adjustedIndex = this->getAdjustedIndexOfSenderForSearch(indexOfSender);
        qDebug() << "indexOfSender: " << indexOfSender;
        qDebug() << "adjustedIndex: " << adjustedIndex;
        qDebug() << "scrollGrid Count: " << this->ui->scrollGrid->count();
        bool focusHasBeenSet = false;
        if(indexIsInBounds(adjustedIndex, this->ui->scrollGrid->count())){
            qDebug() << "index is in bounds";
            QLayoutItem *item = this->ui->scrollGrid->itemAt(adjustedIndex);
            if(item){
                QWidget *wdgt = item->widget();
                if(wdgt && wdgt != &this->dynBtn){
                    contentButton *btn = (contentButton*)wdgt;
                    this->focusContentButton(btn);
                    focusHasBeenSet = true;
                }
            }
        }
        if(!focusHasBeenSet){
            this->focusDynBtn();
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
    qDebug() << "start: processEscapeKey";
    this->ui->textInputField->clearFocus();
    this->dynBtn.clearFocus();
    contentButton::clearFocusedButton();
    contentButton::clearLastUnfocusedButton();
    this->unmarkAllContentButtons();
}

void MainWindow::processRemainingKeys(int key){
    qDebug() << "start: processRemainingKeys";
    if(key == Qt::Key_Plus){
        this->processAddANewButton("");
    }else if(key == Qt::Key_Minus){
        this->processMinusKey();
    }else if(key == Qt::Key_Escape){
        this->processEscapeKey();
    }else if(key == Qt::Key_P){
        contentButton::clearFocusedButton(); //this is done to make it equal to pressing the profile button, otherwise there is different (buggy) behaviour
        this->profMenu.show();
    }else if(key == Qt::Key_S){
        qDebug() << "key == S";
        if(this->searchActive){
            qDebug() << "search is active";
            this->ui->buttonSearch->click(); //basically disables the search
        }else{
            qDebug() << "search is not active";
            this->ui->textInputField->setFocus();
        }
    }else{
        qDebug() << "No action for pressed key. (" << QKeySequence(key).toString() << ")";
    }
}

void MainWindow::processDynBtnMainAction(){
    if(this->checkProfileSettingsValidPopUpIfNot()){
        if(this->dynBtn.getCurrBtnMode() == dynAddRmButton::btnModeADD){
            this->processAddANewButton("");
            this->focusHackSolution();
        }else if(this->dynBtn.getCurrBtnMode() == dynAddRmButton::btnModeRM){
            this->processRemoveAllMarkedButtons();
        }else{
            qDebug() << "button mode is invalid.";
        }
    }
}

void MainWindow::processDynBtnKeyPress(int key){
    qDebug() << "start: processDynBtnKeyPress";
    if(key == Qt::Key_P || this->profileSettingsValid == true){
        if(key == Qt::Key_Left || key == Qt::Key_Right
            || key == Qt::Key_Up || key == Qt::Key_Down){
            //expl: The dynBtn is always at the very end of all buttons
            //so that's the index we will be working with
            this->processArrowKeyPress(key, this->contentBtnList.count());
        }else if(key == Qt::Key_Return || key == Qt::Key_Enter){
            this->processDynBtnMainAction();
        }else{
            this->processRemainingKeys(key);
        }
    }
    qDebug() << "end: processDynBtnKeyPress";
}

void MainWindow::processContentButtonKeyPress(int key, qsizetype indexOfSender){
    qDebug() << "start: processKeyPress (index of sender: " << indexOfSender << ")";
    if(indexIsInBounds(indexOfSender, this->contentBtnList.count())){
        if(key == Qt::Key_Left || key == Qt::Key_Right
            || key == Qt::Key_Up || key == Qt::Key_Down){
            this->processArrowKeyPress(key, indexOfSender);
        }else if(key == Qt::Key_Delete || key == Qt::Key_Backspace){
            this->processSingleButtonDeletion(indexOfSender);
        }else{
            if(key == Qt::Key_Minus && contentButton::getMarkedForDelCnt() == 0){
                this->processSingleButtonDeletion(indexOfSender);
            }else{
                this->processRemainingKeys(key);
            }
        }
    }else{
        qDebug() << "index is out of bounds.";
    }
    qDebug() << "end: processKeyPress";
}

void MainWindow::keyPressEvent(QKeyEvent *event){
    qDebug() << "start: key press event! (MainWindow)";
    int key = event->key();
    if(key == Qt::Key_P || this->profileSettingsValid == true){
        qDebug() << "got key! (" << QKeySequence(key).toString() << ")";
        if(key == Qt::Key_Left || key == Qt::Key_Right
            || key == Qt::Key_Up || key == Qt::Key_Down){
            qDebug() << "Arrow Key!";
            this->doDefaultFocus();
        }else if(key == Qt::Key_Return || key == Qt::Key_Enter){
            qDebug() << "Enter pressed on MainWindow.";
            Qt::KeyboardModifiers mod = event->modifiers();
            if(mod == Qt::CTRL || mod == Qt::SHIFT){
                if(this->ui->textInputField->hasFocus() || this->ui->buttonAdd->hasFocus()){
                    qDebug() << "Text field or add button has focus!";
                    this->processActionForAddButton();
                }
            }else{
                this->ui->buttonSearch->click();
            }
        }else{
            this->processRemainingKeys(key);
        }
    }
    qDebug() << "end: Key press event! (MainWindow)";
}

void MainWindow::processActionForAddButton(){
    qDebug() << "Add button clicked.";
    QString currInputFieldText = this->ui->textInputField->text();
    if(!currInputFieldText.isEmpty()){
        qDebug() << "There is text.";
        this->processAddANewButton(currInputFieldText);
        this->ui->textInputField->setText(""); //keep this before focus logic or else this will cause
                                               //"pseudo focused content buttons" for some strange reason...
        this->focusHackSolution();
    }else{
        qDebug() << "No text in field.";
    }
}

void MainWindow::disableMoveButtonForAllButtons(){
    for(qsizetype i = 0; i < this->contentBtnList.count(); ++i){
        this->contentBtnList.at(i)->disableMoveButton();
    }
}

void MainWindow::enablePasteForAllButtons(){
    for(qsizetype i = 0; i < this->contentBtnList.count(); ++i){
        this->contentBtnList.at(i)->enablePasteContent();
    }
}

void MainWindow::setSearchActive(){
    if(!this->searchActive){
        this->ui->buttonAdd->setDisabled(true);
        this->ui->textInputField->setDisabled(true);
        this->ui->textInputField->setStyleSheet("color: black; border: 1px solid black; border-radius: 30%; font-weight: bold;"
                                                "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #fff178, stop:1 #fff9ae)");
        this->ui->buttonSearch->setStyleSheet("background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #fff178, stop:1 #fff9ae)");
        this->disableMoveButtonForAllButtons();
        this->searchActive = true;
    }
}

void MainWindow::setSearchInactive(){
    if(this->searchActive){
        this->ui->buttonProfile->setEnabled(true);
        this->ui->buttonAdd->setEnabled(true);
        this->ui->textInputField->setEnabled(true);
        this->ui->textInputField->setFocusPolicy(Qt::StrongFocus); //somehow the input field is no longer clickable after search otherwise
        this->ui->textInputField->setStyleSheet("");
        this->ui->buttonSearch->setStyleSheet("");
        this->enableMoveButtonForAllButtons();
        this->searchActive = false;
        qDebug() << "end of setSearchInactive (search was active before)";
    }
}

void MainWindow::checkSearchMatchForAllButtons(QString searchString){
    for(qsizetype i = 0; i < this->contentBtnList.count(); i++){
        this->contentBtnList.at(i)->checkIfSearchIsMatched(searchString);
    }
    this->rebuildGrid();
}

void MainWindow::resetSearchStatusOfAllButtons(){
    for(qsizetype i = 0; i < this->contentBtnList.count(); i++){
        this->contentBtnList.at(i)->resetSearchStatus();
    }
    this->rebuildGrid();
}

void MainWindow::popUpForProfileSettingsInvalid(){
    if(this->profMenu.getProfilesCount() > 0){
        timedPopUp(this, defaultLongPopUpTimer, "No profile selected",
                   "Selection of a profile is required."
                   "<br>Afterwards buttons can be added."
                   "<br>"
                   "<br>Please press <b>select here</b> at the top or"
                   "<br>open the profiles menu by pressing <b>P</b>.");
    }else{
        timedPopUp(this, defaultLongPopUpTimer, "No profiles",
                   "Creation of a new profile is required."
                   "<br>Afterwards buttons can be added."
                   "<br>"
                   "<br>Please press <b>create here</b> at the top or"
                   "<br>open the profiles menu by pressing <b>P</b>.");
    }
}

void MainWindow::updateProfileSettingsValidity(){
    if(this->profMenu.getProfilesCount() > 0 && this->currSelectedProfileName.length() > 0){
        if(this->profileSettingsValid == false){
            if(this->dynBtn.getCurrBtnMode() == dynAddRmButton::btnModeDISABLED){
                this->dynBtn.setMode(dynAddRmButton::btnModeADD);
            }
        }
        this->profileSettingsValid = true;
    }else{
        if(this->profileSettingsValid == true){
            this->dynBtn.setMode(dynAddRmButton::btnModeDISABLED);
        }
        this->profileSettingsValid = false;
    }
}

bool MainWindow::checkProfileSettingsValidPopUpIfNot(){
    if(this->profileSettingsValid){
        return true;
    }else{
        this->popUpForProfileSettingsInvalid();
        return false;
    }
}

void MainWindow::processActionForSearchButton(){
    qDebug() << "Search button clicked";
    QString currInputFieldText = this->ui->textInputField->text();
    if(!currInputFieldText.isEmpty()){
        qDebug() << "There is text.";
        if(this->ui->buttonSearch->isChecked()){
            qDebug() << "Search button is checked.";
            this->checkSearchMatchForAllButtons(currInputFieldText);
            this->setSearchActive();
        }else{
            qDebug() << "Search button is not checked.";
            this->resetSearchStatusOfAllButtons();
            this->setSearchInactive();
        }

        //ToDo check if needed, but they might be a good idea
        // contentButton::clearFocusedButton();
        // contentButton::clearLastUnfocusedButton();

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

void MainWindow::updateIndexOfAllButtons(){
    for(qsizetype i = 0; i < this->contentBtnList.count(); ++i){
        this->contentBtnList.at(i)->setIndexInList(i);
    }
}

void MainWindow::removeSelectedButton(qsizetype index){

    qDebug() << "start: removeSelectedButton (index: " << index;

    contentButton *cntBtnLstItem = this->contentBtnList.at(index);

    if(cntBtnLstItem && cntBtnLstItem->isFocused()){
        qDebug() << "Item does exist and is selected right now";

        //delete from list
        // this->contentBtnList.erase(this->contentBtnList.cbegin() + index);
        this->contentBtnList.remove(index);
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
    qsizetype totalBtnCnt = this->contentBtnList.count();

    qsizetype i = 0;
    //i gets incremented in loop body if item(i) is NOT marked for deletion
    //totalBtnCnt gets decremented instead, if item(i) WAS marked for deletion
    while(i < totalBtnCnt){

        qDebug() << "i: " << i;
        contentButton *cntBtnLstItem = this->contentBtnList.at(i);

        //check if item exists + is marked
        //  1) if yes -> delete the item
        //  2) if no  -> do nothing except i++
        if(cntBtnLstItem && cntBtnLstItem->isMarkedForDeletion()){
            qDebug() << "Item does exist (i: " << i << "totalCnt: " << totalBtnCnt << ")";
            qDebug() << "Item is marked --> DELETE!";

            //delete from list
            // this->contentBtnList.erase(this->contentBtnList.cbegin() + i);
            this->contentBtnList.remove(i);
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
    qDebug() << "start: deleteAllItemsFromGrid";
    for(qsizetype i = 0; i < this->contentBtnList.count(); ++i){
        this->ui->scrollGrid->removeWidget(this->contentBtnList.at(i));
    }
    this->ui->scrollGrid->removeWidget(&(this->dynBtn));
    qDebug() << "scrollGrid row count: " << this->ui->scrollGrid->rowCount();
    qDebug() << "scrollGrid column count: " << this->ui->scrollGrid->columnCount();
}

void MainWindow::buildGridFromContentList(){
    qDebug() << "ui->scrollGrid->count(): " << this->ui->scrollGrid->count();
    qsizetype newIndexInGrid = 0;
    for(qsizetype i = 0; i < this->contentBtnList.count(); ++i){
        qDebug() << "button i: " << i;
        contentButton *contentBtnToBeAdded = this->contentBtnList.at(i);
        // QString newBtnTxt = QString(QString::number(row) + "," + QString::number(col));
        if(contentBtnToBeAdded->getSearchStatus() != contentButton::searchStatusNoMatch){
            qsizetype row = newIndexInGrid / this->maxItemsPerRow;
            qsizetype col = newIndexInGrid % this->maxItemsPerRow;
            qDebug() << "Button's search status is Matched/Default --> Add it to grid!";
            qDebug() << "insert at: " << " (" << row << "," << col << ")";
            this->ui->scrollGrid->addWidget(contentBtnToBeAdded, row, col);
            contentBtnToBeAdded->show();
            qDebug() << "contentButton added";
            ++newIndexInGrid;
        }else{
            qDebug() << "Button's search status is No Match --> DO NOT add it to grid!";
            contentBtnToBeAdded->hide();
        }
    }
}

void MainWindow::rebuildGrid(){
    this->deleteAllItemsFromGrid();
    this->buildGridFromContentList();
    this->addDynBtnAtEndOfContentButtons();
    this->fixTabOrder();
}

void MainWindow::processRemoveAllMarkedButtons(){
    qDebug() << "start: processRemoveAllMarkedButtons";

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Delete ALL MARKED buttons confirmation",
                                  "Do you really want to delete <b>ALL MARKED</b> buttons?"
                                  "<br><br><i><b><u>WARNING:</u></b> This action is <b>irreversible!</b></i>",
                                  QMessageBox::Reset|QMessageBox::No|QMessageBox::Yes, QMessageBox::No);

    if(reply == QMessageBox::Yes){
        qDebug() << "No clicked";

        this->removeAllButtonsThatAreMarkedForDel();
        this->saveCurrentButtonsAsJson();

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

    //clear button selection before opening the menu
    this->profMenu.show();

    qDebug() << "profiles view should now be active...";
}

void MainWindow::clearContentButtonList(){
    qDebug() << "(clear list) count of contentBtnList: " << this->contentBtnList.count();
    while(this->contentBtnList.count()){
        contentButton* tmpContentButton = this->contentBtnList.takeFirst();
        delete tmpContentButton;
        qDebug() << "count of contentBtnList: " << this->contentBtnList.count();
    }
    qDebug() << "(clear list) count of scrollGrid items: " << this->ui->scrollGrid->count();
}

void MainWindow::changeProfileName(QString newName){
    this->currSelectedProfileName = newName;
    this->pathToFileForSelectedProfile = profileMenu::constructFilePathForProfileJson(currSelectedProfileName);
}

void MainWindow::setDisplayedProfileName(QString name){
    this->ui->buttonProfile->setText(name);
}

void MainWindow::updateButtonsForProfileChange(QString profileName, bool currentActiveProfHasBeenDeleted){
    qDebug() << "start: updateButtonsForProfileChange";
    qDebug() << "received name: " << profileName;
    qDebug() << "currentActiveProfHasBeenDeleted: " << currentActiveProfHasBeenDeleted;
    if(profileName == this->currSelectedProfileName && currentActiveProfHasBeenDeleted == false){
        qDebug() << "Profile is already the current selected one, no change, do nothing.";
        if(profileName == ""){
            this->updatePlaceholderProfileNameText();
        }
    }else{
        if(this->profileSettingsValid){
            if(currentActiveProfHasBeenDeleted == false){
                qDebug() << "current active profile has NOT been deleted";
                if(this->profMenu.checkIfProfileIsInList(this->currSelectedProfileName)){
                    qDebug() << "current active profile is still in list --> save current buttons!";
                    this->saveCurrentButtonsAsJson();
                }else{
                    qDebug() << "current active profile is no longer in list of profiles --> skip saving buttons";
                }
            }else{
                qDebug() << "current active profile HAS been deleted --> skip saving buttons";
            }
        }else{
            qDebug() << "previous profileSettings were invalid. No need to save.";
        }
        this->clearContentButtonList();
        this->changeProfileName(profileName);

        //ToDo what do when this part fails?
        this->loadJsonOrCreateDefault();
        this->rebuildGrid();

        qDebug() << "(update buttons) count of contentBtnList: " << this->contentBtnList.count();
        qDebug() << "(update buttons) count of scrollGrid items: " << this->ui->scrollGrid->count();
    }

    if(this->searchActive){
        qDebug() << "Search is active!";
        this->ui->buttonSearch->click(); //basically disables the search
        this->setSearchInactive();
    }
    this->updateProfileSettingsValidity();
    this->ui->buttonProfile->setFocus();
    // this->ui->scrollArea->scroll(0,0); //scroll to top
    qDebug() << "end: updateButtonsForProfileChange";
}

void MainWindow::createDefaultJsonForNewProfile(QString profileName){
    qDebug() << "start: createDefaultJsonForNewProfile (profileName: " << profileName << ")";
    this->saveDefaultJsonForProfile(profileMenu::constructFilePathForProfileJson(profileName));
    qDebug() << "end: createDefaultJsonForNewProfile";
}

void MainWindow::startButtonEdit(qsizetype indexOfSender){
    qDebug() << "start: startButtonEdit";
    if(indexIsInBounds(indexOfSender, this->contentBtnList.count())){
        this->btnEdit.editButton(this->contentBtnList.at(indexOfSender));
    }else{
        qDebug() << "Invalid index.";
    }
    qDebug() << "end: startButtonEdit";
}

void MainWindow::openMoveButtonMenu(qsizetype indexOfSender){
    qDebug() << "start: openMoveButtonMenu";
    if(!(this->searchActive)){
        this->moveBtnMenu.openMenu(indexOfSender, contentButton::getTotalCnt()-1);
    }else{
        qDebug() << "Not allowed to open move button menu while search is active";
    }
}

void MainWindow::moveButtonInList(qsizetype oldIndex, qsizetype newIndex){
    qDebug() << "start: moveButtonInList";
    if(indexIsInBounds(oldIndex, this->contentBtnList.count())
        && indexIsInBounds(newIndex, this->contentBtnList.count())){
        contentButton *btn = this->contentBtnList.takeAt(oldIndex);
        this->contentBtnList.insert(newIndex, btn);
    }
    this->updateIndexOfAllButtons();
    this->rebuildGrid();
}

void MainWindow::mousePressEvent(QMouseEvent *event){
    Q_UNUSED(event);
    qDebug() << "start: mousePressEvent";
    contentButton::clearFocusedButton();
    contentButton::clearLastUnfocusedButton();
    if(this->contentBtnList.empty()){
        this->focusDynBtn();
    }else{
        this->ui->centralwidget->setFocus();
    }
    qDebug() << "end: mousePressEvent";
}

void MainWindow::adjustMenuOfContentButtons(dynAddRmButton::btnMode mode){
    qDebug() << "start: adjustMenuOfContentButtons";
    if(mode == dynAddRmButton::btnModeADD){
        for(qsizetype i = 0; i < this->contentBtnList.count(); ++i){
            this->contentBtnList.at(i)->disableDeleteAllMarked();
        }
    }else if(mode == dynAddRmButton::btnModeRM){
        for(qsizetype i = 0; i < this->contentBtnList.count(); ++i){
            this->contentBtnList.at(i)->enableDeleteAllMarked();
        }
    }
}

void MainWindow::adjustButtons(dynAddRmButton::btnMode mode){
    qDebug() << "start: adjustButtons";
    this->adjustMenuOfContentButtons(mode);
    if(mode == dynAddRmButton::btnModeADD){
        qDebug() << "ADD (in adjustButtons)";
        this->ui->textInputField->show();
        this->ui->buttonSearch->show();
        this->ui->buttonAdd->show();
        this->unmarkAllBtn.hide();
    }else if(mode == dynAddRmButton::btnModeRM){
        qDebug() << "RM (in adjustButtons)";
        this->ui->textInputField->hide();
        this->ui->buttonSearch->hide();
        this->ui->buttonAdd->hide();
        this->unmarkAllBtn.show();
    }else if(mode == dynAddRmButton::btnModeDISABLED){
        qDebug() << "DISABLED (in adjustButtons)";
        this->ui->textInputField->hide();
        this->ui->buttonSearch->hide();
        this->ui->buttonAdd->hide();
        this->unmarkAllBtn.hide();
    }else{
        qDebug() << "mode: " << mode << " is not a valid mode..";
    }
    qDebug() << "end: adjustButtons";
}

void MainWindow::restoreLastUnfocused(){
    qDebug() << "restoreLastUnfocused!";
    contentButton::restoreLastUnfocusedButtonToFocusedButton();
}

void MainWindow::enableMoveButtonForAllButtons(){
    for(qsizetype i = 0; i < this->contentBtnList.count(); ++i){
        this->contentBtnList.at(i)->enableMoveButton();
    }
}

void MainWindow::disablePasteForAllButtons(){
    for(qsizetype i = 0; i < this->contentBtnList.count(); ++i){
        this->contentBtnList.at(i)->disablePasteContent();
    }
}

void MainWindow::processClipBoard(){
    qDebug() << "start: process ClipBoard";
    const QMimeData *mimeData = clipboard->mimeData();
    if(mimeData->hasText()){
        qDebug() << "Clipboard data is text.";
        this->enablePasteForAllButtons();
    }else if(mimeData->hasImage()){
        qDebug() << "Clipboard data is image.";
        //ToDo (image paste implementation pending, for now disable pasting)
        this->disablePasteForAllButtons();
    }else{
        qDebug() << "Clipboard data is neither text not image.";
        this->disablePasteForAllButtons();
    }
}
