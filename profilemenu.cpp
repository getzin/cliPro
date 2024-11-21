#include "profilemenu.h"
#include "ui_profilemenu.h"

#include <QLineEdit>
#include <QMessageBox>
#include <QInputDialog>
#include <QDir>
#include <QSettings>

#include "apputils.h"

profileMenu::profileMenu(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::profileMenu)
    , dialog(this)
{
    qDebug() << "start: profileMenu cTor";

    this->ui->setupUi(this);
    this->ui->visibleProfileList->setSortingEnabled(true);
    this->setWindowTitle("Pick a profile");
    this->setModal(true);

    this->loadProfiles(); //needs to happen before construction of visibleProfileList
    this->constructVisibleListFromInternal();
    this->dialog.setPtrToVisibleProfileList(this->ui->visibleProfileList);
    qDebug() << "Profiles List is set";

    for(qsizetype i = 0; i < this->internalProfilesList.count(); ++i){
        qDebug() << "profilesList.at(" << i << ") : " << internalProfilesList.at(i);
    }

    connect(this->ui->btnNew, SIGNAL(clicked()), &(this->dialog), SLOT(startNewProfileCreation())); //new button
    connect(this->ui->btnEdit, SIGNAL(clicked()), &(this->dialog), SLOT(startEditOfProfileName())); //edit button
    connect(this->ui->btnDelete, SIGNAL(clicked()), this, SLOT(deleteButtonPressed())); //delete button
    connect(this->ui->btnCancel, SIGNAL(clicked()), this, SLOT(cancelButtonPressed())); //cancel button
    connect(this->ui->btnSave, SIGNAL(clicked()), this, SLOT(saveButtonPressed())); //save button

    //callback from dialog, after new profile // editing of existing profile name
    connect(&(this->dialog), SIGNAL(profileNameEdited(QString,QString)), this, SLOT(handleProfileNameEdited(QString,QString)));
    connect(&(this->dialog), SIGNAL(createNewProfile(QString)), this, SLOT(handleNewProfileCreation(QString)));

    connect(this->ui->visibleProfileList, SIGNAL(itemSelectionChanged()), this, SLOT(handleSelectedProfileChanged()));

    // if(ui->visibleProfileList->selectedItems().count() == 0){
    //     ui->btnEdit->setDisabled(true);
    //     ui->btnDelete->setDisabled(true);
    //     ui->btnSave->setDisabled(true);
    // }
    // this->setEditDelSaveDisabled();

    //ToDo logic for "last used profile" should go here

    qDebug() << "end: profileMenu cTor";
}

profileMenu::~profileMenu()
{
    qDebug() << "begin: profileMenu dTor";
    delete this->ui;
    qDebug() << "end: profileMenu dTor";
}

void profileMenu::constructVisibleListFromInternal(){
    qDebug() << "Start : initVisibleListFromInternal";

    this->ui->visibleProfileList->clear();
    for(qsizetype i = 0; i < internalProfilesList.count(); ++i){
        qDebug() << "i: " << i << " ; value: " << this->internalProfilesList.at(i);
        this->ui->visibleProfileList->insertItem(i, this->internalProfilesList.at(i));
    }
    qDebug() << "Size of visibleProfileList: " << this->ui->visibleProfileList->count();

    //check that the currSelectedProfileID lies within our now inited profiles list
    if(indexIsInBounds(this->lastSavedSelProfileID, this->ui->visibleProfileList->count())){
        qDebug() << "ID (" << this->lastSavedSelProfileID << ") lies in in-bounds. Set it!";
        this->ui->visibleProfileList->setCurrentRow(this->lastSavedSelProfileID);
    }else{
        qDebug() << "ID (" << this->lastSavedSelProfileID << ") lies out of bounds... init as -1 instead.";
        this->ui->visibleProfileList->setCurrentRow(-1);
    }
    qDebug() << "End : initVisibleListFromInternal";
}

