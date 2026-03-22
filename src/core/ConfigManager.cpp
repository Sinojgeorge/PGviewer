#include "ConfigManager.h"

#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>

namespace PGViewer {

ConfigManager::ConfigManager()
{
    // Store config in the platform-appropriate app-data directory
    QString dir = QStandardPaths::writableLocation(
        QStandardPaths::AppConfigLocation
    );
    QDir().mkpath(dir);  // create directory if it doesn't exist
    m_filePath = dir + "/pgviewer_config.json";

    loadJson();
}
//boolean loadJson() - loads the config from the JSON file. If the file doesn't exist, it initializes an empty config. If the file is corrupt, it logs a warning and resets to an empty config. Returns true on success, false on failure (e.g., file I/O issues).
bool ConfigManager::loadJson()
{
    QFile f(m_filePath);
    if (!f.exists()) {
        // First run — start with empty object
        m_root = QJsonObject();
        return true;
    }
    if (!f.open(QIODevice::ReadOnly)) {
        qWarning() << "[Config] Cannot open config file:" << m_filePath;
        m_root = QJsonObject();
        return false;
    }

    QByteArray data = f.readAll();
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(data, &err);
    if (err.error != QJsonParseError::NoError || !doc.isObject()) {
        qWarning() << "[Config] Corrupt config file, resetting. Error:"
                   << err.errorString();
        m_root = QJsonObject();
        return false;
    }

    m_root = doc.object();
    return true;
}

bool ConfigManager::saveJson()
{
    QFile f(m_filePath);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        qWarning() << "[Config] Cannot write config file:" << m_filePath;
        return false;
    }
    QJsonDocument doc(m_root);
    f.write(doc.toJson(QJsonDocument::Indented));
    return true;
}

bool ConfigManager::reload()
{
    return loadJson();
}

bool ConfigManager::saveConnection(const ConnectionParams& params)
{
    QJsonObject conn;
    conn["host"]     = params.host;
    conn["port"]     = params.port;
    conn["database"] = params.database;
    conn["username"] = params.username;
    // NOTE: Password is stored in plaintext here for simplicity.
    // In production, use platform keychain (SecretService / Windows Credential Manager).
    conn["password"] = params.password;

    m_root["connection"] = conn;
    return saveJson();
}

std::optional<ConnectionParams> ConfigManager::loadConnection() const
{
    if (!m_root.contains("connection")) return std::nullopt;

    QJsonObject conn = m_root["connection"].toObject();
    if (conn.isEmpty()) return std::nullopt;

    ConnectionParams p;
    p.host     = conn.value("host").toString("localhost");
    p.port     = conn.value("port").toInt(5432);
    p.database = conn.value("database").toString();
    p.username = conn.value("username").toString();
    p.password = conn.value("password").toString();
    return p;
}

bool ConfigManager::setSetting(const QString& key, const QString& value)
{
    QJsonObject settings = m_root.value("settings").toObject();
    settings[key] = value;
    m_root["settings"] = settings;
    return saveJson();
}

QString ConfigManager::getSetting(const QString& key,
                                   const QString& defaultVal) const
{
    QJsonObject settings = m_root.value("settings").toObject();
    return settings.value(key).toString(defaultVal);
}

} // namespace PGViewer
