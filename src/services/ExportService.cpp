#include "ExportService.h"

#include <QFile>
#include <QTextStream>
#include <QDebug>

namespace PGViewer {

QString ExportService::exportToCsv(const QueryResult& result, const QString& filePath)
{
    if (!result.success || result.columns.isEmpty())
        return QStringLiteral("No data to export.");

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text))
        return QStringLiteral("Cannot open file for writing: ") + filePath;

    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);

    // Header row
    QStringList headerFields;
    for (const QString& col : result.columns)
        headerFields << escapeCsvField(col);
    out << headerFields.join(',') << "\n";

    // Data rows
    for (const auto& row : result.rows) {
        QStringList fields;
        for (const QVariant& val : row) {
            fields << escapeCsvField(val.isNull() ? QString() : val.toString());
        }
        out << fields.join(',') << "\n";
    }

    return {};  // success
}

QString ExportService::escapeCsvField(const QString& field)
{
    // RFC 4180: fields containing commas, quotes, or newlines must be quoted.
    if (field.contains(',') || field.contains('"') || field.contains('\n')) {
        QString escaped = field;
        escaped.replace('"', "\"\"");
        return '"' + escaped + '"';
    }
    return field;
}

} // namespace PGViewer
