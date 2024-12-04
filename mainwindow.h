#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidget>
#include <QStandardPaths>

#include "contentbutton.h"
#include "dynbutton.h"
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
    ~MainWindow();
    explicit MainWindow(QWidget *const parent = nullptr);
    MainWindow(const MainWindow&) = delete;
    MainWindow& operator=(const MainWindow&) = delete;

protected:
    void mousePressEvent(QMouseEvent *const event) override;
    void keyPressEvent(QKeyEvent *const event) override;

private slots:
    void saveCurrentButtonsAsJson();
    void processDynBtnMainAction();
    void processDynBtnKeyPress(int const key);
    void unmarkAllContentButtons();
    void profileButtonClicked();
    void updateButtonsForProfileChange(QString const &profileName, bool const currentActiveProfHasBeenDeleted);
    void createDefaultJsonForNewProfile(QString const &profileName);
    void processContentButtonKeyPress(int const key, qsizetype const indexOfSender);
    void startButtonEdit(qsizetype const indexOfSender);
    void openMoveButtonMenu(qsizetype const indexOfSender);
    void moveButtonInList(qsizetype const oldIndex, qsizetype const newIndex);
    void adjustButtons(dynButton::btnMode const mode);
    void processActionForAddButton();
    void processActionForSearchButton();
    void processTextFieldChange(QString const &text);
    void processSingleButtonDeletion(qsizetype const indexOfSender);
    void restoreLastUnfocused();
    void processClipBoard();
    void processRemoveAllMarkedButtons();

private:
    void initMainWindow();
    void initUIButtons();
    void setUpUnmarkAllBtn();
    void initScrollGrid();
    void initConnects();
    void initClipBoard();
    void updatePlaceholderProfileNameText();
    void loadJsonOrCreateDefault();
    void fixTabOrder();
    void loadAppSettings();
    void saveAppSettings();
    void loadButtonsFromJson();
    void saveButtonsAsJson(QString const &pathToFile, QVector<contentButton*> const &listOfBtns);
    void saveDefaultJsonForProfile(QString const &profileName);
    void addDynBtnAtEndOfContentButtons();
    void createAndAddNewButton(qsizetype const row, qsizetype const col, QString const &title, QString const &content);
    void processAddANewButton(QString const &defaultText);
    void processMinusKey();
    void processEscapeKey();
    void processRemainingKeys(int const key);
    void updateIndexOfAllButtons();
    void removeAllButtonsThatAreMarkedForDel();
    void deleteAllItemsFromGrid();
    void buildGridFromContentList();
    void clearContentButtonList();
    void changeProfileName(QString const &newName);
    void setDisplayedProfileName(QString const &name);
    void rebuildGrid();
    void removeSelectedButton(qsizetype const index);
    void focusContentButton(contentButton *const btn);
    void focusDynBtn();
    void focusHackSolution();
    void doDefaultFocus();
    qsizetype getHiddenButtonCount() const;
    qsizetype getAdjustedIndexOfSenderForActiveSearch(qsizetype indexOfSender) const;
    qsizetype getAdjustedNewIndexForActiveSearch(qsizetype newIndex) const;
    qsizetype getSearchAdjustedIndexIfSearchIsActive(qsizetype index) const;
    void processArrowKeyPress(int const key, qsizetype indexOfSender);
    void adjustMenuOfContentButtons(dynButton::btnMode const mode);
    void enableMoveButtonForAllButtons();
    void disableMoveButtonForAllButtons();
    void setSearchActive();
    void setSearchInactive();
    void checkSearchMatchForAllButtons(QString const &searchString);
    void resetSearchStatusOfAllButtons();
    void popUpForProfileSettingsInvalid();
    void updateProfileSettingsValidity();
    bool checkProfileSettingsValidPopUpIfNot();
    void enablePasteForAllButtons();
    void disablePasteForAllButtons();

    Ui::MainWindow *ui;
    QVector<contentButton*> contentBtnList;

    dynButton dynBtn;
    profileMenu profMenu; //changing profile(s)
    buttonEdit btnEdit; //editing button content+title (if there is one)
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
