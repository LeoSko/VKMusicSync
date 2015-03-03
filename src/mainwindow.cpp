#include "mainwindow.h"
#include "ui_mainwindow.h"
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
    m_auth(new Vreen::OAuthConnection(4378706, this)),
    m_settings(new QSettings("bAnanapOtato"))
{
    m_ui->setupUi(this);
    m_auth->setConnectionOption(Vreen::Connection::ShowAuthDialog, true);
    m_auth->setConnectionOption(Vreen::Connection::KeepAuthData, true);
    m_client->setConnection(m_auth);

    connect(m_client, &Vreen::Client::onlineStateChanged, this, &MainWindow::onOnlineChanged);
    connect(m_client->roster(), &Vreen::Roster::syncFinished, this, &MainWindow::onSynced);
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

}

MainWindow::~MainWindow()
{
    delete m_client;
    delete m_ui;
}

void MainWindow::login(bool currentState)
{
    m_client->connectToHost();
}

void MainWindow::onOnlineChanged(bool online)
{
    m_ui->onlineStateLbl->setText((online)?"Online":"Offline");
    if (online)
    {
        // Do some actions as we logged in and use onSynced to process answer

    }
}

void MainWindow::onSynced(bool success)
{
    if (success)
    {
        // Process answer from server

    }
    else
    {
        qDebug() << "Failed to sync";
    }
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
