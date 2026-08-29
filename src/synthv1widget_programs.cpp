// synthv1widget_programs.cpp
//
/****************************************************************************
   Copyright (C) 2012-2026, rncbc aka Rui Nuno Capela. All rights reserved.

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

#include "synthv1widget_programs.h"

#include "synthv1_programs.h"
#include "synthv1_presets.h"
#include "synthv1_config.h"

#include "synthv1widget_presets.h"

#include <QStyledItemDelegate>
#include <QHeaderView>
#include <QSpinBox>
#include <QLineEdit>


//----------------------------------------------------------------------------
// synthv1widget_programs::ItemDelegate -- Custom (tree) list item delegate.

class synthv1widget_programs::ItemDelegate : public QStyledItemDelegate
{
public:

	// ctor.
	ItemDelegate(QObject *pParent = nullptr);

	// painting
	void paint(QPainter *painter,
		const QStyleOptionViewItem &option,
		const QModelIndex &index) const;

	QSize sizeHint(
		const QStyleOptionViewItem& option,
		const QModelIndex& index) const;

	// editing
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
// synthv1widget_programs_item_delegate -- Custom (tree) list item delegate.

// ctor.
synthv1widget_programs::ItemDelegate::ItemDelegate ( QObject *pParent )
	: QStyledItemDelegate(pParent)
{
}

// painting
//
void synthv1widget_programs::ItemDelegate::paint ( QPainter *pPainter,
	const QStyleOptionViewItem& option, const QModelIndex& index ) const
{
	QStyleOptionViewItem opt = option;

	synthv1widget_programs *pWidget
		= qobject_cast<synthv1widget_programs *>(parent());
	if (pWidget) {
		QTreeWidgetItem *pItem = pWidget->itemFromIndex(index);
		if (pItem && pItem->parent() == nullptr) {
			if (index.column() == 1)
				opt.font.setWeight(QFont::Bold);
			if (pWidget->currentItem() == pItem) {
				opt.palette.setColor(QPalette::Text,
					opt.palette.color(QPalette::HighlightedText));
			}
		}
	}

	QStyledItemDelegate::paint(pPainter, opt, index);
}


QSize synthv1widget_programs::ItemDelegate::sizeHint (
	const QStyleOptionViewItem& option, const QModelIndex& index ) const
{
	return QStyledItemDelegate::sizeHint(option, index) + QSize(4, 4);
}


// editing
//
QWidget *synthv1widget_programs::ItemDelegate::createEditor ( QWidget *pParent,
	const QStyleOptionViewItem& /*option*/, const QModelIndex& index ) const
{
	QWidget *pEditor = nullptr;

	switch (index.column()) {
	case 0: // Data.
	{
		QSpinBox *pSpinBox = new QSpinBox(pParent);
		pSpinBox->setMinimum(0);
		if (index.parent().isValid())
			pSpinBox->setMaximum(127);
		else
			pSpinBox->setMaximum(16383);
		pEditor = pSpinBox;
		break;
	}

	case 1: // Text.
	{
		if (index.parent().isValid()) {
			synthv1widget_presets::ComboBox *pComboBox
				= new synthv1widget_presets::ComboBox(pParent);
			pComboBox->setEditable(false);
			synthv1_config *pConfig = synthv1_config::getInstance();
			if (pConfig)
				(pComboBox->presetsView())->loadPresets(&(pConfig->presets));
			pEditor = pComboBox;
		} else {
			QLineEdit *pLineEdit = new QLineEdit(pParent);
			pEditor = pLineEdit;
		}
		break;
	}

	default:
		break;
	}

#ifdef CONFIG_DEBUG_0
	qDebug("synthv1widget_programs::ItemDelegate::createEditor(%p, %d, %d) = %p",
		pParent, index.row(), index.column(), pEditor);
#endif

	return pEditor;
}


