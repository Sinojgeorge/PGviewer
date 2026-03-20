#pragma once

#include "core/DatabaseConnection.h"
#include "core/QueryResult.h"

#include <QObject>
#include <QStringList>
#include <QFuture>
#include <memory>
#include <functional>

namespace PGViewer {

struct TableInfo {
    QString schema;
    QString name;
    QString type;   // "BASE TABLE" or "VIEW"
};

/**
 * @brief High-level database service layer.
 *
 * All blocking operations are exposed as synchronous methods designed to be
 * called from a background QThread (via QFuture / QtConcurrent).  The UI
 * layer must never call these directly on the main thread.
 */
class DatabaseService : public QObject {
    Q_OBJECT
public:
    explicit DatabaseService(DatabaseConnection* conn, QObject* parent = nullptr);

    // ── Schema browsing ──────────────────────────────────────────────────────
    QStringList fetchDatabases();
    QStringList fetchSchemas();
    QStringList fetchTables(const QString& schema);
    QStringList fetchViews(const QString& schema);
    QVector<TableInfo> fetchAllTablesAndViews();

    // ── Table data ───────────────────────────────────────────────────────────
    QueryResult fetchTableData(const QString& schema,
                               const QString& table,
                               int limit  = 500,
                               int offset = 0,
                               const QString& orderBy   = {},
                               bool ascending = true,
                               const QString& filterCol = {},
                               const QString& filterVal = {});

    int countRows(const QString& schema, const QString& table);

    QStringList fetchColumnNames(const QString& schema, const QString& table);

    // ── CRUD ─────────────────────────────────────────────────────────────────
    QueryResult executeQuery(const QString& sql);

    bool updateCell(const QString& schema,
                    const QString& table,
                    const QString& pkColumn,
                    const QVariant& pkValue,
                    const QString& column,
                    const QVariant& newValue);

    bool deleteRow(const QString& schema,
                   const QString& table,
                   const QString& pkColumn,
                   const QVariant& pkValue);

    bool insertRow(const QString& schema,
                   const QString& table,
                   const QStringList& columns,
                   const QVector<QVariant>& values);

    // ── Metadata ─────────────────────────────────────────────────────────────
    QString fetchPrimaryKeyColumn(const QString& schema, const QString& table);

private:
    DatabaseConnection* m_conn;  ///< Non-owning pointer — owned by MainWindow
};

} // namespace PGViewer
