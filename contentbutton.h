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

    bool isMarkedForDeletion() const;
    void unsetMarkedForDeletion();

    bool isFocused() const;
    void gainFocus();
    void unsetAsFocusedButton();
    static void clearFocusedButton();
    static bool isAnyButtonInFocus();

    void setIndexInGrid(qsizetype index);
    qsizetype getIndexInGrid() const;

    QString getTitle() const;
    void setTitle(QString title);
    bool hasTitle() const;

    QString getContent() const;
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
    bool isNotMarkedForDeletion() const;
    void setMarkedForDeletion();

    bool isNotFocused() const;
    void setAsFocusedButton();

    void setStyleDefault();
    void setStyleFocused();
    void setStyleMarkedForDeletion();
    void setStyleMarkedForDelAndFocus();

    void checkForDynBtnSwitch();

    void openOptionsMenu(QPoint p);
    void mouseLeftClick();
    void mouseRightClick(QMouseEvent *event);

    static contentButton *focusedButton;
    bool markedForDeletion = false;

    QMenu optionsMenu; //right-click menu (can be opened via keyboard too)
    QAction newEditTitleAction;
    QAction removeTitleAction;
    QAction markForDeleteAction;
    QAction deleteButtonAction;

    QAction* titleActionSeparator; //we enable/disable this separator depending on button state

    qsizetype indexInGrid = -1;
    QString title;
    QString content;

    static QString const textForNewTitleAct;
    static QString const textForEditTitleAct;
    static QString const textForRemoveTitleAct;
    static QString const textForMarkDeletionAct;
    static QString const textForUnmarkDeletionAct;
    static QString const textForDeleteButton;

    static int constexpr minButtonSize_w = 10;
    static int constexpr minButtonSize_h = 10;

};

#endif // CONTENTBUTTON_H
