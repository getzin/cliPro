#include "profilenamedialog.h"
#include "ui_profilenamedialog.h"

#include <QKeyEvent>

#include "apputils.h"

profileNameDialog::profileNameDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::profileNameDialog)
{
    this->ui->setupUi(this);
    this->setModal(true);
    this->setFixedSize(this->windowSize_w, this->WindowSize_h);

    connect(this->ui->buttonOK, SIGNAL(clicked()), this, SLOT(processOKbuttonPressed()));
    connect(this->ui->buttonCancel, SIGNAL(clicked()), this, SLOT(close()));
    connect(this->ui->nameInputField, SIGNAL(textEdited(QString)), this, SLOT(adjustOKButton()));
}

profileNameDialog::~profileNameDialog()
{
    delete this->ui;
}

void profileNameDialog::setPtrToVisibleProfileList(QListWidget *visibleProfileList){
    if(visibleProfileList){
        this->visibleProfileListPtr = visibleProfileList;
    }
}

void profileNameDialog::adjustOKButton(){
    qDebug() << "start: adjustOKButton";
    if(this->ui->nameInputField->text().isEmpty()){
        this->ui->buttonOK->setDisabled(true);
    }else{
        this->ui->buttonOK->setEnabled(true);
    }
    qDebug() << "end: adjustOKButton";
}

void profileNameDialog::start(){
    this->ui->nameInputField->setFocus();
    this->open();
}

void profileNameDialog::startNewProfileCreation(){
    if(this->visibleProfileListPtr){
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

void profileNameDialog::startEditOfProfileName(){
    qDebug() << "start: editProfile";
    if(this->visibleProfileListPtr && (this->visibleProfileListPtr->count() > 0)){
        if(this->visibleProfileListPtr->selectedItems().count() == 1){
            QString currentItemTxt = this->visibleProfileListPtr->selectedItems().at(0)->text();
            qDebug() << "got string";
            if(!currentItemTxt.isEmpty()){
                qDebug() << "currentItemTxt not empty (good)";
                this->setWindowTitle("Edit profile");
                this->ui->labelText->setText("Change profile name to: ");
                this->ui->nameInputField->setText(currentItemTxt);
                this->ui->buttonOK->setFocus();
                this->currMode = dialogModeEdit;
                this->currEditName = currentItemTxt;
                this->currEditIndex = this->visibleProfileListPtr->currentRow();
                this->start();
            }else{
                qDebug() << "currentItemTxt empty (bad)";
                ; //ToDo error?
            }
        }else{
            qDebug() << "Either no or more than 1 profile has been selected (bad)";
        }
    }else{
        qDebug() << "listWidgetPtr has not been set, or has no items (bad).";
    }
    qDebug() << "end: editProfile";
}

void profileNameDialog::keyPressEvent(QKeyEvent *event){
    qDebug() << "start: Key press event! (profileDialog)";
    int key = event->key();
    if(key == Qt::Key_Return || key == Qt::Key_Enter){
        qDebug() << "Enter!";
        this->processOKbuttonPressed();
    }else if(key == Qt::Key_Escape){
        qDebug() << "Escape!";
        this->close();
    }
    qDebug() << "end: Key press event! (profileDialog)";
}

//static
bool profileNameDialog::checkStringIsAlphanumeric(QString strToCheck) const{
    bool stringIsValid = true;
    qsizetype strSize = strToCheck.size();
    qDebug() << "Str Size: " << strSize;
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
    qDebug() << "stringIsValid: " << stringIsValid;
    return stringIsValid;
}

bool profileNameDialog::checkNameIsNotTaken(QString nameTocheck) const{
    qDebug() << "start: nameCanBeUsed";
    qDebug() << "isEditOperation: " << this->currMode;
    if(this->visibleProfileListPtr){
        for(qsizetype i = 0; i < this->visibleProfileListPtr->count(); ++i){
            if(nameTocheck == this->visibleProfileListPtr->item(i)->text()){
                qDebug() << "Same text!";
                //if we are editing an existing name, and it hasn't change
                //  then that is not a duplicate & we will accept the input
                qDebug() << "i: " << i;
                qDebug() << "this->listWidgetPtr->currentRow(): " << this->visibleProfileListPtr->currentRow();
                if((this->currMode == dialogModeEdit) && (this->visibleProfileListPtr->currentRow() == i)){
                    qDebug() << "Profile name edit -- Text has not changed! (OK)";
                    return true;
                }else{
                    qDebug() << "Profile name edit -- Text is a duplicate!";
                    return false;
                }
            }
        }
    }else{
        qDebug() << "listWidgetPtr has not been set, or has no items (bad).";
    }
    qDebug() << "end: checkForDuplicate (none found)";
    return true;
}

void profileNameDialog::processOKbuttonPressed(){
    QString userInput = this->ui->nameInputField->text();
    if((!userInput.isEmpty())){
        if(this->checkStringIsAlphanumeric(userInput)){
            //check for possible duplicate (not allowed) before adding the new string
            if(this->checkNameIsNotTaken(userInput)){
                qDebug() << "Input OK";
                if(this->currMode == dialogModeEdit){
                    emit this->profileNameEdited(currEditName, userInput);
                }else{
                    emit this->createNewProfile(userInput);
                }
            }else{
                QString tmpErrorStr;
                tmpErrorStr.append("<b>").append(userInput).append("</b> is already in the list!<br>Please choose another name.");
                timedPopUp(this, defaultLongPopUpTimer, tmpErrorStr);
                qDebug() << "Input (" << userInput << ") is a duplicate!";
            }
        }else{
            QString tmpErrorStr;
            tmpErrorStr.append("<b>").append(userInput).append("</b> is not a valid string.<br>Only alphanumeric characters, _ and - are allowed.");
            timedPopUp(this, defaultLongPopUpTimer, tmpErrorStr);
            qDebug() << "Input not OK";
        }
    }else{
        timedPopUp(this, defaultLongPopUpTimer, "Empty name is not allowed.");
        qDebug() << "Input is empty!";
    }
}
