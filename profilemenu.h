#ifndef PROFILEMENU_H
#define PROFILEMENU_H

#include "profilenamedialog.h"
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
    qsizetype getProfilesCount();
    QString getCurrSelProfileName() const;
    bool checkIfProfileIsInList(QString nameToCheck);
    static void createProfilesFolderIfNotExist();
    static QString constructFilePathForProfileJson(QString profileName);
    static QString const profilesFolderName;

signals:
    void selProfileHasChanged(QString newProfileName, bool);
    void newProfileCreated(QString newProfileName);
    void profMenuRejected();

protected:
    void loadProfiles();
    void saveProfiles();

private slots:
    void handleNewProfileCreation(QString newName);
    void handleProfileNameEdited(QString oldName, QString newName);
    void deleteButtonPressed();
    void cancelButtonPressed();
    void saveButtonPressed();
    void handleSelectedProfileChanged();
    void handleRejectedSignal();

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
    profileNameDialog dialog;
    qsizetype currentActiveProfile = -1; //-1 means "none selected/invalid" //ToDo ... "selected/invalid/deleted"
    qsizetype savedIDOffset = 0; //comes into play when deleting items with lower index than the currentActiveProfile,
                                 //value "grows" but will only ever subtracted from currentActiveProfile
    bool currentActiveProfHasBeenDeleted = false;
    bool editDelAreEnabled = true;
    QStringList internalProfilesList;

    void constructVisibleListFromInternal();
    void saveVisibleListToInternal();
    void processProfilesActions();
    void commonCloseActions();
    void renameProfilesJson(QString oldName, QString newName);
    void createNewProfilesJson(QString name);
    void deleteProfilesJson(QString name);

    void setEditDelEnabled();
    void setEditDelDisabled();
    void fixTabOrder();
    void checkProfilesCountAndSetEditDel();
};

#endif // PROFILEMENU_H
