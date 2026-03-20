#pragma once

#include <QStatusBar>
#include <QLabel>

namespace PGViewer {

/**
 * @brief Custom status bar with a permanent connection indicator on the right.
 */
class StatusBar : public QStatusBar {
    Q_OBJECT
public:
    explicit StatusBar(QWidget* parent = nullptr);

    /** Update the connection indicator. */
    void setConnectionStatus(bool connected, const QString& info = {});

private:
    QLabel* m_connIcon  = nullptr;
    QLabel* m_connLabel = nullptr;
};

} // namespace PGViewer
