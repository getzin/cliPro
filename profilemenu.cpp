#include "profilemenu.h"
#include "ui_profilemenu.h"

#include <QLineEdit>
#include <QMessageBox>
#include <QInputDialog>
#include <QDir>
#include <QSettings>

#include "timedpopup.h"

const QString profileMenu::appName = "cliProV1";
const QString profileMenu::appAuthor = "Andreas Getzin";
const QString profileMenu::settingsFile = "settings/cliProSettings.ini";
const QString profileMenu::settingsProfilesGroup = "profiles";
const QString profileMenu::settingsProfilesListVal = "profiles_list";
const QString profileMenu::settingsProfilesCurrSelID = "current_profile_id";

profileMenu::profileMenu(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::profileMenu)
{
    qDebug() << "YYYYY begin: profileMenu cTor";

    this->ui->setupUi(this);

    this->setWindowTitle("Pick a profile");

    // QStringList* profileList = getProfiles();

    this->loadProfiles(); //needs to happen before init of list
    this->initVisibleListFromInternal();
    this->ui->visibleProfileList->setSortingEnabled(true);

    qDebug() << "Profiles List is set";

    for(int i = 0; i < this->internalProfilesList.length(); i++){

        qDebug() << "profilesList.at(" << i << ") : " << internalProfilesList.at(i);
    }

    //profilesList = this->loadProfiles();

    // for(int i = 0; i < this->internalProfilesList->length(); i++){
    //     // QLineEdit* profile = new QLineEdit(profileList->at(i));
    //     // ui->scrollArea->setWidget(profile, Qt::AlignTop);

    //     this->ui->visibleProfileList->insertItem(i, this->internalProfilesList->at(i));
    // }

    // connect(ui->btnNew, SIGNAL(clicked()), this, SLOT(newButtonPressed())); //new button
    // connect(ui->btnEdit, SIGNAL(clicked()), this, SLOT(editButtonPressed())); //edit button
    connect(ui->btnNew, SIGNAL(clicked()), &dialog, SLOT(newProfile())); //new button
    connect(ui->btnEdit, SIGNAL(clicked()), &dialog, SLOT(editProfile())); //edit button
    connect(ui->btnDelete, SIGNAL(clicked()), this, SLOT(deleteButtonPressed())); //delete button
    connect(ui->btnCancel, SIGNAL(clicked()), this, SLOT(cancelButtonPressed())); //cancel button
    connect(ui->btnSave, SIGNAL(clicked()), this, SLOT(saveButtonPressed())); //save button

    connect(&dialog, SIGNAL(editName(QString,QString)), this, SLOT(addEditNameActionToUnsavedActions(QString,QString)));
    connect(&dialog, SIGNAL(newName(QString)), this, SLOT(addNewNameActionToUnsavedActions(QString)));

    connect(ui->visibleProfileList, SIGNAL(itemSelectionChanged()), this, SLOT(handleSelectionChange()));

    // if(ui->visibleProfileList->selectedItems().length() == 0){
    //     ui->btnEdit->setDisabled(true);
    //     ui->btnDelete->setDisabled(true);
    //     ui->btnSave->setDisabled(true);
    // }
    // this->setEditDelSaveDisabled();

    qDebug() << "YYYYY end: profileMenu cTor";


    //ToDo logic for "last used profile" should go here

    dialog.setPtrToListWidget(this->ui->visibleProfileList);

}

profileMenu::~profileMenu()
{
    qDebug() << "begin: profileMenu dTor";
    delete ui;
    qDebug() << "end: profileMenu dTor";
}

// QStringList* profileMenu::getProfiles(){
//     QStringList* profileList = new QStringList;

//     //for now
//     profileList->emplace_back("Work");
//     profileList->emplace_back("Language");
//     profileList->emplace_back("Youtube");

//     return profileList;
// }

