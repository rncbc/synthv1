// synthv1widget_controls.cpp
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

#include "synthv1widget_controls.h"

#include "synthv1_controls.h"
#include "synthv1_config.h"

#include <QHeaderView>

#include <QSpinBox>
#include <QLineEdit>
#include <QComboBox>


//----------------------------------------------------------------------------
// synthv1widget_controls_item_delegate -- Custom (tree) list item delegate.

// ctor.
synthv1widget_controls_item_delegate::synthv1widget_controls_item_delegate (
	QObject *pParent ) : QItemDelegate(pParent)
{
}


// QItemDelegate interface...
QSize synthv1widget_controls_item_delegate::sizeHint (
	const QStyleOptionViewItem& option, const QModelIndex& index ) const
{
	return QItemDelegate::sizeHint(option, index) + QSize(4, 4);
}


QWidget *synthv1widget_controls_item_delegate::createEditor ( QWidget *pParent,
	const QStyleOptionViewItem& /*option*/, const QModelIndex& index ) const
{
	QWidget *pEditor = NULL;

	switch (index.column()) {
	case 0: // Channel.
	{
		QSpinBox *pSpinBox = new QSpinBox(pParent);
		pSpinBox->setMinimum(1);
		pSpinBox->setMaximum(16);
		pEditor = pSpinBox;
		break;
	}

	case 1: // Type.
	{
		QComboBox *pComboBox = new QComboBox(pParent);
		pComboBox->setEditable(false);
		pComboBox->addItem(synthv1_controls::textFromType(synthv1_controls::CC));
		pComboBox->addItem(synthv1_controls::textFromType(synthv1_controls::RPN));
		pComboBox->addItem(synthv1_controls::textFromType(synthv1_controls::NRPN));
		pComboBox->addItem(synthv1_controls::textFromType(synthv1_controls::CC14));
		pEditor = pComboBox;
		break;
	}

	case 2: // Parameter.
	{
		QComboBox *pComboBox = new QComboBox(pParent);
		pComboBox->setEditable(true);
		pEditor = pComboBox;
		break;
	}

	case 3: // Subject.
	{
		QComboBox *pComboBox = new QComboBox(pParent);
		pComboBox->setEditable(false);
		for (uint32_t i = 0; i < synthv1::NUM_PARAMS; ++i)
			pComboBox->addItem(synthv1_param::paramName(synthv1::ParamIndex(i)));
		pEditor = pComboBox;
		break;
	}

	default:
		break;
	}

#ifdef CONFIG_DEBUG_0
	qDebug("synthv1widget_controls_item_delegate::createEditor(%p, %d, %d) = %p",
		pParent, index.row(), index.column(), pEditor);
#endif

	return pEditor;
}


void synthv1widget_controls_item_delegate::setEditorData (
	QWidget *pEditor, const QModelIndex& index ) const
{
#ifdef CONFIG_DEBUG_0
	qDebug("synthv1widget_controls_item_delegate::setEditorData(%p, %d, %d)",
		pEditor, index.row(), index.column());
#endif

	switch (index.column()) {
	case 0: // Channel.
	{
		const int iChannel = index.data().toInt();
		//	= index.model()->data(index, Qt::DisplayRole).toInt();
		QSpinBox *pSpinBox = qobject_cast<QSpinBox *> (pEditor);
		if (pSpinBox) pSpinBox->setValue(iChannel);
		break;
	}

	case 1: // Type.
	{
		const QString& sText = index.data().toString();
		//	= index.model()->data(index, Qt::DisplayRole).toString();
		QComboBox *pComboBox = qobject_cast<QComboBox *> (pEditor);
		if (pComboBox) pComboBox->setEditText(sText);
		break;
	}

	case 2: // Parameter.
	{
		const QString& sText = index.data().toString();
		//	= index.model()->data(index, Qt::DisplayRole).toString();
		QComboBox *pComboBox = qobject_cast<QComboBox *> (pEditor);
		if (pComboBox) pComboBox->setEditText(sText);
		break;
	}

	case 3: // Subject.
	{
		const int iIndex = index.data(Qt::UserRole).toInt();
		//	= index.model()->data(index, Qt::DisplayRole).toInt();
		QComboBox *pComboBox = qobject_cast<QComboBox *> (pEditor);
		if (pComboBox) pComboBox->setCurrentIndex(iIndex);
		break;
	}

	default:
		break;
	}
}


