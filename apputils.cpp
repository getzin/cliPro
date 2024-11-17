#include "apputils.h"

#include <QMessageBox>
#include <QTimer>

void timedPopUp(QWidget *parent, int timer_ms, QString message){
    QMessageBox msgBox(parent);
    QTimer::singleShot(timer_ms, &msgBox, &QMessageBox::close);
    msgBox.setStandardButtons(QMessageBox::Close);
    msgBox.setDefaultButton(QMessageBox::Close);
    msgBox.setText(message);
    msgBox.setWindowTitle("ERROR"); //ToDo maybe set via parameter
    msgBox.exec();
}

bool indexIsInBounds(qsizetype index, qsizetype upperBound){
    if(index >= 0 && index < upperBound){
        return true;
    }
    return false;
}
