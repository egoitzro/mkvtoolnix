#ifndef MTX_MKVTOOLNIX_GUI_UTIL_MODEL_H
#define MTX_MKVTOOLNIX_GUI_UTIL_MODEL_H

#include "common/common_pch.h"

class QAbstractItemModel;
class QAbstractItemView;
class QItemSelection;
class QItemSelectionModel;
class QTreeView;

namespace mtx { namespace gui { namespace Util {

// Model stuff
enum MtxGuiRoles {
  SourceFileRole = Qt::UserRole + 1,
  TrackRole,
  JobIdRole,
  HeaderEditorPageIdRole,
  ChapterEditorChapterOrEditionRole,
  ChapterEditorChapterDisplayRole,
  AttachmentRole,
  HiddenDescriptionRole,
  SymbolicNameRole,
};

void resizeViewColumnsToContents(QTreeView *view);
void setSymbolicColumnNames(QAbstractItemModel &model, QStringList const &names);
int numSelectedRows(QItemSelection &selection);
QModelIndex selectedRowIdx(QItemSelection const &selection);
QModelIndex selectedRowIdx(QAbstractItemView *view);
void withSelectedIndexes(QItemSelectionModel *selectionModel, std::function<void(QModelIndex const &)> worker);
void withSelectedIndexes(QAbstractItemView *view, std::function<void(QModelIndex const &)> worker);
void selectRow(QAbstractItemView *view, int row, QModelIndex const &parentIdx = QModelIndex{});
QModelIndex toTopLevelIdx(QModelIndex const &idx);
void walkTree(QAbstractItemModel &model, QModelIndex const &idx, std::function<void(QModelIndex const &)> const &worker);

}}}

#endif  // MTX_MKVTOOLNIX_GUI_UTIL_MODEL_H
