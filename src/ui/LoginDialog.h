#pragma once

#include "core/DatabaseConnection.h"
#include <QDialog>
#include <QLineEdit>
#include <QSpinBox>
#include <QPushButton>
#include <QLabel>
#include <memory>

namespace PGViewer {

class ConfigManager;

/**
 * @brief Modal login screen for entering PostgreSQL connection credentials.
 *
 * On successful authentication the dialog calls accept().
 * The filled-in ConnectionParams can then be retrieved via params().
 */
class LoginDialog : public QDialog {
    Q_OBJECT
public:
    explicit LoginDialog(ConfigManager& config, QWidget* parent = nullptr);

    /** @return The parameters that were used for the successful connection. */
    ConnectionParams params() const;

private slots:
    void onConnectClicked();

private:
    void buildUi();
    void populateFromConfig();
    void setLoading(bool loading);
    void showError(const QString& msg);
    void clearError();

    ConfigManager& m_config;

    QLineEdit*   m_hostEdit     = nullptr;
    QSpinBox*    m_portSpin     = nullptr;
    QLineEdit*   m_dbEdit       = nullptr;
    QLineEdit*   m_userEdit     = nullptr;
    QLineEdit*   m_passEdit     = nullptr;
    QPushButton* m_connectBtn   = nullptr;
    QLabel*      m_errorLabel   = nullptr;
    QLabel*      m_statusLabel  = nullptr;

    ConnectionParams m_params;
};

} // namespace PGViewer
