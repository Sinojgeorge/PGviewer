#include "DatabaseExplorer.h"
#include "services/DatabaseService.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QHeaderView>
#include <QIcon>
#include <QTreeWidgetItem>

namespace PGViewer {

// Item "type" roles stored in UserRole+1 on each tree item
static const int ItemKindRole = Qt::UserRole + 1;
enum ItemKind { KindSchema = 0, KindTable, KindView, KindGroup };

DatabaseExplorer::DatabaseExplorer(DatabaseService* svc, QWidget* parent)
    : QWidget(parent), m_svc(svc)
{
    buildUi();
    refresh();
}

void DatabaseExplorer::buildUi()
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    // ── Header bar ─────────────────────────────────────────────────────────
    auto* headerBar = new QWidget(this);
    headerBar->setFixedHeight(42);
    headerBar->setStyleSheet(
        "background-color: palette(window); border-bottom: 1px solid palette(mid);"
    );
    auto* hLayout = new QHBoxLayout(headerBar);
    hLayout->setContentsMargins(8, 0, 8, 0);

    auto* titleLabel = new QLabel("Explorer", headerBar);
    titleLabel->setStyleSheet("font-weight: 700; font-size: 13px;");

    m_refreshBtn = new QPushButton("↻", headerBar);
    m_refreshBtn->setObjectName("secondaryBtn");
    m_refreshBtn->setFixedSize(28, 28);
    m_refreshBtn->setToolTip("Refresh schema tree");
    connect(m_refreshBtn, &QPushButton::clicked, this, &DatabaseExplorer::refresh);

    hLayout->addWidget(titleLabel);
    hLayout->addStretch();
    hLayout->addWidget(m_refreshBtn);
    layout->addWidget(headerBar);

    // ── Search box ─────────────────────────────────────────────────────────
    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setPlaceholderText("Search tables…");
    m_searchEdit->setStyleSheet("margin: 6px 8px; border-radius: 4px;");
    connect(m_searchEdit, &QLineEdit::textChanged,
            this,          &DatabaseExplorer::onSearchChanged);
    layout->addWidget(m_searchEdit);

    // ── Tree widget ────────────────────────────────────────────────────────
    m_tree = new QTreeWidget(this);
    m_tree->setHeaderHidden(true);
    m_tree->setIndentation(16);
    m_tree->setAnimated(true);
    m_tree->setRootIsDecorated(true);
    m_tree->setAlternatingRowColors(false);
    m_tree->setSelectionMode(QAbstractItemView::SingleSelection);

    connect(m_tree, &QTreeWidget::itemClicked,
            this,    &DatabaseExplorer::onItemClicked);

    layout->addWidget(m_tree);
}

void DatabaseExplorer::refresh()
{
    m_tree->clear();
    buildTree();
}

void DatabaseExplorer::buildTree()
{
    auto allItems = m_svc->fetchAllTablesAndViews();

    // Group by schema
    QMap<QString, QVector<TableInfo>> bySchema;
    for (const auto& ti : allItems)
        bySchema[ti.schema].push_back(ti);

    for (auto it = bySchema.begin(); it != bySchema.end(); ++it) {
        // Schema node
        auto* schemaItem = new QTreeWidgetItem(m_tree);
        schemaItem->setText(0, it.key());
        schemaItem->setData(0, ItemKindRole, KindSchema);
        schemaItem->setExpanded(true);
        QFont sf = schemaItem->font(0);
        sf.setBold(true);
        schemaItem->setFont(0, sf);

        // Group: Tables
        auto* tablesGroup = new QTreeWidgetItem(schemaItem);
        tablesGroup->setText(0, "Tables");
        tablesGroup->setData(0, ItemKindRole, KindGroup);
        tablesGroup->setExpanded(true);
        tablesGroup->setForeground(0, QColor("#7F8C9A"));
        QFont gf = tablesGroup->font(0);
        gf.setItalic(true);
        tablesGroup->setFont(0, gf);

        // Group: Views
        auto* viewsGroup = new QTreeWidgetItem(schemaItem);
        viewsGroup->setText(0, "Views");
        viewsGroup->setData(0, ItemKindRole, KindGroup);
        viewsGroup->setExpanded(false);
        viewsGroup->setForeground(0, QColor("#7F8C9A"));
        viewsGroup->setFont(0, gf);

        for (const auto& ti : it.value()) {
            auto* node = new QTreeWidgetItem();
            node->setText(0, ti.name);

            bool isView = (ti.type == "VIEW");
            node->setData(0, ItemKindRole, isView ? KindView : KindTable);
            node->setData(0, Qt::UserRole, ti.schema);   // store schema for retrieval

            if (isView) {
                node->setForeground(0, QColor("#8E44AD"));
                viewsGroup->addChild(node);
            } else {
                tablesGroup->addChild(node);
            }
        }

        // Remove empty groups
        if (tablesGroup->childCount() == 0)
            delete schemaItem->takeChild(schemaItem->indexOfChild(tablesGroup));
        if (viewsGroup->childCount() == 0)
            delete schemaItem->takeChild(schemaItem->indexOfChild(viewsGroup));
    }
}

void DatabaseExplorer::onItemClicked(QTreeWidgetItem* item, int /*column*/)
{
    if (!item) return;
    int kind = item->data(0, ItemKindRole).toInt();
    if (kind == KindTable || kind == KindView) {
        QString schema = item->data(0, Qt::UserRole).toString();
        QString table  = item->text(0);
        emit tableSelected(schema, table);
    }
}

void DatabaseExplorer::onSearchChanged(const QString& text)
{
    // Show/hide items matching the search text
    for (int si = 0; si < m_tree->topLevelItemCount(); ++si) {
        auto* schema = m_tree->topLevelItem(si);
        for (int gi = 0; gi < schema->childCount(); ++gi) {
            auto* group = schema->child(gi);
            bool anyVisible = false;
            for (int ti = 0; ti < group->childCount(); ++ti) {
                auto* tableNode = group->child(ti);
                bool match = text.isEmpty() ||
                             tableNode->text(0).contains(text, Qt::CaseInsensitive);
                tableNode->setHidden(!match);
                if (match) anyVisible = true;
            }
            group->setHidden(!anyVisible && !text.isEmpty());
        }
    }
}

} // namespace PGViewer
