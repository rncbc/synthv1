// synthv1widget_presets.h
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

#ifndef __synthv1widget_presets_h
#define __synthv1widget_presets_h

#include <QTreeWidget>
#include <QComboBox>


// forward decls.
class synthv1_presets;

class QRubberBand;
class QTimer;
class QValidator;


//----------------------------------------------------------------------------
// synthv1widget_presets -- Custom (tree) widget.

class synthv1widget_presets : public QTreeWidget
{
	Q_OBJECT

public:

	// ctor.
	synthv1widget_presets(QWidget *pParent = nullptr);
	// dtor.
	~synthv1widget_presets();

	// utilities.
	void loadPresets(synthv1_presets *pPresets);
	void savePresets(synthv1_presets *pPresets);

	QString currentPreset() const;

	void setPresetItem(const QString& sPreset);
	QTreeWidgetItem *presetItem(const QString& sPreset) const;
	bool isPresetItem(QTreeWidgetItem *pItem) const;

	void setBankItem(const QString& sBank, int iPreset = 0);
	QTreeWidgetItem *bankItem(const QString& sBank) const;
	bool isBankItem(QTreeWidgetItem *pItem) const;

	QString bankRenum(const QString& sBank) const;
	QString presetRenum(const QString& sPreset) const;

	// file prompts.
	QStringList openPresetFiles();
	QString savePresetFile(const QString& sPreset);

	// config stuff.
	void setPresetDir(const QString& sPresetDir);
	const QString& presetDir() const;

	void setPresetExt(const QString& sPresetExt);
	const QString& presetExt() const;

	void setDontUseNativeDialogs(bool bDontUseNativeDialogs);
	bool dontUseNativeDialogs() const;

	// common validator accessor.
	QValidator *presetsValidator() const;

	// dirty flag accessors.
	void setDirtyPresets(bool bDirtyPresets);
	bool isDirtyPresets() const;

	// custom combo-box decl.
	//
	class ComboBox;

public slots:

	// slots.
	void addBankItem();
	void addPresetItem();

protected slots:

	// private slots.
	void itemChangedSlot(QTreeWidgetItem *, int);

	void itemExpandedSlot(QTreeWidgetItem *);
	void itemCollapsedSlot(QTreeWidgetItem *);

	void autoExpandTimeoutSlot();

protected:

	// item types.
	enum {
		BankItem   = QTreeWidgetItem::UserType,
		PresetItem = QTreeWidgetItem::UserType + 1
	};

	// item delegate decl..
	class ItemDelegate;

	// factory methods.
	QTreeWidgetItem *newBankItem();
	QTreeWidgetItem *newPresetItem();

	// name helpers.
	QTreeWidgetItem *nameItem(const QString& sName, int iType) const;
	QString nameRenum(const QString& sName, int iType) const;

	// Drag-n-drop stuff.
	//
	void mousePressEvent(QMouseEvent *pMouseEvent);
	void mouseMoveEvent(QMouseEvent *pMouseEvent);
	void mouseReleaseEvent(QMouseEvent *pMouseEvent);

	void dragEnterEvent(QDragEnterEvent *pDragEnterEvent);
	void dragMoveEvent(QDragMoveEvent *pDragMoveEvent);
	void dragLeaveEvent(QDragLeaveEvent *pDragLeaveEvent);
	void dropEvent(QDropEvent *pDropEvent);

	bool canDropEvent(QDropEvent *pDropEvent);

	QTreeWidgetItem *dragDropItem(const QPoint& pos);

	void dropItem(QTreeWidgetItem *pDropItem, bool bOutdent);

	void moveRubberBand(QTreeWidgetItem *pDropItem, bool bOutdent);
	void ensureVisibleItem(QTreeWidgetItem *pItem);

private:

	// config stuff.
	QString m_sPresetDir;
	QString m_sPresetExt;
	bool    m_bDontUseNativeDialogs;

	// common validator.
	QValidator *m_pPresetsValidator;

	// dirty flag.
	int m_iDirtyPresets;

	// Drag-n-drop stuff.
	//
	QPoint           m_posDrag;
	QTreeWidgetItem *m_pDragItem;
	QTreeWidgetItem *m_pDropItem;
	QRubberBand     *m_pRubberBand;
	QTimer          *m_pAutoExpandTimer;
};


// custom combo-box decl.
//
class synthv1widget_presets::ComboBox : public QComboBox
{
	Q_OBJECT

public:

	ComboBox(QWidget *pParent = nullptr);

	synthv1widget_presets *presetsView() const;

	void setCurrentPreset(const QString& sPreset);
	QString currentPreset() const;

	void resetCurrentPreset();

private:

	synthv1widget_presets *m_pPresetsView;
};


#endif	// __synthv1widget_presets_h

// end of synthv1widget_presets.h
