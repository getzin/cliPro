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

    QPushButton *unmarkAllBtn = nullptr; //ToDo non-pointer
    dynAddRmButton *dynBtn = nullptr; //ToDo non-pointer
    profileMenu *profMenu = nullptr; //ToDo non-pointer
    buttonEdit *contEdit = nullptr; //ToDo non-pointer

    QString currSelectedProfileName;
    QString pathToFileForSelectedProfile;

    static qsizetype constexpr maxItemsPerRow = 4;
    static int constexpr minWindowSize_w = 500;
    static int constexpr minWindowSize_h = 200;
    static int constexpr defaultWindowSize_w = 800;
    static int constexpr defaultWindowSize_h = 400;

    static QString const appName;
    static QString const appAuthor;
    static QString const settingsFile;
    static QString const settingsGroupGeneral;
    static QString const settingsValWindowWidth;
    static QString const settingsValWindowHeight;
};

#endif // MAINWINDOW_H
