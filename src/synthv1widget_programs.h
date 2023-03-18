// synthv1widget_programs.h
//
/****************************************************************************
   Copyright (C) 2012-2023, rncbc aka Rui Nuno Capela. All rights reserved.

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

#ifndef __synthv1widget_programs_h
#define __synthv1widget_programs_h

#include <QTreeWidget>


// forward decls.
class synthv1_programs;


//----------------------------------------------------------------------------
// synthv1widget_programs -- Custom (tree) widget.

class synthv1widget_programs : public QTreeWidget
{
	Q_OBJECT

public:

	// ctor.
	synthv1widget_programs(QWidget *pParent = nullptr);
	// dtor.
	~synthv1widget_programs();

	// utilities.
	void loadPrograms(synthv1_programs *pPrograms);
	void savePrograms(synthv1_programs *pPrograms);

	QString currentProgramName() const;

public slots:

	// slots.
	void addBankItem();
	void addProgramItem();

protected slots:

	// private slots.
	void itemChangedSlot(QTreeWidgetItem *, int);

	void itemExpandedSlot(QTreeWidgetItem *);
	void itemCollapsedSlot(QTreeWidgetItem *);

protected:

	// item delegate decl..
	class ItemDelegate;

	// factory methods.
	QTreeWidgetItem *newBankItem();
	QTreeWidgetItem *newProgramItem();
};


#endif	// __synthv1widget_programs_h

// end of synthv1widget_programs.h
