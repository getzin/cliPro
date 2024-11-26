#include "profilemenu.h"
#include "ui_profilemenu.h"

#include <QLineEdit>
#include <QMessageBox>
#include <QInputDialog>
#include <QDir>
#include <QSettings>

#include "apputils.h"

QString const profileMenu::profilesFolderName = "/profiles/";

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
    connect(this, SIGNAL(rejected()), this, SLOT(handleRejectedSignal()));

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

    //check that the currentActiveProfile lies within our now inited profiles list
    if(indexIsInBounds(this->currentActiveProfile, this->ui->visibleProfileList->count())){
        qDebug() << "ID (" << this->currentActiveProfile << ") lies in in-bounds. Set it!";
        this->ui->visibleProfileList->setCurrentRow(this->currentActiveProfile);
    }else{
        qDebug() << "ID (" << this->currentActiveProfile << ") lies out of bounds... init as -1 instead.";
        this->ui->visibleProfileList->setCurrentRow(-1);
    }
    this->checkProfilesCountAndSetEditDel();
    this->currentActiveProfHasBeenDeleted = false; //reset
    this->savedIDOffset = 0;
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
                qDebug() << "id (" << currSelRow << ") lies in in-bounds. Set currentActiveProfile to: " << currSelRow;
                this->currentActiveProfile = currSelRow;
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
        qDebug() << "Set currentActiveProfile as -1 instead.";
        this->currentActiveProfile = -1;
    }
    this->savedIDOffset = 0;
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
    qDebug() << "currentActiveProfile before: " << this->currentActiveProfile;

    //make sure the index is valid //ToDo combine cases?
    if(isOK){
        if(tmpCurrSelIndex < this->internalProfilesList.count()){
            qDebug() << "index is valid";
            this->currentActiveProfile = tmpCurrSelIndex;
            this->ui->visibleProfileList->setCurrentRow(tmpCurrSelIndex);
        }else{
            qDebug() << "index out of range";
            this->currentActiveProfile = -1; //index was invalid
        }
    }else{
        qDebug() << "index not valid";
        this->currentActiveProfile = -1; //index was invalid
    }

    qDebug() << "currentActiveProfile after: " << this->currentActiveProfile;
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
    if(tmpListSize > 0){
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

    confirmationText.append("</b> with all its buttons? <br><br><i><b><u>WARNING:</u></b> This will be <b>irreversible</b> after pressing 'Save profiles'</i>");

    QMessageBox::StandardButton reply = messageBox.question(this, "Delete confirmation", confirmationText,
                                QMessageBox::No|QMessageBox::Yes, QMessageBox::No);

    if(reply == QMessageBox::Yes){
        int currRow = this->ui->visibleProfileList->currentRow();

        if(indexIsInBounds(currRow, this->ui->visibleProfileList->count())){
            QListWidgetItem* itemToDelete = this->ui->visibleProfileList->takeItem(currRow);
            QString delName = itemToDelete->text();
            this->unsavedActions.append(profAction{delName, ""});
            delete itemToDelete;

            qDebug() << "currentActiveProfHasBeenDeleted: " << currentActiveProfHasBeenDeleted;

            if(currRow == this->currentActiveProfile){
                this->currentActiveProfHasBeenDeleted = true;
            }

            if(this->currentActiveProfHasBeenDeleted){
                this->ui->visibleProfileList->setCurrentRow(-1); //makes it so that no profile is selected
            }else{
                //Next block of code:
                // restore "currently selected profile", but process whether
                // deleted profile was before or after and set it accordingly
                qsizetype lastSavedIDOffsetApplied = this->currentActiveProfile - this->savedIDOffset;

                //if the deleted item appeared before the last saved ID, then reduce values by 1
                if(currRow < lastSavedIDOffsetApplied){
                    ++savedIDOffset;
                    --lastSavedIDOffsetApplied;
                }

                qDebug() << "index (lastSavedIDOffsetApplied): " << lastSavedIDOffsetApplied;
                if(indexIsInBounds(lastSavedIDOffsetApplied, this->ui->visibleProfileList->count())){
                    this->ui->visibleProfileList->setCurrentRow(lastSavedIDOffsetApplied);
                }else{
                    this->ui->visibleProfileList->setCurrentRow(-1); //makes it so that no profile is selected
                }
            }
        }else{
            ; //do nothing?
        }
    }else{
        ; //do nothing?
    }
}

void profileMenu::commonCloseActions(){
    qDebug() << "start: common close actions";
    this->constructVisibleListFromInternal();
    this->unsavedActions.clear();
    this->resetFocusedButton();
    this->close();
}

