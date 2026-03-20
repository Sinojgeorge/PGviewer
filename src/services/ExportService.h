#pragma once

#include "core/QueryResult.h"
#include <QString>

namespace PGViewer {

/**
 * @brief Provides data export functionality (CSV, etc.).
 */
class ExportService {
public:
    /**
     * @brief Export a QueryResult to a CSV file.
     * @param result  The data to export.
     * @param filePath  Destination file path.
     * @return Empty string on success, error message on failure.
     */
    static QString exportToCsv(const QueryResult& result, const QString& filePath);

private:
    /** Escape a single field for CSV (quote if needed). */
    static QString escapeCsvField(const QString& field);
};

} // namespace PGViewer
