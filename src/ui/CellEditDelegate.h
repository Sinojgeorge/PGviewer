#pragma once

#include <QStyledItemDelegate>

namespace PGViewer {

/**
 * @brief Provides inline editing for table cells with proper commit/cancel handling.
 */
class CellEditDelegate : public QStyledItemDelegate {
    Q_OBJECT
public:
    explicit CellEditDelegate(QObject* parent = nullptr);

    QWidget* createEditor(QWidget* parent,
                          const QStyleOptionViewItem& option,
                          const QModelIndex& index) const override;

    void setEditorData(QWidget* editor,
                       const QModelIndex& index) const override;

    void setModelData(QWidget* editor,
                      QAbstractItemModel* model,
                      const QModelIndex& index) const override;

    void updateEditorGeometry(QWidget* editor,
                              const QStyleOptionViewItem& option,
                              const QModelIndex& index) const override;
};

} // namespace PGViewer
