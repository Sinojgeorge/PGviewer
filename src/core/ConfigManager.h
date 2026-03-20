#pragma once

#include "DatabaseConnection.h"
#include <QString>
#include <QJsonObject>
#include <optional>

namespace PGViewer {

/**
 * @brief Persists connection credentials and application preferences to a
 *        JSON file stored in the user's application-data directory.
 *
 * The file is created automatically when first saved.
 * If the file is missing or corrupted, safe defaults are returned.
 */
class ConfigManager {
public:
    ConfigManager();

    /** Save connection parameters and optional extra settings. */
    bool saveConnection(const ConnectionParams& params);

    /** Load previously saved connection parameters. Returns nullopt on failure. */
    std::optional<ConnectionParams> loadConnection() const;

    /** Save a generic key-value string setting. */
    bool setSetting(const QString& key, const QString& value);

    /** Read a generic string setting. Returns defaultVal if not found. */
    QString getSetting(const QString& key,
                       const QString& defaultVal = {}) const;

    /** @return Absolute path of the config file. */
    QString configFilePath() const { return m_filePath; }

    /** Force-reload config from disk. */
    bool reload();

private:
    bool        loadJson();
    bool        saveJson();

    QString     m_filePath;
    QJsonObject m_root;   ///< In-memory representation of the JSON file
};

} // namespace PGViewer
