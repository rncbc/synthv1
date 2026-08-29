// synthv1widget_presets.cpp
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

#include "synthv1widget_presets.h"

#include "synthv1_presets.h"

#include <QStyledItemDelegate>
#include <QHeaderView>
#include <QLineEdit>

#include <QMessageBox>
#include <QFileDialog>
#include <QUrl>

#include <QRegularExpressionValidator>

// Drag-n-drop stuff.
//
#include <QApplication>
#include <QMouseEvent>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDragLeaveEvent>
#include <QMimeData>
#include <QDrag>
#include <QRubberBand>
#include <QTimer>


//----------------------------------------------------------------------------
// synthv1widget_presets::ItemDelegate -- Custom (tree) list item delegate.

class synthv1widget_presets::ItemDelegate : public QStyledItemDelegate
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
// synthv1widget_presets::ItemDelegate -- Custom (tree) list item delegate.

// ctor.
synthv1widget_presets::ItemDelegate::ItemDelegate (
	QObject *pParent ) : QStyledItemDelegate(pParent)
{
}


// painting
//
void synthv1widget_presets::ItemDelegate::paint ( QPainter *pPainter,
	const QStyleOptionViewItem& option, const QModelIndex& index ) const
{
	QStyleOptionViewItem opt = option;

	synthv1widget_presets *pWidget
		= qobject_cast<synthv1widget_presets *>(parent());
	if (pWidget) {
		const bool bRootIsDecorated = pWidget->rootIsDecorated();
		QTreeWidgetItem *pItem = pWidget->itemFromIndex(index);
		if (pItem && pWidget->isBankItem(pItem)) {
			if (!bRootIsDecorated)
				opt.state |= QStyle::State_Enabled;
			opt.font.setWeight(QFont::Bold);
			if (pWidget->currentItem() == pItem) {
				opt.palette.setColor(QPalette::Text,
					opt.palette.color(QPalette::HighlightedText));
			}
		}
		else
		if (!bRootIsDecorated)
			opt.decorationAlignment = Qt::AlignRight|Qt::AlignVCenter;
	}

	QStyledItemDelegate::paint(pPainter, opt, index);
}


QSize synthv1widget_presets::ItemDelegate::sizeHint (
	const QStyleOptionViewItem& option, const QModelIndex& index ) const
{
	return QStyledItemDelegate::sizeHint(option, index) + QSize(4, 4);
}


// editing
//
QWidget *synthv1widget_presets::ItemDelegate::createEditor ( QWidget *pParent,
	const QStyleOptionViewItem& /*option*/, const QModelIndex& index ) const
{
	QWidget *pEditor = nullptr;

	switch (index.column()) {
	case 0: // Text.
	{
		QLineEdit *pLineEdit = new QLineEdit(pParent);
		synthv1widget_presets *pWidget
			= qobject_cast<synthv1widget_presets *>(parent());
		if (pWidget)
			pLineEdit->setValidator(pWidget->presetsValidator());
		pEditor = pLineEdit;
		break;
	}

	default:
		break;
	}

#ifdef CONFIG_DEBUG
	qDebug("synthv1widget_presets::ItemDelegate::createEditor(%p, %d, %d) = %p",
		pParent, index.row(), index.column(), pEditor);
#endif

	return pEditor;
}


// editing
//
void synthv1widget_presets::ItemDelegate::setEditorData (
	QWidget *pEditor, const QModelIndex& index ) const
{
#ifdef CONFIG_DEBUG
	qDebug("synthv1widget_presets::ItemDelegate::setEditorData(%p, %d, %d)",
		pEditor, index.row(), index.column());
#endif

	switch (index.column()) {
	case 0: // Text.
	{
		const QString& sText = index.data().toString();
		//	= index.model()->data(index, Qt::DisplayRole).toString();
		QLineEdit *pLineEdit = qobject_cast<QLineEdit *> (pEditor);
		if (pLineEdit)
			pLineEdit->setText(sText.simplified());
		break;
	}

	default:
		break;
	}
}


