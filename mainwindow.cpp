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


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    this->ui->setupUi(this);

    this->setWindowTitle(appName);


    //does not work
    // QSizeGrip sizeGrip(this);

    //CREATE dynamic +/- button, BEFORE(!) loadButtonsFromJson
    // (each content button will hold a reference to the dynBtn)
    // getOrCreateDynBtn();
    this->dynBtn = new dynAddRmButton(dynAddRmButton::btnModeADD);

    //this will load all existing profiles into a list of the class
    this->profMenu = new profileMenu(this);
    this->profMenu->setModal(true);

    this->contEdit = new contentEdit(this);
    this->contEdit->setModal(true);

    connect(this->ui->buttonProfile, SIGNAL(clicked()), this, SLOT(profileButtonClicked()));
    connect(this->profMenu, SIGNAL(selProfileHasChanged(QString)), this, SLOT(updateButtonsForProfileChange(QString)));
    connect(this->profMenu, SIGNAL(newProfileCreated(QString)), this, SLOT(createDefaultJsonForNewProfile(QString)));



    this->currSelectedProfileName = profMenu->getCurrSelProfileName();
    qDebug() << "currSelectedProfileName: " << currSelectedProfileName;

    if(currSelectedProfileName == nullptr || currSelectedProfileName == ""){
        pathToFileForSelectedProfile = "";
        qDebug() << "Profile name is invalid. Load default app settings instead.";
    }else{
        // pathToFileForSelectedProfile = QDir::currentPath() + "/" + (*(this->currSelectedProfileName)) + ".json";
        pathToFileForSelectedProfile = profileMenu::constructFilePathForProfileJson(this->currSelectedProfileName);

        qDebug() << pathToFileForSelectedProfile;

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


    qDebug() << "****RIGHT BEFORE ADDING DYN BTN***";
    qDebug() << "****RIGHT BEFORE ADDING DYN BTN***";
    qDebug() << "****RIGHT BEFORE ADDING DYN BTN***";

    qDebug() << (dynBtn == nullptr ? true : false);

    this->addDynBtnAtEndOfContentButtons();
    qDebug() << "****RIGHT AFTER ADDING DYN BTN***";
    qDebug() << "****RIGHT AFTER ADDING DYN BTN***";
    qDebug() << "****RIGHT AFTER ADDING DYN BTN***";

    connect(this->dynBtn, SIGNAL(released()), this, SLOT(processDynBtnAction()));
    connect(this->dynBtn, SIGNAL(dynBtnEnterKey()), this, SLOT(processDynBtnAction()));
    connect(this->dynBtn, SIGNAL(mainWindowButtonsNeedSwitch(dynAddRmButton::btnMode)), this, SLOT(adjustButtons(dynAddRmButton::btnMode)));

    //not visible at first
    this->unmarkAllBtn = new QPushButton();
    this->unmarkAllBtn->setText("Cancel: Do not delete selected buttons");
    // QFont font("Monospace");
    // font.setStyleHint(QFont::TypeWriter, QFont::PreferBitmap);
    // font.setWeight(QFont::Bold);
    // font.setPointSize(10);
    // font.setBold(true);
    // this->unmarkAllBtn->setFont(font);
    qDebug() << "S E T   S T Y L E";
    this->unmarkAllBtn->setStyleSheet("color: darkgreen; border: 1px solid lightgreen; border-radius: 30%; font-weight: bold;"
                                      "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #f6ffed, stop:1 #edfff4)");
    qDebug() << "/ S E T   S T Y L E";
    this->unmarkAllBtn->hide();
    this->ui->layoutAtTop->addWidget(this->unmarkAllBtn);

    //the stretch of "26" is a combination of the stretch of the other elements + a tiny bit more to make it look nice
    ui->layoutAtTop->setStretch(ui->layoutAtTop->indexOf(this->unmarkAllBtn), 26);
    connect(this->unmarkAllBtn, SIGNAL(clicked()), this, SLOT(unmarkAllCntButtons()));


    //create the profile button, add it to the scrollGrid and connect
    //ToDo get last profile name, and set it
    // this->profileBtn = new profileButton(ui->centralwidget);
    // this->profileBtn->setText("PROFILES");

    // // connect(this->profileBtn, SIGNAL(clicked()), this, SLOT(profileButtonClicked()));

    // this->ui->layoutAtTop->insertWidget(2, profileBtn);
    // // connect(this->ui->buttonProfile, SIGNAL(clicked()), this, SLOT(profileButtonClicked()));

    //loadAppSettings();
}

