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
    void gainFocus();
    void unsetAsFocusedButton();
    static void clearFocusedButton();

    // void obtainFocus();
    int indexInGrid = -1;

    QString getTitle();
    QString getContent();
    void setTitle(QString title);
    void setContent(QString content);
    bool hasTitle();
    // bool hasContent();


signals:
    // void contentButtonWasLeftClicked();
    void dynBtnSetMode(dynAddRmButton::btnMode);
    // void dynBtnSwitchMode(dynAddRmButton::btnMode);
    // void dynBtnSetAddMode();
    // void dynBtnSetRmMode();
    void deleteButton(int indexOfSender);
    void keyWasPressed(int key, int indexOfSender);
    void startContentButtonEdit(int indexOfSender);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    // void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void focusOutEvent(QFocusEvent *event) override;
    void focusInEvent(QFocusEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private slots:
    void titleAdjust();
    void removeTitle();
    void markForDeletion();
    void deleteThisButton();

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
    QAction newEditTitleAction;
    QAction removeTitleAction;
    QAction markForDeleteAction;
    QAction deleteButtonAction;

    QAction* titleActionSeparator;

    QString title;
    QString content;

    static const QString textForNewTitleAct;
    static const QString textForEditTitleAct;
    static const QString textForRemoveTitleAct;
    static const QString textForMarkDeletionAct;
    static const QString textForUnmarkDeletionAct;
    static const QString textForDeleteButton;


    static const int minButtonSize_w = 10;
    static const int minButtonSize_h = 10;

};

#endif // CONTENTBUTTON_H
