#ifndef STRINGCONSTANTS
#define STRINGCONSTANTS

#include <QString>
#include <QMap>
#include <map>
#include <vreen/client.h>

const QString ORG_NAME = "bAnanapOtato";
const QString APP_NAME = "VKMusicSync";
const QString APP_VERSION = "0.2";

const QString AUDIO_LIST_SHOW_PATTERN = "%1 - %2 [%3:%4]";
const QString FILE_PATH_PATTERN = "%1/%2 - %3.mp3";

const QString AUDIO_FIELD_ID = "id";
const QString AUDIO_FIELD_TITLE = "title";
const QString AUDIO_FIELD_ARTIST = "artist";
const QString AUDIO_FIELD_DURATION = "duration";
const QString AUDIO_FIELD_URL = "url";
const QString AUDIO_FIELD_GENRE = "genre";
const QString ALBUMS_FIELD_TITLE = "title";
const QString ALBUMS_FIELD_ID = "album_id";

const QString AUDIO_GET_METHOD = "audio.get";
const QString AUDIO_GET_ALBUMS_METHOD = "audio.getAlbums";
const QString AUDIO_GETPOPULAR_METHOD = "audio.getPopular";
const QString AUDIO_GET_FIELD_COUNT = "count";
const QString AUDIO_GET_FIELD_ALBUM = "album_id";
const QString AUDIO_GET_ALBUMS_FIELD_COUNT = "count";
const QString AUDIO_GETPOPULAR_FIELD_COUNT = "count";
const QString AUDIO_GETPOPULAR_FIELD_GENRE = "genre_id";
const QString AUDIO_GETPOPULAR_FIELD_ENGLISH_ONLY = "only_eng";

const QString ONLINE_STATE_LBL_ONLINE = "<html><head/><body><p><span style='font-size:11pt; font-weight:600; color:#00ee00;'>Online</span></p></body></html>";
const QString ONLINE_STATE_LBL_OFFLINE = "<html><head/><body><p><span style='font-size:11pt; font-weight:600; color:#ff0000;'>Offline</span></p></body></html>";

const QString STATUS_BAR_GONE_ONLINE = "Logged in";
const QString STATUS_BAR_GONE_OFFLINE = "Logged out";
const QString STATUS_BAR_PROCESSING_ANSWER = "Processing";
const QString STATUS_BAR_REFRESHED_AUDIO_LIST = "Refreshed";
const QString STATUS_BAR_REFRESHING_AUDIO_LIST = "Refreshing audio list";
const QString STATUS_BAR_REFRESHING_ALBUMS_LIST = "Refreshing albums list";
const QString STATUS_BAR_SYNCED_MESSAGE = "Ready to go";

const QString FOLDER_SELECTOR_TITLE = "Select folder";
const QString FILE_DELETION_DIALOG_TITLE = "Removing files";
const QString FILE_DELETION_TEXT_PATTERN = "The following files are going to be removed: %1 \nAre you sure want to delete them?";
const QString FILE_UNREMOVABLE_DIALOG_TITLE = "Error removing files";
const QString FILE_UNREMOVABLE_DIALOG_TEXT_PATTERN = "The following files could not be removed: %1";

const QString ABOUT_TITLE = "About VKMusicSync";
const QString ABOUT_TEXT = "This program was created by\nLeonid Skorospelov (<a href='mailto:leosko94@gmail.com?Subject=VKMusicSync'>leosko94@gmail.com</a>).\nFeel free to contact me in order to improve this software.";

const QString TRAY_SHOW_ACTION_TEXT = "Show/raise window";
const QString TRAY_EXIT_ACTION_TEXT = "Exit";
const QString TRAY_MINIMIZED_TITLE = "Minimized to tray";
const QString TRAY_MINIMIZED_TEXT = "I'm working from tray now, right click the icon and select 'Exit' to exit the program";
const QString TRAY_SYNCED_TITLE = "Sync completed";
const QString TRAY_SYNCED_TEXT_PATTERN = "Synced additionally %1 tracks, now there are %2 old tracks to remove.";

const QString LOGOUT_BUTTON = "Logout";
const QString LOGIN_BUTTON = "Login";

const QString ERROR_TITLE = "Error";
const std::map<Vreen::Client::Error, QString> E_T
{
    {Vreen::Client::ErrorUnknown, "Unknown error"},
    {Vreen::Client::ErrorApplicationDisabled, "Application disabled"},
    {Vreen::Client::ErrorIncorrectSignature, "Incorrect signature"},
    {Vreen::Client::ErrorAuthorizationFailed, "Authorization failed"},
    {Vreen::Client::ErrorToManyRequests, "Too many requests"},
    {Vreen::Client::ErrorPermissionDenied, "Permission denied"},
    {Vreen::Client::ErrorCaptchaNeeded, "Captcha needed"},
    {Vreen::Client::ErrorMissingOrInvalidParameter, "Invalid parameters, internal error"},
    {Vreen::Client::ErrorNetworkReply, "Network error"}
};
const QMap<Vreen::Client::Error, QString> ERROR_TEXTS(E_T);
const QString ERROR_FOLDER_TITLE = "Unaccessible folder";
const QString ERROR_FOLDER_TEXT = "Check selected folder for existing and ability to read/write to.";
const QString ERROR_WRITING_FILE_TITLE = "Error writing file to disk";
const QString ERROR_WRITING_FILE_TEXT = "The error occured while writing file to %1. Check if\n1. You have enough space on your disk.\n2. You are able to write to selected directory.\n3. The disk is still available from your computer.";
const QString ERROR_REMOVING_HALFED_FILE_TITLE = "Error removing unsaved file";
const QString ERROR_REMOVING_HALFED_FILE_TEXT = "The error occured while removing the file that was not fully saved to disk.";

const QString ICON_PATH = ":/icons/VKMusicSyncIcon.ico";

const QString DEFAULT_ALBUM_NAME = "My audio";

const std::map<QString, int> P_G
{
    {"Rock", 1},
    {"Pop", 2},
    {"Rap & Hip-Hop", 3},
    {"Easy Listening", 4},
    {"Dance & House", 5},
    {"Instrumental", 6},
    {"Metal", 7},
    {"Alternative", 21},
    {"Dubstep", 8},
    {"Jazz & Blues", 9},
    {"Drum & Bass", 10},
    {"Trance", 11},
    {"Chanson", 12},
    {"Ethnic", 13},
    {"Acoustic & Vocal", 14},
    {"Reggae", 15},
    {"Classical", 16},
    {"Indie Pop", 17},
    {"Speech", 19},
    {"Electropop & Disco", 22},
    {"Other", 18}
};
const QMap<QString, int> POPULAR_GENRES(P_G);

#endif // STRINGCONSTANTS