void profileMenu::initVisibleListFromInternal(){
    qDebug() << "Start : initVisibleListFromInternal";
    for(int i = 0; i < internalProfilesList.length(); i++){
        qDebug() << "i: " << i << " ; value: " << this->internalProfilesList.at(i);
        this->ui->visibleProfileList->insertItem(i, this->internalProfilesList.at(i));
    }

    qDebug() << "Size of visibleProfileList: " << this->ui->visibleProfileList->count();

    //check that the currSelectedProfileID lies within our now inited profiles list
    if((this->lastSavedSelProfileID >= 0)
        && (this->lastSavedSelProfileID < this->ui->visibleProfileList->count())){
        qDebug() << "ID (" << lastSavedSelProfileID << ") lies in in-bounds. Set it!";
        this->ui->visibleProfileList->setCurrentRow(this->lastSavedSelProfileID);
    }else{
        qDebug() << "ID (" << lastSavedSelProfileID << ") lies out of bounds... init as -1 instead.";
        this->ui->visibleProfileList->setCurrentRow(-1);
    }
    qDebug() << "End : /initVisibleListFromInternal";
}

void profileMenu::reconstructVisibleListFromInternal(){
    qDebug() << "Start : reconstructVisibleListFromInternal";
    ui->visibleProfileList->clear();
    initVisibleListFromInternal();
    qDebug() << "End : /reconstructVisibleListFromInternal";
}

void profileMenu::saveVisibleListToInternal(){
    qDebug() << "Start : saveVisibleListToInternal";

    this->internalProfilesList.clear();
    for(int i = 0; i < this->ui->visibleProfileList->count(); i++){
        qDebug() << "i: " << i << " ; value: " << this->ui->visibleProfileList->item(i)->text();
        // this->ui->visibleProfileList->insertItem(i, this->internalProfilesList->at(i));
        this->internalProfilesList.emplace_back(this->ui->visibleProfileList->item(i)->text());
    }


    // if((this->currSelectedProfileID >= 0)
    //     && (this->currSelectedProfileID < this->ui->visibleProfileList->count())){
    //     qDebug() << "ID (" << currSelectedProfileID << ") lies in in-bounds. Set it!";
    //     this->ui->visibleProfileList->setCurrentRow(this->currSelectedProfileID);
    // }else{
    //     qDebug() << "ID (" << currSelectedProfileID << ") lies out of bounds... init as -1 instead.";
    //     this->ui->visibleProfileList->setCurrentRow(-1);
    // }

    int visibleProfileCnt = this->ui->visibleProfileList->count();
    int currSelRow = this->ui->visibleProfileList->currentRow();
    if((visibleProfileCnt >= 0)
        && (currSelRow < visibleProfileCnt)){
        qDebug() << " id (" << currSelRow << ") lies in in-bounds. Set it!";
        this->lastSavedSelProfileID = currSelRow;
    }else{
        qDebug() << " id (" << currSelRow << ") lies out of bounds... set as -1 instead.";
        this->lastSavedSelProfileID = -1;
    }

    qDebug() << "End : /saveVisibleListToInternal";
}

void profileMenu::loadProfiles(){
    qDebug() << "Start: Loading profiles";
    QSettings settings(settingsFile, QSettings::IniFormat);

    if(!internalProfilesList.empty()){
        internalProfilesList.clear();
    }

    settings.beginGroup(settingsProfilesGroup);

    //read "profiles" (list of string)
    internalProfilesList.append(settings.value(settingsProfilesListVal).toStringList());

    // qDebug() << "Size: " << internalProfilesList.size();

    QString tmpCurrSelIdVal = settings.value(settingsProfilesCurrSelID).toString();
    bool isOK = false;
    int tmpCurrSelIndex = tmpCurrSelIdVal.toInt(&isOK);

    qDebug() << "read ID: " << tmpCurrSelIdVal << " ; profiles list size: " << internalProfilesList.size();

    qDebug() << "currSelectedProfileID before: " << lastSavedSelProfileID;

    //make sure the index is valid
    if(isOK){
        if(tmpCurrSelIndex < internalProfilesList.size()){
            qDebug() << "index is valid";
            lastSavedSelProfileID = tmpCurrSelIndex;
            ui->visibleProfileList->setCurrentRow(tmpCurrSelIndex);
        }else{
            qDebug() << "index out of range";
            lastSavedSelProfileID = -1; //index was invalid
        }
    }else{
        qDebug() << "index not valid";
        lastSavedSelProfileID = -1; //index was invalid
    }

    qDebug() << "currSelectedProfileID after: " << lastSavedSelProfileID;

    settings.endGroup();

    for(int i = 0; i < internalProfilesList.count(); i++){
        qDebug() << "i: " << i << " ; value: " << internalProfilesList.at(i);
    }

    qDebug() << "End: Loading profiles";
}

