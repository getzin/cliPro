#include "apputils.h"

#include <QMessageBox>
#include <QTimer>

void timedPopUp(QWidget *const parent, int const timer_ms, QString const &windowTitle, QString const &message){
    QMessageBox msgBox(parent);
    QTimer::singleShot(timer_ms, &msgBox, &QMessageBox::close);
    msgBox.setStandardButtons(QMessageBox::Close);
    msgBox.setDefaultButton(QMessageBox::Close);
    msgBox.setText(message);
    msgBox.setWindowTitle(windowTitle);
    msgBox.exec();
}

bool indexIsInBounds(qsizetype const index, qsizetype const upperBound){
    if(index >= 0 && index < upperBound){
        return true;
    }
    return false;
}
