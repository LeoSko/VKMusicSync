#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "numericalconstants.h"
#include "stringconstants.h"
#include <vreen/connection.h>
#include <vreen/auth/oauthconnection.h>
#include <vreen/roster.h>
#include <vreen/longpoll.h>
#include <QDebug>
#include <QThread>
#include <QSettings>
#include <QDir>
#include <QFileDialog>
#include <QNetworkReply>
#include <QQueue>
#include <QTextDocument>
#include <QMessageBox>
#include <QTimer>
#include <QSystemTrayIcon>
#include "fileremovedialog.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    m_ui(new Ui::MainWindow),
    m_auth(new Vreen::OAuthConnection(APPLICATION_VK_ID, this)),
    m_client(new Vreen::Client(this)),
    m_settings(new QSettings(ORG_NAME, APP_NAME, this)),
    m_audioList(new QList<Audio>()),
    m_albums(new QMap<QString, int>()),
    m_downloadList(new QQueue<QPair<QUrl, QPair<QString, int>>>()),
    m_networkManager(new QNetworkAccessManager(this)),
    m_oldFiles(new QFileInfoList()),
    m_syncTimer(new QTimer(this)),
    m_trayIcon(new QSystemTrayIcon(this)),
    m_icon(QIcon(ICON_PATH)),
    m_lastSynced(0)
{
    m_ui->setupUi(this);
    this->setWindowIcon(m_icon);
    m_ui->folderLineEdit->setText(QDir::rootPath());
    m_ui->countOfTracksSpinBox->setValue(30);
    m_ui->countOfTracksSpinBox->setMaximum(MAX_AUDIO_GET_COUNT);
    this->setWindowTitle(APP_NAME + " " + APP_VERSION);

    m_ui->audioList->reset();

    loadSettings();

    m_auth->setConnectionOption(Vreen::Connection::ShowAuthDialog, true);
    m_auth->setConnectionOption(Vreen::Connection::KeepAuthData, true);
    m_auth->setScopes(Vreen::OAuthConnection::Audio);
    m_client->setConnection(m_auth);
    m_client->setTrackMessages(false);

    createConnections();
    createTrayIcon();
    refreshUiBlock(false);
}

MainWindow::~MainWindow()
{
    m_trayIcon->hide();
    saveSettings();
    delete m_syncTimer;
    delete m_trayIcon;
    delete m_oldFiles;
    delete m_downloadList;
    delete m_audioList;
    delete m_albums;
    delete m_client;
    delete m_auth;
    delete m_settings;
    delete m_ui;
}

void MainWindow::loadSettings()
{
    m_settings->beginGroup("ui");
    m_ui->rememberMeCheckBox->setChecked(m_settings->value("rememberLoggedInState").toBool());
    this->restoreGeometry(m_settings->value("geometry").toByteArray());
    // TODO (LeoSko) Think about how to implement autoSync from loading settings
    //m_ui->autoSyncCheckBox->setChecked(m_settings->value("autoSyncEnabled").toBool());
    m_settings->endGroup();

    m_settings->beginGroup("internal");
    m_ui->countOfTracksSpinBox->setValue(m_settings->value("countOfTracks").toInt());
    m_ui->englishOnlyCheckBox->setChecked(m_settings->value("englishOnly").toBool());
    m_ui->folderLineEdit->setText(m_settings->value("dir").toString());
    m_settings->endGroup();
}

void MainWindow::saveSettings()
{
    m_settings->beginGroup("ui");
    m_settings->setValue("rememberLoggedInState", m_ui->rememberMeCheckBox->isChecked());
    m_settings->setValue("geometry", this->saveGeometry());
    m_settings->setValue("autoSyncEnabled", m_ui->autoSyncCheckBox->isChecked());
    m_settings->endGroup();

    m_settings->beginGroup("internal");
    m_settings->setValue("countOfTracks", m_ui->countOfTracksSpinBox->value());
    m_settings->setValue("englishOnly", m_ui->englishOnlyCheckBox->isChecked());
    m_settings->setValue("dir", m_ui->folderLineEdit->text());
    m_settings->endGroup();
}

void MainWindow::clientError(Vreen::Client::Error err)
{
    if (err != Vreen::Client::ErrorAuthorizationFailed)
    {
        QMessageBox::critical(this, ERROR_TITLE, ERROR_TEXTS[err], QMessageBox::Ok);
    }
}

