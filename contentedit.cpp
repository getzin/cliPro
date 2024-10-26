#include "contentedit.h"
#include "ui_contentedit.h"

contentEdit::contentEdit(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::contentEdit)
{
    ui->setupUi(this);

    this->setWindowTitle("Edit text"); //ToDo consider Title? Or maybe contentEdit should have 2 text inputs (title + text)?

    connect(ui->btnCancel, SIGNAL(clicked()), this, SLOT(close()));
    connect(ui->btnSaveOnly, SIGNAL(clicked()), this, SLOT(handleSaveOnly()));
    connect(ui->btnSaveAndClose, SIGNAL(clicked()), this, SLOT(handleSaveAndClose()));
}

contentEdit::~contentEdit()
{
    delete ui;
}


// void contentEdit::showWithText(QString text){
//     this->ui->textEdit->setText(text);
//     this->show();
// }


void contentEdit::editButton(contentButton* btn){
    if(btn == nullptr){
        qDebug() << "button is NULL.";
    }else{
        currentlyEditing = btn;
        this->ui->textEdit->setText(btn->text());
        this->ui->textEdit->setFocus(); //ToDo check again if really needed
        this->show();
    }
}

void contentEdit::handleSave(bool closeAtEnd){
    qDebug() << "handleSave!";
    if(currentlyEditing == nullptr){
        qDebug() << "currently not editing a button.";
    }else{
        currentlyEditing->setText(this->ui->textEdit->toPlainText());
        if(closeAtEnd){
            this->close();
        }
    }
    qDebug() << "/handleSave!";
}

void contentEdit::handleSaveOnly(){
    qDebug() << "handleSaveOnly!";

    handleSave(false);

    qDebug() << "/handleSaveOnly!";
}

void contentEdit::handleSaveAndClose(){
    qDebug() << "handleSaveAndClose!";

    handleSave(true);

    qDebug() << "/handleSaveAndClose!";
}

// QPushButton* contentEdit::getCancelBtn(){
//     return ui->btnCancel;
// }

// QPushButton* contentEdit::getSaveBtn(){
//     return ui->btnSave;
// }
