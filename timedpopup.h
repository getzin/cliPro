#ifndef TIMEDPOPUP_H
#define TIMEDPOPUP_H

#include <QString>
#include <QMessageBox>
#include <QTimer>

void timedPopUp(QWidget *parent, int timer_ms, QString message);
int constexpr defaultPopUpTimer = 3000; //ms

#endif // TIMEDPOPUP_H