void synthv1widget_programs::ItemDelegate::setEditorData (
	QWidget *pEditor, const QModelIndex& index ) const
{
#ifdef CONFIG_DEBUG_0
	qDebug("synthv1widget_programs::ItemDelegate::setEditorData(%p, %d, %d)",
		pEditor, index.row(), index.column());
#endif

	switch (index.column()) {
	case 0: // Data.
	{
		const QString& sData = index.data().toString();
		//	= index.model()->data(index, Qt::DisplayRole).toString();
		QSpinBox *pSpinBox = qobject_cast<QSpinBox *> (pEditor);
		if (pSpinBox) {
			const int iData = sData.section("=", 0, 0).toInt();
			pSpinBox->setValue(iData);
		}
		break;
	}

	case 1: // Text.
	{
		const QString& sText = index.data().toString();
		//	= index.model()->data(index, Qt::DisplayRole).toString();
		if (index.parent().isValid()) {
			synthv1widget_presets::ComboBox *pComboBox
				= qobject_cast<synthv1widget_presets::ComboBox *> (pEditor);
			if (pComboBox) {
				pComboBox->setCurrentPreset(sText);
			}
		} else {
			QLineEdit *pLineEdit = qobject_cast<QLineEdit *> (pEditor);
			if (pLineEdit) pLineEdit->setText(sText);
		}
		break;
	}

	default:
		break;
	}
}


void synthv1widget_programs::ItemDelegate::setModelData ( QWidget *pEditor,
	QAbstractItemModel *pModel,	const QModelIndex& index ) const
{
#ifdef CONFIG_DEBUG_0
	qDebug("synthv1widget_programs::ItemDelegate::setModelData(%p, %d, %d)",
		pEditor, index.row(), index.column());
#endif

	switch (index.column()) {
	case 0: // Data.
	{
		QSpinBox *pSpinBox = qobject_cast<QSpinBox *> (pEditor);
		if (pSpinBox) {
			const int iData = pSpinBox->value();
			QString sData = QString::number(iData);
			if (index.parent().isValid())
				sData += " =";
			pModel->setData(index, sData);
		}
		break;
	}

	case 1: // Text.
	{
		if (index.parent().isValid()) {
			synthv1widget_presets::ComboBox *pComboBox
				= qobject_cast<synthv1widget_presets::ComboBox *> (pEditor);
			if (pComboBox) {
				const QString& sText = pComboBox->currentPreset();
				pModel->setData(index, sText);
			}
		} else {
			QLineEdit *pLineEdit = qobject_cast<QLineEdit *> (pEditor);
			if (pLineEdit) {
				const QString& sText = pLineEdit->text();
				pModel->setData(index, sText);
			}
		}
		break;
	}

	default:
		break;
	}

	// Done.
}


//----------------------------------------------------------------------------
// synthv1widget_programs -- UI wrapper form.

// ctor.
synthv1widget_programs::synthv1widget_programs ( QWidget *pParent )
	: QTreeWidget(pParent)
{
	QTreeWidget::setColumnCount(2);

	QTreeWidget::setRootIsDecorated(true);
	QTreeWidget::setAlternatingRowColors(true);
	QTreeWidget::setUniformRowHeights(true);
	QTreeWidget::setAllColumnsShowFocus(true);

	QTreeWidget::setSelectionBehavior(QAbstractItemView::SelectRows);
	QTreeWidget::setSelectionMode(QAbstractItemView::SingleSelection);

	QHeaderView *pHeaderView = QTreeWidget::header();
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
	pHeaderView->setResizeMode(QHeaderView::ResizeToContents);
#else
	pHeaderView->setSectionResizeMode(QHeaderView::ResizeToContents);
#endif
	pHeaderView->hide();

	QTreeWidget::setItemDelegate(new ItemDelegate(this));

	QObject::connect(this,
		SIGNAL(itemChanged(QTreeWidgetItem *, int)),
		SLOT(itemChangedSlot(QTreeWidgetItem *, int)));

	QObject::connect(this,
		SIGNAL(itemExpanded(QTreeWidgetItem *)),
		SLOT(itemExpandedSlot(QTreeWidgetItem *)));
	QObject::connect(this,
		SIGNAL(itemCollapsed(QTreeWidgetItem *)),
		SLOT(itemCollapsedSlot(QTreeWidgetItem *)));
}