MainWindow::~MainWindow()
{
    saveCurrentButtonsAsJson();
    //saveAppSettings();
    delete dynBtn;
    // delete profileBtn;
    delete profMenu;
    delete contEdit;
    // delete currSelectedProfileName;
    delete ui;
}

// dynAddRmButton* MainWindow::getOrCreateDynBtn(){
//     qDebug() << " --GET OR CREATE DYNBTN-- ";
//     if(dynBtn == nullptr){
//         qDebug() << "Create new dynBtn!";
//         this->dynBtn = new dynAddRmButton(dynAddRmButton::btnModeADD);
//     }
//     return (this->dynBtn);
// }

void MainWindow::addDynBtnAtEndOfContentButtons(){
    //ADD dynamic +/- button to the end of scrollGrid widget and connect
    if(dynBtn == nullptr){
        qDebug() << "dynBtn is nullptr, exit!";
        exit(1);
    }else{
        qDebug() << "dnyBtn is OK";
    }
    int currItemCount = contentBtnList.count();
    qDebug() << "currItemCount: " << currItemCount;

    int rowAdd = (currItemCount) / this->maxItemsPerRow;
    int colAdd = (currItemCount) % this->maxItemsPerRow;
    qDebug() << "   row/col: " << rowAdd << colAdd;

    this->ui->scrollGrid->addWidget(dynBtn, rowAdd, colAdd);
    qDebug() << "dynBtn added to widget";

    // connect(dynBtn, SIGNAL(released()), this, SLOT(dynBtnWasPressed())); //moved elsewhere

    qDebug() << "(CTOR) count: " << ui->scrollGrid->count()
             << ui->scrollGrid->rowCount() << ","
             << ui->scrollGrid->columnCount() << "\n";
}

