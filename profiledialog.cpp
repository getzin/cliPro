#include "profiledialog.h"
#include "ui_profiledialog.h"

#include <QKeyEvent>

#include "timedpopup.h"


profileDialog::profileDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::profileDialog)
{
    ui->setupUi(this);
    this->setModal(true);
    this->setFixedSize(240,130);

    connect(this->ui->buttonOK, SIGNAL(clicked()), this, SLOT(processOK()));
    connect(this->ui->buttonCancel, SIGNAL(clicked()), this, SLOT(close()));
    connect(this->ui->nameInputField, SIGNAL(textEdited(QString)), this, SLOT(adjustOKButton()));
}

profileDialog::~profileDialog()
{
    delete ui;
}

void profileDialog::setPtrToListWidget(QListWidget *listWidget){
    if(listWidget){
        listWidgetPtr = listWidget;
    }
}

void profileDialog::adjustOKButton(){
    qDebug() << "start: adjustOKButton";
    if(this->ui->nameInputField->text().isEmpty()){
        this->ui->buttonOK->setDisabled(true);
    }else{
        this->ui->buttonOK->setEnabled(true);
    }

    qDebug() << "end: adjustOKButton";
}

void profileDialog::start(){
    this->ui->nameInputField->setFocus();
    this->open();
}

void profileDialog::newProfile(){

    if(this->listWidgetPtr){
        this->setWindowTitle("New profile");
        this->ui->labelText->setText("Enter name for new profile: ");
        this->ui->nameInputField->setText("");
        this->ui->buttonCancel->setFocus();

        currMode = dialogModeNew;
        this->currEditName.clear();
        this->currEditIndex = -1;

        this->start();
    }else{
        qDebug() << "listWidgetPtr has not been set";
    }
}

