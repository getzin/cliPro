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
    enum searchStatus {
        searchStatusDefault = -1,
        searchStatusNoMatch = 0,
        searchStatusMatched = 1,
    };
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

    void setIndexInList(qsizetype index);
    qsizetype getIndexInList() const;

    void enableCopyCutRemoveContent();
    void disableCopyCutRemoveContent();
    void enablePasteContent();
    void disablePasteContent();
    void enableDeleteAllMarked();
    void disableDeleteAllMarked();

    void saveJSON();
    QString getTitle() const;
    void setTitle(QString newTitle);
    bool hasTitle() const;
    QString getContent() const;
    void setContent(QString newContent);
    void checkIfSearchIsMatched(QString searchString);
    void resetSearchStatus();
    searchStatus getSearchStatus();

signals:
    void dynBtnSetMode(dynAddRmButton::btnMode);
    void deleteButton(qsizetype indexOfSender);
    void keyWasPressed(int key, qsizetype indexOfSender);
    void startContentButtonEdit(qsizetype indexOfSender);
    void deleteAllMarkedButtons();

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
    void removeTitle();
    void copyContentToClipboard();
    void pasteContentFromClipboard();
    void cutContentToClipboard();
    void removeContent();
    void switchMarkedForDeletion();
    void deleteThisButton();
    void emitDeleteAllSignal();

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
    void clearContent();

    void openOptionsMenu(QPoint p);
    void mouseLeftClick();
    void mouseRightClick(QMouseEvent *event);

    QMenu optionsMenu; //right-click menu (can be opened via keyboard too)
    QAction newEditTitleAction;
    QAction removeTitleAction;
    QAction copyContentAction;
    QAction pasteContentAction;
    QAction cutContentAction;
    QAction removeContentAction;
    QAction markForDeleteAction;
    QAction deleteAllMarkedAction;
    QAction deleteButtonAction;
    QAction* removeTitleActionSeparator; //we enable/disable this separator depending on button state
    QAction* copyContentActionSeparator; //we enable/disable this separator depending on button content
    QAction* pasteContentActionSeparator; //we enable/disable this separator depending on clipboard content
    QAction* cutContentActionSeparator; //we enable/disable this separator depending on button content
    QAction* removeContentActionSeparator; //we enable/disable this separator depending on button content
    QAction* deleteAllMarkedActionSeparator; //we enable/disable this separator depening on markedCount (>0 or not)

    qsizetype indexInList = -1;
    QString title;
    QString content;
    QString titleDisplayed;
    QString contentDisplayed;
    static int constexpr maxTitleLengthForDisplaying = 50; //after this many letters, the title starts becoming unreadible
    static int constexpr maxContentLengthForDisplaying = 1000; //there will be heavy performance issues past a certain size
    static int constexpr maxContentLinesForDisplaying = 20; //same as length
    bool markedForDeletion = false;
    searchStatus buttonMatchesSearch = searchStatusDefault;

    static contentButton *focusedButton;
    static contentButton *lastUnfocusedButton;
    static int constexpr minButtonSize_w = 70;
    static int constexpr minButtonSize_h = 70;

    static QString const textForNewTitleAct;
    static QString const textForEditTitleAct;
    static QString const textForRemoveTitleAct;
    static QString const textForCopyContentAct;
    static QString const textForPasteContentAct;
    static QString const textForCutContentAct;
    static QString const textForRemoveContentAct;
    static QString const textForMarkDeletionAct;
    static QString const textForUnmarkDeletionAct;
    static QString const textForDeleteAllMarkedAct;
    static QString const textForDeleteButton;
};

#endif // CONTENTBUTTON_H
