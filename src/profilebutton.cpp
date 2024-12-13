#include "profilebutton.h"

profileButton::profileButton(QWidget* &, profileMenu* menu){
    qDebug() << "begin: profileButton ctor";
    qDebug() << "create profile menu";
    // profilesMenu = new profileMenu(this);
    // profilesMenu->setModal(true);
    this->profilesMenuRef = menu;
    qDebug() << "end: profileButton ctor";
}

profileButton::~profileButton(){
    qDebug() << "begin: profileButton dtor";
    // qDebug() << "delete profile menu";
    // delete profilesMenu;
    qDebug() << "end: profileButton dtor";
}

void profileButton::mouseReleaseEvent(QMouseEvent *event){
    qDebug() << "profileButton PRESSED!";
    if(profilesMenuRef){
        qDebug() << "profiles exist";
        profilesMenuRef->show();
    }else{
        qDebug() << "no profiles..";
    }
}