void profileDialog::editProfile(){

    qDebug() << "start: editProfile";

    if(this->listWidgetPtr && (this->listWidgetPtr->count() > 0)){
        if(this->listWidgetPtr->selectedItems().size() == 1){
            QString currentItemTxt = this->listWidgetPtr->selectedItems().at(0)->text();

            qDebug() << "got string";

            if(!currentItemTxt.isEmpty()){
                qDebug() << "currentItemTxt not empty (good)";
                this->setWindowTitle("Edit profile");
                this->ui->labelText->setText("Change profile name to: ");
                this->ui->nameInputField->setText(currentItemTxt);
                this->ui->buttonOK->setFocus();

                currMode = dialogModeEdit;
                this->currEditName = currentItemTxt;
                this->currEditIndex = this->listWidgetPtr->currentRow();

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

void profileDialog::keyPressEvent(QKeyEvent *event){
    qDebug() << "Key press event! (profileDialog)";

    int key = event->key();

    if(key == Qt::Key_Return || key == Qt::Key_Enter){
        qDebug() << "Enter!";
        this->processOK();
    }else if(key == Qt::Key_Escape){
        qDebug() << "Escape!";
        this->close();
    }
    qDebug() << "/Key press event! (profileDialog)";
}

// void profileDialog::processOK(){
//     qDebug() << "processOK";

//     // if(0){
//     //     ; //display error messages?
//     // }else{
//     //     ; //do stuff
//     //     // close();
//     // }

//     // if(currMode == )
//     // emit verifyInput();

//     QString currTxt = this->ui->nameInputField->text();

//     qDebug() << "/processOK";
// }




//static
bool profileDialog::checkStringIsAlphanumeric(QString strToCheck){
    bool stringIsValid = true;
    int strSize = strToCheck.size();
    qDebug() << "Str Size: " << strSize;
    if(strSize > 0){
        //check each character individually
        //(sadly there doesn't appear to exist a library utility function for this)
        for(int i = 0; i < strSize && stringIsValid == true; i++){
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

bool profileDialog::nameCanBeUsed(QString userInput){

    qDebug() << "start: checkForDuplicate";

    qDebug() << "isEditOperation: " << this->currMode;

    for(int i = 0; i < this->listWidgetPtr->count(); i++){
        if(userInput == this->listWidgetPtr->item(i)->text()){

            qDebug() << "Same text!";
            //if we are editing an existing name, and it hasn't change
            //  then that is not a duplicate & we will accept the input

            qDebug() << "i: " << i;
            qDebug() << "this->listWidgetPtr->currentRow(): " << this->listWidgetPtr->currentRow();

            if((this->currMode == dialogModeEdit)
                && (this->listWidgetPtr->currentRow() == i)){
                qDebug() << "Profile name edit -- Text has not changed! (OK)";
                return true;
            }else{
                qDebug() << "Profile name edit -- Text is a duplicate!";
                return false;
            }
        }
    }

    qDebug() << "end: checkForDuplicate (none found)";
    return true;
}

// void profileDialog::addEditNameActionToUnsavedActions(QString userInput){
//     //if(1){} //maybe add additional check for correctness of index before editing?
//     QString oldName = this->listWidgetPtr->selectedItems().at(0)->text();
//     this->listWidgetPtr->selectedItems().at(0)->setText(userInput);

//     qDebug() << "   ## (edit) oldName: " << oldName << ", userInput: " << userInput;

//     profAction act{oldName, userInput};
//     this->unsavedActions.append(act);
// }

// void profileDialog::addNewNameActionToUnsavedActions(QString userInput){
//     this->listWidgetPtr->addItem(userInput);

//     qDebug() << "   ## (new) userInput: " << userInput;

//     profAction act{"", userInput};
//     this->unsavedActions.append(act);
// }


// #include <QInputDialog>
//isEditOperation == false --> is "new" operation (new item added to list)
void profileDialog::processOK(){
// void profileDialog::processUserInput(bool isEditOperation, QString windowName, QString promptText, QString defaultTxtForInput){
    // bool inputOK;
    // QString userInput = QInputDialog::getText(this, windowName, promptText,
    //                                           QLineEdit::Normal, defaultTxtForInput, &inputOK);
    // bool stringValid = false;
    // if (inputOK){

    QString userInput = this->ui->nameInputField->text();

        if((!userInput.isEmpty())){

            if(this->checkStringIsAlphanumeric(userInput)){

                // if(this->currMode == dialogModeEdit && (this->listWidgetPtr->currentRow() == this->currEditIndex)){
                //     qDebug() << "Profile name edit -- Text has not changed!";
                //     return true;
                // }else{
                //     qDebug() << "Profile name edit -- Text is a duplicate!";
                //     return false;
                // }

                //check for possible duplicate (not allowed) before adding the new string
                if(this->nameCanBeUsed(userInput)){
                    qDebug() << "Input OK";
                    // stringValid = true;

                    if(this->currMode == dialogModeEdit){
                        // this->addEditNameActionToUnsavedActions(userInput);
                        //ToDo
                        emit editName(currEditName, userInput);
                    }else{
                        // this->addNewNameActionToUnsavedActions(userInput);
                        //ToDo
                        emit newName(userInput);
                    }
                }else{
                    QString tmpErrorStr;
                    tmpErrorStr.append("<b>").append(userInput).append("</b> is already in the list!<br>Please choose another name.");
                    timedPopUp(this, defaultPopUpTimer, tmpErrorStr);
                    qDebug() << "Input (" << userInput << ") is a duplicate!";
                }
            }else{
                QString tmpErrorStr;
                tmpErrorStr.append("<b>").append(userInput).append("</b> is not a valid string.<br>Only alphanumeric characters, _ and - are allowed.");
                timedPopUp(this, defaultPopUpTimer, tmpErrorStr);
                qDebug() << "Input not OK";
            }
        }else{
            timedPopUp(this, defaultPopUpTimer, "Empty name is not allowed.");
            qDebug() << "Input is empty!";
        }
    // }
//     return stringValid;
}
