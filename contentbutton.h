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
    explicit contentButton(QWidget *parent);
    ~contentButton();

    bool isMarkedForDeletion();
    void unsetMarkedForDeletion();

    bool isFocused();
    void gainFocus();
    void unsetAsFocusedButton();
    static void clearFocusedButton();
    static bool isAnyButtonInFocus();

    qsizetype indexInGrid = -1; //ToDo move to private and use getter/setter instead

    QString getTitle();
    void setTitle(QString title);
    bool hasTitle();

    QString getContent();
    void setContent(QString content);


signals:
    void dynBtnSetMode(dynAddRmButton::btnMode);
    void deleteButton(qsizetype indexOfSender);
    void keyWasPressed(int key, qsizetype indexOfSender);
    void startContentButtonEdit(qsizetype indexOfSender);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void focusOutEvent(QFocusEvent *event) override;
    void focusInEvent(QFocusEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private slots:
    void titleAdjust();
    void removeTitle();
    void switchMarkedForDeletion();
    void deleteThisButton();

private:
    static contentButton *focusedButton;
    bool markedForDeletion = false;

    bool notMarkedForDeletion();
    bool notFocused();
    void setMarkedForDeletion();
    void setAsFocusedButton();

    void setStyleDefault();
    void setStyleFocused();
    void setStyleMarkedForDeletion();
    void setStyleMarkedForDelAndFocus();

    void checkForDynBtnSwitch();

    void openOptionsMenu(QPoint p);
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
