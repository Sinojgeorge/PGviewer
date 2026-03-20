#pragma once

#include "core/QueryResult.h"
#include <QWidget>
#include <QTextEdit>
#include <QTableView>
#include <QStandardItemModel>
#include <QLabel>
#include <QPushButton>
#include <QSplitter>

namespace PGViewer {

class DatabaseService;

/**
 * @brief SQL editor tab: syntax-highlighted editor, run button (F5),
 *        results table, execution time / error display.
 */
class QueryEditor : public QWidget {
    Q_OBJECT
public:
    explicit QueryEditor(DatabaseService* svc, QWidget* parent = nullptr);

signals:
    void statusMessage(const QString& msg);

private slots:
    void onExecute();
    void onClear();
    void onFormatQuery();

private:
    void buildUi();
    void displayResult(const QueryResult& res);

    DatabaseService*    m_svc       = nullptr;
    QTextEdit*          m_editor    = nullptr;
    QTableView*         m_results   = nullptr;
    QStandardItemModel* m_model     = nullptr;
    QLabel*             m_statusLbl = nullptr;
    QLabel*             m_timeLbl   = nullptr;
    QPushButton*        m_runBtn    = nullptr;
};

} // namespace PGViewer
