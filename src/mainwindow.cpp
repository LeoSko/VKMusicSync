#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "numericalconstants.h"
#include "stringconstants.h"
#include <vreen/connection.h>
#include <vreen/auth/oauthconnection.h>
#include <vreen/roster.h>
#include <QDebug>
#include <QThread>
#include <QSettings>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    m_ui(new Ui::MainWindow),
    m_client(new Vreen::Client(this)),
    m_auth(new Vreen::OAuthConnection(APPLICATION_VK_ID, this)),
    m_settings(new QSettings(ORG_NAME)),
    m_audioList(new QList<Audio>()),
    m_albums(new QMap<QString, int>())
{
    m_ui->setupUi(this);
    m_ui->countOfTracksSpinBox->setMaximum(MAX_AUDIO_GET_COUNT);
    this->setWindowTitle(APP_NAME + " " + APP_VERSION);
    m_auth->setConnectionOption(Vreen::Connection::ShowAuthDialog, true);
    m_auth->setConnectionOption(Vreen::Connection::KeepAuthData, true);
    m_auth->setScopes(Vreen::OAuthConnection::Audio);
    m_client->setConnection(m_auth);

    connect(m_client, &Vreen::Client::onlineStateChanged, this, &MainWindow::onOnlineChanged);
    connect(m_client->roster(), &Vreen::Roster::syncFinished, this, &MainWindow::onSynced);
    connect(m_ui->refreshButton, &QPushButton::clicked, this, &MainWindow::refreshAudioList);
    connect(m_ui->syncButton, &QPushButton::clicked, this, &MainWindow::syncAudio);
    connect(m_ui->refreshAlbumsButton, &QPushButton::clicked, this, &MainWindow::refreshAlbumList);
    //connect(m_auth, &Vreen::OAuthConnection::accessTokenChanged, this, &MainWindow::saveToken);

    connect(m_ui->loginButton, &QPushButton::clicked, this, &MainWindow::login);

    /* Trying to save access_token, although it should be saved automatically from
     * m_auth->setConnectionOption(Vreen::Connection::KeepAuthData, true);
     * line
    m_settings->beginGroup("auth");
    if (m_settings->value("has_token").toBool())
    {
        if (!m_settings->value("access_token").toByteArray().isEmpty())
        {
            qDebug() << "we have token saved: " << m_settings->value("access_token").toByteArray();
            m_auth->setAccessToken(m_settings->value("access_token").toByteArray());
            qDebug() << m_auth->connectionState();
        }
    }
    m_settings->endGroup();*/
    m_ui->audioList->reset();
}

MainWindow::~MainWindow()
{
    delete m_audioList;
    delete m_albums;
    delete m_client;
    delete m_auth;
    delete m_settings;
    delete m_ui;
}

void MainWindow::login(bool currentState)
{
    m_client->connectToHost();
}

void MainWindow::onOnlineChanged(bool online)
{
    m_ui->onlineStateLbl->setText((online)?STATUS_BAR_ONLINE_STATE:STATUS_BAR_OFFLINE_STATE);
    if (online)
    {
        // Do some actions as we logged in and use onSynced to process answer
        m_ui->refreshButton->setEnabled(true);
        m_ui->refreshButton->click();
        m_ui->refreshAlbumsButton->click();
    }
}

void MainWindow::onSynced(const QVariant &vars)
{

}

void MainWindow::onRefreshed(const QVariant &vars)
{
    // Here we need to convert given answer to our list of audio
    // so that we could easily operate with them
    QVariantList answer = vars.toList();
    m_ui->statusBar->showMessage(STATUS_BAR_PROCESSING_ANSWER, SHORT_STATUS_BAR_MESSAGE);
    // Process answer from server
    for (QVariant v : answer)
    {
        QVariantMap vm = v.toMap();
        Audio a(vm[AUDIO_FIELD_ID].toInt(), vm[AUDIO_FIELD_ARTIST].toString(), vm[AUDIO_FIELD_TITLE].toString(),
                vm[AUDIO_FIELD_URL].toString(), vm[AUDIO_FIELD_DURATION].toInt(), vm[AUDIO_FIELD_GENRE].toInt());
        m_audioList->append(a);
        m_ui->audioList->addItem(AUDIO_LIST_SHOW_PATTERN.arg(a.artist, a.title,
                                                             QString::number(a.duration/60), QString::number(a.duration%60)));
    }
    m_ui->statusBar->showMessage(STATUS_BAR_REFRESHED_AUDIO_LIST, SHORT_STATUS_BAR_MESSAGE);
    m_ui->syncButton->setEnabled(true);
}

void MainWindow::onAlbumsListReceived(const QVariant &vars)
{
    QVariantList answer = vars.toList();
    //m_albums->insert(DEFAULT_ALBUM_NAME, DEFAULT_ALBUM_ID);
    for (QVariant v : answer)
    {
        QVariantMap vm = v.toMap();
        if (vm[ALBUMS_FIELD_TITLE].toString().isEmpty())
        {
            m_albums->insert(DEFAULT_ALBUM_NAME, DEFAULT_ALBUM_ID);
        }
        else
        {
            m_albums->insert(vm[ALBUMS_FIELD_TITLE].toString(), vm[ALBUMS_FIELD_ID].toInt());
        }
    }

    // fill the albums we got into combobox
    for (QString key : m_albums->keys())
    {
        m_ui->albumsComboBox->addItem(key);
    }
}

void MainWindow::refreshAudioList()
{
    m_audioList->clear();
    m_ui->audioList->clear();
    m_ui->statusBar->showMessage(STATUS_BAR_REFRESHING_AUDIO_LIST, SHORT_STATUS_BAR_MESSAGE);
    QVariantMap args;
    args.insert(AUDIO_GET_FIELD_COUNT, m_ui->countOfTracksSpinBox->value());
    args.insert(AUDIO_GET_FIELD_ALBUM, m_albums->value(m_ui->albumsComboBox->currentText()));
    auto reply = m_client->request(AUDIO_GET_METHOD, args);
    connect(reply, &Vreen::Reply::resultReady, this, &MainWindow::onRefreshed);
}

void MainWindow::refreshAlbumList()
{
    m_ui->albumsComboBox->clear();
    m_albums->clear();
    m_ui->statusBar->showMessage(STATUS_BAR_REFRESHING_ALBUMS_LIST, SHORT_STATUS_BAR_MESSAGE);
    QVariantMap args;
    args.insert(AUDIO_GET_ALBUMS_FIELD_COUNT, MAX_ALBUMS_GET_COUNT);
    auto reply = m_client->request(AUDIO_GET_ALBUMS_METHOD, args);
    connect(reply, &Vreen::Reply::resultReady, this, &MainWindow::onAlbumsListReceived);
}

void MainWindow::syncAudio()
{

}

/*void MainWindow::saveToken(const QByteArray &token, time_t expires)
{
    if (!token.isEmpty())
    {
        qDebug() << token << " " << expires;
        m_settings->beginGroup("auth");
        m_settings->setValue("has_token", true);
        m_settings->setValue("access_token", token);
        m_settings->endGroup();
    }
    else
    {
        qDebug() << "empty token";
        m_settings->beginGroup("auth");
        m_settings->setValue("has_token", false);
        m_settings->setValue("access_token", NULL);
        m_settings->endGroup();
    }
}*/
