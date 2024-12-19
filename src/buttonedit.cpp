#include "buttonedit.h"
#include "ui_buttonedit.h"

buttonEdit::~buttonEdit()
{
    delete ui;
}

buttonEdit::buttonEdit(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::buttonEdit)
{
    this->init();
}

void buttonEdit::init(){
    this->ui->setupUi(this);
    this->setWindowTitle("Edit button");
    this->setModal(true);
    connect(this->ui->btnCancel, SIGNAL(clicked()), this, SLOT(close()));
    connect(this->ui->btnSaveOnly, SIGNAL(clicked()), this, SLOT(handleSaveOnly()));
    connect(this->ui->btnSaveAndClose, SIGNAL(clicked()), this, SLOT(handleSaveAndClose()));
}

void buttonEdit::editButton(contentButton *const btn){
    if(btn == nullptr){
        qDebug() << "button is NULL.";
    }else{
        this->currentlyEditing = btn;
        this->ui->editContent->setText(btn->getContent());
        this->ui->editTitle->setText(btn->getTitle());
        if(btn->hasTitle()){
            this->ui->editTitle->setVisible(true);
        }else{
            this->ui->editTitle->setVisible(false);
        }
        this->ui->editContent->setFocus(); //ToDo check again if really needed
        this->ui->editContent->moveCursor(QTextCursor::End);
        this->show();
    }
}

//slot
void buttonEdit::save(bool const closeAtEnd){
    if(this->currentlyEditing == nullptr){
        qDebug() << "currently not editing a button.";
        this->close();
    }else{
        this->currentlyEditing->setTitle(this->ui->editTitle->toPlainText().remove('\n')); //ToDo Optimize
        this->currentlyEditing->setContent(this->ui->editContent->toPlainText().remove('\n')); //ToDo Optimize
        this->currentlyEditing->saveJSON(); //ToDo optimize the JSON saving process (only single button) + decouple saving content/title
        this->currentlyEditing->repaint();
        if(closeAtEnd){
            this->close();
        }
    }
}

//slot
void buttonEdit::handleSaveOnly(){
    this->save(false);
}

//slot
void buttonEdit::handleSaveAndClose(){
    this->save(true);
}