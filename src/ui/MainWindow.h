#pragma once

#include "core/DatabaseConnection.h"
#include <QMainWindow>
#include <memory>

namespace PGViewer {

class DatabaseService;
class DatabaseExplorer;
class TableViewer;
class QueryEditor;
class ThemeManager;
class StatusBar;
class ConfigManager;

/**
 * @brief The application's main window.
 *
 * Owns the DatabaseConnection and DatabaseService, and wires together
 * the DatabaseExplorer, TableViewer, and QueryEditor panels.
 */
class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(const ConnectionParams& params,
                        ConfigManager& config,
                        QWidget* parent = nullptr);
    ~MainWindow() override;

protected:
    void closeEvent(QCloseEvent* event) override;

private slots:
    void onTableSelected(const QString& schema, const QString& table);
    void onStatusMessage(const QString& msg);
    void onDisconnect();
    void onReconnect();
    void onToggleTheme();
    void onAbout();
    void onCheckConnection();

private:
    void buildUi();
    void buildMenuBar();
    void buildToolBar();
    void startConnectionTimer();

    std::unique_ptr<DatabaseConnection> m_conn;
    std::unique_ptr<DatabaseService>    m_svc;

    ConfigManager&     m_config;
    ThemeManager*      m_theme       = nullptr;
    DatabaseExplorer*  m_explorer    = nullptr;
    TableViewer*       m_tableView   = nullptr;
    QueryEditor*       m_queryEditor = nullptr;
    StatusBar*         m_statusBar   = nullptr;

    ConnectionParams   m_params;     ///< Saved for reconnect
    class QTimer*      m_pingTimer   = nullptr;
};

} // namespace PGViewer
