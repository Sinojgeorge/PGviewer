#include "CellEditDelegate.h"
#include <QLineEdit>

namespace PGViewer {

CellEditDelegate::CellEditDelegate(QObject* parent)
    : QStyledItemDelegate(parent) {}

QWidget* CellEditDelegate::createEditor(QWidget* parent,
    const QStyleOptionViewItem&, const QModelIndex&) const
{
    auto* editor = new QLineEdit(parent);
    editor->setFrame(false);
    editor->setStyleSheet("padding: 2px 4px; background: palette(base);");
    return editor;
}

void CellEditDelegate::setEditorData(QWidget* editor,
    const QModelIndex& index) const
{
    auto* le = qobject_cast<QLineEdit*>(editor);
    if (le) le->setText(index.data(Qt::EditRole).toString());
}

void CellEditDelegate::setModelData(QWidget* editor,
    QAbstractItemModel* model, const QModelIndex& index) const
{
    auto* le = qobject_cast<QLineEdit*>(editor);
    if (le) model->setData(index, le->text(), Qt::EditRole);
}

void CellEditDelegate::updateEditorGeometry(QWidget* editor,
    const QStyleOptionViewItem& option, const QModelIndex&) const
{
    editor->setGeometry(option.rect);
}

} // namespace PGViewer
