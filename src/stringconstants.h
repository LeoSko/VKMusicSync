#ifndef STRINGCONSTANTS
#define STRINGCONSTANTS

#include <QString>
#include <QMap>
#include <map>

const QString ORG_NAME = "bAnanapOtato";
const QString APP_NAME = "VKMusicSync";
const QString APP_VERSION = "0.1b";

const QString AUDIO_LIST_SHOW_PATTERN = "%1 - %2 [%3:%4]";

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

const QString LOGOUT_BUTTON = "Logout";
const QString LOGIN_BUTTON = "Login";

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

