#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <vreen/client.h>
#include <vreen/auth/oauthconnection.h>

class QSettings;

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

public slots:
    void login(bool currentState);
    void onOnlineChanged(bool online);
    void onSynced(bool success);
    void saveToken(const QByteArray &token, time_t expires);
};

#endif // MAINWINDOW_H