// dtor.
synthv1widget_programs::~synthv1widget_programs (void)
{
}


// utilities.
void synthv1widget_programs::loadPrograms ( synthv1_programs *pPrograms )
{
	QTreeWidget::clear();

	QList<QTreeWidgetItem *> items;
	QTreeWidgetItem *pCurrentItem = nullptr;
	const synthv1_programs::Banks& banks = pPrograms->banks();
	synthv1_programs::Banks::ConstIterator bank_iter = banks.constBegin();
	const synthv1_programs::Banks::ConstIterator& bank_end = banks.constEnd();
	for ( ; bank_iter != bank_end; ++bank_iter) {
		synthv1_programs::Bank *pBank = bank_iter.value();
		QTreeWidgetItem *pBankItem = new QTreeWidgetItem(this);
		pBankItem->setIcon(0, QIcon(":/images/presetBankOpen.png"));
		pBankItem->setText(0, QString::number(pBank->id()));
		pBankItem->setText(1, pBank->name());
		pBankItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsEditable);
		pBankItem->setData(0, Qt::UserRole, pBank->id());
		const synthv1_programs::Progs& progs = pBank->progs();
		synthv1_programs::Progs::ConstIterator prog_iter = progs.constBegin();
		const synthv1_programs::Progs::ConstIterator& prog_end = progs.constEnd();
		for ( ; prog_iter != prog_end; ++prog_iter) {
			synthv1_programs::Prog *pProg = prog_iter.value();
			QTreeWidgetItem *pProgItem = new QTreeWidgetItem(pBankItem);
			pProgItem->setIcon(1, QIcon(":/images/synthv1_preset.png"));
			pProgItem->setText(0, QString::number(pProg->id()) + " =");
			pProgItem->setText(1, pProg->name());
			pProgItem->setFlags(
				Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsSelectable);
			pProgItem->setData(0, Qt::TextAlignmentRole,
				int(Qt::AlignRight | Qt::AlignVCenter));
			pProgItem->setData(0, Qt::UserRole, pProg->id());
			if (pBank == pPrograms->current_bank() &&
				pProg == pPrograms->current_prog())
				pCurrentItem = pProgItem;
		}
		items.append(pBankItem);
	}
	QTreeWidget::addTopLevelItems(items);
	QTreeWidget::expandAll();

	QTreeWidget::setCurrentItem(pCurrentItem);
}


void synthv1widget_programs::savePrograms ( synthv1_programs *pPrograms )
{
	pPrograms->clear_banks();

	const int iBankCount = QTreeWidget::topLevelItemCount();
	for (int iBank = 0 ; iBank < iBankCount; ++iBank) {
		QTreeWidgetItem *pBankItem = QTreeWidget::topLevelItem(iBank);
		uint16_t bank_id = pBankItem->data(0, Qt::UserRole).toInt();
		const QString& bank_name = pBankItem->text(1).simplified();
		synthv1_programs::Bank *pBank = pPrograms->add_bank(bank_id, bank_name);
		const int iProgCount = pBankItem->childCount();
		for (int iProg = 0 ; iProg < iProgCount; ++iProg) {
			QTreeWidgetItem *pProgItem = pBankItem->child(iProg);
			uint16_t prog_id = pProgItem->data(0, Qt::UserRole).toInt();
			const QString& prog_name = pProgItem->text(1).simplified();
			pBank->add_prog(prog_id, prog_name);
		}
	}
}


QString synthv1widget_programs::currentProgramName (void) const
{
	QString sProgramName;

	const QList<QTreeWidgetItem *>& selectedItems
		= QTreeWidget::selectedItems();
	if (!selectedItems.isEmpty()) {
		QTreeWidgetItem *pProgItem = selectedItems.first();
		sProgramName = pProgItem->text(1).simplified();
	}

	return sProgramName;
}


