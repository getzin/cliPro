#include "profilenamedialog.h"
#include "ui_profilenamedialog.h"

#include <QKeyEvent>

#include "apputils.h"

profileNameDialog::~profileNameDialog()
{
    delete this->ui;
}

profileNameDialog::profileNameDialog(QWidget * const parent)
    : QDialog(parent)
    , ui(new Ui::profileNameDialog)
{
    this->init();
}

void profileNameDialog::init(){
    this->ui->setupUi(this);
    this->setModal(true);
    this->setFixedSize(this->windowSize_w, this->WindowSize_h);

    connect(this->ui->buttonOK, SIGNAL(clicked()), this, SLOT(processOKbuttonPressed()));
    connect(this->ui->buttonCancel, SIGNAL(clicked()), this, SLOT(close()));
    connect(this->ui->nameInputField, SIGNAL(textEdited(QString)), this, SLOT(adjustOKButton()));
}

void profileNameDialog::setPtrToVisibleProfileList(QListWidget const * const visibleProfileList){
    if(visibleProfileList){
        this->PtrToVisibleProfileList = visibleProfileList;
    }
}

void profileNameDialog::adjustOKButton(){
    if(this->ui->nameInputField->text().isEmpty()){
        this->ui->buttonOK->setDisabled(true);
    }else{
        this->ui->buttonOK->setEnabled(true);
    }
}

void profileNameDialog::start(){
    this->ui->nameInputField->setFocus();
    this->open();
}

//slot
void profileNameDialog::startNewProfileCreation(){
    if(this->PtrToVisibleProfileList){
        this->setWindowTitle("New profile");
        this->ui->labelText->setText("Enter name for new profile: ");
        this->ui->nameInputField->setText("");
        this->ui->buttonCancel->setFocus();
        this->currMode = dialogModeNew;
        this->currEditName.clear();
        this->currEditIndex = -1;
        this->start();
    }else{
        qDebug() << "listWidgetPtr has not been set, or has no items (bad).";
        //ToDo error handling?
    }
}

//slot
void profileNameDialog::startEditOfProfileName(){
    qDebug() << "start: startEditOfProfileName";
    if(this->PtrToVisibleProfileList){
        if(this->PtrToVisibleProfileList->count() > 0){
            if(this->PtrToVisibleProfileList->selectedItems().count() == 1){
                QString currentItemTxt = this->PtrToVisibleProfileList->selectedItems().at(0)->text();
                qDebug() << "got string";
                if(!currentItemTxt.isEmpty()){
                    qDebug() << "currentItemTxt not empty (good)";
                    this->setWindowTitle("Edit profile");
                    this->ui->labelText->setText("Change profile name to: ");
                    this->ui->nameInputField->setText(currentItemTxt);
                    this->ui->buttonOK->setFocus();
                    this->currMode = dialogModeEdit;
                    this->currEditName = currentItemTxt;
                    this->currEditIndex = this->PtrToVisibleProfileList->currentRow();
                    this->start();
                }else{
                    qDebug() << "currentItemTxt empty (BAD, should never happen)";
                    //ToDo error handling?
                }
            }else{
                qDebug() << "Either 0, or more than 1 profiles are currently selected. Not necessarily an error, but got nothing to do.";
            }
        }else{
            qDebug() << "The profiles list is empty. Not necessarily an error, but got nothing to do.";
        }
    }else{
        qDebug() << "listWidgetPtr has not been set (BAD, should never happen).";
        //ToDo error handling?
    }
    qDebug() << "end: startEditOfProfileName";
}

void profileNameDialog::keyPressEvent(QKeyEvent * const event){
    int key = event->key();
    if(key == Qt::Key_Return || key == Qt::Key_Enter){
        this->processOKbuttonPressed();
    }else if(key == Qt::Key_Escape){
        this->close();
    }
}

