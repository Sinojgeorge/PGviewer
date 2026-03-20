#pragma once

#include <QString>
#include <QStringList>
#include <QVector>
#include <QVariant>
#include <chrono>

namespace PGViewer {

/**
 * @brief Stores the complete result of a SQL query execution, including
 *        column metadata, all data rows, execution timing, and error info.
 */
struct QueryResult {
    bool        success      = false;
    QString     errorMessage;
    QStringList columns;
    QVector<QVector<QVariant>> rows;
    qint64      elapsedMs    = 0;   ///< Wall-clock execution time in milliseconds
    int         rowsAffected = 0;   ///< For INSERT/UPDATE/DELETE

    /** @return true if the result contains tabular data. */
    bool hasData() const { return !columns.isEmpty(); }

    /** @return A human-readable summary of the result. */
    QString summary() const {
        if (!success) return QStringLiteral("Error: ") + errorMessage;
        if (hasData()) {
            return QStringLiteral("%1 row(s) returned in %2 ms")
                .arg(rows.size()).arg(elapsedMs);
        }
        return QStringLiteral("%1 row(s) affected in %2 ms")
            .arg(rowsAffected).arg(elapsedMs);
    }
};

} // namespace PGViewer
