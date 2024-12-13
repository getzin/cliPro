#include "profilemenu.h"
#include "ui_profilemenu.h"

#include <QLineEdit>
#include <QMessageBox>
#include <QInputDialog>
#include <QDir>
#include <QSettings>

#include "apputils.h"

QString const profileMenu::profilesFolderName = "/profiles/";

profileMenu::~profileMenu()
{
    this->cleanUpZombieProfiles();
    delete this->ui;
}

profileMenu::profileMenu(QWidget * const parent)
    : QDialog(parent)
    , ui(new Ui::profileMenu)
    , nameDialog(this)
{
    this->initProfMenu();
    this->initConnects();
}

void profileMenu::initProfMenu(){
    this->ui->setupUi(this);
    this->ui->visibleProfileList->setSortingEnabled(true);

    this->setWindowTitle("Pick a profile");
    this->setModal(true);

    this->loadProfiles(); //needs to happen before construction of visibleProfileList
    this->constructVisibleListFromInternal();
    this->nameDialog.setPtrToVisibleProfileList(this->ui->visibleProfileList);

    this->ui->btnEdit->setFocusProxy(nullptr);
    this->ui->btnNew->setFocusProxy(nullptr);
    this->ui->btnDelete->setFocusProxy(nullptr);
    this->ui->btnCancel->setFocusProxy(nullptr);
    this->ui->visibleProfileList->setFocusProxy(nullptr);
    this->ui->btnSave->setFocusProxy(this->ui->visibleProfileList);
    this->ui->visibleProfileList->setFocus();
    this->fixTabOrder();
}

void profileMenu::initConnects(){
    connect(this->ui->btnNew, SIGNAL(clicked()), &(this->nameDialog), SLOT(startNewProfileCreation())); //new button
    connect(this->ui->btnEdit, SIGNAL(clicked()), &(this->nameDialog), SLOT(startEditOfProfileName())); //edit button
    connect(this->ui->btnDelete, SIGNAL(clicked()), this, SLOT(deleteButtonPressed())); //delete button
    connect(this->ui->btnCancel, SIGNAL(clicked()), this, SLOT(cancelButtonPressed())); //cancel button
    connect(this->ui->btnSave, SIGNAL(clicked()), this, SLOT(saveButtonPressed())); //save button
    connect(this, SIGNAL(rejected()), this, SLOT(handleRejectedSignal()));

    //callback from dialog, after new profile // editing of existing profile name
    connect(&(this->nameDialog), SIGNAL(profileNameEdited(QString,QString)), this, SLOT(handleProfileNameEdited(QString,QString)));
    connect(&(this->nameDialog), SIGNAL(createNewProfile(QString)), this, SLOT(handleNewProfileCreation(QString)));

    connect(this->ui->visibleProfileList, SIGNAL(itemSelectionChanged()), this, SLOT(handleSelectedProfileChanged()));
}

void profileMenu::cleanUpZombieProfiles(){
    //this function deletes all .json files in the /profile/ folder that are
    //not listed in profiles/profiles_list of /settings/cliProSettings.ini file
    //... just in case a json file was not properly deleted (should never happen)
    qDebug() << "start: cleanUpZombieProfiles";
    QString profilesFolderPath = QDir::currentPath() + profileMenu::profilesFolderName;
    QDir profilesFolder(profilesFolderPath);
    if(profilesFolder.exists()){
        QStringList profileJsons = profilesFolder.entryList({"*.json"}, QDir::Files);
        foreach(QString profile, profileJsons){
            profile.remove(".json"); //remove file ending before checking if it is in list
            if(this->internalProfilesList.contains(profile)){
                qDebug() << "profile: " << profile << ".json is in list! (keep it / do nothing)";
            }else{
                qDebug() << "profile: " << profile << ".json is NOT in list (delete the json file!)";
                QString profileJsonFilePath = constructFilePathForProfileJson(profile);
                bool deleteSuccess = QFile::remove(profileJsonFilePath);
                qDebug() << "deleteSuccess: " << deleteSuccess;
            }
        }
    }
    qDebug() << "end: cleanUpZombieProfiles";
}

