#include "MainWindow.h"
#include "DatabaseExplorer.h"
#include "TableViewer.h"
#include "QueryEditor.h"
#include "ThemeManager.h"
#include "StatusBar.h"
#include "core/ConfigManager.h"
#include "services/DatabaseService.h"

#include <QSplitter>
#include <QTabWidget>
#include <QTabBar>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QToolBar>
#include <QMessageBox>
#include <QApplication>
#include <QCloseEvent>
#include <QTimer>
#include <QLabel>

namespace PGViewer {

MainWindow::MainWindow(const ConnectionParams& params,
                       ConfigManager& config,
                       QWidget* parent)
    : QMainWindow(parent), m_config(config), m_params(params)
{
    setWindowTitle(
        QString("PGViewer  —  %1 @ %2 / %3")
        .arg(params.username, params.host, params.database)
    );
    setMinimumSize(900, 600);
    resize(1300, 820);

    // ── Connect to database ────────────────────────────────────────────────
    m_conn = std::make_unique<DatabaseConnection>("pgviewer_main");
    QString err = m_conn->connect(params);
    if (!err.isEmpty()) {
        QMessageBox::critical(nullptr, "Connection Error",
            "Failed to open main connection:\n" + err);
    }

    m_svc = std::make_unique<DatabaseService>(m_conn.get());

    // ── Theme ──────────────────────────────────────────────────────────────
    m_theme = new ThemeManager(this);
    bool dark = (m_config.getSetting("theme", "light") == "dark");
    ThemeManager::apply(*qApp, dark);

    // ── Build UI ───────────────────────────────────────────────────────────
    buildUi();
    buildMenuBar();
    buildToolBar();

    // ── Status bar initial state ───────────────────────────────────────────
    m_statusBar->setConnectionStatus(
        m_conn->isConnected(),
        QString("%1@%2/%3").arg(params.username, params.host, params.database)
    );

    // ── Ping timer — check connection every 30 s ───────────────────────────
    startConnectionTimer();
}

MainWindow::~MainWindow() = default;

// ── UI Construction ───────────────────────────────────────────────────────────

void MainWindow::buildUi()
{
    // Custom status bar
    m_statusBar = new StatusBar(this);
    setStatusBar(m_statusBar);

    // Central splitter: explorer | main content
    auto* splitter = new QSplitter(Qt::Horizontal, this);
    splitter->setHandleWidth(1);
    splitter->setChildrenCollapsible(false);

    // ── Left: Database explorer ────────────────────────────────────────────
    m_explorer = new DatabaseExplorer(m_svc.get(), splitter);
    m_explorer->setMinimumWidth(180);
    m_explorer->setMaximumWidth(400);
    connect(m_explorer, &DatabaseExplorer::tableSelected,
            this,        &MainWindow::onTableSelected);

    // ── Right: tabbed main panel ───────────────────────────────────────────
    auto* tabs = new QTabWidget(splitter);
    tabs->setDocumentMode(true);
    tabs->setTabPosition(QTabWidget::North);

    m_tableView = new TableViewer(m_svc.get(), tabs);
    connect(m_tableView, &TableViewer::statusMessage,
            this,         &MainWindow::onStatusMessage);

    m_queryEditor = new QueryEditor(m_svc.get(), tabs);
    connect(m_queryEditor, &QueryEditor::statusMessage,
            this,           &MainWindow::onStatusMessage);

    tabs->addTab(m_tableView,    "  📋  Table Viewer  ");
    tabs->addTab(m_queryEditor,  "  🔍  Query Editor  ");

    splitter->addWidget(m_explorer);
    splitter->addWidget(tabs);
    splitter->setStretchFactor(0, 0);
    splitter->setStretchFactor(1, 1);
    splitter->setSizes({240, 1060});

    setCentralWidget(splitter);
}

void MainWindow::buildMenuBar()
{
    // ── File ──────────────────────────────────────────────────────────────
    auto* fileMenu = menuBar()->addMenu("&File");

    auto* reconnectAct = fileMenu->addAction("&Reconnect");
    reconnectAct->setShortcut(QKeySequence("Ctrl+R"));

    auto* disconnectAct = fileMenu->addAction("&Disconnect");
    fileMenu->addSeparator();
    auto* quitAct = fileMenu->addAction("&Quit");
    quitAct->setShortcut(QKeySequence("Ctrl+Q"));

    connect(reconnectAct,  &QAction::triggered, this, &MainWindow::onReconnect);
    connect(disconnectAct, &QAction::triggered, this, &MainWindow::onDisconnect);
    connect(quitAct,       &QAction::triggered, qApp, &QApplication::quit);

    // ── View ──────────────────────────────────────────────────────────────
    auto* viewMenu = menuBar()->addMenu("&View");

    auto* refreshAct = viewMenu->addAction("&Refresh Explorer");
    refreshAct->setShortcut(QKeySequence("F5"));

    auto* themeAct = viewMenu->addAction("Toggle &Dark Mode");
    themeAct->setShortcut(QKeySequence("Ctrl+Shift+D"));

    connect(refreshAct, &QAction::triggered, m_explorer, &DatabaseExplorer::refresh);
    connect(themeAct,   &QAction::triggered, this,        &MainWindow::onToggleTheme);

    // ── Help ──────────────────────────────────────────────────────────────
    auto* helpMenu = menuBar()->addMenu("&Help");
    auto* aboutAct = helpMenu->addAction("&About PGViewer");
    connect(aboutAct, &QAction::triggered, this, &MainWindow::onAbout);
}

void MainWindow::buildToolBar()
{
    auto* tb = addToolBar("Main Toolbar");
    tb->setMovable(false);
    tb->setFloatable(false);

    auto* refreshAct = tb->addAction("↻  Refresh");
    refreshAct->setToolTip("Reload schema tree (F5)");

    tb->addSeparator();

    auto* themeAct = tb->addAction("◑  Theme");
    themeAct->setToolTip("Toggle light/dark mode (Ctrl+Shift+D)");

    tb->addSeparator();

    auto* reconnectAct = tb->addAction("⚡  Reconnect");
    reconnectAct->setToolTip("Reconnect to database (Ctrl+R)");

    // Spacer
    auto* spacer = new QWidget(tb);
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    tb->addWidget(spacer);

    // Database info label
    auto* dbLabel = new QLabel(
        QString("  %1 / %2  ").arg(m_params.host, m_params.database), tb
    );
    dbLabel->setStyleSheet("color: palette(mid); font-size: 12px;");
    tb->addWidget(dbLabel);

    connect(refreshAct,   &QAction::triggered, m_explorer, &DatabaseExplorer::refresh);
    connect(themeAct,     &QAction::triggered, this,        &MainWindow::onToggleTheme);
    connect(reconnectAct, &QAction::triggered, this,        &MainWindow::onReconnect);
}

// ── Periodic ping ─────────────────────────────────────────────────────────────

void MainWindow::startConnectionTimer()
{
    m_pingTimer = new QTimer(this);
    m_pingTimer->setInterval(30'000); // 30 seconds
    connect(m_pingTimer, &QTimer::timeout, this, &MainWindow::onCheckConnection);
    m_pingTimer->start();
}

// ── Slots ─────────────────────────────────────────────────────────────────────

void MainWindow::onTableSelected(const QString& schema, const QString& table)
{
    m_tableView->loadTable(schema, table);
    m_config.setSetting("lastSchema", schema);
    m_config.setSetting("lastTable",  table);
    m_statusBar->showMessage(
        QString("Opened %1.%2").arg(schema, table), 3000
    );
}

void MainWindow::onStatusMessage(const QString& msg)
{
    m_statusBar->showMessage(msg, 5000);
}

void MainWindow::onDisconnect()
{
    m_pingTimer->stop();
    m_conn->disconnect();
    m_statusBar->setConnectionStatus(false);
    close();
}

void MainWindow::onReconnect()
{
    m_statusBar->showMessage("Reconnecting…", 2000);
    m_conn->disconnect();

    QString err = m_conn->connect(m_params);
    if (err.isEmpty()) {
        m_statusBar->setConnectionStatus(
            true,
            QString("%1@%2/%3")
            .arg(m_params.username, m_params.host, m_params.database)
        );
        m_explorer->refresh();
        m_statusBar->showMessage("Reconnected successfully.", 3000);
    } else {
        m_statusBar->setConnectionStatus(false);
        QMessageBox::critical(this, "Reconnect Failed",
            "Could not reconnect:\n" + err);
    }
}

void MainWindow::onToggleTheme()
{
    bool nowDark = !m_theme->isDark();
    m_theme->toggle(*qApp);
    m_config.setSetting("theme", nowDark ? "dark" : "light");
}

void MainWindow::onAbout()
{
    QMessageBox::about(this, "About PGViewer",
        "<h2>PGViewer 1.0</h2>"
        "<p>A lightweight, modern PostgreSQL database viewer and editor.</p>"
        "<p><b>Features:</b><br/>"
        "• Schema explorer with search<br/>"
        "• Table viewer with pagination, sort, filter, inline editing<br/>"
        "• Full CRUD support<br/>"
        "• SQL query editor with syntax highlighting<br/>"
        "• CSV export<br/>"
        "• Light / Dark theme</p>"
        "<p>Built with <b>C++17</b>, <b>Qt6</b>, and <b>PostgreSQL</b>.</p>"
    );
}

void MainWindow::onCheckConnection()
{
    bool alive = m_conn->ping();
    m_statusBar->setConnectionStatus(
        alive,
        alive
          ? QString("%1@%2/%3")
              .arg(m_params.username, m_params.host, m_params.database)
          : QString()
    );
    if (!alive) {
        m_statusBar->showMessage(
            "⚠ Connection lost — use Reconnect to restore.", 0
        );
    }
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    m_pingTimer->stop();
    m_conn->disconnect();
    event->accept();
}

} // namespace PGViewer