void synthv1widget_presets::ItemDelegate::setModelData ( QWidget *pEditor,
	QAbstractItemModel *pModel,	const QModelIndex& index ) const
{
#ifdef CONFIG_DEBUG
	qDebug("synthv1widget_presets::ItemDelegate::setModelData(%p, %d, %d)",
		pEditor, index.row(), index.column());
#endif

	switch (index.column()) {
	case 0: // Text.
	{
		QLineEdit *pLineEdit = qobject_cast<QLineEdit *> (pEditor);
		if (pLineEdit) {
			QString sText = pLineEdit->text().simplified();
			if (!sText.isEmpty()) {
				QTreeWidgetItem *pItem = nullptr;
				synthv1widget_presets *pWidget
					= qobject_cast<synthv1widget_presets *>(parent());
				if (pWidget) {
					pItem = pWidget->itemFromIndex(index);
					if (pItem && sText != pItem->text(index.column()))
						sText = pWidget->nameRenum(sText, pItem->type());
				}
				pModel->setData(index, sText);
				if (pWidget && pItem) {
					pWidget->setDirtyPresets(true);
					emit pWidget->itemActivated(pItem, index.column());
				}
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
// synthv1widget_presets -- UI wrapper form.

// ctor.
synthv1widget_presets::synthv1widget_presets ( QWidget *pParent )
	: QTreeWidget(pParent)
{
	QTreeWidget::setColumnCount(1);

	QTreeWidget::setAlternatingRowColors(true);
	QTreeWidget::setUniformRowHeights(true);
	QTreeWidget::setAllColumnsShowFocus(true);

	QTreeWidget::setSelectionBehavior(QAbstractItemView::SelectRows);
	QTreeWidget::setSelectionMode(QAbstractItemView::SingleSelection);

	QHeaderView *pHeaderView = QTreeWidget::header();
	pHeaderView->setSectionResizeMode(QHeaderView::ResizeToContents);
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

	// config default.
	m_bDontUseNativeDialogs = false;

	// common validator.
	m_pPresetsValidator
		= new QRegularExpressionValidator(
			QRegularExpression("[^/]+"));

	// start clean.
	m_iDirtyPresets = 0;

	// Drag-n-drop stuff.
	//
	m_pDragItem = nullptr;
	m_pDropItem = nullptr;
	m_pRubberBand = nullptr;

	QTreeWidget::setAcceptDrops(true);
	QTreeWidget::setAutoScroll(true);

	m_pAutoExpandTimer = new QTimer(this);
	QObject::connect(m_pAutoExpandTimer,
		SIGNAL(timeout()),
		SLOT(autoExpandTimeoutSlot()));
}


// dtor.
synthv1widget_presets::~synthv1widget_presets (void)
{
	delete m_pAutoExpandTimer;
	delete m_pPresetsValidator;
}


// utilities.
void synthv1widget_presets::loadPresets ( synthv1_presets *pPresets )
{
	const bool bRootIsDecorated
		= QTreeWidget::rootIsDecorated();

	QTreeWidget::clear();

	QList<QTreeWidgetItem *> items;

	QStringListIterator preset_iter(pPresets->preset_list());
	while (preset_iter.hasNext()) {
		const QString& sPreset = preset_iter.next();
		synthv1_presets::Preset *pPreset = pPresets->find_preset(sPreset);
		if (pPreset == nullptr)
			continue;
		QTreeWidgetItem *pPresetItem = new QTreeWidgetItem(this, PresetItem);
		pPresetItem->setFlags(
			Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsSelectable);
		pPresetItem->setIcon(0, QIcon(":/images/synthv1_preset.png"));
		pPresetItem->setText(0, pPreset->name());
		pPresetItem->setData(0, Qt::UserRole, pPreset->file());
		items.append(pPresetItem);
	}

	int iBankData = 0;
	QStringListIterator bank_iter(pPresets->bank_list());
	while (bank_iter.hasNext()) {
		const QString& sBank = bank_iter.next();
		synthv1_presets::Bank *pBank = pPresets->find_bank(sBank);
		if (pBank == nullptr)
			continue;
		QTreeWidgetItem *pBankItem = new QTreeWidgetItem(this, BankItem);
		if (bRootIsDecorated) {
			pBankItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsEditable);
		} else {
			pBankItem->setFlags(Qt::NoItemFlags);
		}
		pBankItem->setIcon(0, QIcon(":/images/presetBankOpen.png"));
		pBankItem->setText(0, pBank->name());
		pBankItem->setData(0, Qt::UserRole, iBankData++);
		QStringListIterator bank_preset_iter(pBank->preset_list());
		while (bank_preset_iter.hasNext()) {
			const QString& sPreset = bank_preset_iter.next();
			QTreeWidgetItem *pPresetItem = bRootIsDecorated
				? new QTreeWidgetItem(pBankItem, PresetItem)
				: new QTreeWidgetItem(this, PresetItem);
			pPresetItem->setFlags(
				Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsSelectable);
			pPresetItem->setIcon(0, QIcon(":/images/synthv1_preset.png"));
			pPresetItem->setText(0, sPreset);
			synthv1_presets::Preset *pPreset = pPresets->find_preset(sPreset);
			QString sPresetFile;
			if (pPreset)
				sPresetFile = pPreset->file();
			pPresetItem->setData(0, Qt::UserRole, sPresetFile);
		}
		items.append(pBankItem);
	}

	QTreeWidget::addTopLevelItems(items);
	QTreeWidget::expandAll();

	setDirtyPresets(false);
}


void synthv1widget_presets::savePresets ( synthv1_presets *pPresets )
{
	const bool bRootIsDecorated
		= QTreeWidget::rootIsDecorated();

	pPresets->clear_banks();
	pPresets->clear_presets();

	const int iItemCount = QTreeWidget::topLevelItemCount();
	for (int iItem = 0 ; iItem < iItemCount; ++iItem) {
		QTreeWidgetItem *pItem = QTreeWidget::topLevelItem(iItem);
		if (isBankItem(pItem)) {
			QTreeWidgetItem *pBankItem = pItem;
			const QString& sBank
				= pBankItem->text(0).simplified();
		//	const int iBankData = pBankItem->data(0, Qt::UserRole).toInt();
			synthv1_presets::Bank *pBank = pPresets->add_bank(sBank);
			if (bRootIsDecorated) {
				const int iChildCount = pBankItem->childCount();
				for (int iChild = 0 ; iChild < iChildCount; ++iChild) {
					QTreeWidgetItem *pPresetItem = pBankItem->child(iChild);
					const QString& sPreset
						= pPresetItem->text(0).simplified();
					pBank->add_preset(sPreset);
					synthv1_presets::Preset *pPreset
						= pPresets->add_preset(sPreset);
					if (pPreset) {
						const QString& sPresetFile
							= pPresetItem->data(0, Qt::UserRole).toString();
						pPreset->set_file(sPresetFile);
					}
				}
			}
		}
		else
		if (isPresetItem(pItem)) {
			QTreeWidgetItem *pPresetItem = pItem;
			const QString& sPreset
				= pPresetItem->text(0).simplified();
			synthv1_presets::Preset *pPreset
				= pPresets->add_preset(sPreset);
			if (pPreset) {
				const QString& sPresetFile
					= pPresetItem->data(0, Qt::UserRole).toString();
				pPreset->set_file(sPresetFile);
			}
		}
	}
}


QString synthv1widget_presets::currentPreset (void) const
{
	QString sPreset;

	const QList<QTreeWidgetItem *>& selectedItems
		= QTreeWidget::selectedItems();
	if (!selectedItems.isEmpty()) {
		QTreeWidgetItem *pPresetItem = selectedItems.first();
		sPreset = pPresetItem->text(0).simplified();
	}

	return sPreset;
}


// slots.
void synthv1widget_presets::addBankItem (void)
{
	QTreeWidget::setFocus();

	QTreeWidgetItem *pBankItem = newBankItem();
	if (pBankItem) {
		QTreeWidget::setCurrentItem(pBankItem);
		QTreeWidget::editItem(pBankItem, 0);
	}
}


void synthv1widget_presets::addPresetItem (void)
{
	QTreeWidget::setFocus();

	QTreeWidgetItem *pPresetItem = newPresetItem();
	if (pPresetItem) {
		QTreeWidget::setCurrentItem(pPresetItem);
	//	QTreeWidget::editItem(pPresetItem, 0);
	}
}


// factory methods.
QTreeWidgetItem *synthv1widget_presets::newBankItem (void)
{
	QTreeWidgetItem *pBankItem;

 	int iBank = 0;
 	int iItem = 0;

	const int iItemCount
		= QTreeWidget::topLevelItemCount();
 	for ( ; iItem < iItemCount; ++iItem) {
		pBankItem = QTreeWidget::topLevelItem(iItem);
		if (isBankItem(pBankItem))
			++iBank;
 	}

 	const QString& sBank
 		= bankRenum(tr("Bank %1").arg(iBank + 1));

 	iItem = -1;
	pBankItem = QTreeWidget::currentItem();
	if (pBankItem && pBankItem->parent())
		pBankItem = pBankItem->parent();
	if (pBankItem && isBankItem(pBankItem))
		iItem = QTreeWidget::indexOfTopLevelItem(pBankItem);

	pBankItem = new QTreeWidgetItem(QStringList() << sBank, BankItem);
 	pBankItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsEditable);
 	pBankItem->setIcon(0, QIcon(":/images/presetBank.png"));
 	pBankItem->setData(0, Qt::UserRole, iBank + 1);
	if (iItem >= 0 && iItem < iItemCount)
 		QTreeWidget::insertTopLevelItem(iItem + 1, pBankItem);
 	else
 		QTreeWidget::addTopLevelItem(pBankItem);

	setDirtyPresets(true);

	return pBankItem;
}


QTreeWidgetItem *synthv1widget_presets::newPresetItem (void)
{
	const QStringList& files = openPresetFiles();
	if (files.isEmpty())
		return nullptr;

	int iItem = 0;
	QTreeWidgetItem *pCurrentItem = QTreeWidget::currentItem();
	QTreeWidgetItem *pBankItem = nullptr;
	if (pCurrentItem) {
		if (isBankItem(pCurrentItem))
			pBankItem = pCurrentItem;
		else
			pBankItem = pCurrentItem->parent();
		if (pBankItem)
			iItem = pBankItem->indexOfChild(pCurrentItem) + 1;
		else
			iItem = indexOfTopLevelItem(pCurrentItem) + 1;
	}

	QTreeWidgetItem *pPresetItem = nullptr;

	QStringListIterator iter(files);
	while (iter.hasNext()) {
		const QString& sPresetFile = iter.next();
		if (QFileInfo::exists(sPresetFile)) {
			const QFileInfo fi(sPresetFile);
			const QString& sPreset
				= presetRenum(fi.completeBaseName());
			pPresetItem = new QTreeWidgetItem(QStringList() << sPreset, PresetItem);
			pPresetItem->setFlags(
				Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsSelectable);
			pPresetItem->setIcon(0, QIcon(":/images/synthv1_preset.png"));
			pPresetItem->setData(0, Qt::UserRole, sPresetFile);
			if (pBankItem)
				pBankItem->insertChild(iItem++, pPresetItem);
			else
				QTreeWidget::insertTopLevelItem(iItem++, pPresetItem);
		}
	}

	if (pBankItem)
		pBankItem->setExpanded(true);

	setDirtyPresets(true);

	return pPresetItem;
}


// file prompts.
//
QStringList synthv1widget_presets::openPresetFiles (void)
{
	QStringList files;

	const QString& sTitle  = tr("Open Preset");
	const QString& sFilter = tr("Preset files (*.%1)").arg(m_sPresetExt);

	QWidget *pParentWidget = nullptr;
	QFileDialog::Options options;
	if (m_bDontUseNativeDialogs) {
		options |= QFileDialog::DontUseNativeDialog;
		pParentWidget = QWidget::window();
	}
#if 1//QT_VERSION < QT_VERSION_CHECK(4, 4, 0)
	files = QFileDialog::getOpenFileNames(pParentWidget,
		sTitle, m_sPresetDir, sFilter, nullptr, options);
#else
	QFileDialog fileDialog(pParentWidget,
		sTitle, m_sPresetDir, sFilter);
	fileDialog.setAcceptMode(QFileDialog::AcceptOpen);
	fileDialog.setFileMode(QFileDialog::ExistingFiles);
	fileDialog.setDefaultSuffix(m_sPresetExt);
	QList<QUrl> urls(fileDialog.sidebarUrls());
	urls.append(QUrl::fromLocalFile(m_sPresetDir));
	fileDialog.setSidebarUrls(urls);
	fileDialog.setOptions(options);
	if (fileDialog.exec())
		files = fileDialog.selectedFiles();
#endif

	return files;
}


QString synthv1widget_presets::savePresetFile ( const QString& sPreset )
{
	const QFileInfo fi(QDir(m_sPresetDir), sPreset + '.' + m_sPresetExt);
	QString sPresetFile = fi.absoluteFilePath();
	if (fi.exists()
		&& QMessageBox::warning(QWidget::window(),
			tr("Warning"),
			tr("About to replace preset:\n\n"
			"\"%1\"\n\n"
			"Are you sure?")
			.arg(sPreset),
			QMessageBox::Ok | QMessageBox::Cancel)
			== QMessageBox::Cancel) {
		sPresetFile.clear();
		return sPresetFile;
	}

	const QString& sTitle  = tr("Save Preset");
	const QString& sFilter = tr("Preset files (*.%1)").arg(m_sPresetExt);

	QWidget *pParentWidget = nullptr;
	QFileDialog::Options options;
	if (m_bDontUseNativeDialogs) {
		options |= QFileDialog::DontUseNativeDialog;
		pParentWidget = QWidget::window();
	}
#if 1//QT_VERSION < QT_VERSION_CHECK(4, 4, 0)
	sPresetFile = QFileDialog::getSaveFileName(pParentWidget,
		sTitle, sPresetFile, sFilter, nullptr, options);
#else
	QFileDialog fileDialog(pParentWidget,
		sTitle, sPresetFile, sFilter);
	fileDialog.setAcceptMode(QFileDialog::AcceptSave);
	fileDialog.setFileMode(QFileDialog::AnyFile);
	fileDialog.setDefaultSuffix(m_sPresetExt);
	QList<QUrl> urls(fileDialog.sidebarUrls());
	urls.append(QUrl::fromLocalFile(m_sPresetDir));
	fileDialog.setSidebarUrls(urls);
	fileDialog.setOptions(options);
	if (fileDialog.exec())
		sPresetFile = fileDialog.selectedFiles().first();
#endif

	if (!sPresetFile.isEmpty()
		&& QFileInfo(sPresetFile).suffix() != m_sPresetExt) {
		sPresetFile += '.' + m_sPresetExt;
	}

	return sPresetFile;
}


// config stuff.
//
void synthv1widget_presets::setPresetDir ( const QString& sPresetDir )
{
	m_sPresetDir = sPresetDir;
}

const QString& synthv1widget_presets::presetDir (void) const
{
	return m_sPresetDir;
}


void synthv1widget_presets::setPresetExt ( const QString& sPresetExt )
{
	m_sPresetExt = sPresetExt;
}

const QString& synthv1widget_presets::presetExt (void) const
{
	return m_sPresetExt;
}


void synthv1widget_presets::setDontUseNativeDialogs ( bool bDontUseNativeDialogs )
{
	m_bDontUseNativeDialogs = bDontUseNativeDialogs;
}

bool synthv1widget_presets::dontUseNativeDialogs (void) const
{
	return m_bDontUseNativeDialogs;
}


// common validator accessor.
QValidator *synthv1widget_presets::presetsValidator (void) const
{
	return m_pPresetsValidator;
}


// dirty flag.
void synthv1widget_presets::setDirtyPresets ( bool bDirtyPresets )
{
	if (bDirtyPresets)
		++m_iDirtyPresets;
	else
		m_iDirtyPresets = 0;
}

bool synthv1widget_presets::isDirtyPresets (void) const
{
	return (m_iDirtyPresets > 0);
}


// private slots.
//
void synthv1widget_presets::itemChangedSlot ( QTreeWidgetItem *pItem, int )
{
	const bool bBlockSignals
		= QTreeWidget::blockSignals(true);
	QTreeWidget::setCurrentItem(pItem);
	QTreeWidget::blockSignals(bBlockSignals);
}


void synthv1widget_presets::itemExpandedSlot ( QTreeWidgetItem *pItem )
{
	if (QTreeWidget::rootIsDecorated() && isBankItem(pItem))
		pItem->setIcon(0, QIcon(":/images/presetBankOpen.png"));
}

void synthv1widget_presets::itemCollapsedSlot ( QTreeWidgetItem *pItem )
{
	if (QTreeWidget::rootIsDecorated() && isBankItem(pItem))
		pItem->setIcon(0, QIcon(":/images/presetBank.png"));
}


void synthv1widget_presets::setPresetItem ( const QString& sPreset )
{
	QTreeWidgetItem *pPresetItem = presetItem(sPreset);
	if (pPresetItem) {
		QTreeWidgetItem *pBankItem = pPresetItem->parent();
		if (pBankItem && !pBankItem->isExpanded())
			pBankItem->setExpanded(true);
	}
	QTreeWidget::setCurrentItem(pPresetItem);
}

QTreeWidgetItem *synthv1widget_presets::presetItem ( const QString& sPreset ) const
{
	return nameItem(sPreset, PresetItem);
}

bool synthv1widget_presets::isPresetItem ( QTreeWidgetItem *pItem ) const
{
	return (pItem->type() == PresetItem);
}


void synthv1widget_presets::setBankItem ( const QString& sBank, int iPreset )
{
	QTreeWidgetItem *pBankItem = bankItem(sBank);
	if (pBankItem) {
		if (!pBankItem->isExpanded())
			pBankItem->setExpanded(true);
		const int iChildCount = pBankItem->childCount();
		if (iPreset >= 0 && iPreset < iChildCount)
			setPresetItem((pBankItem->child(iPreset))->text(0));
	}
}

QTreeWidgetItem *synthv1widget_presets::bankItem ( const QString& sBank ) const
{
	return nameItem(sBank, BankItem);
}

bool synthv1widget_presets::isBankItem ( QTreeWidgetItem *pItem ) const
{
	return (pItem->type() == BankItem);
}


QTreeWidgetItem *synthv1widget_presets::nameItem (
	const QString& sName, int iType ) const
{
	const QList<QTreeWidgetItem *>& items
		= QTreeWidget::findItems(sName, Qt::MatchExactly|Qt::MatchRecursive);
	QListIterator<QTreeWidgetItem *> iter(items);
	while (iter.hasNext()) {
		QTreeWidgetItem *pItem = iter.next();
		if (pItem->type() == iType)
			return pItem;
	}

	return nullptr;
}


QString synthv1widget_presets::nameRenum (
	const QString& sName, int iType ) const
{
	QString sNameRenum = sName;
	QString sNameBase = sNameRenum;
	sNameBase.remove(QRegularExpression(" \\([0-9]+\\)$"));
	int iNameNum = 0;
	while (nameItem(sNameRenum, iType)) {
		sNameRenum = sNameBase + QString(" (%1)").arg(++iNameNum);
	}

	return sNameRenum;
}


QString synthv1widget_presets::bankRenum ( const QString& sBank ) const
{
	return nameRenum(sBank, BankItem);
}


QString synthv1widget_presets::presetRenum ( const QString& sPreset ) const
{
	return nameRenum(sPreset, PresetItem);
}


// Drag-n-drop stuff.
//
void synthv1widget_presets::mousePressEvent ( QMouseEvent *pMouseEvent )
{
	dragLeaveEvent(nullptr);

	if (QTreeWidget::rootIsDecorated()
		&& pMouseEvent->button() == Qt::LeftButton) {
		m_posDrag = pMouseEvent->pos();
		m_pDragItem = QTreeWidget::itemAt(m_posDrag);
	}

	QTreeWidget::mousePressEvent(pMouseEvent);
}


void synthv1widget_presets::mouseMoveEvent ( QMouseEvent *pMouseEvent )
{
	QTreeWidget::mouseMoveEvent(pMouseEvent);

	if (m_pDragItem
		&& (pMouseEvent->buttons() & Qt::LeftButton)
		&& ((pMouseEvent->pos() - m_posDrag).manhattanLength()
			>= QApplication::startDragDistance())) {
		// We'll start dragging something alright...
		QMimeData *pMimeData = new QMimeData();
		pMimeData->setText(m_pDragItem->text(0));
		QDrag *pDrag = new QDrag(this);
		pDrag->setMimeData(pMimeData);
		pDrag->setPixmap(m_pDragItem->icon(0).pixmap(16));
		pDrag->setHotSpot(QPoint(-4, -4));
		pDrag->exec(Qt::MoveAction);
		// We've dragged and maybe dropped it by now...
		dragLeaveEvent(nullptr);
		m_pDragItem = nullptr;
	}
}


void synthv1widget_presets::mouseReleaseEvent ( QMouseEvent *pMouseEvent )
{
	QTreeWidget::mouseReleaseEvent(pMouseEvent);

	dragLeaveEvent(nullptr);
	m_pDragItem = nullptr;
}


void synthv1widget_presets::dragEnterEvent ( QDragEnterEvent *pDragEnterEvent )
{
	// Always accept the drag-enter event,
	// so let we deal with it during move later...
	pDragEnterEvent->accept();
}


void synthv1widget_presets::dragMoveEvent ( QDragMoveEvent *pDragMoveEvent )
{
	if (!canDropEvent(pDragMoveEvent)) {
		pDragMoveEvent->ignore();
		return;
	}

	QTreeWidgetItem *pDropItem = dragDropItem(
	#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
		pDragMoveEvent->position().toPoint());
	#else
		pDragMoveEvent->pos());
	#endif
	if (pDropItem) {
		if (!pDragMoveEvent->isAccepted()) {
			pDragMoveEvent->setDropAction(Qt::MoveAction);
			pDragMoveEvent->accept();
		}
	} else {
		pDragMoveEvent->ignore();
	}
}


void synthv1widget_presets::dragLeaveEvent ( QDragLeaveEvent */*pDragLeaveEvent*/ )
{
	if (m_pRubberBand)
		delete m_pRubberBand;
	m_pRubberBand = nullptr;

	m_pDropItem = nullptr;

	m_pAutoExpandTimer->stop();
}


void synthv1widget_presets::dropEvent ( QDropEvent *pDropEvent )
{
	const QPoint& pos
	#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
		= pDropEvent->position().toPoint();
	#else
		= pDropEvent->pos();
	#endif
	QTreeWidgetItem *pDropItem = dragDropItem(pos);
	if (pDropItem)
		dropItem(pDropItem, (pos.x() < QTreeWidget::indentation()));

	dragLeaveEvent(nullptr);
	m_pDragItem = nullptr;
}


bool synthv1widget_presets::canDropEvent ( QDropEvent *pDropEvent )
{
	if (m_pDragItem == nullptr)
		return false;

	if (pDropEvent->source() != this)
		return false;

	return (pDropEvent->mimeData())->hasText();
}


QTreeWidgetItem *synthv1widget_presets::dragDropItem ( const QPoint& pos )
{
	// We must be dragging something...
	QTreeWidgetItem *pDragItem = m_pDragItem;
	if (pDragItem == nullptr)
		return nullptr;

	QTreeWidgetItem *pDropItem = QTreeWidget::itemAt(pos);
	if (pDropItem && pDropItem == m_pDropItem)
		return pDropItem;

	const bool bOutdent
		= (pos.x() < QTreeWidget::indentation());

	if (pDropItem && !isBankItem(pDropItem)) {
		if (isBankItem(pDragItem) || bOutdent)
			pDropItem = pDropItem->parent();
	}

	moveRubberBand(pDropItem, bOutdent);

	m_pDropItem = pDropItem;

	return pDropItem;
}


void synthv1widget_presets::dropItem (
	QTreeWidgetItem *pDropItem, bool bOutdent )
{
	// We must be dragging something...
	QTreeWidgetItem *pDragItem = m_pDragItem;
	if (pDragItem == nullptr)
		return;

	if (pDropItem == nullptr || pDropItem == pDragItem)
		return;

	bool bExpanded = false;
	if (isBankItem(pDragItem))
		bExpanded = pDragItem->isExpanded();

	// Take the item from list...
	int iItem = 0;
	QTreeWidgetItem *pParentItem = pDragItem->parent();
	if (pParentItem) {
		iItem = pParentItem->indexOfChild(pDragItem);
		if (iItem >= 0)
			pDragItem = pParentItem->takeChild(iItem);
	} else {
		iItem = QTreeWidget::indexOfTopLevelItem(pDragItem);
		if (iItem >= 0)
			pDragItem = QTreeWidget::takeTopLevelItem(iItem);
	}

	if (pDragItem == nullptr)
		return;

	// Insert it back...
	if (isBankItem(pDropItem) && !isBankItem(pDragItem)) {
		if (bOutdent) {
			const int iItemCount = QTreeWidget::topLevelItemCount();
			for (iItem = 0; iItem < iItemCount; ++iItem) {
				QTreeWidgetItem *pItem = QTreeWidget::topLevelItem(iItem);
				if (isBankItem(pItem))
					break;
			}
			QTreeWidget::insertTopLevelItem(iItem, pDragItem);
		} else {
			pDropItem->insertChild(0, pDragItem);
		}
	} else {
		pParentItem = pDropItem->parent();
		if (pParentItem) {
			iItem = pParentItem->indexOfChild(pDropItem);
			pParentItem->insertChild(iItem + 1, pDragItem);
		} else {
			iItem = QTreeWidget::indexOfTopLevelItem(pDropItem);
			QTreeWidget::insertTopLevelItem(iItem + 1, pDragItem);
		}
	}

	if (isBankItem(pDragItem) && bExpanded)
		pDragItem->setExpanded(true);
	QTreeWidget::setCurrentItem(pDragItem);

	setDirtyPresets(true);

	emit QTreeWidget::itemChanged(pDragItem, 0);
}


// Draw a dragging separator line.
void synthv1widget_presets::moveRubberBand (
	QTreeWidgetItem *pDropItem, bool bOutdent )
{
	// We must be dragging something...
	QTreeWidgetItem *pDragItem = m_pDragItem;
	if (pDragItem == nullptr)
		return;

	// Is there any item upon we might drop anything?
	if (pDropItem == nullptr) {
		if (m_pRubberBand)
			m_pRubberBand->hide();
		return;
	}

	// Find the drop point below this item...
	QTreeWidgetItem *pBelowItem = pDropItem;
	if (isBankItem(pDragItem)) {
		const int iChildCount = pDropItem->childCount();
		if (iChildCount > 0 && pDropItem->isExpanded())
			pBelowItem = pDropItem->child(iChildCount - 1);
	}
	else
	if (isBankItem(pDropItem)) {
		if (bOutdent) {
			const int iItemCount = QTreeWidget::topLevelItemCount();
			for (int iItem = 0; iItem < iItemCount; ++iItem) {
				pBelowItem = QTreeWidget::topLevelItem(iItem);
				if (isBankItem(pBelowItem))
					break;
			}
		}
		else
		if (!isBankItem(pDragItem) && !pDropItem->isExpanded())
			m_pAutoExpandTimer->start(1200);
	}

	ensureVisibleItem(pBelowItem);

	// Create the rubber-band if there's none...
	if (m_pRubberBand == nullptr) {
		m_pRubberBand = new QRubberBand(QRubberBand::Line, QTreeWidget::viewport());
	}

	// Just move it...
	QRect rect = QTreeWidget::visualItemRect(pBelowItem);
	if (isBankItem(pDragItem)) {
		if (!isBankItem(pBelowItem))
			rect.setX(rect.x() - QTreeWidget::indentation());
	}
	else
	if (isBankItem(pDropItem)) {
		if (bOutdent)
			rect.setBottom(qMax(rect.top() - 3, 0) - 1);
		else
			rect.setX(rect.x() + QTreeWidget::indentation());
	}
	rect.setTop(rect.bottom() + 1);
	rect.setHeight(3);
	m_pRubberBand->setGeometry(rect);

	// Ah, and make it visible, of course...
	if (!m_pRubberBand->isVisible())
		m_pRubberBand->show();
}


// Ensure given item is brought to viewport visibility...
void synthv1widget_presets::ensureVisibleItem ( QTreeWidgetItem *pItem )
{
	QTreeWidgetItem *pItemAbove = pItem->parent();
	if (pItemAbove) {
		const int iItem = pItemAbove->indexOfChild(pItem);
		if (iItem > 0)
			pItemAbove = pItemAbove->child(iItem - 1);
	} else {
		const int iItem = QTreeWidget::indexOfTopLevelItem(pItem);
		if (iItem > 0) {
			pItemAbove = QTreeWidget::topLevelItem(iItem - 1);
			if (pItemAbove) {
				const int iChildCount = pItemAbove->childCount();
				if (iChildCount > 0 && pItemAbove->isExpanded())
					pItemAbove = pItemAbove->child(iChildCount - 1);
			}
		}
	}

	if (pItemAbove)
		QTreeWidget::scrollToItem(pItemAbove);

	QTreeWidget::scrollToItem(pItem);
}


void synthv1widget_presets::autoExpandTimeoutSlot (void)
{
	m_pAutoExpandTimer->stop();

	QTreeWidgetItem *pDropItem = m_pDropItem;
	if (pDropItem && isBankItem(pDropItem) && !pDropItem->isExpanded())
		pDropItem->setExpanded(true);
}


// custom combo-box impl.
//

// ctor.
synthv1widget_presets::ComboBox::ComboBox ( QWidget *pParent )
	: QComboBox(pParent),
	m_pPresetsView(new synthv1widget_presets(pParent))
{
	m_pPresetsView->setRootIsDecorated(false);
	m_pPresetsView->setItemsExpandable(false);

	QComboBox::setModel(m_pPresetsView->model());
	QComboBox::setView(m_pPresetsView);
	QComboBox::setRootModelIndex(QModelIndex());

	QComboBox::setInsertPolicy(QComboBox::NoInsert);
}


synthv1widget_presets *synthv1widget_presets::ComboBox::presetsView (void) const
{
	return m_pPresetsView;
}


void synthv1widget_presets::ComboBox::setCurrentPreset ( const QString& sPreset )
{
	m_pPresetsView->setPresetItem(sPreset);

	QComboBox::setEditText(sPreset);

	resetCurrentPreset();
}


QString synthv1widget_presets::ComboBox::currentPreset (void) const
{
	return QComboBox::currentText();
}


void synthv1widget_presets::ComboBox::resetCurrentPreset (void)
{
	const QModelIndex& current_index
		= m_pPresetsView->currentIndex();
	QComboBox::setRootModelIndex(current_index.parent());
	QComboBox::setCurrentIndex(current_index.row());
	QComboBox::setRootModelIndex(QModelIndex());
}


// end of synthv1widget_presets.cpp