// slots.
void synthv1widget_programs::addBankItem (void)
{
	QTreeWidget::setFocus();

	QTreeWidgetItem *pBankItem = newBankItem();
	if (pBankItem) {
		QTreeWidget::setCurrentItem(pBankItem);
		QTreeWidget::editItem(pBankItem, 1);
	}
}


void synthv1widget_programs::addProgramItem (void)
{
	QTreeWidget::setFocus();

	QTreeWidgetItem *pProgItem = newProgramItem();
	if (pProgItem) {
		QTreeWidget::setCurrentItem(pProgItem);
		QTreeWidget::editItem(pProgItem, 1);
	}
}


void synthv1widget_programs::addPresetItems ( synthv1_presets *pPresets )
{

	QStringList preset_list = pPresets->preset_list();
	QStringListIterator bank_iter(pPresets->bank_list());
	while (bank_iter.hasNext()) {
		const QString& sBank = bank_iter.next();
		synthv1_presets::Bank *pBank = pPresets->find_bank(sBank);
		if (pBank)
			preset_list.append(pBank->preset_list());
	}

	// Skip presets already mapped to programs...
	int iBankData = 0;
	QTreeWidgetItem *pBank0Item = nullptr;
	const int iBankCount = QTreeWidget::topLevelItemCount();
	for (int iBank = 0; iBank < iBankCount; ++iBank) {
		QTreeWidgetItem *pBankItem = QTreeWidget::topLevelItem(iBank);
		const int iData = pBankItem->data(0, Qt::UserRole).toInt();
		if (iData == 0)
			pBank0Item = pBankItem;
		if (iBankData < iData)
			iBankData = iData;
		if (iBankData >= 16383)
			return;
		const int iProgCount = pBankItem->childCount();
		for (int iProg = 0; iProg < iProgCount; ++iProg) {
			QTreeWidgetItem *pProgItem = pBankItem->child(iProg);
			preset_list.removeAll(pProgItem->text(1));
		}
	}

	if (preset_list.isEmpty())
		return;

	QStringListIterator preset_iter(preset_list);
	while (preset_iter.hasNext()) {
		const QString& sPreset = preset_iter.next();
		synthv1_presets::Bank *pBank = pPresets->find_preset_bank(sPreset);
		QTreeWidgetItem *pBankItem = nullptr;
		if (pBank) {
			const QString& sBank = pBank->name();
			const QList<QTreeWidgetItem *>& items
				= QTreeWidget::findItems(
					sBank, Qt::MatchExactly|Qt::MatchRecursive, 1);
			QListIterator<QTreeWidgetItem *> iter(items);
			while (iter.hasNext()) {
				QTreeWidgetItem *pItem = iter.next();
				if (pItem->parent() == nullptr) {
					pBankItem = pItem;
					break;
				}
			}
			if (pBankItem == nullptr) {
				pBankItem = new QTreeWidgetItem(QStringList()
					<< QString::number(++iBankData) << sBank);
				pBankItem->setIcon(0, QIcon(":/images/presetBank.png"));
				pBankItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsEditable);
				pBankItem->setData(0, Qt::UserRole, iBankData);
				QTreeWidget::addTopLevelItem(pBankItem);
			}
		}
		else
		// Check for default bank...
		if (pBank0Item == nullptr) {
			pBank0Item = new QTreeWidgetItem(QStringList()
				<< QString::number(0) << tr("Bank 0"));
			pBank0Item->setIcon(0, QIcon(":/images/presetBank.png"));
			pBank0Item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsEditable);
			pBank0Item->setData(0, Qt::UserRole, 0);
			QTreeWidget::insertTopLevelItem(0, pBank0Item);
		}
		if (pBankItem == nullptr)
			pBankItem = pBank0Item;
		// Add program, filling in the gaps...
		int iProg = 0;
		int iProgData = 0;
		const int iProgCount = pBankItem->childCount();
		for ( ; iProg < iProgCount; ++iProg) {
			QTreeWidgetItem *pProgItem = pBankItem->child(iProg);
			const int iData = pProgItem->data(0, Qt::UserRole).toInt();
			if (iData != iProg) {
				iProgData = iProg - 1;
				break;
			}
			iProgData = iData;
		}
		if (iProgCount > 0)
			++iProgData;
		if (iProgData >= 128)
			continue;
		QTreeWidgetItem *pProgItem = new QTreeWidgetItem(QStringList()
			<< QString::number(iProgData) + " =" << sPreset);
		pProgItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsSelectable);
		pProgItem->setData(0, Qt::TextAlignmentRole, int(Qt::AlignRight | Qt::AlignVCenter));
		pProgItem->setData(0, Qt::UserRole, iProgData);
		pProgItem->setIcon(1, QIcon(":/images/synthv1_preset.png"));
		pBankItem->insertChild(iProg, pProgItem);
		pBankItem->setExpanded(true);
	}
}