void profileMenu::saveProfiles(){
    qDebug() << "Start: Saving profiles";
    QSettings settings(settingsFile, QSettings::IniFormat);

    settings.beginGroup(settingsProfilesGroup);
    settings.setValue(settingsProfilesListVal, this->internalProfilesList);
    int tmpListSize = internalProfilesList.size();
    if(tmpListSize > 1){
        qDebug() << "1 or more profiles exist! (good)";
        if(ui->visibleProfileList->selectedItems().size() == 1){

            int tmpValForCurrSelID = ui->visibleProfileList->currentRow();

            qDebug() << "tmpValForCurrSelID: " << tmpValForCurrSelID;

            //check that index is in bounds
            if(tmpValForCurrSelID >= 0 && tmpValForCurrSelID < tmpListSize){
                qDebug() << "index is in-bounds, great!";
                settings.setValue(settingsProfilesCurrSelID, QString::number(tmpValForCurrSelID));
            }else{
                qDebug() << "index is out of bounds.. bad.";
                settings.setValue(settingsProfilesCurrSelID,  QString::number(-1));
            }
        }else{
            qDebug() << "Index is invalid...";
            settings.setValue(settingsProfilesCurrSelID,  QString::number(-1));
        }
    }else{
        qDebug() << "no profiles exist?";
        settings.setValue(settingsProfilesCurrSelID,  QString::number(-1));
    }
    settings.endGroup();
    qDebug() << "End: Saving profiles";
}

// void profileMenu::newButtonPressed(){
//     qDebug("Profiles: New Button pressed");

//     // QInputDialog nameEntry;
//     // nameEntry.setVisible(true);
//     // nameEntry.show();

//     // bool isOK;
//     // QString newProfileName = QInputDialog::getText(this, tr("New profile"),
//     //                                      tr("Name (letters/numbers only):"), QLineEdit::Normal,
//     //                                      "", &isOK);

//     // if(isOK && !newProfileName.isEmpty()
//     //         && checkStringIsAlphanumeric(newProfileName)){
//     //     qDebug() << "Input OK";
//     //     ui->visibleProfileList->addItem(newProfileName);
//     // }else{
//     //     qDebug() << "Input not OK";
//     // }

//     //
//     //ToDo make it permanent... need to update the QSettings thing... maybe emit a signal?
//     //

//     //=====previous working code=====
//     // getUserInputAndCheck(false, tr("New profile"), tr("Name (letters/numbers only):"), "");

//     dialog.newProfile();
// }

// void profileMenu::editButtonPressed(){
//     qDebug("Profiles: Edit Button pressed");

//     //we can only select 1 item, but it also has to be selected
//     // if(ui->visibleProfileList->selectedItems().size() == 1){
//         // QString currentItemTxt = ui->visibleProfileList->selectedItems().at(0)->text();

//         // bool isOK;
//         // QString editedProfileName = QInputDialog::getText(this, tr("Edit profile name"),
//         //                                      tr("Change name to:"), QLineEdit::Normal,
//         //                                      currentItemTxt, &isOK);
//         // if (isOK && !editedProfileName.isEmpty()
//         //          && checkStringIsAlphanumeric(editedProfileName)){
//         //     qDebug() << "Input OK";
//         //     ui->visibleProfileList->selectedItems().at(0)->setText(editedProfileName);
//         // }else{
//         //     QString tmpErrorStr;
//         //     tmpErrorStr.append("ERROR: <b>").append(editedProfileName).append("</b> is not a valid string");
//         //     this->timedPopUp(this->defaultTimer, tmpErrorStr);
//         //     qDebug() << "Input not OK";
//         // }


//         //=====previous working code=====
//         // getUserInputAndCheck(true, tr("Edit profile name"), tr("Change name to:"), currentItemTxt);


