// synthv1widget_controls.h
//
/****************************************************************************
   Copyright (C) 2012-2015, rncbc aka Rui Nuno Capela. All rights reserved.

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License along
   with this program; if not, write to the Free Software Foundation, Inc.,
   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

*****************************************************************************/

#ifndef __synthv1widget_controls_h
#define __synthv1widget_controls_h

#include <QItemDelegate>
#include <QTreeWidget>


// forward decls.
class synthv1_controls;


//----------------------------------------------------------------------------
// synthv1widget_controls_item_delegate -- Custom (tree) list item delegate.

class synthv1widget_controls_item_delegate : public QItemDelegate
{
	Q_OBJECT

public:

	// ctor.
	synthv1widget_controls_item_delegate(QObject *pParent = 0);

	// QItemDelegate interface...
	QSize sizeHint(
		const QStyleOptionViewItem& option,
		const QModelIndex& index) const;

	QWidget *createEditor(QWidget *pParent,
		const QStyleOptionViewItem& option,
		const QModelIndex& index) const;

	void setEditorData(QWidget *pEditor,
		const QModelIndex& index) const;

	void setModelData(QWidget *pEditor,
		QAbstractItemModel *pModel,
		const QModelIndex& index) const;
};



//----------------------------------------------------------------------------
// synthv1widget_controls -- Custom (tree) widget.

class synthv1widget_controls : public QTreeWidget
{
	Q_OBJECT

public:

	// ctor.
	synthv1widget_controls(QWidget *pParent = 0);
	// dtor.
	~synthv1widget_controls();

	// utilities.
	void loadControls(synthv1_controls *pControls);
	void saveControls(synthv1_controls *pControls);

public slots:

	// slots.
	void addControlItem();

protected slots:

	// private slots.
	void itemChangedSlot(QTreeWidgetItem *, int);

protected:

	// factory methods.
	QTreeWidgetItem *newControlItem();
};


#endif	// __synthv1widget_controls_h

// end of synthv1widget_controls.h
