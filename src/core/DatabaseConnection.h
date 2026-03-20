#pragma once

#include <QString>
#include <QSqlDatabase>
#include <QSqlError>
#include <memory>
#include <functional>

namespace PGViewer {

/**
 * @brief Holds PostgreSQL connection parameters.
 */
struct ConnectionParams {
    QString host;
    int     port     = 5432;
    QString database;
    QString username;
    QString password;
    int     connectTimeout = 10;  // seconds
};

/**
 * @brief Manages a single PostgreSQL database connection via Qt SQL.
 *
 * Each instance owns a uniquely-named QSqlDatabase handle so that
 * multiple connections can coexist in the same process.
 */
class DatabaseConnection {
public:
    explicit DatabaseConnection(const QString& connectionName = "pgviewer_main");
    ~DatabaseConnection();

    // Non-copyable, movable
    DatabaseConnection(const DatabaseConnection&)            = delete;
    DatabaseConnection& operator=(const DatabaseConnection&) = delete;
    DatabaseConnection(DatabaseConnection&&)                 = default;

    /** Open a connection with the given parameters. Returns empty string on success. */
    QString connect(const ConnectionParams& params);

    /** Close the active connection. */
    void disconnect();

    /** @return true if the connection is currently open. */
    bool isConnected() const;

    /** @return the underlying QSqlDatabase (may be invalid if not connected). */
    QSqlDatabase& db();

    /** @return the connection parameters that were last used. */
    const ConnectionParams& params() const { return m_params; }

    /** @return the internal Qt connection name. */
    QString connectionName() const { return m_connectionName; }

    /** Executes a lightweight query to verify the connection is alive. */
    bool ping();

private:
    QString          m_connectionName;
    QSqlDatabase     m_db;
    ConnectionParams m_params;
    bool             m_connected = false;
};

} // namespace PGViewer
