#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <vreen/client.h>
#include <vreen/auth/oauthconnection.h>
#include <QList>

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

private:
    Ui::MainWindow *m_ui;
    Vreen::OAuthConnection *m_auth;
    Vreen::Client *m_client;
    QSettings *m_settings;
    QList<Audio> *m_audioList;

public slots:
    void login(bool currentState);
    void onOnlineChanged(bool online);
    void onSynced(const QVariant &vars);
    void onRefreshed(const QVariant &vars);
    void refreshAudioList();
    //void saveToken(const QByteArray &token, time_t expires);
    void syncAudio();
};

#endif // MAINWINDOW_H
