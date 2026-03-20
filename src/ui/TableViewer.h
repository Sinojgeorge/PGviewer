#pragma once

#include "core/QueryResult.h"
#include <QWidget>
#include <QTableView>
#include <QStandardItemModel>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>

namespace PGViewer {

class DatabaseService;

/**
 * @brief Paginated table data viewer with inline editing, filtering, sorting,
 *        row insertion/deletion, and CSV export.
 */
class TableViewer : public QWidget {
    Q_OBJECT
public:
    explicit TableViewer(DatabaseService* svc, QWidget* parent = nullptr);

    /** Load and display data for the given schema.table. */
    void loadTable(const QString& schema, const QString& table);

signals:
    void statusMessage(const QString& msg);

private slots:
    void onNextPage();
    void onPrevPage();
    void onAddRow();
    void onDeleteRow();
    void onCellChanged(QStandardItem* item);
    void onExportCsv();
    void onApplyFilter();
    void onSortChanged(int col);

private:
    void buildUi();
    void fetchPage();
    void updatePagination();

    DatabaseService*    m_svc            = nullptr;
    QTableView*         m_tableView      = nullptr;
    QStandardItemModel* m_model          = nullptr;
    QPushButton*        m_prevBtn        = nullptr;
    QPushButton*        m_nextBtn        = nullptr;
    QLabel*             m_pageLabel      = nullptr;
    QLabel*             m_rowCountLabel  = nullptr;
    QLineEdit*          m_filterEdit     = nullptr;
    QComboBox*          m_filterColCombo = nullptr;

    QString  m_schema, m_table, m_pkColumn;
    int      m_page           = 0;
    int      m_pageSize       = 100;
    int      m_totalRows      = 0;
    int      m_sortCol        = -1;
    bool     m_sortAsc        = true;
    bool     m_ignoreChanges  = false;
};

} // namespace PGViewer
