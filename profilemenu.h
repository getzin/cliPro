#ifndef PROFILEMENU_H
#define PROFILEMENU_H

#include <QDialog>

namespace Ui {
class profileMenu;
}

class profileMenu : public QDialog
{
    Q_OBJECT

public:

    explicit profileMenu(QWidget *parent = nullptr);
    ~profileMenu();
    QString getCurrSelProfileName();
    static QString constructFilePathForProfileJson(QString profileName);

signals:
    //void selectedProfile(const QString &profile);
    void selProfileHasChanged(QString newProfileName);
    void newProfileCreated(QString newProfileName);

protected:
    //QStringList* getProfiles();
    void loadProfiles();
    void saveProfiles();

private:
    //checkForDuplicate();

    Ui::profileMenu *ui;

    int lastSavedSelProfileID = -1; //-1 is none selected / invalid / ...
    QStringList internalProfilesList;

    // enum profAction{
    //     newProf = 0,
    //     delProf = 1,
    //     editProfName = 2,
    // };

    //for a del/new action, only one of the 2 strings will be set, the other remains unset
    //for an edit action, both strings will be set
    struct profAction{
        //profAction act;
        QString delName;
        QString newName;
    };

    QList<profAction> unsavedActions;

    static const QString appName;
    static const QString appAuthor;
    static const QString settingsProfilesGroup;
    static const QString settingsProfilesListVal;
    static const QString settingsProfilesCurrSelID;
    static const QString settingsFile;
    static const int defaultTimer = 3000; //ms

    void initVisibleListFromInternal();
    void reconstructVisibleListFromInternal();
    void saveVisibleListToInternal();
    void processProfilesActions();
    void renameProfilesJson(QString oldName, QString newName);
    void createNewProfilesJson(QString name);
    void deleteProfilesJson(QString name);
    void timedPopUp(int timer_ms, QString message);
    bool getUserInputAndCheck(bool isEditOperation, QString windowName, QString promptText, QString defaultTxtForInput);
    static bool checkStringIsAlphanumeric(QString strToCheck);
    // void setSaveEnabled();
    // void setSaveDisabled();
    void setEditDelEnabled();
    void setEditDelDisabled();

private slots:
    void newButtonPressed();
    void editButtonPressed();
    void deleteButtonPressed();
    void cancelButtonPressed();
    void saveButtonPressed();

    void handleSelectionChange();
};

#endif // PROFILEMENU_H
