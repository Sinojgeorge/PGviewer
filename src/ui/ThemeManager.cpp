#include "ThemeManager.h"

#include <QStyle>
#include <QStyleFactory>

namespace PGViewer {

ThemeManager::ThemeManager(QObject* parent) : QObject(parent) {}

void ThemeManager::apply(QApplication& app, bool dark)
{
    app.setStyle(QStyleFactory::create("Fusion"));
    app.setPalette(dark ? darkPalette() : lightPalette());
    app.setStyleSheet(dark ? darkStyleSheet() : lightStyleSheet());
}

void ThemeManager::toggle(QApplication& app)
{
    m_dark = !m_dark;
    apply(app, m_dark);
    emit themeChanged(m_dark);
}

// ── Palettes ──────────────────────────────────────────────────────────────────

QPalette ThemeManager::lightPalette()
{
    QPalette p;
    p.setColor(QPalette::Window,          QColor(0xF5F7FA));
    p.setColor(QPalette::WindowText,      QColor(0x2C3E50));
    p.setColor(QPalette::Base,            QColor(0xFFFFFF));
    p.setColor(QPalette::AlternateBase,   QColor(0xEEF1F5));
    p.setColor(QPalette::ToolTipBase,     QColor(0xFFFFFF));
    p.setColor(QPalette::ToolTipText,     QColor(0x2C3E50));
    p.setColor(QPalette::Text,            QColor(0x2C3E50));
    p.setColor(QPalette::Button,          QColor(0xE8ECF0));
    p.setColor(QPalette::ButtonText,      QColor(0x2C3E50));
    p.setColor(QPalette::BrightText,      Qt::red);
    p.setColor(QPalette::Link,            QColor(0x3498DB));
    p.setColor(QPalette::Highlight,       QColor(0x3498DB));
    p.setColor(QPalette::HighlightedText, QColor(0xFFFFFF));
    return p;
}

QPalette ThemeManager::darkPalette()
{
    QPalette p;
    p.setColor(QPalette::Window,          QColor(0x1E2329));
    p.setColor(QPalette::WindowText,      QColor(0xE8ECF0));
    p.setColor(QPalette::Base,            QColor(0x252B33));
    p.setColor(QPalette::AlternateBase,   QColor(0x2C3440));
    p.setColor(QPalette::ToolTipBase,     QColor(0x1E2329));
    p.setColor(QPalette::ToolTipText,     QColor(0xE8ECF0));
    p.setColor(QPalette::Text,            QColor(0xE8ECF0));
    p.setColor(QPalette::Button,          QColor(0x2C3440));
    p.setColor(QPalette::ButtonText,      QColor(0xE8ECF0));
    p.setColor(QPalette::BrightText,      Qt::red);
    p.setColor(QPalette::Link,            QColor(0x5DADE2));
    p.setColor(QPalette::Highlight,       QColor(0x2E86C1));
    p.setColor(QPalette::HighlightedText, QColor(0xFFFFFF));
    return p;
}

// ── Style sheets ──────────────────────────────────────────────────────────────

QString ThemeManager::lightStyleSheet()
{
    return R"(
QMainWindow, QDialog {
    background-color: #F5F7FA;
}
QSplitter::handle {
    background-color: #D5DBE2;
    width: 1px;
    height: 1px;
}
QTabWidget::pane {
    border: 1px solid #D5DBE2;
    border-radius: 4px;
}
QTabBar::tab {
    background: #E8ECF0;
    color: #5D6D7E;
    padding: 6px 16px;
    border: 1px solid #D5DBE2;
    border-bottom: none;
    border-top-left-radius: 4px;
    border-top-right-radius: 4px;
}
QTabBar::tab:selected {
    background: #FFFFFF;
    color: #2C3E50;
    font-weight: 600;
}
QTabBar::tab:hover:!selected {
    background: #F0F3F7;
}
QTreeWidget {
    background-color: #FFFFFF;
    border: 1px solid #D5DBE2;
    border-radius: 4px;
    outline: none;
}
QTreeWidget::item {
    height: 26px;
    padding-left: 4px;
}
QTreeWidget::item:selected {
    background-color: #EBF5FB;
    color: #2980B9;
}
QTreeWidget::item:hover {
    background-color: #F2F9FF;
}
QHeaderView::section {
    background-color: #EEF1F5;
    color: #5D6D7E;
    padding: 5px 8px;
    border: none;
    border-right: 1px solid #D5DBE2;
    border-bottom: 1px solid #D5DBE2;
    font-weight: 600;
    font-size: 12px;
}
QTableView {
    gridline-color: #E8ECF0;
    border: 1px solid #D5DBE2;
    outline: none;
}
QTableView::item:selected {
    background-color: #D6EAF8;
    color: #1A252F;
}
QPushButton {
    background-color: #3498DB;
    color: #FFFFFF;
    border: none;
    border-radius: 4px;
    padding: 6px 14px;
    font-weight: 600;
    min-height: 28px;
}
QPushButton:hover    { background-color: #2E86C1; }
QPushButton:pressed  { background-color: #2874A6; }
QPushButton:disabled { background-color: #AEB6BF; }
QPushButton#secondaryBtn {
    background-color: #E8ECF0;
    color: #2C3E50;
    border: 1px solid #D5DBE2;
}
QPushButton#secondaryBtn:hover { background-color: #D5DBE2; }
QPushButton#dangerBtn {
    background-color: #E74C3C;
}
QPushButton#dangerBtn:hover { background-color: #CB4335; }
QLineEdit, QSpinBox, QComboBox {
    background-color: #FFFFFF;
    border: 1px solid #CCD5DC;
    border-radius: 4px;
    padding: 5px 8px;
    color: #2C3E50;
    selection-background-color: #3498DB;
    min-height: 28px;
}
QLineEdit:focus, QSpinBox:focus, QComboBox:focus {
    border-color: #3498DB;
    outline: none;
}
QTextEdit {
    background-color: #FFFFFF;
    border: 1px solid #D5DBE2;
    border-radius: 4px;
    color: #2C3E50;
    font-family: 'Cascadia Code', 'JetBrains Mono', 'Consolas', monospace;
    font-size: 13px;
}
QStatusBar {
    background-color: #FFFFFF;
    border-top: 1px solid #D5DBE2;
    color: #5D6D7E;
    font-size: 12px;
}
QScrollBar:vertical {
    width: 8px;
    background: transparent;
}
QScrollBar::handle:vertical {
    background: #C8D0DA;
    border-radius: 4px;
    min-height: 30px;
}
QScrollBar::handle:vertical:hover { background: #ADB6C4; }
QScrollBar:horizontal {
    height: 8px;
    background: transparent;
}
QScrollBar::handle:horizontal {
    background: #C8D0DA;
    border-radius: 4px;
    min-width: 30px;
}
QScrollBar::add-line, QScrollBar::sub-line { width: 0; height: 0; }
QToolBar {
    background-color: #FFFFFF;
    border-bottom: 1px solid #D5DBE2;
    spacing: 4px;
    padding: 2px 4px;
}
QToolButton {
    border: none;
    border-radius: 4px;
    padding: 4px 8px;
    color: #5D6D7E;
}
QToolButton:hover { background-color: #EEF1F5; color: #2C3E50; }
QLabel#sectionHeader {
    font-size: 11px;
    font-weight: 700;
    color: #7F8C9A;
    letter-spacing: 0.5px;
    text-transform: uppercase;
    padding: 6px 0 2px 0;
}
)";
}

QString ThemeManager::darkStyleSheet()
{
    return R"(
QMainWindow, QDialog {
    background-color: #1E2329;
}
QSplitter::handle {
    background-color: #3A424F;
    width: 1px;
    height: 1px;
}
QTabWidget::pane {
    border: 1px solid #3A424F;
    border-radius: 4px;
}
QTabBar::tab {
    background: #252B33;
    color: #8995A3;
    padding: 6px 16px;
    border: 1px solid #3A424F;
    border-bottom: none;
    border-top-left-radius: 4px;
    border-top-right-radius: 4px;
}
QTabBar::tab:selected {
    background: #2C3440;
    color: #E8ECF0;
    font-weight: 600;
}
QTabBar::tab:hover:!selected { background: #2A3038; }
QTreeWidget {
    background-color: #252B33;
    border: 1px solid #3A424F;
    border-radius: 4px;
    outline: none;
    color: #C8D0DA;
}
QTreeWidget::item {
    height: 26px;
    padding-left: 4px;
}
QTreeWidget::item:selected {
    background-color: #1B3A5C;
    color: #5DADE2;
}
QTreeWidget::item:hover { background-color: #2A3748; }
QHeaderView::section {
    background-color: #252B33;
    color: #8995A3;
    padding: 5px 8px;
    border: none;
    border-right: 1px solid #3A424F;
    border-bottom: 1px solid #3A424F;
    font-weight: 600;
    font-size: 12px;
}
QTableView {
    gridline-color: #3A424F;
    border: 1px solid #3A424F;
    outline: none;
    background-color: #252B33;
    color: #C8D0DA;
    alternate-background-color: #2C3440;
}
QTableView::item:selected {
    background-color: #1B3A5C;
    color: #E8ECF0;
}
QPushButton {
    background-color: #2E86C1;
    color: #FFFFFF;
    border: none;
    border-radius: 4px;
    padding: 6px 14px;
    font-weight: 600;
    min-height: 28px;
}
QPushButton:hover    { background-color: #2980B9; }
QPushButton:pressed  { background-color: #2471A3; }
QPushButton:disabled { background-color: #3A424F; color: #5D6D7E; }
QPushButton#secondaryBtn {
    background-color: #2C3440;
    color: #C8D0DA;
    border: 1px solid #3A424F;
}
QPushButton#secondaryBtn:hover { background-color: #3A424F; }
QPushButton#dangerBtn { background-color: #C0392B; }
QPushButton#dangerBtn:hover { background-color: #A93226; }
QLineEdit, QSpinBox, QComboBox {
    background-color: #252B33;
    border: 1px solid #3A424F;
    border-radius: 4px;
    padding: 5px 8px;
    color: #E8ECF0;
    selection-background-color: #2E86C1;
    min-height: 28px;
}
QLineEdit:focus, QSpinBox:focus, QComboBox:focus { border-color: #2E86C1; }
QTextEdit {
    background-color: #1A1F27;
    border: 1px solid #3A424F;
    border-radius: 4px;
    color: #C8D0DA;
    font-family: 'Cascadia Code', 'JetBrains Mono', 'Consolas', monospace;
    font-size: 13px;
}
QStatusBar {
    background-color: #1A1F27;
    border-top: 1px solid #3A424F;
    color: #8995A3;
    font-size: 12px;
}
QScrollBar:vertical { width: 8px; background: transparent; }
QScrollBar::handle:vertical {
    background: #3A424F;
    border-radius: 4px;
    min-height: 30px;
}
QScrollBar::handle:vertical:hover { background: #4A5568; }
QScrollBar:horizontal { height: 8px; background: transparent; }
QScrollBar::handle:horizontal {
    background: #3A424F;
    border-radius: 4px;
    min-width: 30px;
}
QScrollBar::add-line, QScrollBar::sub-line { width: 0; height: 0; }
QToolBar {
    background-color: #1A1F27;
    border-bottom: 1px solid #3A424F;
    spacing: 4px;
    padding: 2px 4px;
}
QToolButton {
    border: none;
    border-radius: 4px;
    padding: 4px 8px;
    color: #8995A3;
}
QToolButton:hover { background-color: #2C3440; color: #E8ECF0; }
QLabel#sectionHeader {
    font-size: 11px;
    font-weight: 700;
    color: #4A5568;
    letter-spacing: 0.5px;
    padding: 6px 0 2px 0;
}
)";
}

} // namespace PGViewer