bool profileNameDialog::checkStringIsAlphanumeric(QString const &strToCheck) const{
    bool stringIsValid = true;
    qsizetype strSize = strToCheck.size();
    if(strSize > 0){
        //check each character individually
        //(sadly there doesn't appear to exist a library utility function for this)
        for(qsizetype i = 0; i < strSize && stringIsValid == true; ++i){
            qDebug() << "i :" << i;
            if (!(strToCheck[i].isDigit() || strToCheck[i].isLetter() ||
                  strToCheck[i] == '_' || strToCheck[i] == '-')){
                qDebug() << "Break! String is not valid";
                stringIsValid = false;
                break;
            }
        }
    }else{
        stringIsValid = false;
    }
    return stringIsValid;
}

bool profileNameDialog::checkNameIsNotTaken(QString const &nameTocheck) const{
    qDebug() << "start: checkNameIsNotTaken";
    qDebug() << "current mode (0: edit, 1: new): " << this->currMode;
    if(this->PtrToVisibleProfileList){
        for(qsizetype i = 0; i < this->PtrToVisibleProfileList->count(); ++i){
            if(nameTocheck == this->PtrToVisibleProfileList->item(i)->text()){
                qDebug() << "Same text!";
                qDebug() << "i: " << i;
                qDebug() << "currentRow: " << this->PtrToVisibleProfileList->currentRow();
                //if we are editing an existing name, and it hasn't change
                //  then that is not a duplicate & we will accept the input
                if((this->currMode == dialogModeEdit) && (this->PtrToVisibleProfileList->currentRow() == i)){
                    qDebug() << "We are editing and the name has not changed --> This case is OK.";
                    return true;
                }else{
                    qDebug() << "Text already exists in list and we are not editing. We are trying to create a duplicate entry! (REJECT)";
                    return false;
                }
            }
        }
    }else{
        qDebug() << "listWidgetPtr has not been set (BAD, should never happen).";
    }
    qDebug() << "end: checkNameIsNotTaken (none found)";
    return true;
}

void profileNameDialog::processOKbuttonPressed(){
    QString userInput = this->ui->nameInputField->text();
    if((!userInput.isEmpty())){
        if(userInput.length() <= this->maxProfileNameLength){
            if(this->checkStringIsAlphanumeric(userInput)){
                //check for possible duplicate (not allowed) before adding the new string
                if(this->checkNameIsNotTaken(userInput)){
                    qDebug() << "Input (" << userInput << ") OK";
                    if(this->currMode == dialogModeEdit){
                        emit this->profileNameEdited(currEditName, userInput);
                    }else{
                        emit this->createNewProfile(userInput);
                    }
                }else{
                    QString tmpErrorStr;
                    tmpErrorStr.append("<b>").append(userInput).append("</b> is already in the list!"
                                                                       "<br>Please choose another name.");
                    timedPopUp(this, defaultPopUpTimer, "Error: Duplicate name", tmpErrorStr);
                    qDebug() << "Input (" << userInput << ") is a duplicate!";
                }
            }else{
                QString tmpErrorStr;
                tmpErrorStr.append("<b>").append(userInput).append("</b> is not a valid string."
                                                                   "<br>Only alphanumeric characters, _ and - are allowed."
                                                                   "<br>(No spaces, $, !, ?, ...)");
                timedPopUp(this, defaultPopUpTimer, "Error: Invalid string", tmpErrorStr);
                qDebug() << "Input (" << userInput << ") not OK.";
            }
        }else{
            QString tmpErrorStr;
            tmpErrorStr.append("<b>").append(userInput).append("</b> is too long."
                                                               "<br>Maximum number of allowed characters: "
                                                               + QString::number(this->maxProfileNameLength));
            timedPopUp(this, defaultPopUpTimer, "Error: Name too long", tmpErrorStr);
            qDebug() << "Input (" << userInput << ") is too long! (Max: " << this->maxProfileNameLength << ")";
        }
    }else{
        timedPopUp(this, defaultPopUpTimer, "Error: Empty name", "Empty profile name is not allowed.");
        qDebug() << "Input is empty.";
    }
}
