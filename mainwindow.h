#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidget>
#include <QStandardPaths>

#include "contentbutton.h"
#include "dynaddrmbutton.h"
#include "profilemenu.h"
#include "contentedit.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void loadAppSettings();
    void saveAppSettings();
    void loadButtonsFromJson();
    void saveButtonsAsJson(QString pathToFile, QVector<contentButton*> listOfBtns);
    void saveDefaultJsonForProfile(QString profileName);
    void saveCurrentButtonsAsJson();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    // void mouseReleaseEvent(QMouseEvent *event) override;
    // void mouseDoubleClickEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void processDynBtnAction();
    void unmarkAllCntButtons();
    void profileButtonClicked();
    void updateButtonsForProfileChange(QString profileName);
    void createDefaultJsonForNewProfile(QString profileName);
    void processKeyPress(int key, int indexOfSender);
    void openContentEdit(int indexOfSender);
    void adjustButtons(dynAddRmButton::btnMode mode);
    void processActionForAddButton();
    void processTextFieldChange(QString);

private:
    //methods
    void addDynBtnAtEndOfContentButtons();
    // dynAddRmButton* getOrCreateDynBtn();
    void processActionAdd(QString defaultText);
    void processActionRm();
    void updateIndexOfAllButtons();
    void removeAllMarkedButtons();
    void deleteAllItemsFromGrid();
    void buildGridFromContentList();
    void clearContentButtonList();
    void changeProfileName(QString newName);
    void setDisplayedProfileName(QString name);
    void rebuildGrid();
    void removeSelectedButton(int index);
    void processArrowKeyPress(int key, int indexOfSender);
    void processDeleteKeyPress(int indexOfSender);

    // void setUpDynBtn();
    void setUpUnmarkAllBtn();

    //variables & consts
    Ui::MainWindow *ui;
    static const int maxItemsPerRow = 4;
    //profileButton *profileBtn = nullptr;
    QVector<contentButton*> contentBtnList;

    // QString path = QStandardPaths::writableLocation(
    //                    QStandardPaths::HomeLocation) +
    //                "/qtProjects/dynTiling/buttons.json";

    QString appName = "cliProV1";
    // QString appAuthor = "Andreas Getzin";
    // QString recentFilesGroup = "recentFiles";
    // QString recentFilesValPrefix = "file";
    // QStringList recentFiles;


    QPushButton *unmarkAllBtn = nullptr; //ToDo non-pointer
    dynAddRmButton *dynBtn = nullptr; //ToDo non-pointer
    profileMenu *profMenu = nullptr; //ToDo non-pointer
    contentEdit *contEdit = nullptr; //ToDo non-pointer
    QString currSelectedProfileName;
    QString pathToFileForSelectedProfile;
};

#endif // MAINWINDOW_H