void MainWindow::login()
{
    m_client->connectToHost();
    if (m_client->isOnline())
    {
        refreshAudioList();
    }
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
    refreshUiBlock(m_ui->autoSyncCheckBox->isChecked());
    m_ui->loginButton->setText((online)?LOGOUT_BUTTON:LOGIN_BUTTON);
    if (online)
    {
        // Do some actions as we logged in and use onSynced to process answer
        refreshAlbumList();
        refreshAudioList();
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
    refreshItemListHighlight();
    refreshOldAudioList();
}

void MainWindow::onRefreshed(const QVariant &vars)
{
    // Here we need to convert given answer to our list of audio
    // so that we could easily operate with them
    m_audioList->clear();
    m_ui->audioList->clear();
    QVariantList answer = vars.toList();
    m_ui->statusBar->showMessage(STATUS_BAR_PROCESSING_ANSWER, SHORT_STATUS_BAR_MESSAGE);
    // Process answer from server
    for (QVariant v : answer)
    {
        QVariantMap vm = v.toMap();
        QTextDocument tdTitle, tdArtist;
        // we need to replace some strange symbols that are incorrect to be in name of files at most systems
        tdTitle.setHtml(vm[AUDIO_FIELD_TITLE].toString().replace(QRegExp("[?\\/|:*\"<>]"), ""));
        tdArtist.setHtml(vm[AUDIO_FIELD_ARTIST].toString().replace(QRegExp("[?\\/|:*\"<>]"), ""));
        Audio a(vm[AUDIO_FIELD_ID].toInt(), tdArtist.toPlainText(), tdTitle.toPlainText(),
                vm[AUDIO_FIELD_URL].toString(), vm[AUDIO_FIELD_DURATION].toInt(), vm[AUDIO_FIELD_GENRE].toInt());
        m_audioList->append(a);
        m_ui->audioList->addItem(AUDIO_LIST_SHOW_PATTERN.arg(a.artist, a.title,
                                                             QString::number(a.duration/60),
                                                             QString::number(a.duration%60)));
    }
    m_ui->statusBar->showMessage(STATUS_BAR_REFRESHED_AUDIO_LIST, LONG_STATUS_BAR_MESSAGE);
    refreshUiBlock(m_ui->autoSyncCheckBox->isChecked());
    refreshItemListHighlight();
    refreshOldAudioList();
    if (m_ui->autoSyncCheckBox->isChecked() && m_downloadList->isEmpty())
    {
        createDownloadQueue();
        qDebug() << *m_downloadList;
        m_lastSynced = m_downloadList->size();
        if (m_lastSynced > 0)
        {
            syncAudio();
        }
    }
}

void MainWindow::refreshItemListHighlight()
{
    for (int i = 0; i < m_ui->audioList->count(); i++)
    {
        Audio a = m_audioList->at(i);
        QString path = FILE_PATH_PATTERN.arg(m_ui->folderLineEdit->text(), a.artist, a.title);
        if (QFile::exists(path))
        {
            m_ui->audioList->item(i)->setBackgroundColor(Qt::gray);
        }
        else
        {
            m_ui->audioList->item(i)->setBackgroundColor(Qt::white);
        }
    }
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

void MainWindow::changeEvent(QEvent* e)
{
    switch (e->type())
    {
        case QEvent::LanguageChange:
            this->m_ui->retranslateUi(this);
            break;
        case QEvent::WindowStateChange:
            {
                if (this->windowState() & Qt::WindowMinimized)
                {
                    QTimer::singleShot(0, this, SLOT(hide()));
                    m_trayIcon->showMessage(TRAY_MINIMIZED_TITLE, TRAY_MINIMIZED_TEXT);
                }
                break;
            }
        default:
            break;
    }

    QMainWindow::changeEvent(e);
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
        qDebug() << args;
        auto reply = m_client->request(AUDIO_GET_METHOD, args);
        connect(reply, &Vreen::Reply::resultReady, this, &MainWindow::onRefreshed);
    }
    else
    {
        args.insert(AUDIO_GETPOPULAR_FIELD_COUNT, m_ui->countOfTracksSpinBox->value());
        args.insert(AUDIO_GETPOPULAR_FIELD_GENRE, -m_albums->value(m_ui->albumsComboBox->currentText()));
        args.insert(AUDIO_GETPOPULAR_FIELD_ENGLISH_ONLY, (m_ui->englishOnlyCheckBox->isChecked())?"1":"0");
        qDebug() << args;
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

void MainWindow::albumChanged(const QString &arg1)
{
    // enable english-only checkbox only if we selected some kind of "popular" list
    m_ui->englishOnlyCheckBox->setEnabled(m_albums->value(arg1) < 0);
    if (m_ui->autoSyncCheckBox->isChecked())
    {
        return;
    }
    refreshAudioList();
    refreshOldAudioList();
}

void MainWindow::chooseFolder()
{
    QString dir = QFileDialog::getExistingDirectory(this, FOLDER_SELECTOR_TITLE, m_ui->folderLineEdit->text());
    if (dir.isEmpty())
    {
        return;
    }
    m_ui->folderLineEdit->setText(dir);
    refreshItemListHighlight();
    refreshOldAudioList();
}

void MainWindow::createDownloadQueue()
{
    m_downloadList->clear();
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
}

void MainWindow::refreshUiBlock(bool autoChecked)
{
    bool res = !autoChecked && m_client->isOnline() && m_downloadList->isEmpty();
    m_ui->albumsComboBox->setEnabled(res);
    m_ui->syncButton->setEnabled(res);
    m_ui->folderLineEdit->setEnabled(res);
    m_ui->folderToolButton->setEnabled(res);
    m_ui->refreshAlbumsButton->setEnabled(res);
    m_ui->refreshButton->setEnabled(res);
    m_ui->removeToolButton->setEnabled(res);
    m_ui->countOfTracksSpinBox->setEnabled(res);
    m_ui->countOfTrackslbl->setEnabled(res);
    m_ui->folderLineEdit->setEnabled(res);
    m_ui->folderToolButton->setEnabled(res);
    m_ui->folderLbl->setEnabled(res);
    m_ui->progressBarFile->setEnabled(res);
    m_ui->progressBarAll->setEnabled(res);

    m_ui->englishOnlyCheckBox->setEnabled(res && (m_albums->value(m_ui->albumsComboBox->currentText()) < 0));
}

void MainWindow::syncAudio()
{
    QDir dir(m_ui->folderLineEdit->text());
    if (dir.exists() && dir.isReadable())
    {
        createDownloadQueue();
        m_ui->progressBarAll->setValue(0);
        m_ui->progressBarFile->setValue(0);
        m_ui->progressBarAll->setMaximum(m_downloadList->size());
        refreshUiBlock(m_ui->autoSyncCheckBox->isChecked());
        downloadNext();
    }
    else
    {
        QMessageBox::critical(this, ERROR_FOLDER_TITLE, ERROR_FOLDER_TEXT, QMessageBox::Ok);
    }
}

void MainWindow::downloadNext()
{
    if (m_downloadList->empty())
    {
        m_ui->progressBarAll->setValue(m_ui->progressBarAll->maximum());
        m_ui->progressBarFile->setValue(m_ui->progressBarFile->maximum());
        refreshUiBlock(m_ui->autoSyncCheckBox->isChecked());
        refreshOldAudioList();
        if (m_ui->autoSyncCheckBox->isChecked())
        {
            m_trayIcon->showMessage(TRAY_SYNCED_TITLE, TRAY_SYNCED_TEXT_PATTERN.arg(m_lastSynced).arg(QString::number(m_oldFiles->size())));
        }
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
    if (file->open(QFile::Append))
    {
        if (file->write(r->readAll()) == -1) // error of writing
        {
            QMessageBox::critical(this, ERROR_WRITING_FILE_TITLE, ERROR_WRITING_FILE_TEXT.arg(last.second.first), QMessageBox::Ok);
            if (!file->remove())
            {
                QMessageBox::critical(this, ERROR_REMOVING_HALFED_FILE_TITLE, ERROR_REMOVING_HALFED_FILE_TEXT, QMessageBox::Ok);
                m_ui->audioList->item(last.second.second)->setBackgroundColor(Qt::red);
                m_downloadList->clear();
                return;
            }
        }
        file->flush();
        file->close();
    }
    else
    {
        qDebug() << file->errorString();
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

bool MainWindow::canDownloadAudio(QString filename)
{
    for (int i = 0; i < m_audioList->size(); i++)
    {
        Audio a = m_audioList->at(i);
        QString fn = a.artist + " - " + a.title + ".mp3";
        if (filename == fn)
        {
            return true;
        }
    }
    return false;
}

void MainWindow::refreshOldAudioList()
{
    m_oldFiles->clear();
    // get list of files in current directory and check if
    // we have same name
    QDir currentDir = QDir(m_ui->folderLineEdit->text());
    if (!currentDir.exists())
    {
        m_ui->removeToolButton->setEnabled(false);
        return;
    }
    QStringList filter;
    filter << "*.mp3";
    QFileInfoList fileList = currentDir.entryInfoList(filter, QDir::Files | QDir::NoDotAndDotDot);
    for (QFileInfo fi : fileList)
    {
        qDebug() << fi.absoluteFilePath();
        if (fi.isFile() && (fi.absoluteDir().absolutePath() == currentDir.absolutePath())
                && !canDownloadAudio(fi.fileName()))
        {
            m_oldFiles->append(fi);
        }
    }
    m_ui->removeToolButton->setText("X (" + QString::number(m_oldFiles->size()) + ")");
    m_ui->removeToolButton->setEnabled(m_oldFiles->size() != 0);
}

void MainWindow::removeOldAudio()
{
    QStringList res;
    for (QFileInfo fi : *m_oldFiles)
    {
        res << fi.absoluteFilePath();
    }
    FileRemoveDialog *frd = new FileRemoveDialog(this);
    frd->setFileList(res);
    frd->exec();
    int r = frd->result();
    //QMessageBox::warning(this, FILE_DELETION_DIALOG_TITLE, FILE_DELETION_TEXT_PATTERN.arg(res), QMessageBox::Yes, QMessageBox::Cancel);
    if (r == QDialog::Rejected)
    {
        return;
    }
    QString unremoved;
    for (QFileInfo fi : *m_oldFiles)
    {
        QFile f(fi.absoluteFilePath());
        if (!f.remove())
        {
            unremoved += fi.absoluteFilePath() + '\n';
        }
    }
    if (!unremoved.isEmpty())
    {
        QMessageBox::information(this, FILE_UNREMOVABLE_DIALOG_TITLE, FILE_UNREMOVABLE_DIALOG_TEXT_PATTERN.arg(unremoved), QMessageBox::Ok);
    }
    refreshOldAudioList();
}

void MainWindow::createConnections()
{
    connect(m_client, &Vreen::Client::onlineStateChanged, this, &MainWindow::onOnlineChanged);
    connect(m_client->roster(), &Vreen::Roster::syncFinished, this, &MainWindow::onSynced);
    connect(m_ui->refreshButton, &QPushButton::clicked, this, &MainWindow::refreshAudioList);
    connect(m_ui->refreshAlbumsButton, &QPushButton::clicked, this, &MainWindow::refreshAlbumList);
    connect(m_ui->albumsComboBox, &QComboBox::currentTextChanged, this, &MainWindow::albumChanged);
    connect(m_ui->syncButton, &QPushButton::clicked, this, &MainWindow::syncAudio);
    connect(m_ui->removeToolButton, &QToolButton::clicked, this, &MainWindow::removeOldAudio);
    connect(m_ui->autoSyncCheckBox, &QCheckBox::toggled, this, &MainWindow::setAutoSyncMode);

    connect(m_ui->folderToolButton, &QToolButton::clicked, this, &MainWindow::chooseFolder);
    connect(m_ui->loginButton, &QPushButton::clicked, this, &MainWindow::login);
    connect(m_client, &Vreen::Client::error, this, &MainWindow::clientError);

    connect(m_networkManager, &QNetworkAccessManager::finished, this, &MainWindow::fileDownloaded);

    connect(m_syncTimer, &QTimer::timeout, this, &MainWindow::timedSync);
}

void MainWindow::timedSync()
{
    if (!m_client->isOnline())
    {
        return;
    }
    if (m_downloadList->isEmpty())
    {
        refreshAudioList();
        // and so we process inside
    }
}

void MainWindow::createTrayIcon()
{
    m_trayIcon->setIcon(m_icon);
    m_trayIcon->setToolTip(APP_NAME);
    connect(m_trayIcon, &QSystemTrayIcon::activated, this, &MainWindow::showFromTray);

    QAction *showAction = new QAction(TRAY_SHOW_ACTION_TEXT, m_trayIcon);
    connect(showAction, &QAction::triggered, this, &MainWindow::raiseFromTray);

    QAction *exitAction = new QAction(TRAY_EXIT_ACTION_TEXT, m_trayIcon);
    connect(exitAction, &QAction::triggered, this, &MainWindow::close);

    QMenu *trayMenu = new QMenu(this);
    trayMenu->addAction(showAction);
    trayMenu->addAction(exitAction);

    m_trayIcon->setContextMenu(trayMenu);
    m_trayIcon->show();
}

void MainWindow::raiseFromTray()
{
    show();
    raise();
    activateWindow();
}

void MainWindow::showFromTray(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::DoubleClick)
    {
        raiseFromTray();
    }
}

void MainWindow::setAutoSyncMode(bool mode)
{
    if (!QDir(m_ui->folderLineEdit->text()).isReadable())
    {
        QMessageBox::critical(this, ERROR_FOLDER_TITLE, ERROR_FOLDER_TEXT, QMessageBox::Ok);
        return;
    }
    refreshUiBlock(mode);
    if (mode)
    {
        m_syncTimer->start(60000);
    }
    else
    {
        m_syncTimer->stop();
    }
}

void MainWindow::on_actionExit_triggered()
{
    close();
}

void MainWindow::on_actionSync_triggered()
{
    syncAudio();
}

void MainWindow::on_actionAbout_triggered()
{
    QMessageBox box;
    box.setWindowTitle(ABOUT_TITLE);
    box.setWindowIcon(this->windowIcon());
    box.setTextFormat(Qt::RichText);
    box.setText(ABOUT_TEXT);
    box.exec();
}
