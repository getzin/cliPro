#ifndef CONTENTBUTTON_H
#define CONTENTBUTTON_H

#include <QAbstractButton>
#include <QPushButton>
#include <QMainWindow>

#include "contentbtncount.h"
// #include "dynaddrmbutton.h"

class contentButton : public QPushButton, public contentBtnCount
{
    Q_OBJECT

public:
    // contentButton(dynAddRmButton *dynBtnPtr);
    contentButton();
    ~contentButton();
    bool getIsMarked();
    bool getIsSelected();
    void setMarked();
    void setUnmarked();
    void switchMarking();
    void setAsSelectedButton();
    void unsetAsSelectedButton();
    static void clearButtonSelection();
    // void obtainFocus();

    int indexInGrid = -1;

signals:
    void cntButtonWasLeftClicked();
    void dynBtnNeedsSwitch();
    void keyWasPressed(int key, int indexOfSender);
    void startContentButtonEdit(int indexOfSender);

protected:
    // void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void focusOutEvent(QFocusEvent *event) override;
    void focusInEvent(QFocusEvent *event) override;

private:
    bool isMarked = false;
    bool isSelected = false;
    static contentButton *currSelectedBtn;

    void setStyleSheetWrap(QString str);
    void setStyleButtonIsMarked();
    void setStyleButtonIsSelected();
    void setStyleButtonDefault();
    void mouseReleaseLeftBtn();
    void mouseReleaseRightBtn();
};

#endif // CONTENTBUTTON_H
