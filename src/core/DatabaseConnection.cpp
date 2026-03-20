#include "DatabaseConnection.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

namespace PGViewer {

DatabaseConnection::DatabaseConnection(const QString& connectionName)
    : m_connectionName(connectionName)
{
    // Remove any stale connection with this name
    if (QSqlDatabase::contains(m_connectionName)) {
        QSqlDatabase::removeDatabase(m_connectionName);
    }
    m_db = QSqlDatabase::addDatabase("QPSQL", m_connectionName);
}

DatabaseConnection::~DatabaseConnection()
{
    disconnect();
    // Qt requires the QSqlDatabase object to go out of scope before removal
    m_db = QSqlDatabase();
    QSqlDatabase::removeDatabase(m_connectionName);
}

QString DatabaseConnection::connect(const ConnectionParams& params)
{
    m_params = params;

    if (m_connected) {
        disconnect();
    }

    m_db.setHostName(params.host);
    m_db.setPort(params.port);
    m_db.setDatabaseName(params.database);
    m_db.setUserName(params.username);
    m_db.setPassword(params.password);

    // Pass connect_timeout via connection options
    m_db.setConnectOptions(
        QString("connect_timeout=%1").arg(params.connectTimeout)
    );

    if (!m_db.open()) {
        QString err = m_db.lastError().text();
        qWarning() << "[DB] Connection failed:" << err;
        m_connected = false;
        return err;
    }

    m_connected = true;
    qDebug() << "[DB] Connected to" << params.host << "/" << params.database;
    return {};  // empty = success
}

void DatabaseConnection::disconnect()
{
    if (m_connected && m_db.isOpen()) {
        m_db.close();
        m_connected = false;
        qDebug() << "[DB] Disconnected from" << m_params.database;
    }
}

bool DatabaseConnection::isConnected() const
{
    return m_connected && m_db.isOpen();
}

QSqlDatabase& DatabaseConnection::db()
{
    return m_db;
}

bool DatabaseConnection::ping()
{
    if (!isConnected()) return false;
    QSqlQuery q(m_db);
    return q.exec("SELECT 1");
}

} // namespace PGViewer
