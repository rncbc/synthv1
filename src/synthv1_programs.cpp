// synthv1_programs.cpp
//
/****************************************************************************
   Copyright (C) 2012-2024 rncbc aka Rui Nuno Capela. All rights reserved.

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

#include "synthv1_programs.h"


//-------------------------------------------------------------------------
// synthv1_programs - Bank/programs database class (singleton).
//

// ctor.
synthv1_programs::synthv1_programs ( synthv1 *pSynth )
	: m_enabled(false), m_sched(pSynth),
		m_bank_msb(0), m_bank_lsb(0),
		m_bank(nullptr), m_prog(nullptr)
{
}


// dtor.
synthv1_programs::~synthv1_programs (void)
{
	clear_banks();
}


// prog. managers
synthv1_programs::Prog *synthv1_programs::Bank::find_prog ( uint16_t prog_id ) const
{
	return m_progs.value(prog_id, nullptr);
}


synthv1_programs::Prog *synthv1_programs::Bank::add_prog (
	uint16_t prog_id, const QString& prog_name )
{
	Prog *prog = find_prog(prog_id);
	if (prog) {
		prog->set_name(prog_name);
	} else {
		prog = new Prog(prog_id, prog_name);
		m_progs.insert(prog_id, prog);
	}
	return prog;
}


void synthv1_programs::Bank::remove_prog ( uint16_t prog_id )
{
	Prog *prog = find_prog(prog_id);
	if (prog && m_progs.remove(prog_id))
		delete prog;
}


void synthv1_programs::Bank::clear_progs (void)
{
	qDeleteAll(m_progs);
	m_progs.clear();
}


// bank managers
synthv1_programs::Bank *synthv1_programs::find_bank ( uint16_t bank_id ) const
{
	return m_banks.value(bank_id, nullptr);
}


synthv1_programs::Bank *synthv1_programs::add_bank (
	uint16_t bank_id, const QString& bank_name )
{
	Bank *bank = find_bank(bank_id);
	if (bank) {
		bank->set_name(bank_name);
	} else {
		bank = new Bank(bank_id, bank_name);
		m_banks.insert(bank_id, bank);
	}
	return bank;
}


void synthv1_programs::remove_bank ( uint16_t bank_id )
{
	Bank *bank = find_bank(bank_id);
	if (bank && m_banks.remove(bank_id))
		delete bank;
}


void synthv1_programs::clear_banks (void)
{
	m_bank_msb = 0;
	m_bank_lsb = 0;

	m_bank = nullptr;
	m_prog = nullptr;

	qDeleteAll(m_banks);
	m_banks.clear();
}


// current bank/prog. managers
void synthv1_programs::bank_select_msb ( uint8_t bank_msb )
{
	m_bank_msb = 0x80 | (bank_msb & 0x7f);
}


void synthv1_programs::bank_select_lsb ( uint8_t bank_lsb )
{
	m_bank_lsb = 0x80 | (bank_lsb & 0x7f);
}


void synthv1_programs::bank_select ( uint16_t bank_id )
{
	bank_select_msb(bank_id >> 7);
	bank_select_lsb(bank_id);
}


uint16_t synthv1_programs::current_bank_id (void) const
{
	uint16_t bank_id = 0;

	if (m_bank_msb & 0x80)
		bank_id   = (m_bank_msb & 0x7f);
	if (m_bank_lsb & 0x80) {
		bank_id <<= 7;
		bank_id  |= (m_bank_lsb & 0x7f);
	}

	return bank_id;
}


void synthv1_programs::prog_change ( uint16_t prog_id )
{
	select_program(current_bank_id(), prog_id);
}


void synthv1_programs::select_program ( uint16_t bank_id, uint16_t prog_id )
{
	if (!enabled())
		return;

	if (m_bank && m_bank->id() == bank_id &&
		m_prog && m_prog->id() == prog_id)
		return;

	m_sched.select_program(bank_id, prog_id);
}


void synthv1_programs::process_program (
	synthv1 *pSynth, uint16_t bank_id, uint16_t prog_id )
{
	m_bank = find_bank(bank_id);
	m_prog = (m_bank ? m_bank->find_prog(prog_id) : nullptr);

	if (m_prog) {
		synthv1_param::loadPreset(pSynth, m_prog->name());
		pSynth->updateParams();
	}
}


// end of synthv1_programs.cpp
