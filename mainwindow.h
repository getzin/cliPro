#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidget>
#include <QStandardPaths>

#include "contentbutton.h"
#include "dynaddrmbutton.h"
#include "profilemenu.h"
#include "buttonedit.h"

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
    void loadAppSettings();
    void saveAppSettings();
    void loadButtonsFromJson();
    void saveButtonsAsJson(QString pathToFile, QVector<contentButton*> listOfBtns);
    void saveDefaultJsonForProfile(QString profileName);
    void saveCurrentButtonsAsJson();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void processDynBtnMainAction();
    void processDynBtnKeyPress(int key);
    void unmarkAllContentButtons();
    void profileButtonClicked();
    void updateButtonsForProfileChange(QString profileName);
    void createDefaultJsonForNewProfile(QString profileName);
    void processContentButtonKeyPress(int key, qsizetype indexOfSender);
    void startButtonEdit(qsizetype indexOfSender);
    void adjustButtons(dynAddRmButton::btnMode mode);
    void processActionForAddButton();
    void processActionForSearchButton();
    void processTextFieldChange(QString);
    void processSingleButtonDeletion(qsizetype indexOfSender);

private:
    void fixTabOrder();
    void addDynBtnAtEndOfContentButtons();
    void processAddANewButton(QString defaultText);
    void processRemoveAllMarkedButtons();
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
    void doDefaultFocus();
    void processArrowKeyPress(int key, qsizetype indexOfSender);
    void setUpUnmarkAllBtn();

    Ui::MainWindow *ui;
    QVector<contentButton*> contentBtnList;

    dynAddRmButton *dynBtn = nullptr; //ToDo non-pointer

    profileMenu profMenu;
    buttonEdit btnEdit;

    QString currSelectedProfileName; //ToDo rethink this variable / maybe move to profMenu and use getter/setter in all places
    QString pathToFileForSelectedProfile; //ToDo rethink this variable / maybe move to profMenu and use getter/setter in all places

    QPushButton unmarkAllBtn;

    static int constexpr stretchOfUnmarkAllBtn = 26; //this value has to be determined empirically
        //(it's roughly equal to the stretch of the inputField+addBtn+searchBtn+a bit more for each spacing between the 3)

    static qsizetype constexpr maxItemsPerRow = 4;
    static int constexpr minWindowSize_w = 500;
    static int constexpr minWindowSize_h = 200;
    static int constexpr defaultWindowSize_w = 800;
    static int constexpr defaultWindowSize_h = 400;
};

#endif // MAINWINDOW_H