void profileMenu::saveVisibleListToInternal(){
    qDebug() << "Start : saveVisibleListToInternal";

    this->internalProfilesList.clear();
    for(qsizetype i = 0; i < this->ui->visibleProfileList->count(); ++i){
        qDebug() << "i: " << i << " ; value: " << this->ui->visibleProfileList->item(i)->text();
        this->internalProfilesList.emplace_back(this->ui->visibleProfileList->item(i)->text());
    }

    bool indexOK = false;
    qsizetype visibleProfileCnt = this->ui->visibleProfileList->count();
    if(visibleProfileCnt >= 0){
        if(!(this->ui->visibleProfileList->selectedItems().empty())){
            qsizetype currSelRow = this->ui->visibleProfileList->currentRow();
            if(indexIsInBounds(currSelRow, visibleProfileCnt)){
                indexOK = true;
                qDebug() << "id (" << currSelRow << ") lies in in-bounds. Set lastSavedSelProfileID to: " << currSelRow;
                this->lastSavedSelProfileID = currSelRow;
            }else{
                qDebug() << "id (" << currSelRow << ") lies out of bounds (bad)";
            }
        }else{
            qDebug() << "No profile is currently selected (bad)";
        }
    }else{
        qDebug() << "visibleProfileCnt is less than 0 (bad)";
    }
    if(!indexOK){
        qDebug() << "Set lastSavedSelProfileID as -1 instead.";
        this->lastSavedSelProfileID = -1;
    }
    qDebug() << "End : saveVisibleListToInternal";
}

void profileMenu::loadProfiles(){
    qDebug() << "start: Loading profiles";
    QSettings settings(appSettings::settingsFile, QSettings::IniFormat);

    if(!this->internalProfilesList.empty()){
        this->internalProfilesList.clear();
    }

    settings.beginGroup(appSettings::settingsGroupProfiles);
    this->internalProfilesList.append(settings.value(appSettings::settingsValProfilesList).toStringList()); //read "profiles" (list of string)

    QString tmpCurrSelIdVal = settings.value(appSettings::settingsValCurrProfileID).toString();

    bool isOK = false;
    qsizetype tmpCurrSelIndex = tmpCurrSelIdVal.toInt(&isOK);

    qDebug() << "read ID: " << tmpCurrSelIdVal << " ; profiles list size: " << this->internalProfilesList.size();
    qDebug() << "currSelectedProfileID before: " << this->lastSavedSelProfileID;

    //make sure the index is valid //ToDo combine cases?
    if(isOK){
        if(tmpCurrSelIndex < this->internalProfilesList.count()){
            qDebug() << "index is valid";
            this->lastSavedSelProfileID = tmpCurrSelIndex;
            this->ui->visibleProfileList->setCurrentRow(tmpCurrSelIndex);
        }else{
            qDebug() << "index out of range";
            this->lastSavedSelProfileID = -1; //index was invalid
        }
    }else{
        qDebug() << "index not valid";
        this->lastSavedSelProfileID = -1; //index was invalid
    }

    qDebug() << "currSelectedProfileID after: " << this->lastSavedSelProfileID;
    settings.endGroup();

    for(qsizetype i = 0; i < this->internalProfilesList.count(); ++i){
        qDebug() << "i: " << i << " ; value: " << this->internalProfilesList.at(i);
    }
    qDebug() << "end: Loading profiles";
}

void profileMenu::saveProfiles(){
    qDebug() << "start: Saving profiles";
    QSettings settings(appSettings::settingsFile, QSettings::IniFormat);

    settings.beginGroup(appSettings::settingsGroupProfiles);
    settings.setValue(appSettings::settingsValProfilesList, this->internalProfilesList);

    bool indexOK = false;
    qsizetype tmpListSize = this->internalProfilesList.count();
    if(tmpListSize > 1){
        qDebug() << "1 or more profiles exist! (good)";
        if(this->ui->visibleProfileList->selectedItems().count() == 1){

            qsizetype tmpValForCurrSelID = this->ui->visibleProfileList->currentRow();
            qDebug() << "tmpValForCurrSelID: " << tmpValForCurrSelID;

            //check that index is in bounds
            if(indexIsInBounds(tmpValForCurrSelID, tmpListSize)){
                indexOK = true;
                qDebug() << "index is in-bounds, good!";
                settings.setValue(appSettings::settingsValCurrProfileID, QString::number(tmpValForCurrSelID));
            }else{
                qDebug() << "index is out of bounds, bad.";
            }
        }else{
            qDebug() << "Index is invalid, bad.";
        }
    }else{
        qDebug() << "no profiles exist, probably bad.";
    }

    if(!indexOK){
        settings.setValue(appSettings::settingsValCurrProfileID,  QString::number(-1));
    }

    settings.endGroup();
    qDebug() << "end: Saving profiles";
}

