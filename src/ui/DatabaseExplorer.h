#pragma once

#include <QWidget>
#include <QTreeWidget>
#include <QPushButton>
#include <QLineEdit>
#include <memory>

namespace PGViewer {

class DatabaseService;

/**
 * @brief Left-panel tree widget showing Schemas → Tables / Views.
 *
 * Emits tableSelected(schema, table) when the user clicks a table or view.
 */
class DatabaseExplorer : public QWidget {
    Q_OBJECT
public:
    explicit DatabaseExplorer(DatabaseService* svc, QWidget* parent = nullptr);

    /** Reload the entire schema tree from the database. */
    void refresh();

signals:
    /** Emitted when the user selects a table or view in the tree. */
    void tableSelected(const QString& schema, const QString& table);

private slots:
    void onItemClicked(QTreeWidgetItem* item, int column);
    void onSearchChanged(const QString& text);

private:
    void buildUi();
    void buildTree();

    DatabaseService* m_svc;        ///< Non-owning
    QTreeWidget*     m_tree        = nullptr;
    QLineEdit*       m_searchEdit  = nullptr;
    QPushButton*     m_refreshBtn  = nullptr;
};

} // namespace PGViewer
