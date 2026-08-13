// synthv1_presets.h
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

#ifndef __synthv1_presets_h
#define __synthv1_presets_h

#include <QHash>
#include <QString>


//-------------------------------------------------------------------------
// synthv1_presets - Bank/presets database class.
//

class synthv1_presets
{
public:

	// ctor.
	synthv1_presets();

	// dtor.
	~synthv1_presets();

	// bank node
	class Bank
	{
	public:

		Bank(const QString& name)
			: m_name(name) {}

		~Bank() { clear_presets(); }

		const QString& name() const
			{ return m_name; }
		void set_name(const QString& name)
			{ m_name = name; }

		const QStringList& preset_list() const
			{ return m_preset_list; }

		// preset managers
		void add_preset(const QString& preset_name);
		void remove_preset(const QString& preset_name);
		void clear_presets();

	private:

		QString     m_name;
		QStringList m_preset_list;
	};

	typedef QHash<QString, Bank *> Banks;

	const Banks& banks() const
		{ return m_banks; }

	const QStringList& bank_list() const
		{ return m_bank_list; }

	// preset node
	class Preset
	{
	public:

		Preset(const QString& name)
			: m_name(name) {}

		const QString& name() const
			{ return m_name; }
		void set_name(const QString& name)
			{ m_name = name; }

		const QString& file() const
			{ return m_file; }
		void set_file(const QString& file)
			{ m_file = file; }

	private:

		QString m_name;
		QString m_file;
	};

	typedef QHash<QString, Preset *> Presets;

	const Presets& presets() const
		{ return m_presets; }

	const QStringList& preset_list() const
		{ return m_preset_list; }

	bool isEmpty() const
		{ return m_preset_list.isEmpty() && m_bank_list.isEmpty(); }

	// bank managers
	Bank *find_bank(const QString& bank_name) const;
	Bank *find_preset_bank(const QString& preset_name) const;
	Bank *add_bank(const QString& bank_name);
	void remove_bank(const QString& bank_name);
	void clear_banks();

	// preset managers
	Preset *find_preset(const QString& preset_name) const;
	Preset *add_preset(const QString& preset_name);
	void remove_preset(const QString& preset_name);
	void clear_presets();

private:

	Banks       m_banks;
	QStringList m_bank_list;

	Presets     m_presets;
	QStringList m_preset_list;
};


#endif	// __synthv1_presets_h

// end of synthv1_presets.h
