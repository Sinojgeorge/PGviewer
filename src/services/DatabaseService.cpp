#include "DatabaseService.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QDebug>
#include <QElapsedTimer>

namespace PGViewer {

DatabaseService::DatabaseService(DatabaseConnection* conn, QObject* parent)
    : QObject(parent), m_conn(conn)
{}

// ── Schema browsing ──────────────────────────────────────────────────────────

QStringList DatabaseService::fetchDatabases()
{
    QStringList dbs;
    QSqlQuery q(m_conn->db());
    if (!q.exec("SELECT datname FROM pg_database WHERE datistemplate = false ORDER BY datname")) {
        qWarning() << "[Service] fetchDatabases error:" << q.lastError().text();
        return dbs;
    }
    while (q.next()) dbs << q.value(0).toString();
    return dbs;
}

QStringList DatabaseService::fetchSchemas()
{
    QStringList schemas;
    QSqlQuery q(m_conn->db());
    if (!q.exec("SELECT schema_name FROM information_schema.schemata "
                "WHERE schema_name NOT IN ('pg_catalog','information_schema','pg_toast') "
                "ORDER BY schema_name")) {
        qWarning() << "[Service] fetchSchemas error:" << q.lastError().text();
        return schemas;
    }
    while (q.next()) schemas << q.value(0).toString();
    return schemas;
}

QStringList DatabaseService::fetchTables(const QString& schema)
{
    QStringList tables;
    QSqlQuery q(m_conn->db());
    q.prepare("SELECT table_name FROM information_schema.tables "
              "WHERE table_schema = :schema AND table_type = 'BASE TABLE' "
              "ORDER BY table_name");
    q.bindValue(":schema", schema);
    if (!q.exec()) {
        qWarning() << "[Service] fetchTables error:" << q.lastError().text();
        return tables;
    }
    while (q.next()) tables << q.value(0).toString();
    return tables;
}

QStringList DatabaseService::fetchViews(const QString& schema)
{
    QStringList views;
    QSqlQuery q(m_conn->db());
    q.prepare("SELECT table_name FROM information_schema.views "
              "WHERE table_schema = :schema ORDER BY table_name");
    q.bindValue(":schema", schema);
    if (!q.exec()) {
        qWarning() << "[Service] fetchViews error:" << q.lastError().text();
        return views;
    }
    while (q.next()) views << q.value(0).toString();
    return views;
}

QVector<TableInfo> DatabaseService::fetchAllTablesAndViews()
{
    QVector<TableInfo> result;
    QSqlQuery q(m_conn->db());
    if (!q.exec("SELECT table_schema, table_name, table_type "
                "FROM information_schema.tables "
                "WHERE table_schema NOT IN ('pg_catalog','information_schema','pg_toast') "
                "ORDER BY table_schema, table_type, table_name")) {
        qWarning() << "[Service] fetchAllTablesAndViews error:" << q.lastError().text();
        return result;
    }
    while (q.next()) {
        result.push_back({
            q.value(0).toString(),
            q.value(1).toString(),
            q.value(2).toString()
        });
    }
    return result;
}

// ── Table data ────────────────────────────────────────────────────────────────

QueryResult DatabaseService::fetchTableData(
    const QString& schema,
    const QString& table,
    int limit,
    int offset,
    const QString& orderBy,
    bool ascending,
    const QString& filterCol,
    const QString& filterVal)
{
    QueryResult result;
    QElapsedTimer timer;
    timer.start();

    QString sql = QString("SELECT * FROM \"%1\".\"%2\"").arg(schema, table);

    // Optional filter
    if (!filterCol.isEmpty() && !filterVal.isEmpty()) {
        sql += QString(" WHERE \"%1\"::text ILIKE :filterVal").arg(filterCol);
    }

    // Optional sort
    if (!orderBy.isEmpty()) {
        sql += QString(" ORDER BY \"%1\" %2").arg(orderBy, ascending ? "ASC" : "DESC");
    }

    sql += QString(" LIMIT %1 OFFSET %2").arg(limit).arg(offset);

    QSqlQuery q(m_conn->db());
    if (!filterCol.isEmpty() && !filterVal.isEmpty()) {
        q.prepare(sql);
        q.bindValue(":filterVal", QString("%%1%").arg(filterVal));
        if (!q.exec()) {
            result.errorMessage = q.lastError().text();
            result.elapsedMs    = timer.elapsed();
            return result;
        }
    } else {
        if (!q.exec(sql)) {
            result.errorMessage = q.lastError().text();
            result.elapsedMs    = timer.elapsed();
            return result;
        }
    }

    QSqlRecord rec = q.record();
    for (int i = 0; i < rec.count(); ++i)
        result.columns << rec.fieldName(i);

    while (q.next()) {
        QVector<QVariant> row;
        for (int i = 0; i < rec.count(); ++i)
            row << q.value(i);
        result.rows << row;
    }

    result.success   = true;
    result.elapsedMs = timer.elapsed();
    return result;
}

int DatabaseService::countRows(const QString& schema, const QString& table)
{
    QSqlQuery q(m_conn->db());
    q.prepare(QString("SELECT COUNT(*) FROM \"%1\".\"%2\"").arg(schema, table));
    if (!q.exec() || !q.next()) return -1;
    return q.value(0).toInt();
}

QStringList DatabaseService::fetchColumnNames(const QString& schema, const QString& table)
{
    QStringList cols;
    QSqlQuery q(m_conn->db());
    q.prepare("SELECT column_name FROM information_schema.columns "
              "WHERE table_schema = :schema AND table_name = :table "
              "ORDER BY ordinal_position");
    q.bindValue(":schema", schema);
    q.bindValue(":table",  table);
    if (!q.exec()) return cols;
    while (q.next()) cols << q.value(0).toString();
    return cols;
}

// ── CRUD ──────────────────────────────────────────────────────────────────────

QueryResult DatabaseService::executeQuery(const QString& sql)
{
    QueryResult result;
    QElapsedTimer timer;
    timer.start();

    QSqlQuery q(m_conn->db());
    if (!q.exec(sql)) {
        result.errorMessage = q.lastError().text();
        result.elapsedMs    = timer.elapsed();
        return result;
    }

    QSqlRecord rec = q.record();
    if (rec.count() > 0) {
        for (int i = 0; i < rec.count(); ++i)
            result.columns << rec.fieldName(i);

        while (q.next()) {
            QVector<QVariant> row;
            for (int i = 0; i < rec.count(); ++i)
                row << q.value(i);
            result.rows << row;
        }
    } else {
        result.rowsAffected = q.numRowsAffected();
    }

    result.success   = true;
    result.elapsedMs = timer.elapsed();
    return result;
}

bool DatabaseService::updateCell(
    const QString& schema,
    const QString& table,
    const QString& pkColumn,
    const QVariant& pkValue,
    const QString& column,
    const QVariant& newValue)
{
    QSqlQuery q(m_conn->db());
    QString sql = QString("UPDATE \"%1\".\"%2\" SET \"%3\" = :val WHERE \"%4\" = :pk")
                      .arg(schema, table, column, pkColumn);
    q.prepare(sql);
    q.bindValue(":val", newValue);
    q.bindValue(":pk",  pkValue);
    if (!q.exec()) {
        qWarning() << "[Service] updateCell error:" << q.lastError().text();
        return false;
    }
    return true;
}

bool DatabaseService::deleteRow(
    const QString& schema,
    const QString& table,
    const QString& pkColumn,
    const QVariant& pkValue)
{
    QSqlQuery q(m_conn->db());
    QString sql = QString("DELETE FROM \"%1\".\"%2\" WHERE \"%3\" = :pk")
                      .arg(schema, table, pkColumn);
    q.prepare(sql);
    q.bindValue(":pk", pkValue);
    if (!q.exec()) {
        qWarning() << "[Service] deleteRow error:" << q.lastError().text();
        return false;
    }
    return true;
}

bool DatabaseService::insertRow(
    const QString& schema,
    const QString& table,
    const QStringList& columns,
    const QVector<QVariant>& values)
{
    if (columns.isEmpty() || columns.size() != values.size()) return false;

    QStringList quotedCols, placeholders;
    for (int i = 0; i < columns.size(); ++i) {
        quotedCols   << QString("\"%1\"").arg(columns[i]);
        placeholders << QString(":v%1").arg(i);
    }

    QString sql = QString("INSERT INTO \"%1\".\"%2\" (%3) VALUES (%4)")
                      .arg(schema, table,
                           quotedCols.join(", "),
                           placeholders.join(", "));

    QSqlQuery q(m_conn->db());
    q.prepare(sql);
    for (int i = 0; i < values.size(); ++i)
        q.bindValue(QString(":v%1").arg(i), values[i]);

    if (!q.exec()) {
        qWarning() << "[Service] insertRow error:" << q.lastError().text();
        return false;
    }
    return true;
}

QString DatabaseService::fetchPrimaryKeyColumn(const QString& schema, const QString& table)
{
    QSqlQuery q(m_conn->db());
    q.prepare(R"(
        SELECT kcu.column_name
        FROM information_schema.table_constraints tc
        JOIN information_schema.key_column_usage kcu
            ON tc.constraint_name = kcu.constraint_name
           AND tc.table_schema    = kcu.table_schema
        WHERE tc.constraint_type = 'PRIMARY KEY'
          AND tc.table_schema    = :schema
          AND tc.table_name      = :table
        LIMIT 1
    )");
    q.bindValue(":schema", schema);
    q.bindValue(":table",  table);
    if (!q.exec() || !q.next()) return {};
    return q.value(0).toString();
}

} // namespace PGViewer