void MainWindow::loadButtonsFromJson(){

    qDebug() << "***** LOAD JSON *****";
    qDebug() << "Open file: " << pathToFileForSelectedProfile;

    // QFile file(path);
    QFile file(pathToFileForSelectedProfile);

    //ReadOnly -> does not create File if it does not exist
    if(!file.open(QIODevice::ReadOnly)) {
        qDebug("ReadOnly unsuccessful.");
        QMessageBox::information(0, "error", file.errorString());
        exit(EXIT_FAILURE);
    }else{
        qDebug("ReadOnly successful.");
    }

    QTextStream in(&file);

    //qDebug() << in.readAll() << "\n\n";

    // QTextStream file_text(&file_obj);
    QString fileAsString;
    fileAsString = in.readAll();
    file.close();

    qDebug() << "File as string: " << fileAsString << "\n\n";

    QByteArray jsonBA = fileAsString.toLocal8Bit(); //BA = ByteArray

    QJsonDocument jsonDoc =QJsonDocument::fromJson(jsonBA);

    if(jsonDoc.isNull()){
        qDebug() << "Failed to create JSON document.";
        exit(2);
    }
    if(!jsonDoc.isObject()){
        // qDebug() << "JSON File @ " << path << " is not a proper JSON Document.";
        qDebug() << "JSON File @ " << pathToFileForSelectedProfile << " is not a proper JSON Document.";
        exit(3);
    }

    QJsonObject jsonObjAllButtons = jsonDoc.object();

    if(jsonObjAllButtons.isEmpty()){
        qDebug()<<"JSON object is empty.";
        exit(4);
    }

    qDebug() << "Everything good.";

    qDebug() << "JSON .count(): " << jsonObjAllButtons.count();
    // qDebug() << "JSON .key(): " << jsonObjAllButtons.keys();
    qDebug() << "JSON .value(): " << jsonObjAllButtons.value("contentButtons");

    //ToDo: combine lines
    QJsonValue contentButtonsArrAsVal = jsonObjAllButtons.value("contentButtons");
    QJsonArray contentButtonsArr = contentButtonsArrAsVal.toArray();

    qDebug() << "contentButtonsArr: " << contentButtonsArr <<
        "  -- .count: " << contentButtonsArr.count() << "\n\n\n";

    // QJsonArray jsonArr = jsonObjAllButtons.value(QString("contentButtons"));

    for(int i = 0; i < contentButtonsArr.count(); i++){

        qDebug() << "i (" << i << ")" << contentButtonsArr.at(i);

        QJsonObject contentButtonObj = contentButtonsArr.at(i).toObject();

        QStringList keyStrList = contentButtonObj.keys();

        qDebug() << keyStrList;

        QJsonValue contentButtonValID = contentButtonObj.value("id");
        QJsonValue contentButtonValContent = contentButtonObj.value("content");

        qDebug() << contentButtonValID;
        qDebug() << contentButtonValContent << "\n";

        QString contentButtonValContentAsStr = contentButtonValContent.toString();

        int row = i / this->maxItemsPerRow;
        int col = i % this->maxItemsPerRow;

        qDebug() << "(pre) count: " << ui->scrollGrid->count() << " ("
                 << ui->scrollGrid->rowCount() << ","
                 << ui->scrollGrid->columnCount() << ")";

        qDebug() << "(~~~) insert at: " << " (" << row << "," << col << ") [x]";

        //create and append the new content button to list
        contentButton *newContentBtn = new contentButton();

        if(contentButtonValContentAsStr.isEmpty()){
            //ToDo: do something for this case... whatever it is...
            //      maybe just skip this iteration and go to next element?
            newContentBtn->setText("ERROR");
        }else{
            newContentBtn->setText(contentButtonValContentAsStr);
        }
        qDebug() << "contentButton added [x] -> next: add widget to scrollGrid";
        ui->scrollGrid->addWidget(newContentBtn, row, col);

        qDebug() << "Widget added -> next: append it to list of cntBtns";
        contentBtnList.append(newContentBtn);
        this->updateIndexOfAllButtons();


        qDebug() << "Added to list";
        connect(newContentBtn, SIGNAL(dynBtnNeedsSwitch()), dynBtn, SLOT(switchMode()));
        connect(newContentBtn, SIGNAL(keyWasPressed(int,int)), this, SLOT(processKeyPress(int,int)));
        connect(newContentBtn, SIGNAL(cntButtonWasLeftClicked()), this, SLOT(unmarkAllCntButtons()));
        connect(newContentBtn, SIGNAL(startContentButtonEdit(int)), this, SLOT(openContentEdit(int)));

        qDebug() << "connected!";
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
    if(pathToFile.length() == 0){
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

    QTextStream out(&file);
    QJsonDocument jsonDoc{};
    QJsonObject jsonObjAllButtons{};
    QJsonArray jsonArr{};

    for(int i = 0; i < listOfBtns.count(); i++){
        qDebug() << "contentButtons to JSON for loop, i: " << i;
        QJsonObject jsonObjBtnTmp{};
        jsonObjBtnTmp.insert("id", i);
        jsonObjBtnTmp.insert("content", listOfBtns.at(i)->text());
        jsonArr.append(jsonObjBtnTmp);
    }

    jsonObjAllButtons.insert("contentButtons", jsonArr);
    jsonDoc.setObject(jsonObjAllButtons);
    out << jsonDoc.toJson();
    file.close();
}

void MainWindow::unmarkAllCntButtons(){
    int totalCnt = contentBtnCount::getTotalCnt();
    for(int i = 0; i < totalCnt; i++){
        contentButton *currBtn = contentBtnList.at(i);
        if(currBtn->getIsMarked()){
            currBtn->setUnmarked();
        }
    }
    dynBtn->setMode(dynAddRmButton::btnModeADD);
}

void MainWindow::processArrowKeyPress(int key, int indexOfSender){
    qDebug() << "processArrowKeyPress (index of sender: " << indexOfSender << ")";

    contentBtnList.at(indexOfSender)->unsetAsSelectedButton();

    int newIndex = -1;

    if(key == Qt::Key_Left){
        qDebug() << "left key";
        newIndex = (((indexOfSender - 1) + contentBtnList.count()) % contentBtnList.count());

    }else if(key == Qt::Key_Right){
        qDebug() << "right key";
        newIndex = ((indexOfSender + 1) % contentBtnList.count());

    }else if(key == Qt::Key_Down){
        qDebug() << "down key";
        newIndex = indexOfSender + maxItemsPerRow;
        qDebug() << "newIndex(tmp): " << newIndex << " (count: " << contentBtnList.count() << ")";

        //newIndex is out of bounds --> loop around & newIndex will now land on the first row
        if(newIndex >= contentBtnList.count()){
            qDebug() << "out of bounds!";
            newIndex = newIndex - ((newIndex / maxItemsPerRow) * maxItemsPerRow);
        }

    }else if(key == Qt::Key_Up){
        qDebug() << "up key";
        newIndex = indexOfSender - maxItemsPerRow;
        qDebug() << "newIndex(tmp): " << newIndex << " (count: " << contentBtnList.count() << ")";

        //newIndex is out of bounds --> loop around & newIndex will now land on the last row
        if(newIndex < 0){
            qDebug() << "out of bounds!";
            newIndex = newIndex + ((contentBtnList.count() / maxItemsPerRow) + 1) * maxItemsPerRow;
        }

        //last row might not be filled completely, if newIndex landed on an unoccupied tile, shift it up a single row again
        if(newIndex >= contentBtnList.count()){
            qDebug() << "still out of bounds!";
            newIndex -= maxItemsPerRow;
        }
    }

    qDebug() << "newIndex: " << newIndex;
    if(newIndex >= 0 && newIndex < contentBtnList.count()){
        //contentBtnList.at(newIndex)->setAsSelectedButton();
        contentBtnList.at(newIndex)->setFocus();
    }

    qDebug() << "/processArrowKeyPress";
}

void MainWindow::processDeleteKeyPress(int indexOfSender){
    qDebug() << "processDeleteKeyPress (index of sender: " << indexOfSender << ")";

    QMessageBox::StandardButton reply;

    //ToDo, once buttons have names/labels/titles, include it in the confirmation question's text
    reply = QMessageBox::question(this, "Delete Confirmation", "Do you really want to delete this button?",
                                  QMessageBox::Reset|QMessageBox::No|QMessageBox::Yes);

    if(reply == QMessageBox::Yes){
        qDebug() << "Yes clicked";

        this->removeSelectedButton(indexOfSender);

    }else if(reply == QMessageBox::No){
        qDebug() << "No clicked";
        ; //do nothing (?)
    }else{
        qDebug() << "Reset clicked";

        contentButton *cntBtnLstItem = contentBtnList.at(indexOfSender);
        if(cntBtnLstItem && cntBtnLstItem->getIsSelected()){
            qDebug() << "  Item does exist and is selected right now";
            //clear it from being the selected button
            cntBtnLstItem->unsetAsSelectedButton();
        }
    }
    qDebug() << "/processDeleteKeyPress";

}

void MainWindow::processKeyPress(int key, int indexOfSender){
    qDebug() << "processKeyPress (index of sender: " << indexOfSender << ")";

    if(indexOfSender >= 0 && indexOfSender < contentBtnList.length()){
        if(key == Qt::Key_Left || key == Qt::Key_Right || key == Qt::Key_Up || key == Qt::Key_Down){
            processArrowKeyPress(key, indexOfSender);
        }else if(key == Qt::Key_Delete || key == Qt::Key_Backspace){
            processDeleteKeyPress(indexOfSender);
        }
    }else{
        qDebug() << "index is out of bounds.";
    }

    qDebug() << "/processKeyPress";

}

void MainWindow::dynBtnProcessForModeAdd(){

    //when adding a new button, any previous
    //button selection becomes invalid, clear
    contentButton::clearButtonSelection();


    //ToDo check if a profile has even been selected before...
    //      or initialize app with "default profile" or something?


    int currItemCount = ui->scrollGrid->count();

    qDebug() << "ADD NEW TILE!";

    int row = (currItemCount - 1) / this->maxItemsPerRow;
    int col = (currItemCount - 1) % this->maxItemsPerRow;

    int rowAdd = (currItemCount) / this->maxItemsPerRow;
    int colAdd = (currItemCount) % this->maxItemsPerRow;

    qDebug() << "(pre) count: " << ui->scrollGrid->count() << " ("
             << ui->scrollGrid->rowCount() << ","
             << ui->scrollGrid->columnCount() << ")";

    qDebug() << "(~~~) insert at: " << " (" << row << "," << col << ") [y]";

    QString newBtnTxt = QString(QString::number(row) + "," + QString::number(col));

    QLayoutItem *currItem = ui->scrollGrid->takeAt(currItemCount-1); //get the dynAddRmButton

    //CHECK IF NEEDED
    // if (i.widget())
    //     delete i.widget();
    // if (i.layout())
    //     delete i.layout();

    //create and append the new content button to list
    {
        contentButton *newContentBtn = new contentButton();
        newContentBtn->setText(newBtnTxt);
        ui->scrollGrid->addWidget(newContentBtn, row, col);
        contentBtnList.append(newContentBtn);
        this->updateIndexOfAllButtons();

        connect(newContentBtn, SIGNAL(dynBtnNeedsSwitch()), dynBtn, SLOT(switchMode()));
        connect(newContentBtn, SIGNAL(keyWasPressed(int,int)), this, SLOT(processKeyPress(int,int)));
        connect(newContentBtn, SIGNAL(cntButtonWasLeftClicked()), this, SLOT(unmarkAllCntButtons()));
        connect(newContentBtn, SIGNAL(startContentButtonEdit(int)), this, SLOT(openContentEdit(int)));
    }

    ui->scrollGrid->addWidget(currItem->widget(), rowAdd, colAdd); //re-inserts addButton

    qDebug() << "(post) count: " << ui->scrollGrid->count() << " ("
             << ui->scrollGrid->rowCount() << ","
             << ui->scrollGrid->columnCount() << ")\n\n";
}


void MainWindow::updateIndexOfAllButtons(){
    for(int i = 0; i < contentBtnList.count(); i++){
        contentBtnList.at(i)->indexInGrid = i;
    }
}

void MainWindow::removeSelectedButton(int index){

    qDebug() << "removeSelectedButton (index: " << index;

    contentButton *cntBtnLstItem = contentBtnList.at(index);

    qDebug() << " got item";

    //check if item exists + is marked
    //  1) if yes -> delete the item
    //  2) if no  -> do nothing except i++
    if(cntBtnLstItem && cntBtnLstItem->getIsSelected()){
        qDebug() << "  Item does exist and is selected right now";

        //clear it from being the selected button
        cntBtnLstItem->unsetAsSelectedButton();

        //delete from list
        contentBtnList.erase(contentBtnList.cbegin() + index);
        delete cntBtnLstItem;
        cntBtnLstItem = NULL; //invalidate pointer (for logic that follows)

        //delete from grid ***CHECK IF THIS CODE IS NEEDED***
        // QLayoutItem *takenItem = ui->scrollGrid->takeAt(i);
        // delete takenItem;
    }
    this->updateIndexOfAllButtons();
    this->rebuildGrid();
}


void MainWindow::removeAllMarkedButtons(){
    int totalBtnCnt = contentBtnCount::getTotalCnt();

    //i gets incremented in loop body if item(i) is NOT marked for deletion
    //totalBtnCnt gets decremented instead, if item(i) WAS marked for deletion
    for(int i = 0; i < totalBtnCnt;){

        qDebug() << i;

        contentButton *cntBtnLstItem = contentBtnList.at(i);

        qDebug() << " got item";

        //check if item exists + is marked
        //  1) if yes -> delete the item
        //  2) if no  -> do nothing except i++
        if(cntBtnLstItem && cntBtnLstItem->getIsMarked()){
            qDebug() << "  Item does exist (i: " << i << "totalCnt: " << totalBtnCnt << ") [1]";
            qDebug() << "  Item is marked --> DELETE!";

            //delete from list
            contentBtnList.erase(contentBtnList.cbegin() + i);
            delete cntBtnLstItem;
            cntBtnLstItem = NULL; //invalidate pointer (for logic that follows)

            //delete from grid ***CHECK IF THIS CODE IS NEEDED***
            // QLayoutItem *takenItem = ui->scrollGrid->takeAt(i);
            // delete takenItem;

            totalBtnCnt--;
        }else{
            i++;
        }
    }
    this->updateIndexOfAllButtons();

    qDebug() << " (1) ui->scrollGrid->count(): " << ui->scrollGrid->count();
    qDebug() << "==============================";

    this->rebuildGrid();
}

void MainWindow::deleteAllItemsFromGrid(){
    //REBUILD step 1) delete all existing items (including dynAddRmButton)
    while(ui->scrollGrid->count() > 0){
        //ToDo check what this part actually does and if it's needed..... doesn't seem like it
        // QWidget* widget = ui->scrollGrid->itemAt(0)->widget();
        // if(widget){
        //     ui->scrollGrid->removeWidget(widget);
        //     //delete widget;
        // }


        // QLayoutItem *item = ui->scrollGrid->itemAt(0);
        QLayoutItem *item = ui->scrollGrid->takeAt(0);
        if(item){
            // ui->scrollGrid->takeAt(0);
            delete item;
        }
    }
}

void MainWindow::buildGridFromContentList(){
    qDebug() << " (2) ui->scrollGrid->count(): " << ui->scrollGrid->count();
    //REBUILD step 2) build the grid anew with all remaining content buttons
    int newBtnCnt = contentBtnCount::getTotalCnt();
    qDebug() << " (2) newBtnCnt: " << newBtnCnt;

    for(int k = 0; k < newBtnCnt; k++){

        int row = k / this->maxItemsPerRow;
        int col = k % this->maxItemsPerRow;

        qDebug() << "      insert at: " << " (" << row << "," << col << ") [z]";

        contentButton *contentBtnToBeAdded = contentBtnList.at(k);
        // QString newBtnTxt = QString(QString::number(row) + "," + QString::number(col));
        // contentBtnToBeAdded->setText(newBtnTxt);
        ui->scrollGrid->addWidget(contentBtnToBeAdded, row, col);

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


    // qDebug() << "Add dynBtn";

    // //REBUILD step 3) insert the dynAddRmButton back in
    // int rowAdd = (newBtnCnt) / this->kGridW;
    // int colAdd = (newBtnCnt) % this->kGridW;

    // ui->scrollGrid->addWidget(dynBtn, rowAdd, colAdd); //re-inserts addButton

    // qDebug() << "dynBtn added";
}


void MainWindow::dynBtnProcessForModeRm(){
    qDebug() << "== btnModeRM";

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Delete Confirmation", "Do you really want to delete all marked items?",
                                  QMessageBox::Reset|QMessageBox::No|QMessageBox::Yes);

    if(reply == QMessageBox::Yes){
        qDebug() << "No clicked";

        this->removeAllMarkedButtons();
        this->dynBtn->switchMode();

    }else if(reply == QMessageBox::No){
        qDebug() << "No clicked";
        ; //do nothing (?)
    }else{
        qDebug() << "Reset clicked";
        this->unmarkAllCntButtons();
    }
}

//slot
void MainWindow::processDynBtnAction(){
    if(dynBtn->getCurrBtnMode() == dynAddRmButton::btnModeADD){
        this->dynBtnProcessForModeAdd();
    }else if(dynBtn->getCurrBtnMode() == dynAddRmButton::btnModeRM){
        this->dynBtnProcessForModeRm();
    }else{
        qDebug() << "current button mode is invalid.";
    }
}

//slot
void MainWindow::profileButtonClicked(){
    qDebug() << "profile button clicked";
    // profilePicker* profiles = new profilePicker(this);
    // profiles->show();

    //clear button selection before opening the menu
    contentButton::clearButtonSelection();

    if(profMenu){
        qDebug() << "profiles exist";
        profMenu->show();
    }else{
        //ToDo this case is probably wrong / unneeded?
        qDebug() << "no profiles..";
    }
    qDebug() << "profiles view should now be active...";
}

void MainWindow::clearContentButtonList(){
    qDebug() << "~~~ (PRE) ~~~ count of contentBtnList: " << contentBtnList.count();
    while(this->contentBtnList.count()){
        contentButton* tmpContentButton = contentBtnList.takeFirst();
        delete tmpContentButton;
        qDebug() << "~~~ (PRE) ~~~ count of contentBtnList: " << contentBtnList.count();
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
    qDebug() << "!!! start: updateButtonsForProfileChange";

    qDebug() << "! received name: " << profileName;

    if(profileName == ""){
        qDebug() << "invalid profileName.. load default.";

        //ToDo load default stuff

    }else{
        if(profileName == currSelectedProfileName){
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
    qDebug() << "!!! end: updateButtonsForProfileChange";
}

void MainWindow::createDefaultJsonForNewProfile(QString profileName){
    qDebug() << "createDefaultJsonForNewProfile (profileName: " << profileName << ")";
    QString tmpPath = profileMenu::constructFilePathForProfileJson(profileName);
    this->saveDefaultJsonForProfile(tmpPath);
    qDebug() << "/createDefaultJsonForNewProfile";
}

void MainWindow::openContentEdit(int indexOfSender){
    qDebug() << "openContentEdit";
    if(indexOfSender >= 0 && indexOfSender < contentBtnList.count()){

        // QString textOfButton = contentBtnList.at(indexOfSender)->text();
        // this->contEdit->editButton(textOfButton);

        this->contEdit->editButton(contentBtnList.at(indexOfSender));

    }else{
        qDebug() << "Invalid index.";
    }
    qDebug() << "/openContentEdit";
}

void MainWindow::mousePressEvent(QMouseEvent *event){
    qDebug() << "mousePressEvent";
    // this->unmarkAllCntButtons();
    contentButton::clearButtonSelection();
    qDebug() << "/mousePressEvent";
}

void MainWindow::adjustButtons(dynAddRmButton::btnMode mode){
    qDebug() << "=== adjustButtons";

    //ToDo do stuff

    if(mode == dynAddRmButton::btnModeADD){
        qDebug() << "=== ADD";
        // this->ui->hSpace2->
        this->ui->txtInsert->show();
        // this->ui->hSpace3->
        this->ui->buttonSearch->show();
        // this->ui->hSpace4->
        this->ui->buttonAdd->show();
        // this->ui->hSpace5->

        //ToDo hide "unmark all"
        this->unmarkAllBtn->hide();

    }else if(mode == dynAddRmButton::btnModeRM){
        qDebug() << "=== RM";
        // this->ui->hSpace2->
        this->ui->txtInsert->hide();
        // this->ui->hSpace3->
        this->ui->buttonSearch->hide();
        // this->ui->hSpace4->
        this->ui->buttonAdd->hide();
        // this->ui->hSpace5->

        //ToDo show "unmark all"
        this->unmarkAllBtn->show();

    }else{
        qDebug() << "mode: " << mode << " is not a valid mode..";
    }
    qDebug() << "=== /adjustButtons";
}



void MainWindow::keyPressEvent(QKeyEvent *event){
    qDebug() << "Key press event! (MainWindow)";
    //ToDo check how to block arrow keys from moving the focus cursor
    qDebug() << "/ Key press event! (MainWindow)";
}