void profileMenu::deleteButtonPressed(){
    qDebug("Profiles: Delete Button pressed");

    QMessageBox messageBox;
    messageBox.setTextFormat(Qt::RichText);

    QString confirmationText;
    confirmationText.append("Do you really want to delete profile <b>");

    QList<QListWidgetItem*> allSelectedItems = ui->visibleProfileList->selectedItems();

    //note: We can only select one item in the list,
    //      either one item selected or the list is empty
    //      (we still signal the ERROR in the text box)
    if(allSelectedItems.empty()){
        qDebug() << "No item selected..";
        confirmationText.append("ERROR");
    }else{
        QString textOfselectedItem = this->ui->visibleProfileList->selectedItems().at(0)->text();
        qDebug() << "Item selected: " << textOfselectedItem;
        confirmationText.append(textOfselectedItem);
    }

    confirmationText.append("</b> with all its clips? <br><br><i>(WARNING: This is irreversible)</i>");

    QMessageBox::StandardButton reply = messageBox.question(this, "Delete Confirmation", confirmationText,
                                QMessageBox::No|QMessageBox::Yes, QMessageBox::No);

    if(reply == QMessageBox::Yes){
        int currRow = this->ui->visibleProfileList->currentRow();

        if(indexIsInBounds(currRow, this->ui->visibleProfileList->count())){
            QListWidgetItem* itemToDelete = this->ui->visibleProfileList->takeItem(currRow);
            QString delName = itemToDelete->text();
            this->unsavedActions.append(profAction{delName, ""});
            delete itemToDelete;
            this->ui->visibleProfileList->setCurrentRow(-1); //makes it so that no profile is selected
        }else{
            ; //do nothing?
        }
    }else{
        ; //do nothing?
    }
    //ToDo check case where ALL PROFILES get deleted!! (what happens? what SHOULD happen?)
}

void profileMenu::cancelButtonPressed(){
    qDebug("Profiles: Cancel Button pressed");
    this->constructVisibleListFromInternal();
    this->unsavedActions.clear();
    this->close();
    emit this->cancelled();
}

QString profileMenu::constructFilePathForProfileJson(QString profileName){
    return (QDir::currentPath() + "/profiles/" + (profileName)) + ".json";
}

void profileMenu::renameProfilesJson(QString oldName, QString newName){
    qDebug() << "renameProfilesJson. input: " << oldName << ", " << newName;
    QString oldNamePath = this->constructFilePathForProfileJson(oldName);
    QString newNamePath = this->constructFilePathForProfileJson(newName);
    qDebug() << "renameProfilesJson. paths: " << oldNamePath << ", " << newNamePath;
    bool renameSuccess = QFile::rename(oldNamePath, newNamePath);
    //ToDo what if fails?
    qDebug() << "renameSuccess: " << renameSuccess;
}

void profileMenu::createNewProfilesJson(QString name){
    qDebug() << "createNewProfilesJson. input: " << name;
    //QString namePath = constructFilePathForProfileJson(name);
    ; //ToDo create some kind of default json
    emit this->newProfileCreated(name);
}

void profileMenu::deleteProfilesJson(QString name){
    qDebug() << "deleteProfilesJson. input: " << name;
    QString namePath = constructFilePathForProfileJson(name);
    qDebug() << "deleteProfilesJson. path: " << namePath;
    bool deleteSuccess = QFile::remove(namePath);
    qDebug() << "deleteSuccess: " << deleteSuccess;
}