//         // if(!currentItemTxt.isEmpty()){
//             // dialog.editProfile(ui->visibleProfileList, ui->visibleProfileList->currentRow(), currentItemTxt);
//         // }else{
//         //     qDebug() << "Error: currentItemTxt is empty!"; //error
//         // }

//     //     dialog.editProfile();

//     // }else{
//     //     timedPopUp(defaultPopUpTimer, "No profile selected.");
//     // }

//     //TRY TO DO INLINE EDITING... DIDN'T WORK...
//     // int currRow = ui->visibleProfileList->currentRow();
//     // qDebug() << "currRow: " << currRow;
//     // QListWidgetItem* currItem = ui->visibleProfileList->item(currRow);
//     // ui->visibleProfileList->editItem(currItem);

//     dialog.editProfile();
// }

void profileMenu::deleteButtonPressed(){
    qDebug("Profiles: Delete Button pressed");
    QMessageBox::StandardButton reply;

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
        QString textOfselectedItem = ui->visibleProfileList->selectedItems().at(0)->text();
        qDebug() << "Item selected: " << textOfselectedItem;
        confirmationText.append(textOfselectedItem);
    }

    confirmationText.append("</b> with all its clips? <br><br><i>(WARNING: This is irreversible)</i>");

    reply = messageBox.question(this, "Delete Confirmation", confirmationText,
                                QMessageBox::No|QMessageBox::Yes, QMessageBox::No);

    if(reply == QMessageBox::Yes){
        int currRow = ui->visibleProfileList->currentRow();
        QListWidgetItem* itemToDelete = ui->visibleProfileList->takeItem(currRow);

        QString delName = itemToDelete->text();
        profAction act{delName, ""};
        //profActions* act = new profActions(delName, "");
        this->unsavedActions.append(act);
        delete itemToDelete;

        ui->visibleProfileList->setCurrentRow(-1);

        // ui->visibleProfileList->sort

    }else{
        ; //do nothing?
    }

    //ToDo check case where ALL PROFILES get deleted!! (what happens? what SHOULD happen?)
}

void profileMenu::cancelButtonPressed(){
    qDebug("Profiles: Cancel Button pressed");

    this->reconstructVisibleListFromInternal();
    unsavedActions.clear();
    this->close();
}

QString profileMenu::constructFilePathForProfileJson(QString profileName){
    return (QDir::currentPath() + "/profiles/" + (profileName)) + ".json";
}

void profileMenu::renameProfilesJson(QString oldName, QString newName){
    qDebug() << "-- renameProfilesJson. input: " << oldName << ", " << newName;
    QString oldNamePath = constructFilePathForProfileJson(oldName);
    QString newNamePath = constructFilePathForProfileJson(newName);
    qDebug() << "-- renameProfilesJson. paths: " << oldNamePath << ", " << newNamePath;
    bool renameSuccess = QFile::rename(oldNamePath, newNamePath);
    //ToDo what if fails?
    qDebug() << "renameSuccess: " << renameSuccess;
}

void profileMenu::createNewProfilesJson(QString name){
    qDebug() << "-- createNewProfilesJson. input: " << name;
    //QString namePath = constructFilePathForProfileJson(name);
    ; //ToDo create some kind of default json
    emit this->newProfileCreated(name);
}

void profileMenu::deleteProfilesJson(QString name){
    qDebug() << "-- deleteProfilesJson. input: " << name;
    QString namePath = constructFilePathForProfileJson(name);
    qDebug() << "-- deleteProfilesJson. path: " << namePath;
    bool deleteSuccess = QFile::remove(namePath);
    qDebug() << "deleteSuccess: " << deleteSuccess;
}

