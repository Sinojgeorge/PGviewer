#include "ui/LoginDialog.h"
#include "ui/MainWindow.h"
#include "core/ConfigManager.h"

#include <QApplication>
#include <QStyleFactory>
#include <QFont>
#include <QScreen>

int main(int argc, char* argv[])
{
    // Enable High-DPI scaling (Qt 5 / 6 compatible)
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif

    QApplication app(argc, argv);

    // Application metadata
    app.setApplicationName("PGViewer");
    app.setApplicationDisplayName("PGViewer");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("PGViewer");

    // Apply Fusion style as a clean cross-platform base
    app.setStyle(QStyleFactory::create("Fusion"));

    // Set a sensible default font
    QFont defaultFont = app.font();
    defaultFont.setPointSize(10);
    app.setFont(defaultFont);

    // ── Config manager ─────────────────────────────────────────────────────
    PGViewer::ConfigManager config;

    // ── Login dialog ───────────────────────────────────────────────────────
    PGViewer::LoginDialog login(config);

    // Centre the login dialog on the primary screen
    QScreen* screen = QGuiApplication::primaryScreen();
    QRect    geo    = screen->availableGeometry();
    login.move(
        geo.center().x() - login.width()  / 2,
        geo.center().y() - login.height() / 2
    );

    if (login.exec() != QDialog::Accepted) {
        // User closed the login window — exit cleanly
        return 0;
    }

    // ── Main window ────────────────────────────────────────────────────────
    PGViewer::MainWindow window(login.params(), config);
    window.show();

    return app.exec();
}
