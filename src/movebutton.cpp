#include "movebutton.h"
#include "ui_movebutton.h"

moveButton::~moveButton()
{
    delete ui;
}

moveButton::moveButton(QWidget * const parent)
    : QDialog(parent)
    , ui(new Ui::moveButton)
{
    this->init();
}

void moveButton::init(){
    ui->setupUi(this);
    this->setWindowTitle("Change button position");
    this->setModal(true);

    connect(this->ui->btnCancel, SIGNAL(clicked()), this, SLOT(close()));
    connect(this->ui->btnSave, SIGNAL(clicked()), this, SLOT(save()));
    connect(this->ui->valueInput, SIGNAL(valueChanged(int)), this->ui->valueSlider, SLOT(setValue(int)));
    connect(this->ui->valueSlider, SIGNAL(valueChanged(int)), this->ui->valueInput, SLOT(setValue(int)));
}

void moveButton::openMenu(qsizetype const index, qsizetype const maxIndex){
    this->oldIndex = index;

    this->ui->valueInput->setMinimum(0);
    this->ui->valueInput->setMaximum(maxIndex);
    this->ui->valueInput->setValue(index);

    this->ui->valueSlider->setMinimum(0);
    this->ui->valueSlider->setMaximum(maxIndex);
    this->ui->valueSlider->setValue(index);

    this->ui->btnSave->setFocus();
    this->ui->btnSave->setFocusProxy(this->ui->valueInput);

    this->show();
}

//slot
void moveButton::save(){
    qsizetype newIndex = this->ui->valueInput->value();
    if(this->oldIndex != newIndex){
        emit this->updateButtonPosition(this->oldIndex, newIndex);
    }
    this->close();
}
