#include "StatusBar.h"

namespace PGViewer {

StatusBar::StatusBar(QWidget* parent) : QStatusBar(parent)
{
    setSizeGripEnabled(false);

    m_connIcon = new QLabel("●", this);
    m_connIcon->setStyleSheet("color: #E74C3C; font-size: 11px; margin: 0 3px;");

    m_connLabel = new QLabel("Disconnected", this);
    m_connLabel->setStyleSheet("font-size: 12px; margin-right: 8px;");

    addPermanentWidget(m_connIcon);
    addPermanentWidget(m_connLabel);
}

void StatusBar::setConnectionStatus(bool connected, const QString& info)
{
    if (connected) {
        m_connIcon->setStyleSheet("color: #2ECC71; font-size: 11px; margin: 0 3px;");
        m_connLabel->setText(info.isEmpty() ? "Connected" : info);
    } else {
        m_connIcon->setStyleSheet("color: #E74C3C; font-size: 11px; margin: 0 3px;");
        m_connLabel->setText("Disconnected");
    }
}

} // namespace PGViewer
