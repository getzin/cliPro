#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidget>
#include <QStandardPaths>

#include "contentbutton.h"
#include "dynaddrmbutton.h"
#include "profilemenu.h"
#include "buttonedit.h"
#include "movebutton.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void saveCurrentButtonsAsJson();
    void processDynBtnMainAction();
    void processDynBtnKeyPress(int key);
    void unmarkAllContentButtons();
    void profileButtonClicked();
    void updateButtonsForProfileChange(QString profileName, bool currentActiveProfHasBeenDeleted);
    void createDefaultJsonForNewProfile(QString profileName);
    void processContentButtonKeyPress(int key, qsizetype indexOfSender);
    void startButtonEdit(qsizetype indexOfSender);
    void openMoveButtonMenu(qsizetype indexOfSender);
    void moveButtonInList(qsizetype oldIndex, qsizetype newIndex);
    void adjustButtons(dynAddRmButton::btnMode mode);
    void processActionForAddButton();
    void processActionForSearchButton();
    void processTextFieldChange(QString);
    void processSingleButtonDeletion(qsizetype indexOfSender);
    void restoreLastUnfocused();
    void processClipBoard();
    void processRemoveAllMarkedButtons();

private:
    void updatePlaceholderProfileNameText();
    void loadJsonOrCreateDefault();
    void fixTabOrder();
    void loadAppSettings();
    void saveAppSettings();
    void loadButtonsFromJson();
    void saveButtonsAsJson(QString pathToFile, QVector<contentButton*> listOfBtns);
    void saveDefaultJsonForProfile(QString profileName);
    void addDynBtnAtEndOfContentButtons();
    void createAndAddNewButton(qsizetype row, qsizetype col, QString title, QString content);
    void processAddANewButton(QString defaultText);
    void processMinusKey();
    void processEscapeKey();
    void processRemainingKeys(int key);
    void updateIndexOfAllButtons();
    void removeAllButtonsThatAreMarkedForDel();
    void deleteAllItemsFromGrid();
    void buildGridFromContentList();
    void clearContentButtonList();
    void changeProfileName(QString newName);
    void setDisplayedProfileName(QString name);
    void rebuildGrid();
    void removeSelectedButton(qsizetype index);
    void focusContentButton(contentButton *btn);
    void focusDynBtn();
    void focusHackSolution();
    void widgetFocusWrap(QWidget *wdgt);
    void doDefaultFocus();
    qsizetype getHiddenButtonCount();
    qsizetype getAdjustedIndexOfSenderForSearch(qsizetype indexOfSender);
    qsizetype getAdjustedNewIndexForSearch(qsizetype newIndex);
    qsizetype getSearchAdjustedIndexIfSearchIsActive(qsizetype index);
    void processArrowKeyPress(int key, qsizetype indexOfSender);
    void setUpUnmarkAllBtn();
    void adjustMenuOfContentButtons(dynAddRmButton::btnMode mode);
    void enableMoveButtonForAllButtons();
    void disableMoveButtonForAllButtons();
    void setSearchActive();
    void setSearchInactive();
    void checkSearchMatchForAllButtons(QString searchString);
    void resetSearchStatusOfAllButtons();
    void popUpForProfileSettingsInvalid();
    void updateProfileSettingsValidity();
    bool checkProfileSettingsValidPopUpIfNot();
    void enablePasteForAllButtons();
    void disablePasteForAllButtons();

    Ui::MainWindow *ui;
    QVector<contentButton*> contentBtnList;

    dynAddRmButton dynBtn;
    profileMenu profMenu; //changing profile(s)
    buttonEdit btnEdit; //editing button content & title (if there is one)
    moveButton moveBtnMenu; //changing button position

    QString currSelectedProfileName; //ToDo rethink this variable / maybe move to profMenu and use getter/setter in all places
    QString pathToFileForSelectedProfile; //ToDo rethink this variable / maybe move to profMenu and use getter/setter in all places

    QPushButton unmarkAllBtn;

    bool profileSettingsValid = false;
    bool searchActive = false;
    static const QClipboard *clipboard;
    static int constexpr stretchOfUnmarkAllBtn = 28; //this value has to be determined empirically
        //(it's roughly equal to the stretch of the inputField+addBtn+searchBtn+a bit more for each spacing between the 3)

    static qsizetype constexpr maxItemsPerRow = 4;
    static int constexpr minWindowSize_w = 550;
    static int constexpr minWindowSize_h = 200;
    static int constexpr defaultWindowSize_w = 800;
    static int constexpr defaultWindowSize_h = 400;
    static int constexpr scrollGridSpacing_h = 5; //horizontal
    static int constexpr scrollGridSpacing_v = 5; //vertical
};

#endif // MAINWINDOW_H
