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
#include <QDir>
#include <QFileDialog>
#include <QNetworkReply>
#include <QQueue>
#include <QTextDocument>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    m_ui(new Ui::MainWindow),
    m_client(new Vreen::Client(this)),
    m_auth(new Vreen::OAuthConnection(APPLICATION_VK_ID, this)),
    m_settings(new QSettings(ORG_NAME)),
    m_audioList(new QList<Audio>()),
    m_albums(new QMap<QString, int>()),
    m_dir(QDir::root().absolutePath()),
    m_downloadList(new QQueue<QPair<QUrl, QPair<QString, int>>>()),
    m_networkManager(new QNetworkAccessManager(this))
{
    m_ui->setupUi(this);
    m_ui->countOfTracksSpinBox->setMaximum(MAX_AUDIO_GET_COUNT);
    this->setWindowTitle(APP_NAME + " " + APP_VERSION);
    m_auth->setConnectionOption(Vreen::Connection::ShowAuthDialog, true);
    m_auth->setConnectionOption(Vreen::Connection::KeepAuthData, true);
    m_auth->setScopes(Vreen::OAuthConnection::Audio);
    m_client->setConnection(m_auth);

    //m_ui->rememberMeCheckBox->setEnabled(!m_auth->accessToken().isEmpty());

    connect(m_client, &Vreen::Client::onlineStateChanged, this, &MainWindow::onOnlineChanged);
    connect(m_client->roster(), &Vreen::Roster::syncFinished, this, &MainWindow::onSynced);
    connect(m_ui->refreshButton, &QPushButton::clicked, this, &MainWindow::refreshAudioList);
    connect(m_ui->syncButton, &QPushButton::clicked, this, &MainWindow::syncAudio);
    connect(m_ui->refreshAlbumsButton, &QPushButton::clicked, this, &MainWindow::refreshAlbumList);

    connect(m_ui->loginButton, &QPushButton::clicked, this, &MainWindow::login);

    connect(m_networkManager, &QNetworkAccessManager::finished, this, &MainWindow::fileDownloaded);

    m_ui->audioList->reset();
}

MainWindow::~MainWindow()
{
    delete m_downloadList;
    delete m_audioList;
    delete m_albums;
    delete m_client;
    delete m_auth;
    delete m_settings;
    delete m_ui;
}

void MainWindow::login()
{
    m_client->connectToHost();
}

void MainWindow::logout()
{
    m_client->disconnectFromHost();
    if (!m_ui->rememberMeCheckBox->isChecked())
    {
        m_auth->clear();
    }
}

void MainWindow::onOnlineChanged(bool online)
{
    m_ui->onlineStateLbl->setText((online)?ONLINE_STATE_LBL_ONLINE:ONLINE_STATE_LBL_OFFLINE);
    m_ui->refreshButton->setEnabled(online);
    m_ui->countOfTrackslbl->setEnabled(online);
    m_ui->refreshAlbumsButton->setEnabled(online);
    m_ui->albumsComboBox->setEnabled(online);
    m_ui->audioList->setEnabled(online);
    m_ui->countOfTracksSpinBox->setEnabled(online);
    m_ui->loginButton->setText((online)?LOGOUT_BUTTON:LOGIN_BUTTON);
    if (online)
    {
        // Do some actions as we logged in and use onSynced to process answer
        m_ui->refreshButton->click();
        m_ui->refreshAlbumsButton->click();
        disconnect(m_ui->loginButton, &QPushButton::clicked, this, &MainWindow::login);
        connect(m_ui->loginButton, &QPushButton::clicked, this, &MainWindow::logout);
    }
    else
    {
        disconnect(m_ui->loginButton, &QPushButton::clicked, this, &MainWindow::logout);
        connect(m_ui->loginButton, &QPushButton::clicked, this, &MainWindow::login);
    }
    m_ui->statusBar->showMessage(online?STATUS_BAR_GONE_ONLINE:STATUS_BAR_GONE_OFFLINE,
                                 LONG_STATUS_BAR_MESSAGE);
}

void MainWindow::onSynced(const QVariant &vars)
{
    m_ui->statusBar->showMessage(STATUS_BAR_SYNCED_MESSAGE, LONG_STATUS_BAR_MESSAGE);
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
        QTextDocument tdTitle, tdArtist;
        tdTitle.setHtml(vm[AUDIO_FIELD_TITLE].toString());
        tdArtist.setHtml(vm[AUDIO_FIELD_ARTIST].toString());
        Audio a(vm[AUDIO_FIELD_ID].toInt(), tdArtist.toPlainText(), tdTitle.toPlainText(),
                vm[AUDIO_FIELD_URL].toString(), vm[AUDIO_FIELD_DURATION].toInt(), vm[AUDIO_FIELD_GENRE].toInt());
        m_audioList->append(a);
        m_ui->audioList->addItem(AUDIO_LIST_SHOW_PATTERN.arg(a.artist, a.title,
                                                             QString::number(a.duration/60),
                                                             QString::number(a.duration%60)));
    }
    m_ui->statusBar->showMessage(STATUS_BAR_REFRESHED_AUDIO_LIST, LONG_STATUS_BAR_MESSAGE);
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
    m_ui->albumsComboBox->insertSeparator(m_albums->size());
    // There is no way of m_albums->size() to be 0 cuz we always have "My audio"
    m_ui->albumsComboBox->setCurrentIndex(m_albums->size() - 1);
    for (QString key : POPULAR_GENRES.keys())
    {
        m_ui->albumsComboBox->addItem(key);
        m_albums->insert(key, -POPULAR_GENRES[key]);
    }
}

