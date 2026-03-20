#include "TableViewer.h"
#include "CellEditDelegate.h"
#include "services/DatabaseService.h"
#include "services/ExportService.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QFileDialog>
#include <QInputDialog>
#include <QApplication>

namespace PGViewer {

TableViewer::TableViewer(DatabaseService* svc, QWidget* parent)
    : QWidget(parent), m_svc(svc)
{
    buildUi();
}

void TableViewer::buildUi()
{
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    // ── Toolbar ────────────────────────────────────────────────────────────
    auto* toolbar = new QWidget(this);
    toolbar->setFixedHeight(44);
    toolbar->setStyleSheet(
        "background: palette(window); border-bottom: 1px solid palette(mid);"
    );
    auto* tl = new QHBoxLayout(toolbar);
    tl->setContentsMargins(8, 0, 8, 0);
    tl->setSpacing(6);

    auto* addBtn = new QPushButton("+ Add Row", toolbar);
    addBtn->setToolTip("Insert a new row");

    auto* delBtn = new QPushButton("✕ Delete Row", toolbar);
    delBtn->setObjectName("dangerBtn");
    delBtn->setToolTip("Delete selected row(s)");

    auto* exportBtn = new QPushButton("↓ Export CSV", toolbar);
    exportBtn->setObjectName("secondaryBtn");
    exportBtn->setToolTip("Export current page to CSV");

    // Filter widgets
    auto* filterLbl = new QLabel("Filter:", toolbar);
    filterLbl->setStyleSheet("color: palette(mid); font-size: 12px;");

    m_filterColCombo = new QComboBox(toolbar);
    m_filterColCombo->setFixedWidth(130);
    m_filterColCombo->setToolTip("Select column to filter on");

    m_filterEdit = new QLineEdit(toolbar);
    m_filterEdit->setPlaceholderText("value…");
    m_filterEdit->setFixedWidth(160);
    m_filterEdit->setToolTip("Press Enter to apply filter");

    auto* applyFilterBtn = new QPushButton("Apply", toolbar);
    applyFilterBtn->setObjectName("secondaryBtn");
    applyFilterBtn->setFixedWidth(60);
    applyFilterBtn->setToolTip("Apply filter");

    m_rowCountLabel = new QLabel(toolbar);
    m_rowCountLabel->setStyleSheet("color: palette(mid); font-size: 12px;");

    tl->addWidget(addBtn);
    tl->addWidget(delBtn);
    tl->addWidget(exportBtn);
    tl->addSpacing(12);
    tl->addWidget(filterLbl);
    tl->addWidget(m_filterColCombo);
    tl->addWidget(m_filterEdit);
    tl->addWidget(applyFilterBtn);
    tl->addStretch();
    tl->addWidget(m_rowCountLabel);

    connect(addBtn,         &QPushButton::clicked,      this, &TableViewer::onAddRow);
    connect(delBtn,         &QPushButton::clicked,      this, &TableViewer::onDeleteRow);
    connect(exportBtn,      &QPushButton::clicked,      this, &TableViewer::onExportCsv);
    connect(applyFilterBtn, &QPushButton::clicked,      this, &TableViewer::onApplyFilter);
    connect(m_filterEdit,   &QLineEdit::returnPressed,  this, &TableViewer::onApplyFilter);

    root->addWidget(toolbar);

    // ── Table view ─────────────────────────────────────────────────────────
    m_model = new QStandardItemModel(this);
    connect(m_model, &QStandardItemModel::itemChanged,
            this,    &TableViewer::onCellChanged);

    m_tableView = new QTableView(this);
    m_tableView->setModel(m_model);
    m_tableView->setItemDelegate(new CellEditDelegate(this));
    m_tableView->horizontalHeader()->setStretchLastSection(false);
    m_tableView->horizontalHeader()->setSectionsMovable(true);
    m_tableView->horizontalHeader()->setHighlightSections(false);
    m_tableView->verticalHeader()->setDefaultSectionSize(26);
    m_tableView->verticalHeader()->setVisible(true);
    m_tableView->setAlternatingRowColors(true);
    m_tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_tableView->setEditTriggers(QAbstractItemView::DoubleClicked |
                                  QAbstractItemView::EditKeyPressed);
    m_tableView->setSortingEnabled(false); // we handle sorting manually
    m_tableView->setShowGrid(true);

    // Sort on column header click
    connect(m_tableView->horizontalHeader(), &QHeaderView::sectionClicked,
            this, &TableViewer::onSortChanged);

    root->addWidget(m_tableView);

    // ── Pagination bar ─────────────────────────────────────────────────────
    auto* pagebar = new QWidget(this);
    pagebar->setFixedHeight(38);
    pagebar->setStyleSheet(
        "background: palette(window); border-top: 1px solid palette(mid);"
    );
    auto* pl = new QHBoxLayout(pagebar);
    pl->setContentsMargins(8, 0, 8, 0);
    pl->setSpacing(8);

    m_prevBtn = new QPushButton("← Prev", pagebar);
    m_nextBtn = new QPushButton("Next →", pagebar);
    m_prevBtn->setObjectName("secondaryBtn");
    m_nextBtn->setObjectName("secondaryBtn");
    m_prevBtn->setFixedWidth(90);
    m_nextBtn->setFixedWidth(90);
    m_pageLabel = new QLabel(pagebar);
    m_pageLabel->setAlignment(Qt::AlignCenter);
    m_pageLabel->setStyleSheet("font-size: 12px; color: palette(text);");

    connect(m_prevBtn, &QPushButton::clicked, this, &TableViewer::onPrevPage);
    connect(m_nextBtn, &QPushButton::clicked, this, &TableViewer::onNextPage);

    pl->addWidget(m_prevBtn);
    pl->addStretch();
    pl->addWidget(m_pageLabel);
    pl->addStretch();
    pl->addWidget(m_nextBtn);

    root->addWidget(pagebar);
}

// ── Public API ────────────────────────────────────────────────────────────────

void TableViewer::loadTable(const QString& schema, const QString& table)
{
    m_schema   = schema;
    m_table    = table;
    m_page     = 0;
    m_sortCol  = -1;
    m_sortAsc  = true;

    // Fetch metadata
    m_pkColumn  = m_svc->fetchPrimaryKeyColumn(schema, table);
    m_totalRows = m_svc->countRows(schema, table);

    // Populate filter column combo
    m_filterColCombo->clear();
    m_filterColCombo->addItem("(any column)");
    for (const QString& col : m_svc->fetchColumnNames(schema, table))
        m_filterColCombo->addItem(col);

    m_filterEdit->clear();
    fetchPage();
}

// ── Private helpers ───────────────────────────────────────────────────────────

void TableViewer::fetchPage()
{
    if (m_schema.isEmpty() || m_table.isEmpty()) return;

    // Determine active filter
    QString filterCol = (m_filterColCombo->currentIndex() > 0)
                        ? m_filterColCombo->currentText()
                        : QString();
    QString filterVal = m_filterEdit->text().trimmed();

    // Determine sort column name
    QString sortColName;
    if (m_sortCol >= 0 && m_sortCol < m_model->columnCount()) {
        auto* hdr = m_model->horizontalHeaderItem(m_sortCol);
        if (hdr) sortColName = hdr->text();
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);
    QueryResult res = m_svc->fetchTableData(
        m_schema, m_table,
        m_pageSize, m_page * m_pageSize,
        sortColName, m_sortAsc,
        filterCol, filterVal
    );
    QApplication::restoreOverrideCursor();

    // Suppress itemChanged signals while populating
    m_ignoreChanges = true;
    m_model->clear();

    if (!res.success) {
        emit statusMessage("Error: " + res.errorMessage);
        m_ignoreChanges = false;
        updatePagination();
        return;
    }

    m_model->setColumnCount(res.columns.size());
    m_model->setHorizontalHeaderLabels(res.columns);

    for (const auto& row : res.rows) {
        QList<QStandardItem*> items;
        for (const QVariant& val : row) {
            auto* cell = new QStandardItem(val.isNull() ? QString() : val.toString());
            cell->setData(val, Qt::UserRole);  // keep original for UPDATE
            if (val.isNull()) {
                cell->setForeground(QColor("#95A5A6"));
                cell->setText("(null)");
            }
            items << cell;
        }
        m_model->appendRow(items);
    }

    // Resize columns to content (cap at 300px)
    m_tableView->resizeColumnsToContents();
    for (int c = 0; c < m_model->columnCount(); ++c) {
        if (m_tableView->columnWidth(c) > 300)
            m_tableView->setColumnWidth(c, 300);
    }

    m_ignoreChanges = false;
    updatePagination();
    emit statusMessage(res.summary());
}

void TableViewer::updatePagination()
{
    int totalPages = (m_totalRows > 0)
                     ? (m_totalRows + m_pageSize - 1) / m_pageSize
                     : 1;
    m_pageLabel->setText(
        QString("Page %1 of %2  ·  %3 total rows")
        .arg(m_page + 1).arg(totalPages).arg(m_totalRows)
    );
    m_prevBtn->setEnabled(m_page > 0);
    m_nextBtn->setEnabled((m_page + 1) * m_pageSize < m_totalRows);
    m_rowCountLabel->setText(
        QString("%1 rows shown").arg(m_model->rowCount())
    );
}

// ── Slots ─────────────────────────────────────────────────────────────────────

void TableViewer::onNextPage()
{
    ++m_page;
    fetchPage();
}

void TableViewer::onPrevPage()
{
    if (m_page > 0) { --m_page; fetchPage(); }
}

void TableViewer::onApplyFilter()
{
    m_page      = 0;
    m_totalRows = m_svc->countRows(m_schema, m_table); // recount for pagination
    fetchPage();
}

void TableViewer::onSortChanged(int col)
{
    if (m_sortCol == col) {
        m_sortAsc = !m_sortAsc;
    } else {
        m_sortCol = col;
        m_sortAsc = true;
    }
    fetchPage();
}

void TableViewer::onCellChanged(QStandardItem* item)
{
    if (m_ignoreChanges || m_pkColumn.isEmpty()) return;

    int row = item->row();

    // Find PK column index
    int pkColIdx = -1;
    for (int c = 0; c < m_model->columnCount(); ++c) {
        auto* h = m_model->horizontalHeaderItem(c);
        if (h && h->text() == m_pkColumn) { pkColIdx = c; break; }
    }
    if (pkColIdx < 0) return;

    QVariant pkVal   = m_model->item(row, pkColIdx)->data(Qt::UserRole);
    QString  colName = m_model->horizontalHeaderItem(item->column())->text();
    QString  newText = item->text();

    // Treat "(null)" as NULL
    QVariant newVal  = (newText == "(null)") ? QVariant() : QVariant(newText);

    bool ok = m_svc->updateCell(m_schema, m_table, m_pkColumn, pkVal, colName, newVal);
    if (ok) {
        item->setData(newVal, Qt::UserRole);
        item->setForeground(QColor("#27AE60")); // flash green briefly
        emit statusMessage(QString("Updated %1 · column: %2").arg(m_table, colName));
    } else {
        emit statusMessage("⚠ Update failed — reverting.");
        // Revert to original
        m_ignoreChanges = true;
        QVariant orig = item->data(Qt::UserRole);
        item->setText(orig.isNull() ? "(null)" : orig.toString());
        item->setForeground(orig.isNull() ? QColor("#95A5A6") : QApplication::palette().text().color());
        m_ignoreChanges = false;
    }
}

void TableViewer::onAddRow()
{
    if (m_schema.isEmpty() || m_table.isEmpty()) return;

    QStringList cols = m_svc->fetchColumnNames(m_schema, m_table);
    if (cols.isEmpty()) {
        QMessageBox::warning(this, "Add Row", "Could not fetch column list.");
        return;
    }

    QVector<QVariant> values;
    for (const QString& col : cols) {
        bool ok = false;
        QString val = QInputDialog::getText(
            this,
            QString("Insert into %1").arg(m_table),
            QString("Value for column \"%1\"\n(leave blank for NULL):").arg(col),
            QLineEdit::Normal, {}, &ok
        );
        if (!ok) return; // user cancelled

        values << (val.isEmpty() ? QVariant() : QVariant(val));
    }

    if (m_svc->insertRow(m_schema, m_table, cols, values)) {
        ++m_totalRows;
        // Jump to last page to show the new row
        int lastPage = std::max(0, (m_totalRows - 1) / m_pageSize);
        m_page = lastPage;
        fetchPage();
        emit statusMessage("Row inserted successfully.");
    } else {
        QMessageBox::critical(this, "Insert Failed",
            "Failed to insert row.\n"
            "Check data types, NOT NULL constraints, and unique constraints.");
    }
}

void TableViewer::onDeleteRow()
{
    if (m_pkColumn.isEmpty()) {
        QMessageBox::warning(this, "Cannot Delete",
            "This table has no primary key detected.\n"
            "Use the Query Editor to delete rows manually.");
        return;
    }

    QModelIndexList sel = m_tableView->selectionModel()->selectedRows();
    if (sel.isEmpty()) {
        QMessageBox::information(this, "No Selection",
            "Please select one or more rows to delete.");
        return;
    }

    int answer = QMessageBox::warning(
        this, "Confirm Delete",
        QString("Permanently delete %1 row(s) from \"%2\"?\n\nThis cannot be undone.")
            .arg(sel.size()).arg(m_table),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No   // default to No for safety
    );
    if (answer != QMessageBox::Yes) return;

    // Find PK column index
    int pkColIdx = -1;
    for (int c = 0; c < m_model->columnCount(); ++c) {
        auto* h = m_model->horizontalHeaderItem(c);
        if (h && h->text() == m_pkColumn) { pkColIdx = c; break; }
    }
    if (pkColIdx < 0) return;

    // Collect rows to delete in reverse order
    QList<int> rows;
    for (const auto& idx : sel) rows << idx.row();
    std::sort(rows.rbegin(), rows.rend());

    int failed = 0;
    for (int r : rows) {
        QVariant pk = m_model->item(r, pkColIdx)->data(Qt::UserRole);
        if (m_svc->deleteRow(m_schema, m_table, m_pkColumn, pk)) {
            m_model->removeRow(r);
            --m_totalRows;
        } else {
            ++failed;
        }
    }

    updatePagination();

    if (failed > 0) {
        QMessageBox::warning(this, "Partial Delete",
            QString("%1 row(s) could not be deleted.\n"
                    "They may be referenced by foreign keys.").arg(failed));
    } else {
        emit statusMessage(QString("Deleted %1 row(s) from %2.")
                           .arg(rows.size()).arg(m_table));
    }
}

void TableViewer::onExportCsv()
{
    if (m_model->rowCount() == 0) {
        QMessageBox::information(this, "Export", "No data to export.");
        return;
    }

    QString defaultName = m_table + ".csv";
    QString path = QFileDialog::getSaveFileName(
        this, "Export to CSV", defaultName, "CSV Files (*.csv);;All Files (*)"
    );
    if (path.isEmpty()) return;

    // Build QueryResult from current model state
    QueryResult res;
    res.success = true;
    for (int c = 0; c < m_model->columnCount(); ++c) {
        auto* h = m_model->horizontalHeaderItem(c);
        res.columns << (h ? h->text() : QString("col%1").arg(c));
    }
    for (int r = 0; r < m_model->rowCount(); ++r) {
        QVector<QVariant> row;
        for (int c = 0; c < m_model->columnCount(); ++c) {
            auto* cell = m_model->item(r, c);
            row << (cell ? cell->text() : QVariant());
        }
        res.rows << row;
    }

    QString err = ExportService::exportToCsv(res, path);
    if (err.isEmpty()) {
        emit statusMessage(
            QString("Exported %1 rows to %2").arg(res.rows.size()).arg(path)
        );
    } else {
        QMessageBox::critical(this, "Export Failed", err);
    }
}

} // namespace PGViewer