void profileMenu::processProfilesActions(){
    // for(int i = 0; i < unsavedActions.count(); i++){

    // }


    // delName | newName


    // edit:
    // "___" + "___"

    // new:
    // "" + "___"

    // del:
    // "___" + ""

    qDebug() << "     ### handleProfileFilesForSaveAction";

    while(!unsavedActions.empty()){

        profAction act = unsavedActions.takeFirst();

        qDebug() << "-- act. input: " << act.delName << ", " << act.newName;

        if(!act.delName.isEmpty() && !act.newName.isEmpty()){
            renameProfilesJson(act.delName, act.newName);
        }else if(act.delName.isEmpty() && !act.newName.isEmpty()){
            createNewProfilesJson(act.newName);
        }else if(!act.delName.isEmpty() && act.newName.isEmpty()){
            deleteProfilesJson(act.delName);
        }else{
            qDebug() << "Error. Both strings for action are empty.";
        }

    }

    qDebug() << "     /### handleProfileFilesForSaveAction";
}

void profileMenu::saveButtonPressed(){
    qDebug() << "\n\n\n       Profiles: SAAAAAAAAAAAAAVEEEEEEE";

    //do not allow saving when no profile has been selected?
    //(or just make it so that you can't add/delete items without having a profile?)
    if(!(this->ui->visibleProfileList->selectedItems().empty())){
        qDebug() << "list of visible profiles is not empty!";
        this->saveVisibleListToInternal();
        this->saveProfiles();

        //3 possible cases:
        // 1) delete files of all profiles that are were deleted
        // 2) create new default JSON files for all new profiles
        // 3) rename files to new names
        this->processProfilesActions();
        close();
    }else{
        timedPopUp(this, 3000, "<p align='center'>Please<br><b>select a profile</b><br>to continue.</p>");
        qDebug() << "no profile selected!";
    }

    //emit signal with new profile name?
    qDebug() << "lastSavedSelProfileID: " << lastSavedSelProfileID;
    if(this->lastSavedSelProfileID >= 0 && lastSavedSelProfileID < this->internalProfilesList.count()){
        qDebug() << "! emit !";
        emit this->selProfileHasChanged(this->internalProfilesList.at(this->lastSavedSelProfileID));
        qDebug() << "! /emit !";
    }else{
        qDebug() << "lastSavedSelProfileID was invalid";
        emit this->selProfileHasChanged("");
    }
}

// //static
// bool profileMenu::checkStringIsAlphanumeric(QString strToCheck){
//     bool stringIsValid = true;
//     int strSize = strToCheck.size();
//     qDebug() << "Str Size: " << strSize;
//     if(strSize > 0){
//         //check each character individually
//         //(sadly there doesn't appear to exist a library utility function for this)
//         for(int i = 0; i < strSize && stringIsValid == true; i++){
//             qDebug() << "i :" << i;
//             if (!(strToCheck[i].isDigit() || strToCheck[i].isLetter())){
//                 qDebug() << "Break! String is not valid";
//                 stringIsValid = false;
//                 break;
//             }
//         }
//     }else{
//         stringIsValid = false;
//     }

//     qDebug() << "stringIsValid: " << stringIsValid;
//     return stringIsValid;
// }

// bool profileMenu::nameCanBeUsed(bool isEditOperation, QString userInput){

//     qDebug() << "start: checkForDuplicate";

//     qDebug() << "isEditOperation: " << isEditOperation;

//     for(int i = 0; i < ui->visibleProfileList->count(); i++){
//         if(userInput == ui->visibleProfileList->item(i)->text()){

//             qDebug() << "Same text!";
//             //if we are editing an existing name, and it hasn't change
//             //  then that is not a duplicate & we will accept the input

//             qDebug() << "i: " << i;
//             qDebug() << "ui->visibleProfileList->currentRow(): " << ui->visibleProfileList->currentRow();

//             if(isEditOperation && (ui->visibleProfileList->currentRow() == i)){
//                 qDebug() << "Profile name edit -- Text has not changed!";
//                 return true;
//             }else{
//                 qDebug() << "Profile name edit -- Text is a duplicate!";
//                 return false;
//             }
//         }
//     }

//     qDebug() << "end: checkForDuplicate (none found)";

//     return true;
// }

// void profileMenu::addEditNameActionToUnsavedActions(QString userInput){
//     //if(1){} //maybe add additional check for correctness of index before editing?
//     QString oldName = ui->visibleProfileList->selectedItems().at(0)->text();
//     ui->visibleProfileList->selectedItems().at(0)->setText(userInput);