void synthv1widget_controls_item_delegate::setModelData ( QWidget *pEditor,
	QAbstractItemModel *pModel,	const QModelIndex& index ) const
{
#ifdef CONFIG_DEBUG_0
	qDebug("synthv1widget_controls_item_delegate::setModelData(%p, %d, %d)",
		pEditor, index.row(), index.column());
#endif

	switch (index.column()) {
	case 0: // Channel.
	{
		QSpinBox *pSpinBox = qobject_cast<QSpinBox *> (pEditor);
		if (pSpinBox) {
			const int iChannel = pSpinBox->value();
			pModel->setData(index, QString::number(iChannel));
		}
		break;
	}

	case 1: // Type.
	{
		QComboBox *pComboBox = qobject_cast<QComboBox *> (pEditor);
		if (pComboBox) {
			const QString& sType = pComboBox->currentText();
			pModel->setData(index, sType);
		}
		break;
	}

	case 2: // Parameter.
	{
		QComboBox *pComboBox = qobject_cast<QComboBox *> (pEditor);
		if (pComboBox) {
			const QString& sParam = pComboBox->currentText();
			pModel->setData(index, sParam);
		}
		break;
	}

	case 3: // Subject.
	{
		QComboBox *pComboBox = qobject_cast<QComboBox *> (pEditor);
		if (pComboBox) {
			const int iIndex = pComboBox->currentIndex();
			pModel->setData(index,
				synthv1_param::paramName(synthv1::ParamIndex(iIndex)));
			pModel->setData(index, iIndex, Qt::UserRole);
		}
		break;
	}

	default:
		break;
	}

	// Done.
}


//----------------------------------------------------------------------------
// synthv1widget_controls -- UI wrapper form.

// ctor.
synthv1widget_controls::synthv1widget_controls ( QWidget *pParent )
	: QTreeWidget(pParent)
{
	QTreeWidget::setColumnCount(4);

	QTreeWidget::setRootIsDecorated(false);
	QTreeWidget::setAlternatingRowColors(true);
	QTreeWidget::setUniformRowHeights(true);
	QTreeWidget::setAllColumnsShowFocus(false);

	QTreeWidget::setSelectionBehavior(QAbstractItemView::SelectRows);
	QTreeWidget::setSelectionMode(QAbstractItemView::SingleSelection);

	QHeaderView *pHeaderView = QTreeWidget::header();
#if QT_VERSION < 0x050000
	pHeaderView->setResizeMode(QHeaderView::ResizeToContents);
#else
	pHeaderView->setSectionResizeMode(QHeaderView::ResizeToContents);
#endif
	pHeaderView->hide();

	QTreeWidget::setItemDelegate(new synthv1widget_controls_item_delegate(this));
}


// dtor.
synthv1widget_controls::~synthv1widget_controls (void)
{
}


// utilities.
void synthv1widget_controls::loadControls ( synthv1_controls *pControls )
{
	QTreeWidget::clear();

	const QIcon icon(":/images/synthv1_preset.png");
	QList<QTreeWidgetItem *> items;
	const synthv1_controls::Map& map = pControls->map();
	synthv1_controls::Map::ConstIterator iter = map.constBegin();
	const synthv1_controls::Map::ConstIterator& iter_end = map.constEnd();
	for ( ; iter != iter_end; ++iter) {
		const synthv1_controls::Key& key = iter.key();
		const synthv1::ParamIndex index = synthv1::ParamIndex(iter.value());
		QTreeWidgetItem *pItem = new QTreeWidgetItem(this);
		pItem->setIcon(0, icon);
		pItem->setText(0, QString::number(
			(key.status & 0x0f) + 1));
		pItem->setText(1, synthv1_controls::textFromType(
			synthv1_controls::Type(key.status & 0xf0)));
		pItem->setText(2, QString::number(key.param));
		pItem->setIcon(3, icon);
		pItem->setText(3, synthv1_param::paramName(index));
		pItem->setData(3, Qt::UserRole, int(index));
		pItem->setFlags(
			Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsSelectable);
		items.append(pItem);
	}
	QTreeWidget::addTopLevelItems(items);
	QTreeWidget::expandAll();
}


void synthv1widget_controls::saveControls ( synthv1_controls *pControls )
{
	pControls->clear();

	const int iItemCount = QTreeWidget::topLevelItemCount();
	for (int iItem = 0 ; iItem < iItemCount; ++iItem) {
		QTreeWidgetItem *pItem = QTreeWidget::topLevelItem(iItem);
		const unsigned short channel
			= pItem->text(0).toInt() - 1;
		const unsigned char ctype
			= synthv1_controls::typeFromText(pItem->text(1));
		synthv1_controls::Key key;
		key.status = ctype | (channel & 0x0f);
		key.param = pItem->text(2).toInt();
		pControls->add_control(key, pItem->data(3, Qt::UserRole).toInt());
	}
}


// slots.
void synthv1widget_controls::addControlItem (void)
{
	QTreeWidget::setFocus();

	QTreeWidgetItem *pItem = newControlItem();
	if (pItem) {
		QTreeWidget::setCurrentItem(pItem);
		QTreeWidget::editItem(pItem, 0);
	}
}


// factory methods.
QTreeWidgetItem *synthv1widget_controls::newControlItem (void)
{
	QTreeWidgetItem *pItem = new QTreeWidgetItem();
	const QIcon icon(":/images/synthv1_preset.png");
	pItem->setIcon(0, icon);
	pItem->setText(0, QString::number(1));
	pItem->setText(1, synthv1_controls::textFromType(synthv1_controls::CC));
	pItem->setText(2, QString::number(0));
	pItem->setIcon(3, icon);
	pItem->setText(3, synthv1_param::paramName(synthv1::ParamIndex(0)));
	pItem->setData(3, Qt::UserRole, 0);
	pItem->setFlags(
		Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsSelectable);
	QTreeWidget::addTopLevelItem(pItem);

	return pItem;
}


// end of synthv1widget_controls.cpp
