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
    void unsetAsLastUnfocusedButton();
    static void clearFocusedButton();
    static bool isAnyButtonInFocus();
    static void restoreLastUnfocusedButtonToFocusedButton();
    static void clearLastUnfocusedButton();

    void setIndexInGrid(qsizetype index);
    qsizetype getIndexInGrid() const;

    void enableCopyContent();
    void disableCopyContent();
    void enablePasteContent();
    void disablePasteContent();

    void saveJSON();
    QString getTitle() const;
    void setTitle(QString newTitle);
    bool hasTitle() const;
    QString getContent() const;
    void setContent(QString newContent);


signals:
    void dynBtnSetMode(dynAddRmButton::btnMode);
    void deleteButton(qsizetype indexOfSender);
    void keyWasPressed(int key, qsizetype indexOfSender);
    void startContentButtonEdit(qsizetype indexOfSender);

    void saveButtonChangesIntoJSON(); //ToDo for editing content and adding/editing/removing title

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void focusOutEvent(QFocusEvent *event) override;
    void focusInEvent(QFocusEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private slots:
    void titleAdjust();
    void copyContentToClipboard();
    void pasteContentFromClipboard();
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

    QMenu optionsMenu; //right-click menu (can be opened via keyboard too)
    QAction newEditTitleAction;
    QAction removeTitleAction;
    QAction copyContentAction;
    QAction pasteContentAction;
    QAction markForDeleteAction;
    QAction deleteButtonAction;
    QAction* removeTitleActionSeparator; //we enable/disable this separator depending on button state
    QAction* copyContentActionSeparator; //we enable/disable this separator depending on button content
    QAction* pasteContentActionSeparator; //we enable/disable this separator depending on clipboard content

    qsizetype indexInGrid = -1;
    QString title;
    QString content;
    bool markedForDeletion = false;

    static contentButton *focusedButton;
    static contentButton *lastUnfocusedButton;
    static int constexpr minButtonSize_w = 70;
    static int constexpr minButtonSize_h = 70;

    static QString const textForNewTitleAct;
    static QString const textForEditTitleAct;
    static QString const textForRemoveTitleAct;
    static QString const textForCopyContentAct;
    static QString const textForPasteContentAct;
    static QString const textForMarkDeletionAct;
    static QString const textForUnmarkDeletionAct;
    static QString const textForDeleteButton;
};

#endif // CONTENTBUTTON_H