// factory methods.
QTreeWidgetItem *synthv1widget_programs::newBankItem (void)
{
	QTreeWidgetItem *pItem = QTreeWidget::currentItem();
	QTreeWidgetItem *pBankItem = (pItem ? pItem->parent() : nullptr);
	if (pBankItem == nullptr)
		pBankItem = pItem;

	int iBank = 0;
	int iBankData = 0;
	if (pBankItem) {
		iBankData = pBankItem->data(0, Qt::UserRole).toInt() + 1;
		if (iBankData > 16383)
			iBankData = 0;
		else
			iBank = QTreeWidget::indexOfTopLevelItem(pBankItem) + 1;
	}

	const int iBankCount = QTreeWidget::topLevelItemCount();
	for ( ; iBank < iBankCount; ++iBank) {
		pBankItem = QTreeWidget::topLevelItem(iBank);
		if (iBankData < pBankItem->data(0, Qt::UserRole).toInt())
			break;
		if (++iBankData > 16383)
			return nullptr;
	}

	pBankItem = new QTreeWidgetItem(QStringList()
		<< QString::number(iBankData) << tr("Bank %1").arg(iBankData));
	pBankItem->setIcon(0, QIcon(":/images/presetBank.png"));
	pBankItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsEditable);
	pBankItem->setData(0, Qt::UserRole, iBankData);
	QTreeWidget::insertTopLevelItem(iBank, pBankItem);

	return pBankItem;
}


QTreeWidgetItem *synthv1widget_programs::newProgramItem (void)
{
	QTreeWidgetItem *pItem = QTreeWidget::currentItem();
	QTreeWidgetItem *pBankItem = (pItem ? pItem->parent() : nullptr);
	QTreeWidgetItem *pProgItem = nullptr;
	if (pBankItem == nullptr)
		pBankItem = pItem;
	else
		pProgItem = pItem;
	if (pBankItem == nullptr)
		pBankItem = QTreeWidget::topLevelItem(0);
	if (pBankItem == nullptr)
		pBankItem = newBankItem();
	if (pBankItem == nullptr)
		return nullptr;

	const int iBankData
		= pBankItem->data(0, Qt::UserRole).toInt();

	int iProg = 0;
	int iProgData = 0;
	if (pProgItem) {
		iProgData = pProgItem->data(0, Qt::UserRole).toInt() + 1;
		if (iProgData > 127)
			iProgData = 0;
		else
			iProg = pBankItem->indexOfChild(pProgItem) + 1;
	}

	const int iProgCount = pBankItem->childCount();
	for ( ; iProg < iProgCount; ++iProg) {
		pProgItem = pBankItem->child(iProg);
		if (iProgData < pProgItem->data(0, Qt::UserRole).toInt())
			break;
		if (++iProgData > 127)
			return nullptr;
	}

	QString sProgram = tr("Program %1.%2").arg(iBankData).arg(iProgData);
	synthv1_config *pConfig = synthv1_config::getInstance();
	if (pConfig) {
		QStringList preset_list;
		const QStringList& bank_list
			= pConfig->presets.bank_list();
		if (iBankData < bank_list.count()) {
			const QString& sBank
				= bank_list.at(iBankData);
			synthv1_presets::Bank *pBank
				= pConfig->presets.find_bank(sBank);
			if (pBank)
				preset_list = pBank->preset_list();
		} else {
			preset_list = pConfig->presets.preset_list();
		}
		if (iProgData < preset_list.count())
			sProgram = preset_list.at(iProgData);
	}

	pProgItem = new QTreeWidgetItem(QStringList()
		<< QString::number(iProgData) + " =" << sProgram);
	pProgItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsSelectable);
	pProgItem->setData(0, Qt::TextAlignmentRole, int(Qt::AlignRight | Qt::AlignVCenter));
	pProgItem->setData(0, Qt::UserRole, iProgData);
	pProgItem->setIcon(1, QIcon(":/images/synthv1_preset.png"));
	pBankItem->insertChild(iProg, pProgItem);
	pBankItem->setExpanded(true);

	return pProgItem;
}


