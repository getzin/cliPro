#include "buttonedit.h"
#include "ui_buttonedit.h"

buttonEdit::buttonEdit(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::buttonEdit)
{
    ui->setupUi(this);

    this->setWindowTitle("Edit text"); //ToDo consider Title? Or maybe buttonEdit should have 2 text inputs (title + text)?

    connect(ui->btnCancel, SIGNAL(clicked()), this, SLOT(close()));
    connect(ui->btnSaveOnly, SIGNAL(clicked()), this, SLOT(handleSaveOnly()));
    connect(ui->btnSaveAndClose, SIGNAL(clicked()), this, SLOT(handleSaveAndClose()));
}

buttonEdit::~buttonEdit()
{
    delete ui;
}


// void buttonEdit::showWithText(QString text){
//     this->ui->editContent->setText(text);
//     this->show();
// }


void buttonEdit::editButton(contentButton* btn){
    if(btn == nullptr){
        qDebug() << "button is NULL.";
    }else{
        currentlyEditing = btn;
        // this->ui->textEdit->setText(btn->text());
        this->ui->editContent->setText(btn->getContent());
        if(btn->hasTitle()){
            this->ui->editTitle->setVisible(true);
            this->ui->editTitle->setText(btn->getTitle());
        }else{
            this->ui->editTitle->setVisible(false);
        }
        this->ui->editContent->setFocus(); //ToDo check again if really needed
        this->ui->editContent->moveCursor(QTextCursor::End);
        this->show();
    }
}

void buttonEdit::handleSave(bool closeAtEnd){
    qDebug() << "handleSave!";
    if(currentlyEditing == nullptr){
        qDebug() << "currently not editing a button.";
    }else{
        // currentlyEditing->setText(this->ui->textEdit->toPlainText());
        currentlyEditing->setContent(this->ui->editContent->toPlainText());
        currentlyEditing->setTitle(this->ui->editTitle->toPlainText());
        currentlyEditing->repaint();

        //ToDo call paintEvent in some way or another

        if(closeAtEnd){
            this->close();
        }
    }
    qDebug() << "/handleSave!";
}

void buttonEdit::handleSaveOnly(){
    qDebug() << "handleSaveOnly!";

    handleSave(false);

    qDebug() << "/handleSaveOnly!";
}

void buttonEdit::handleSaveAndClose(){
    qDebug() << "handleSaveAndClose!";

    handleSave(true);

    qDebug() << "/handleSaveAndClose!";
}

// QPushButton* buttonEdit::getCancelBtn(){
//     return ui->btnCancel;
// }

// QPushButton* buttonEdit::getSaveBtn(){
//     return ui->btnSave;
// }
