#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <vreen/client.h>
#include <vreen/auth/oauthconnection.h>
#include <QList>
#include <QFileInfoList>
#include <QSystemTrayIcon>

class QSettings;

struct Audio
{
    QString url;
    QString artist;
    QString title;
    int duration;
    int genre;
    int id;
    Audio() : url(""), artist(""), title(""), duration(0), genre(0), id(-1) { }
    Audio(int _id, QString _artist, QString _title, QString _url, int _duration, int _genre)
        : url(_url), artist(_artist), title(_title), duration(_duration), genre(_genre), id(_id) { }
};

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void createConnections();
    void createTrayIcon();
    void raiseFromTray();
    void createDownloadQueue();
    void refreshUiBlock(bool checked);
private:
    Ui::MainWindow *m_ui;
    Vreen::OAuthConnection *m_auth;
    Vreen::Client *m_client;
    QSettings *m_settings;
    QList<Audio> *m_audioList;
    QMap<QString, int> *m_albums;
    QQueue<QPair<QUrl, QPair<QString, int>>> *m_downloadList;
    QNetworkAccessManager *m_networkManager;
    QFileInfoList *m_oldFiles;
    QTimer *m_syncTimer;
    QSystemTrayIcon *m_trayIcon;
    QIcon m_icon;
    int m_lastSynced;

    void loadSettings();
    void saveSettings();
    void refreshItemListHighlight();
    bool canDownloadAudio(QString filename);
    void downloadNext();
    void refreshOldAudioList();
public slots:
    void onOnlineChanged(bool online);
    void onSynced(const QVariant &vars);
    void onRefreshed(const QVariant &vars);
    void onAlbumsListReceived(const QVariant &vars);
    void refreshAudioList();
    void refreshAlbumList();
    void logout();
    void login();
    void albumChanged(const QString &arg1);
    void chooseFolder();

    void downloadProgress(quint64 got, quint64 total);
    void changeEvent(QEvent *e);
    void showFromTray(QSystemTrayIcon::ActivationReason reason);
    void timedSync();
    void syncAudio();
    void removeOldAudio();
    void setAutoSyncMode(bool mode);
private slots:
    void fileDownloaded(QNetworkReply *);
    void on_actionExit_triggered();
    void on_actionSync_triggered();
    void on_actionAbout_triggered();
};

#endif // MAINWINDOW_H