void MainWindow::refreshAudioList()
{
    m_audioList->clear();
    m_ui->audioList->clear();
    m_ui->statusBar->showMessage(STATUS_BAR_REFRESHING_AUDIO_LIST, SHORT_STATUS_BAR_MESSAGE);
    QVariantMap args;
    if (m_albums->value(m_ui->albumsComboBox->currentText()) >= 0)
    {
        args.insert(AUDIO_GET_FIELD_COUNT, m_ui->countOfTracksSpinBox->value());
        args.insert(AUDIO_GET_FIELD_ALBUM, m_albums->value(m_ui->albumsComboBox->currentText()));
        auto reply = m_client->request(AUDIO_GET_METHOD, args);
        connect(reply, &Vreen::Reply::resultReady, this, &MainWindow::onRefreshed);
    }
    else
    {
        args.insert(AUDIO_GETPOPULAR_FIELD_COUNT, m_ui->countOfTracksSpinBox->value());
        args.insert(AUDIO_GETPOPULAR_FIELD_GENRE, -m_albums->value(m_ui->albumsComboBox->currentText()));
        args.insert(AUDIO_GETPOPULAR_FIELD_ENGLISH_ONLY, (m_ui->englishOnlyCheckBox->isChecked())?"1":"0");
        auto reply = m_client->request(AUDIO_GETPOPULAR_METHOD, args);
        connect(reply, &Vreen::Reply::resultReady, this, &MainWindow::onRefreshed);
    }
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

void MainWindow::on_albumsComboBox_currentTextChanged(const QString &arg1)
{
    // enable english-only checkbox only if we selected some kind of "popular" list
    m_ui->englishOnlyCheckBox->setEnabled(m_albums->value(arg1) < 0);
}

void MainWindow::on_folderToolButton_clicked()
{
    m_dir = QFileDialog::getExistingDirectory(this, FOLDER_SELECTOR_TITLE, m_dir);
    m_ui->folderLineEdit->setText(m_dir);
}

void MainWindow::on_syncButton_clicked()
{
    QString path = FILE_PATH_PATTERN.arg(m_ui->folderLineEdit->text());
    for (int i = 0; i < m_ui->audioList->count(); i++)
    {
        m_ui->audioList->item(i)->setBackgroundColor(Qt::lightGray);
    }
    for (int i = 0; i < m_audioList->size(); i++)
    {
        Audio a = m_audioList->at(i);
        QString currentPath = path.arg(a.artist, a.title);
        if (!QFile(currentPath).exists())
        {
            m_downloadList->enqueue(QPair<QUrl, QPair<QString, int> >(QUrl(a.url), QPair<QString, int>(currentPath, i)));
            m_ui->audioList->item(i)->setBackgroundColor(Qt::white);
        }
    }
    m_ui->progressBarAll->setValue(0);
    m_ui->progressBarFile->setValue(0);
    m_ui->progressBarAll->setMaximum(m_downloadList->size());
    downloadNext();
}

void MainWindow::downloadNext()
{
    if (m_downloadList->empty())
    {
        m_ui->progressBarAll->setValue(m_ui->progressBarAll->maximum());
        m_ui->progressBarFile->setValue(m_ui->progressBarFile->maximum());
        qDebug() << "Finished downloading";
        return;
    }
    QPair<QUrl, QPair<QString, int>> next = m_downloadList->head();
    auto reply = m_networkManager->get(QNetworkRequest(next.first));
    connect(reply, &QNetworkReply::downloadProgress, this, &MainWindow::downloadProgress);
    qDebug() << "Downloading " << next;
}

void MainWindow::fileDownloaded(QNetworkReply *r)
{
    QPair<QUrl, QPair<QString, int>> last = m_downloadList->dequeue();
    QFile *file = new QFile(last.second.first);
    if(file->open(QFile::Append))
    {
        file->write(r->readAll());
        file->flush();
        file->close();
    }
    delete file;
    m_ui->audioList->item(last.second.second)->setBackgroundColor(Qt::gray);
    m_ui->progressBarAll->setValue(m_ui->progressBarAll->value() + 1);
    downloadNext();
}

void MainWindow::downloadProgress(quint64 got, quint64 total)
{
    m_ui->progressBarFile->setMaximum(total);
    m_ui->progressBarFile->setValue(got);
}