void profileMenu::cancelButtonPressed(){
    qDebug("Profiles: Cancel Button pressed");
    this->commonCloseActions();
}

void profileMenu::createProfilesFolderIfNotExist(){
    QString profilesFolderPath = QDir::currentPath() + profileMenu::profilesFolderName;
    if(!(QDir(profilesFolderPath).exists())){
        QDir().mkdir(profilesFolderPath);
        //ToDo what do if this fails?
    }
}

QString profileMenu::constructFilePathForProfileJson(QString profileName){
    return (QDir::currentPath() + profileMenu::profilesFolderName + (profileName)) + ".json";
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
    emit this->newProfileCreated(name);
}

void profileMenu::deleteProfilesJson(QString name){
    qDebug() << "deleteProfilesJson. input: " << name;
    QString namePath = this->constructFilePathForProfileJson(name);
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
        profAction act = this->unsavedActions.takeFirst();
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

    this->saveVisibleListToInternal();
    this->saveProfiles();

    //3 possible cases:
    // 1) delete files of all profiles that were deleted
    // 2) create new default JSON files for all new profiles
    // 3) rename files to new names
    this->processProfilesActions();

    qDebug() << "currentActiveProfile: " << currentActiveProfile;
    qDebug() << "currentActiveProfHasBeenDeleted: " << currentActiveProfHasBeenDeleted;

    if(indexIsInBounds(this->currentActiveProfile, this->internalProfilesList.count())){
        qDebug() << "currentActiveProfile is valid";
        emit this->selProfileHasChanged(this->internalProfilesList.at(this->currentActiveProfile), this->currentActiveProfHasBeenDeleted);
    }else{
        qDebug() << "currentActiveProfile is invalid";
        emit this->selProfileHasChanged("", this->currentActiveProfHasBeenDeleted);
    }
    this->currentActiveProfHasBeenDeleted = false; //reset
    this->resetFocusedButton();
    this->hide();
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
    //point the "selection cursor" to the newly created item!
    QList<QListWidgetItem*> matchingItems = this->ui->visibleProfileList->findItems(newName, Qt::MatchExactly);
    if(matchingItems.count() > 0){
        //we disallow items in the list having the same name, so this result should never be ambiguous
        QListWidgetItem *newItem = matchingItems.at(0);
        int rowOfNewItem = this->ui->visibleProfileList->row(newItem);
        qDebug() << "row of new item: " << rowOfNewItem;
        this->ui->visibleProfileList->setCurrentRow(rowOfNewItem);
    }
    this->dialog.close();
}

qsizetype profileMenu::getProfilesCount(){
    return this->internalProfilesList.count();
}

QString profileMenu::getCurrSelProfileName() const{
    if(indexIsInBounds(this->currentActiveProfile, this->internalProfilesList.count())){
        return this->internalProfilesList.at(this->currentActiveProfile);
    }else{
        return "";
    }
}

void profileMenu::handleSelectedProfileChanged(){
    qDebug() << "start: handleSelectionChange";
    if(this->ui->visibleProfileList->selectedItems().count() > 0){
        qDebug() << "enable";
        this->setEditDelEnabled();
    }else{
        qDebug() << "disable";
        this->setEditDelDisabled();
    }
    qDebug() << "end: handleSelectionChange";
}

void profileMenu::setEditDelEnabled(){
    if(this->editDelAreEnabled == false){
        this->ui->btnEdit->setEnabled(true);
        this->ui->btnDelete->setEnabled(true);
        this->editDelAreEnabled = true;
    }
}

void profileMenu::setEditDelDisabled(){
    if(this->editDelAreEnabled == true){
        this->ui->btnEdit->setDisabled(true);
        this->ui->btnDelete->setDisabled(true);
        this->editDelAreEnabled = false;
    }
}

void profileMenu::handleRejectedSignal(){
    qDebug() << "rejected!";
    this->commonCloseActions();
    emit this->profMenuRejected();
}

void profileMenu::checkProfilesCountAndSetEditDel(){
    if(this->ui->visibleProfileList->count() > 0
        && this->ui->visibleProfileList->selectedItems().count() > 0){
        this->setEditDelEnabled();
    }else{
        this->setEditDelDisabled();
    }
}

void profileMenu::resetFocusedButton(){
    if(this->editDelAreEnabled == true){
        this->ui->btnEdit->setFocus();
    }else{
        this->ui->btnNew->setFocus();
    }
}
