// synthv1_programs.cpp
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

#include "synthv1_programs.h"


//-------------------------------------------------------------------------
// synthv1_programs - Bank/programs database class (singleton).
//

// ctor.
synthv1_programs::synthv1_programs (void)
	: m_bank_msb(0), m_bank_lsb(0), m_bank(0), m_prog(0)
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
	return m_progs.value(prog_id, 0);
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
	return m_banks.value(bank_id, 0);
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

	m_bank = 0;
	m_prog = 0;

	qDeleteAll(m_banks);
	m_banks.clear();
}


// current bank/prog. managers
void synthv1_programs::set_current_bank_msb ( uint8_t bank_msb )
{
	m_bank_msb = 0x80 | (bank_msb & 0x7f);
}


void synthv1_programs::set_current_bank_lsb ( uint8_t bank_lsb )
{
	m_bank_lsb = 0x80 | (bank_lsb & 0x7f);
}


void synthv1_programs::set_current_bank ( uint16_t bank_id )
{
	set_current_bank_msb(bank_id >> 7);
	set_current_bank_lsb(bank_id);
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


void synthv1_programs::set_current_prog ( uint16_t prog_id )
{
	m_bank = find_bank(current_bank_id());
	m_prog = (m_bank ? m_bank->find_prog(prog_id) : 0);
}


// end of synthv1_programs.cpp