void synthv1widget_programs::itemChangedSlot ( QTreeWidgetItem *pItem, int )
{
	const int iData = pItem->text(0).section("=", 0, 0).toInt();
	if (iData == pItem->data(0, Qt::UserRole).toInt())
		return;

	const bool bBlockSignals = QTreeWidget::blockSignals(true);

	QTreeWidgetItem *pBankItem = pItem->parent();
	if (pBankItem) {
		int iProg, iProgData = 0;
		const int iOldProg = pBankItem->indexOfChild(pItem);
		pItem = pBankItem->takeChild(iOldProg);
		const int iProgCount = pBankItem->childCount();
		for (iProg = 0; iProg < iProgCount; ++iProg) {
			QTreeWidgetItem *pProgItem = pBankItem->child(iProg);
			iProgData = pProgItem->data(0, Qt::UserRole).toInt();
			if (iProgData >= iData)
				break;
		}
		if (iProgData == iData) {
			iProg = iOldProg;
			iProgData = pItem->data(0, Qt::UserRole).toInt();
			pItem->setText(0, QString::number(iProgData) + " =");
		}
		else pItem->setData(0, Qt::UserRole, iData);
		pBankItem->insertChild(iProg, pItem);
	} else {
		int iBank, iBankData = 0;
		const bool bExpanded = pItem->isExpanded();
		const int iOldBank = QTreeWidget::indexOfTopLevelItem(pItem);
		pItem = QTreeWidget::takeTopLevelItem(iOldBank);
		const int iBankCount = QTreeWidget::topLevelItemCount();
		for (iBank = 0; iBank < iBankCount; ++iBank) {
			pBankItem = QTreeWidget::topLevelItem(iBank);
			iBankData = pBankItem->data(0, Qt::UserRole).toInt();
			if (iBankData >= iData)
				break;
		}
		if (iBankData == iData) {
			iBank = iOldBank;
			iBankData = pItem->data(0, Qt::UserRole).toInt();
			pItem->setText(0, QString::number(iBankData));
		}
		else pItem->setData(0, Qt::UserRole, iData);
		QTreeWidget::insertTopLevelItem(iBank, pItem);
		pItem->setExpanded(bExpanded);
	}
	QTreeWidget::setCurrentItem(pItem);

	QTreeWidget::blockSignals(bBlockSignals);
}


void synthv1widget_programs::itemExpandedSlot ( QTreeWidgetItem *pItem )
{
	if (pItem->parent() == nullptr)
		pItem->setIcon(0, QIcon(":/images/presetBankOpen.png"));
}


void synthv1widget_programs::itemCollapsedSlot ( QTreeWidgetItem *pItem )
{
	if (pItem->parent() == nullptr)
		pItem->setIcon(0, QIcon(":/images/presetBank.png"));
}


// end of synthv1widget_programs.cpp
