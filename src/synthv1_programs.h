// synthv1_programs.h
//
/****************************************************************************
   Copyright (C) 2012-2024, rncbc aka Rui Nuno Capela. All rights reserved.

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

#ifndef __synthv1_programs_h
#define __synthv1_programs_h

#include "synthv1_sched.h"
#include "synthv1_param.h"

#include <QMap>


//-------------------------------------------------------------------------
// synthv1_programs - Bank/programs database class.
//

class synthv1_programs
{
public:

	// ctor.
	synthv1_programs(synthv1 *pSynth);

	// dtor.
	~synthv1_programs();

	// operational mode flags.
	void enabled(bool on)
		{ m_enabled = on; }
	bool enabled() const
		{ return m_enabled; }

	// prog. base node
	class Prog
	{
	public:

		Prog(uint16_t id, const QString& name)
			: m_id(id), m_name(name) {}

		uint16_t id() const	{ return m_id; }
		const QString& name() const	{ return m_name; }
		void set_name(const QString& name) { m_name = name; }

	private:

		uint16_t m_id;
		QString  m_name;
	};

	typedef QMap<uint16_t, Prog *> Progs;

	// bank node
	class Bank : public Prog
	{
	public:

		Bank(uint16_t id, const QString& name)
			: Prog(id, name) {}

		~Bank() { clear_progs(); }

		const Progs& progs() const { return m_progs; }

		// prog. managers
		Prog *find_prog(uint16_t prog_id) const;
		Prog *add_prog(uint16_t prog_id, const QString& prog_name);
		void remove_prog(uint16_t prog_id);
		void clear_progs();

	private:

		Progs m_progs;
	};

	typedef QMap<uint16_t, Bank *> Banks;

	const Banks& banks() const { return m_banks; }

	// bank managers
	Bank *find_bank(uint16_t bank_id) const;
	Bank *add_bank(uint16_t bank_id, const QString& bank_name);
	void remove_bank(uint16_t bank_id);

	void clear_banks();

	// current bank/prog. managers
	void bank_select_msb(uint8_t bank_msb);
	void bank_select_lsb(uint8_t bank_lsb);

	void bank_select(uint16_t bank_id);
	void prog_change(uint16_t prog_id);

	void select_program(uint16_t bank_id, uint16_t prog_id);

	void process_program(synthv1 *pSynth, uint16_t bank_id, uint16_t prog_id);

	Bank *current_bank() const { return m_bank; }
	Prog *current_prog() const { return m_prog; }

protected:

	uint16_t current_bank_id() const;

	// current bank/prog. scheduled thread
	class Sched : public synthv1_sched
	{
	public:

		// ctor.
		Sched (synthv1 *pSynth)
			: synthv1_sched(pSynth, Programs), m_bank_id(0), m_prog_id(0) {}

		// schedule (override)
		void select_program(uint16_t bank_id, uint16_t prog_id)
		{
			if (m_bank_id != bank_id ||
				m_prog_id != prog_id) {
				m_bank_id  = bank_id;
				m_prog_id  = prog_id;
				schedule();
			}
		}

		// process (virtual).
		void process(int)
		{
			synthv1 *pSynth = instance();
			synthv1_programs *pPrograms = pSynth->programs();
			pPrograms->process_program(pSynth, m_bank_id, m_prog_id);
		}

	private:

		// instance variables.
		uint16_t m_bank_id;
		uint16_t m_prog_id;
	};

private:

	// instance variables.
	bool m_enabled;

	Sched m_sched;

	uint8_t m_bank_msb;
	uint8_t m_bank_lsb;

	Bank *m_bank;
	Prog *m_prog;

	Banks m_banks;
};


#endif	// __synthv1_programs_h

// end of synthv1_programs.h