void profileMenu::processProfilesActions(){
    //       delName + newName
    // edit:  "____" + "____"
    // new:     ""   + "____"
    // del:   "____" +   ""

    qDebug() << "start: processProfilesActions";
    while(!(this->unsavedActions.empty())){
        profAction act = unsavedActions.takeFirst();
        qDebug() << "input: " << act.delName << ", " << act.newName;
        if(!act.delName.isEmpty() && !act.newName.isEmpty()){
            this->renameProfilesJson(act.delName, act.newName);
        }else if(act.delName.isEmpty() && !act.newName.isEmpty()){
            this->createNewProfilesJson(act.newName);
        }else if(!act.delName.isEmpty() && act.newName.isEmpty()){
            this->deleteProfilesJson(act.delName);
        }else{
            qDebug() << "Error. Both strings for action are empty. Do nothing.";
        }
    }
    qDebug() << "end: processProfilesActions";
}

void profileMenu::saveButtonPressed(){
    qDebug() << "start: saveButtonPressed";

    //do not allow saving when no profile has been selected?
    //(or just make it so that you can't add/delete items without having a profile?)
    if(!(this->ui->visibleProfileList->selectedItems().empty())){
        qDebug() << "list of visible profiles is not empty!";
        this->saveVisibleListToInternal();
        this->saveProfiles();

        //3 possible cases:
        // 1) delete files of all profiles that were deleted
        // 2) create new default JSON files for all new profiles
        // 3) rename files to new names
        this->processProfilesActions();
        this->close();
    }else{
        timedPopUp(this, 3000, "<p align='center'>Please<br><b>select a profile</b><br>to continue.</p>");
        qDebug() << "no profile selected!";
    }

    //emit signal with new profile name?
    qDebug() << "lastSavedSelProfileID: " << lastSavedSelProfileID;

    if(indexIsInBounds(this->lastSavedSelProfileID, this->internalProfilesList.count())){
        emit this->selProfileHasChanged(this->internalProfilesList.at(this->lastSavedSelProfileID));
    }else{
        qDebug() << "lastSavedSelProfileID was invalid";
        emit this->selProfileHasChanged("");
    }
}

void profileMenu::handleProfileNameEdited(QString oldName, QString newName){
    qDebug() << "(edit) oldName: " << oldName << ", newName: " << newName;
    this->ui->visibleProfileList->selectedItems().at(0)->setText(newName);
    this->unsavedActions.append(profAction{oldName, newName});
    this->dialog.close();
}

void profileMenu::handleNewProfileCreation(QString newName){
    qDebug() << "(new) userInput: " << newName;
    this->ui->visibleProfileList->addItem(newName);
    this->unsavedActions.append(profAction{"", newName});
    this->dialog.close();
}

QString profileMenu::getCurrSelProfileName() const{
    if(indexIsInBounds(this->lastSavedSelProfileID, this->internalProfilesList.count())){
        return this->internalProfilesList.at(this->lastSavedSelProfileID);
    }else{
        return "";
    }
}

void profileMenu::handleSelectedProfileChanged(){
    qDebug() << "start: handleSelectionChange";
    if(ui->visibleProfileList->selectedItems().count() > 0){
        qDebug() << "enable";
        this->setEditDelEnabled();
    }else{
        qDebug() << "disable";
        this->setEditDelDisabled();
    }
    qDebug() << "end: handleSelectionChange";
}

void profileMenu::setEditDelEnabled(){
    this->ui->btnEdit->setEnabled(true);
    this->ui->btnDelete->setEnabled(true);
}

void profileMenu::setEditDelDisabled(){
    this->ui->btnEdit->setDisabled(true);
    this->ui->btnDelete->setDisabled(true);
}

void profileMenu::closeEvent(QCloseEvent *event){
    Q_UNUSED(event)
    this->cancelButtonPressed();
}
