#include "LoginDialog.h"
#include "core/ConfigManager.h"
#include "core/DatabaseConnection.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QFrame>
#include <QApplication>
#include <QIcon>

namespace PGViewer {

LoginDialog::LoginDialog(ConfigManager& config, QWidget* parent)
    : QDialog(parent, Qt::Dialog | Qt::WindowCloseButtonHint),
      m_config(config)
{
    setWindowTitle("PGViewer — Connect to PostgreSQL");
    setMinimumWidth(420);
    setModal(true);
    buildUi();
    populateFromConfig();
}

ConnectionParams LoginDialog::params() const { return m_params; }

void LoginDialog::buildUi()
{
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(32, 28, 32, 28);
    root->setSpacing(0);

    // ── Header ──────────────────────────────────────────────────────────────
    auto* headerLabel = new QLabel("PGViewer", this);
    headerLabel->setStyleSheet(
        "font-size: 22px; font-weight: 700; color: #2980B9; margin-bottom: 2px;"
    );

    auto* subLabel = new QLabel("Connect to a PostgreSQL database", this);
    subLabel->setStyleSheet("font-size: 13px; color: #7F8C9A; margin-bottom: 20px;");

    root->addWidget(headerLabel);
    root->addWidget(subLabel);

    // ── Separator ───────────────────────────────────────────────────────────
    auto* sep = new QFrame(this);
    sep->setFrameShape(QFrame::HLine);
    sep->setStyleSheet("border-color: #E0E5EC; margin-bottom: 20px;");
    root->addWidget(sep);
    root->addSpacing(4);

    // ── Form ────────────────────────────────────────────────────────────────
    auto* form = new QFormLayout();
    form->setSpacing(10);
    form->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);
    form->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);

    m_hostEdit = new QLineEdit("localhost", this);
    m_hostEdit->setPlaceholderText("e.g. localhost or 192.168.1.1");

    m_portSpin = new QSpinBox(this);
    m_portSpin->setRange(1, 65535);
    m_portSpin->setValue(5432);
    m_portSpin->setFixedWidth(90);

    m_dbEdit = new QLineEdit(this);
    m_dbEdit->setPlaceholderText("database name");

    m_userEdit = new QLineEdit(this);
    m_userEdit->setPlaceholderText("username");

    m_passEdit = new QLineEdit(this);
    m_passEdit->setPlaceholderText("password");
    m_passEdit->setEchoMode(QLineEdit::Password);

    form->addRow("Host:", m_hostEdit);
    form->addRow("Port:", m_portSpin);
    form->addRow("Database:", m_dbEdit);
    form->addRow("Username:", m_userEdit);
    form->addRow("Password:", m_passEdit);

    root->addLayout(form);
    root->addSpacing(16);

    // ── Error label ─────────────────────────────────────────────────────────
    m_errorLabel = new QLabel(this);
    m_errorLabel->setStyleSheet(
        "color: #E74C3C; background: #FDEDEC; border: 1px solid #F1948A;"
        "border-radius: 4px; padding: 6px 10px; font-size: 12px;"
    );
    m_errorLabel->setWordWrap(true);
    m_errorLabel->hide();
    root->addWidget(m_errorLabel);
    root->addSpacing(4);

    // ── Status label ────────────────────────────────────────────────────────
    m_statusLabel = new QLabel(this);
    m_statusLabel->setStyleSheet("color: #2980B9; font-size: 12px;");
    m_statusLabel->hide();
    root->addWidget(m_statusLabel);
    root->addSpacing(8);

    // ── Connect button ──────────────────────────────────────────────────────
    m_connectBtn = new QPushButton("Connect", this);
    m_connectBtn->setDefault(true);
    m_connectBtn->setMinimumHeight(36);
    m_connectBtn->setStyleSheet(
        "QPushButton { font-size: 14px; font-weight: 600; }"
    );
    connect(m_connectBtn, &QPushButton::clicked,
            this,          &LoginDialog::onConnectClicked);

    root->addWidget(m_connectBtn);

    // Allow Enter key on all fields
    auto connectOnReturn = [this](){ onConnectClicked(); };
    connect(m_hostEdit,  &QLineEdit::returnPressed, this, connectOnReturn);
    connect(m_dbEdit,    &QLineEdit::returnPressed, this, connectOnReturn);
    connect(m_userEdit,  &QLineEdit::returnPressed, this, connectOnReturn);
    connect(m_passEdit,  &QLineEdit::returnPressed, this, connectOnReturn);
}

void LoginDialog::populateFromConfig()
{
    auto saved = m_config.loadConnection();
    if (!saved) return;

    m_hostEdit->setText(saved->host);
    m_portSpin->setValue(saved->port);
    m_dbEdit->setText(saved->database);
    m_userEdit->setText(saved->username);
    m_passEdit->setText(saved->password);
}

void LoginDialog::onConnectClicked()
{
    clearError();

    ConnectionParams p;
    p.host     = m_hostEdit->text().trimmed();
    p.port     = m_portSpin->value();
    p.database = m_dbEdit->text().trimmed();
    p.username = m_userEdit->text().trimmed();
    p.password = m_passEdit->text();

    if (p.host.isEmpty() || p.database.isEmpty() || p.username.isEmpty()) {
        showError("Please fill in Host, Database, and Username.");
        return;
    }

    setLoading(true);

    // Test connection (blocking — acceptable in login dialog modal context)
    DatabaseConnection testConn("pgviewer_login_test");
    QString err = testConn.connect(p);

    setLoading(false);

    if (!err.isEmpty()) {
        showError("Connection failed:\n" + err);
        return;
    }

    // Save credentials and proceed
    m_config.saveConnection(p);
    m_params = p;
    accept();
}

void LoginDialog::setLoading(bool loading)
{
    m_connectBtn->setEnabled(!loading);
    m_connectBtn->setText(loading ? "Connecting…" : "Connect");
    m_statusLabel->setText(loading ? "Connecting to database…" : "");
    m_statusLabel->setVisible(loading);
    QApplication::processEvents();  // update UI during blocking call
}

void LoginDialog::showError(const QString& msg)
{
    m_errorLabel->setText(msg);
    m_errorLabel->show();
}

void LoginDialog::clearError()
{
    m_errorLabel->hide();
    m_errorLabel->clear();
}

} // namespace PGViewer
