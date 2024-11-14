#ifndef APPSETTINGS_H
#define APPSETTINGS_H

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

#endif // APPSETTINGS_H
