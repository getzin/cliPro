#include "buttonedit.h"
#include "ui_buttonedit.h"

buttonEdit::buttonEdit(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::buttonEdit)
{
    this->ui->setupUi(this);
    this->setWindowTitle("Edit text"); //ToDo consider Title? Or maybe buttonEdit should have 2 text inputs (title + text)?
    connect(this->ui->btnCancel, SIGNAL(clicked()), this, SLOT(close()));
    connect(this->ui->btnSaveOnly, SIGNAL(clicked()), this, SLOT(handleSaveOnly()));
    connect(this->ui->btnSaveAndClose, SIGNAL(clicked()), this, SLOT(handleSaveAndClose()));
}

buttonEdit::~buttonEdit()
{
    delete ui;
}

void buttonEdit::editButton(contentButton* btn){
    if(btn == nullptr){
        qDebug() << "button is NULL.";
    }else{
        this->currentlyEditing = btn;
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

void buttonEdit::save(bool closeAtEnd){
    qDebug() << "start: handleSave!";
    if(this->currentlyEditing == nullptr){
        qDebug() << "currently not editing a button.";
    }else{
        this->currentlyEditing->setContent(this->ui->editContent->toPlainText());
        this->currentlyEditing->setTitle(this->ui->editTitle->toPlainText());
        this->currentlyEditing->repaint();
        if(closeAtEnd){
            this->close();
        }
    }
    qDebug() << "end: handleSave!";
}

void buttonEdit::handleSaveOnly(){
    qDebug() << "start: handleSaveOnly!";
    this->save(false);
    qDebug() << "end: handleSaveOnly!";
}

void buttonEdit::handleSaveAndClose(){
    qDebug() << "start: handleSaveAndClose!";
    this->save(true);
    qDebug() << "end: handleSaveAndClose!";
}
