#ifndef CONTENTBUTTON_H
#define CONTENTBUTTON_H

#include <QAbstractButton>
#include <QPushButton>
#include <QMainWindow>
#include <QMenu>
#include <QAction>

#include "contentbtncount.h"
#include "dynaddrmbutton.h"

class contentButton : public QPushButton, public contentBtnCount
{
    Q_OBJECT

public:
    // contentButton(dynAddRmButton *dynBtnPtr);
    contentButton(QWidget *parent);
    ~contentButton();

    bool isMarked();
    void unsetMarked();
    bool isFocused();
    void unsetAsFocusedButton();
    static void clearFocusedButton();

    // void obtainFocus();

    int indexInGrid = -1;

signals:
    // void cntButtonWasLeftClicked();
    void dynBtnSetMode(dynAddRmButton::btnMode);
    // void dynBtnSwitchMode(dynAddRmButton::btnMode);
    // void dynBtnSetAddMode();
    // void dynBtnSetRmMode();
    void keyWasPressed(int key, int indexOfSender);
    void startContentButtonEdit(int indexOfSender);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    // void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void focusOutEvent(QFocusEvent *event) override;
    void focusInEvent(QFocusEvent *event) override;

private slots:
    void handleTitleAdjust();
    void markForDeletion();

private:
    static contentButton *focusedButton;
    bool marked = false;

    bool notMarked();
    bool notFocused();
    void setMarked();
    void switchMarking();
    void setAsFocusedButton();
    static bool isAnyButtonInFocus();

    void setStyleDefault();
    void setStyleMarked();
    void setStyleFocused();
    void setStyleMarkedAndFocus();

    void checkForDynBtnSwitch();

    void openMenu(QPoint p);
    void mouseLeftClick();
    void mouseRightClick(QMouseEvent *event);

    QMenu optionsMenu; //right-click menu (but can be opened via keyboard too)
    QAction adjustTitleAction;
    QAction markForDeleteAction;

    static const QString textForTitleAddition;
    static const QString textForTitleEditing;
    static const QString textForMarkDeletion;
    static const QString textForUnmarkDeletion;
};

#endif // CONTENTBUTTON_H
