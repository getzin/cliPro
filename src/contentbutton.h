#ifndef CONTENTBUTTON_H
#define CONTENTBUTTON_H

#include <QAbstractButton>
#include <QPushButton>
#include <QMainWindow>
#include <QMenu>
#include <QAction>
#include <QTextDocument>

#include "contentbtncount.h"
#include "dynbutton.h"

class contentButton : public QPushButton, public contentBtnCount
{
    Q_OBJECT

public:
    enum searchStatus {
        searchStatusDefault = -1,
        searchStatusNoMatch = 0,
        searchStatusMatched = 1,
    };
    ~contentButton();
    explicit contentButton(QWidget *const parent = nullptr);
    contentButton(const contentButton&) = delete;
    contentButton& operator=(const contentButton&) = delete;

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

    void setIndexInList(qsizetype const index);
    qsizetype getIndexInList() const;

    void enableCopyCutRemoveContent();
    void disableCopyCutRemoveContent();
    void enablePasteContent();
    void disablePasteContent();
    void enableMoveButton();
    void disableMoveButton();
    void enableDeleteAllMarked();
    void disableDeleteAllMarked();

    void saveJSON();
    QString getTitle() const;
    void setTitle(QString const &newTitle);
    bool hasTitle() const;
    QString getContent() const;
    void setContent(QString const &newContent);
    void checkIfSearchIsMatched(QString const &searchString);
    void resetSearchStatus();
    searchStatus getSearchStatus() const;

signals:
    void dynBtnSetMode(dynButton::btnMode const mode);
    void deleteButton(qsizetype const indexOfSender);
    void keyWasPressed(int const key, qsizetype const indexOfSender);
    void startContentButtonEdit(qsizetype const indexOfSender);
    void deleteAllMarkedButtons();
    void saveButtonChangesIntoJSON(); //ToDo for editing content and adding/editing/removing title
    void moveButton(qsizetype const indexOfSender);

protected:
    void mousePressEvent(QMouseEvent *const event) override;
    void mouseDoubleClickEvent(QMouseEvent *const event) override;
    void keyPressEvent(QKeyEvent *const event) override;
    void focusOutEvent(QFocusEvent *const event) override;
    void focusInEvent(QFocusEvent *const event) override;
    void paintEvent(QPaintEvent *const event) override;

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
    void emitIndexForMoveButton();

private:
    void initButtonSettings();
    void setUpContextMenu();
    void connectAllActions();

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

    void openOptionsMenu(QPoint const &p);
    void mouseLeftClick();
    void mouseRightClick(QMouseEvent const * const event);

    QMenu optionsMenu; //right-click menu (can be opened via keyboard too)

    QAction addOrEditTitleAction;
    QAction removeTitleAction;
    QAction copyContentAction;
    QAction pasteContentAction;
    QAction cutContentAction;
    QAction removeContentAction;
    QAction moveButtonAction;
    QAction markForDeleteAction;
    QAction deleteAllMarkedAction;
    QAction deleteButtonAction;

    QAction* removeTitleActionSeparator; //this separator is enabled/disabled depending on button state
    QAction* copyContentActionSeparator; //this separator is enabled/disabled depending on button content
    QAction* pasteContentActionSeparator; //this separator is enabled/disabled depending on clipboard content
    QAction* cutContentActionSeparator; //this separator is enabled/disabled depending on button content
    QAction* moveButtonActionSeparator; //this separator is enabled/disabled depending on search active/inactive
    QAction* removeContentActionSeparator; //this separator is enabled/disabled depending on button content
    QAction* deleteAllMarkedActionSeparator; //this separator is enabled/disabled depening on markedCount (>0 or not)

    qsizetype indexInList = -1;
    QString title;
    QString content;
    QString titleDisplayed;
    QString contentDisplayed;
    qreal originalTitleWidth;
    QTextDocument titleDoc;
    QTextDocument contentDoc;

    static int constexpr maxTitleLengthGeneral = 200; //at this point a pop up is shown (there is no reason to save this large of a title)
    static int constexpr maxTitleLengthForDisplaying = 50; //after this many letters, the title starts becoming unreadible
    static int constexpr maxContentLengthGeneral = 100000; //safety value (this already causes minor performance issues)
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
    static QString const textForMoveButtonAct;
    static QString const textForMarkDeletionAct;
    static QString const textForUnmarkDeletionAct;
    static QString const textForDeleteAllMarkedAct;
    static QString const textForDeleteButton;
};

#endif // CONTENTBUTTON_H