//     qDebug() << "   ## (edit) oldName: " << oldName << ", userInput: " << userInput;

//     profAction act{oldName, userInput};
//     this->unsavedActions.append(act);
// }

// void profileMenu::addNewNameActionToUnsavedActions(QString userInput){
//     ui->visibleProfileList->addItem(userInput);

//     qDebug() << "   ## (new) userInput: " << userInput;

//     profAction act{"", userInput};
//     this->unsavedActions.append(act);
// }

// //isEditOperation == false --> is "new" operation (new item added to list)
// bool profileMenu::getUserInputAndCheck(bool isEditOperation, QString windowName, QString promptText, QString defaultTxtForInput){
//     bool inputOK;
//     QString userInput = QInputDialog::getText(this, windowName, promptText,
//                                               QLineEdit::Normal, defaultTxtForInput, &inputOK);
//     bool stringValid = false;
//     if (inputOK){
//         if((!userInput.isEmpty())){

//             if(checkStringIsAlphanumeric(userInput)){
//                 //check for possible duplicate (not allowed) before adding the new string
//                 if(this->nameCanBeUsed(isEditOperation, userInput)){
//                     qDebug() << "Input OK";
//                     stringValid = true;

//                     if(isEditOperation == true){
//                         this->addEditNameActionToUnsavedActions(userInput);
//                     }else{
//                         this->addNewNameActionToUnsavedActions(userInput);
//                     }
//                 }else{
//                     QString tmpErrorStr;
//                     tmpErrorStr.append("<b>").append(userInput).append("</b> is already in the list! Please choose another name.");
//                     timedPopUp(defaultPopUpTimer, tmpErrorStr);
//                     qDebug() << "Input (" << userInput << ") is a duplicate!";
//                 }
//             }else{
//                 QString tmpErrorStr;
//                 tmpErrorStr.append("<b>").append(userInput).append("</b> is not a valid string.");
//                 timedPopUp(defaultPopUpTimer, tmpErrorStr);
//                 qDebug() << "Input not OK";
//             }
//         }else{
//             timedPopUp(defaultPopUpTimer, "Empty name is not allowed.");
//             qDebug() << "Input is empty!";
//         }
//     }
//     return stringValid;
// }

void profileMenu::addEditNameActionToUnsavedActions(QString oldName, QString newName){
    //if(1){} //maybe add additional check for correctness of index before editing?
    // QString oldName = ui->visibleProfileList->selectedItems().at(0)->text();

    ui->visibleProfileList->selectedItems().at(0)->setText(newName);
    qDebug() << "   ## (edit) oldName: " << oldName << ", newName: " << newName;
    profAction act{oldName, newName};
    this->unsavedActions.append(act);
    this->dialog.close();
}

void profileMenu::addNewNameActionToUnsavedActions(QString newName){
    ui->visibleProfileList->addItem(newName);

    qDebug() << "   ## (new) userInput: " << newName;

    profAction act{"", newName};
    this->unsavedActions.append(act);
    this->dialog.close();
}

QString profileMenu::getCurrSelProfileName(){
    if(this->lastSavedSelProfileID >= 0 && this->lastSavedSelProfileID < this->internalProfilesList.count()){
        return this->internalProfilesList.at(this->lastSavedSelProfileID);
    }else{
        return "";
    }
}

void profileMenu::handleSelectionChange(){
    qDebug() << "handleSelectionChange";

    //ToDo
    if(ui->visibleProfileList->selectedItems().length() == 0){
        qDebug() << "disable";
        this->setEditDelDisabled();
    }else{
        qDebug() << "enable";
        this->setEditDelEnabled();
    }

    qDebug() << "/handleSelectionChange";
}

// void profileMenu::setSaveEnabled(){
//     ui->btnSave->setEnabled(true);
// }

// void profileMenu::setSaveDisabled(){
//     ui->btnSave->setDisabled(true);
// }

void profileMenu::setEditDelEnabled(){
    ui->btnEdit->setEnabled(true);
    ui->btnDelete->setEnabled(true);
}

void profileMenu::setEditDelDisabled(){
    ui->btnEdit->setDisabled(true);
    ui->btnDelete->setDisabled(true);
}
