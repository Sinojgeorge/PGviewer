#pragma once

#include <QObject>
#include <QApplication>
#include <QPalette>
#include <QString>

namespace PGViewer {

/**
 * @brief Manages light / dark theme switching for the entire application.
 *
 * Call ThemeManager::apply(app, isDark) to switch themes at runtime.
 */
class ThemeManager : public QObject {
    Q_OBJECT
public:
    explicit ThemeManager(QObject* parent = nullptr);

    /** Apply the requested theme to the given QApplication instance. */
    static void apply(QApplication& app, bool dark);

    /** @return true if the dark theme is currently active. */
    bool isDark() const { return m_dark; }

public slots:
    void toggle(QApplication& app);

signals:
    void themeChanged(bool dark);

private:
    static QPalette lightPalette();
    static QPalette darkPalette();
    static QString  lightStyleSheet();
    static QString  darkStyleSheet();

    bool m_dark = false;
};

} // namespace PGViewer
