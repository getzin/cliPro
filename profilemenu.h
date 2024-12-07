#ifndef PROFILEMENU_H
#define PROFILEMENU_H

#include <QDialog>

#include "profilenamedialog.h"

namespace Ui {
class profileMenu;
}

class profileMenu : public QDialog
{
    Q_OBJECT

public:
    ~profileMenu();
    explicit profileMenu(QWidget *const parent = nullptr);
    profileMenu(const profileMenu&) = delete;
    profileMenu& operator=(const profileMenu&) = delete;
    qsizetype getProfilesCount() const;
    QString getCurrSelProfileName() const;
    bool checkProfileNameIsInInternalList(QString const &nameToCheck) const;
    static void createProfilesFolderIfNotExist();
    static QString constructFilePathForProfileJson(QString const &profileName);

signals:
    void selProfileHasChanged(QString const &newProfileName, bool const currentProfileWasDeleted);
    void newProfileCreated(QString const &newProfileName);
    void profMenuRejected();

protected:
    void loadProfiles();
    void saveProfiles();

private slots:
    void handleNewProfileCreation(QString const &newName);
    void handleProfileNameEdited(QString const &oldName, QString const &newName);
    void deleteButtonPressed();
    void cancelButtonPressed();
    void handleRejectedSignal();
    void saveButtonPressed();
    void handleSelectedProfileChanged();

private:
    //we derive the action from the states of delName & newName
    //Only newName has been set to non-empty string? ---> New profile
    //Only delName has been set to non-empty string? ---> Delete profile
    //Both strings have been set to non-empty strings? ---> Edit profile name
    struct profAction{
        QString delName;
        QString newName;
    };
    QList<profAction> unsavedActions;
    Ui::profileMenu *ui;
    profileNameDialog nameDialog;
    qsizetype currentActiveProfile = -1; //-1 means "none selected/invalid"
    qsizetype savedIDOffset = 0; //comes into play when deleting items with lower index than the currentActiveProfile,
                                 //value "grows" but will only ever subtracted from currentActiveProfile
    bool currentActiveProfHasBeenDeleted = false;
    bool editDelAreEnabled = true;
    static QString const profilesFolderName;
    QStringList internalProfilesList;

    void initProfMenu();
    void initConnects();
    void cleanUpZombieProfiles();

    void constructVisibleListFromInternal();
    void saveVisibleListToInternal();
    void processProfilesActions();
    void commonCloseActions();
    void renameProfilesJson(QString const &oldName, QString const &newName);
    void createNewProfilesJson(QString const &name);
    void deleteProfilesJson(QString const &name);

    void setEditDelEnabled();
    void setEditDelDisabled();
    void fixTabOrder();
    void checkProfilesCountAndSetEditDel();
};

#endif // PROFILEMENU_H