void profileMenu::constructVisibleListFromInternal(){
    qDebug() << "Start : constructVisibleListFromInternal";

    this->ui->visibleProfileList->clear();
    for(qsizetype i = 0; i < internalProfilesList.count(); ++i){
        qDebug() << "i: " << i << " ; value: " << this->internalProfilesList.at(i);
        this->ui->visibleProfileList->insertItem(i, this->internalProfilesList.at(i));
    }
    qDebug() << "Size of visibleProfileList: " << this->ui->visibleProfileList->count();

    //check that the currentActiveProfile lies within our now inited profiles list
    if(indexIsInBounds(this->currentActiveProfile, this->ui->visibleProfileList->count())){
        qDebug() << "Current active profile (" << this->currentActiveProfile << ") lies in in-bounds. Set it!";
        this->ui->visibleProfileList->setCurrentRow(this->currentActiveProfile);
    }else{
        qDebug() << "Current active profile (" << this->currentActiveProfile << ") lies out of bounds... init as -1 instead.";
        this->ui->visibleProfileList->setCurrentRow(-1);
    }
    this->checkProfilesCountAndSetEditDel();
    this->currentActiveProfHasBeenDeleted = false; //reset
    this->savedIDOffset = 0;
    qDebug() << "End : constructVisibleListFromInternal";
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
                qDebug() << "Selected row (" << currSelRow << ") lies in in-bounds. Set currentActiveProfile to: " << currSelRow;
                this->currentActiveProfile = currSelRow;
            }else{
                qDebug() << "Selected row (" << currSelRow << ") lies out of bounds (bad)";
            }
        }else{
            qDebug() << "No profile is currently selected. OK, but got nothing to do.";
        }
    }else{
        qDebug() << "visibleProfileCnt is less than 0 (BAD, how did this happen?)";
    }
    if(!indexOK){
        qDebug() << "Set currentActiveProfile as -1 instead.";
        this->currentActiveProfile = -1;
    }
    this->savedIDOffset = 0;
    qDebug() << "End : saveVisibleListToInternal";
}

void profileMenu::loadProfiles(){
    qDebug() << "start: loadProfiles";
    QSettings settings(appSettings::settingsFile, QSettings::IniFormat);

    if(!this->internalProfilesList.empty()){
        this->internalProfilesList.clear();
    }

    settings.beginGroup(appSettings::settingsGroupProfiles);
    this->internalProfilesList.append(settings.value(appSettings::settingsValProfilesList).toStringList()); //read "profiles" (list of string)

    //remove all duplicates and names longer than maxProfileNameLength (50)
    //such names should never have gotten into the list in the first place,
    //but the user could manually edit the .ini file to force such a faulty file
    this->internalProfilesList.removeDuplicates();
    for (int i = this->internalProfilesList.length() - 1; i >= 0 ; --i)  {
        if(this->internalProfilesList.at(i).length() > profileNameDialog::maxProfileNameLength){
            qDebug() << "name is too long! Ignore: " << this->internalProfilesList.at(i);
            this->internalProfilesList.removeAt(i);
        }
    }
    QString tmpCurrSelIdVal = settings.value(appSettings::settingsValCurrProfileID).toString();

    bool isOK = false;
    qsizetype tmpCurrSelIndex = tmpCurrSelIdVal.toInt(&isOK);

    qDebug() << "read ID: " << tmpCurrSelIdVal << " ; internalProfilesList size: " << this->internalProfilesList.size();
    qDebug() << "currentActiveProfile before: " << this->currentActiveProfile;

    //make sure the index is valid
    if(isOK
        && tmpCurrSelIndex >= 0
        && tmpCurrSelIndex < this->internalProfilesList.count())
    {
        qDebug() << "index is valid.";
        this->currentActiveProfile = tmpCurrSelIndex;
    }else{
        qDebug() << "index invalid. Set to -1.";
        this->currentActiveProfile = -1; //index was invalid
    }
    this->ui->visibleProfileList->setCurrentRow(this->currentActiveProfile);
    qDebug() << "currentActiveProfile after: " << this->currentActiveProfile;
    settings.endGroup();

    // for(qsizetype i = 0; i < this->internalProfilesList.count(); ++i){
    //     qDebug() << "i: " << i << " ; value: " << this->internalProfilesList.at(i);
    // }
    qDebug() << "end: loadProfiles";
}

