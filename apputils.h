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

static int constexpr defaultQuickPopUpTimer = 2000; //ms
static int constexpr defaultPopUpTimer = 3000; //ms
static int constexpr defaultLongPopUpTimer = 8000; //ms
void timedPopUp(QWidget *const parent = nullptr, int const timer_ms = defaultPopUpTimer, QString const &windowTitle = "", QString const &message = "");
bool indexIsInBounds(qsizetype const index, qsizetype const upperBound);

#endif // APPUTILS_H
