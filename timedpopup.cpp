#include "timedpopup.h"

void timedPopUp(QWidget *parent, int timer_ms, QString message){
    QMessageBox msgBox(parent);
    QTimer::singleShot(timer_ms, &msgBox, &QMessageBox::close);
    msgBox.setStandardButtons(QMessageBox::Close);
    msgBox.setDefaultButton(QMessageBox::Close);
    msgBox.setText(message);
    msgBox.setWindowTitle("ERROR"); //ToDo maybe set via parameter
    msgBox.exec();
}
