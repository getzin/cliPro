#ifndef PROFILEMENU_H
#define PROFILEMENU_H

#include "profiledialog.h"
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
    void selProfileHasChanged(QString newProfileName);
    void newProfileCreated(QString newProfileName);

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

    qsizetype lastSavedSelProfileID = -1; //-1 means "none selected/invalid"
    QStringList internalProfilesList;

    static const QString appName;
    static const QString appAuthor;
    static const QString settingsFile;
    static const QString settingsGroupProfiles;
    static const QString settingsValProfilesList;
    static const QString settingsValCurrProfileID;

    void constructVisibleListFromInternal();
    void saveVisibleListToInternal();
    void processProfilesActions();
    void renameProfilesJson(QString oldName, QString newName);
    void createNewProfilesJson(QString name);
    void deleteProfilesJson(QString name);

    void setEditDelEnabled();
    void setEditDelDisabled();

    profileDialog dialog;
};

#endif // PROFILEMENU_H