void profileMenu::saveProfiles(){
    qDebug() << "start: saveProfiles";
    QSettings settings(appSettings::settingsFile, QSettings::IniFormat);

    settings.beginGroup(appSettings::settingsGroupProfiles);
    settings.setValue(appSettings::settingsValProfilesList, this->internalProfilesList);

    bool indexOK = false;
    qsizetype tmpListSize = this->internalProfilesList.count();
    if(tmpListSize > 0){
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
            qDebug() << "0 or more than 1 profiles are currently selected.";
        }
    }else{
        qDebug() << "No profiles exist.";
    }

    if(!indexOK){
        qDebug() << "Save -1 instead.";
        settings.setValue(appSettings::settingsValCurrProfileID,  QString::number(-1));
    }

    settings.endGroup();
    qDebug() << "end: Saving profiles";
}

//slot
void profileMenu::deleteButtonPressed(){
    qDebug("Profiles: Delete Button pressed");

    QMessageBox messageBox;
    messageBox.setTextFormat(Qt::RichText);

    QString confirmationText;
    confirmationText.append("Do you really want to delete profile <b>");

    QList<QListWidgetItem*> allSelectedItems = ui->visibleProfileList->selectedItems();

    //note: We can only select one item in the list,
    //      either one item is selected or the list is empty
    //      (we still signal the ERROR in the text box)
    if(allSelectedItems.empty()){
        qDebug() << "No item selected.";
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
    this->constructVisibleListFromInternal();
    this->unsavedActions.clear();
    this->ui->visibleProfileList->setFocus();
    this->hide();
}

//slot
void profileMenu::cancelButtonPressed(){
    emit this->rejected();
}

//slot
void profileMenu::handleRejectedSignal(){
    this->commonCloseActions();
    emit this->profMenuRejected();
}

//static
void profileMenu::createProfilesFolderIfNotExist(){
    QString profilesFolderPath = QDir::currentPath() + appSettings::appFolder + profileMenu::profilesFolderName;
    if(!(QDir(profilesFolderPath).exists())){
        QDir().mkdir(profilesFolderPath);
        //ToDo what do if this fails?
    }
}

//static
QString profileMenu::constructFilePathForProfileJson(QString const &profileName){
    return (QDir::currentPath() + appSettings::appFolder + profileMenu::profilesFolderName + profileName + ".json");
}

void profileMenu::renameProfilesJson(QString const &oldName, QString const &newName){
    qDebug() << "renameProfilesJson. input: " << oldName << ", " << newName;
    QString oldNamePath = this->constructFilePathForProfileJson(oldName);
    QString newNamePath = this->constructFilePathForProfileJson(newName);
    qDebug() << "renameProfilesJson. paths: " << oldNamePath << ", " << newNamePath;
    bool fileExistsAlready = QFile(newNamePath).exists();
    qDebug() << "fileExistsAlready? " << fileExistsAlready;
    if(fileExistsAlready){
        qDebug() << "delete existing file!";
        bool deleteSuccess = QFile::remove(newNamePath);
        qDebug() << "deleteSuccess: " << deleteSuccess;
    }
    //ToDo what if fails?
    bool renameSuccess = QFile::rename(oldNamePath, newNamePath);
    qDebug() << "renameSuccess: " << renameSuccess;
}

void profileMenu::createNewProfilesJson(QString const &name){
    qDebug() << "createNewProfilesJson. input: " << name;
    emit this->newProfileCreated(name);
}

void profileMenu::deleteProfilesJson(QString const &name){
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

//slot
void profileMenu::saveButtonPressed(){
    qDebug() << "start: saveButtonPressed";

    this->saveVisibleListToInternal();
    this->saveProfiles();

    //3 possible cases:
    // 1) delete files of all profiles that were deleted
    // 2) create new default JSON files for all new profiles
    // 3) rename files to new names
    this->processProfilesActions();

    qDebug() << "(profMenu) currentActiveProfile: " << this->currentActiveProfile;
    qDebug() << "(profMenu) currentActiveProfHasBeenDeleted: " << this->currentActiveProfHasBeenDeleted;

    if(indexIsInBounds(this->currentActiveProfile, this->internalProfilesList.count())){
        qDebug() << "currentActiveProfile is valid";
        emit this->selProfileHasChanged(this->internalProfilesList.at(this->currentActiveProfile), this->currentActiveProfHasBeenDeleted);
    }else{
        qDebug() << "currentActiveProfile is invalid";
        emit this->selProfileHasChanged("", this->currentActiveProfHasBeenDeleted);
    }
    this->currentActiveProfHasBeenDeleted = false; //reset
    this->hide();
}

//slot
void profileMenu::handleProfileNameEdited(QString const &oldName, QString const &newName){
    qDebug() << "(edit) oldName: " << oldName << ", newName: " << newName;
    this->ui->visibleProfileList->selectedItems().at(0)->setText(newName);
    this->unsavedActions.append(profAction{oldName, newName});
    this->nameDialog.close();
}

//slot
void profileMenu::handleNewProfileCreation(QString const &newName){
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
    this->nameDialog.close();
}

qsizetype profileMenu::getProfilesCount() const{
    return this->internalProfilesList.count();
}

QString profileMenu::getCurrSelProfileName() const{
    if(indexIsInBounds(this->currentActiveProfile, this->internalProfilesList.count())){
        return this->internalProfilesList.at(this->currentActiveProfile);
    }else{
        return "";
    }
}

bool profileMenu::checkProfileNameIsInInternalList(QString const &nameToCheck) const{
    for(auto &prof : this->internalProfilesList){
        if(prof == nameToCheck){
            return true;
        }
    }
    return false;
}

//slot
void profileMenu::handleSelectedProfileChanged(){
    if(this->ui->visibleProfileList->selectedItems().count() > 0){
        this->setEditDelEnabled();
    }else{
        this->setEditDelDisabled();
    }
}

void profileMenu::setEditDelEnabled(){
    if(this->editDelAreEnabled == false){
        this->ui->btnEdit->setEnabled(true);
        this->ui->btnDelete->setEnabled(true);
        this->editDelAreEnabled = true;
    }
    this->fixTabOrder();
}

void profileMenu::setEditDelDisabled(){
    if(this->editDelAreEnabled == true){
        this->ui->btnEdit->setDisabled(true);
        this->ui->btnDelete->setDisabled(true);
        this->editDelAreEnabled = false;
    }
    this->fixTabOrder();
}

void profileMenu::fixTabOrder(){
    QWidget::setTabOrder(this->ui->btnEdit, this->ui->btnNew);
    QWidget::setTabOrder(this->ui->btnNew, this->ui->btnDelete);
    QWidget::setTabOrder(this->ui->btnDelete, this->ui->btnCancel);
    QWidget::setTabOrder(this->ui->btnCancel, this->ui->visibleProfileList);

}

void profileMenu::checkProfilesCountAndSetEditDel(){
    if(this->ui->visibleProfileList->count() > 0
        && this->ui->visibleProfileList->selectedItems().count() > 0){
        this->setEditDelEnabled();
    }else{
        this->setEditDelDisabled();
    }
}
