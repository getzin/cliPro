#ifndef APPUTILS_H
#define APPUTILS_H

#include <QWidget>
#include <QString>

namespace appSettings{
inline QString const appName = "cliProV1";
inline QString const appAuthor = "Andreas Getzin";;
inline QString const settingsFile = "settings/cliProSettings.ini";

inline QString const settingsGroupMainWindow = "mainWindow";
inline QString const settingsValMWWidth = "width";
inline QString const settingsValMWHeight = "height";
inline QString const settingsValMWPosX = "pos_x";
inline QString const settingsValMWPosY = "pos_y";

inline QString const settingsGroupProfiles = "profiles";
inline QString const settingsValProfilesList = "profiles_list";
inline QString const settingsValCurrProfileID = "current_profile_id";
}

int constexpr defaultShortPopUpTimer = 2000; //ms
int constexpr defaultLongPopUpTimer = 3000; //ms
void timedPopUp(QWidget *parent, int timer_ms, QString message);
bool indexIsInBounds(qsizetype index, qsizetype upperBound);

#endif // APPUTILS_H
