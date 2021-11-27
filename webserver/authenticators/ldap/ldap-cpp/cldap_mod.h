/***************************************************************************
 *   Copyright (C) 2012 by Andrey Afletdinov                               *
 *   afletdinov@gmail.com                                                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef CLDAP_MOD_H
#define CLDAP_MOD_H

#include "cldap_types.h"

namespace Ldap
{
    class Entry;

    class ModBase
    {
    public:
	ModBase(int, const char*);
	virtual ~ModBase() {}

	virtual void	Clear(void) = 0;
	virtual bool	Append(const char*, size_t) = 0;

	void		Append(const std::string & str) { if(str.size()) Append(str.c_str(), str.size()); }
	void		Append(const std::vector<char> & v) { if(v.size()) Append(& v[0], v.size()); }


	bool		IsType(const std::string &) const;
	bool		IsBinary(void) const;
	bool		IsOperation(int) const;

	virtual std::string			GetStringValue(void) const = 0;
	virtual std::vector<std::string>	GetStringValues(void) const = 0;
	virtual std::list<std::string>		GetStringList(void) const = 0;

	virtual std::vector<char>		GetBinaryValue(void) const = 0;
	virtual std::vector< std::vector<char> >GetBinaryValues(void) const = 0;
	virtual std::list< std::vector<char> >	GetBinaryList(void) const = 0;

    protected:
	friend class Entry;

	const LDAPMod*	toLDAPMod(void) const;

	LDAPMod		val;
	size_t		mod_vals_size;
    };

    class ModStr : public ModBase
    {
	friend std::ostream & operator<< (std::ostream &, const ModStr &);

    public:
	ModStr(int op, const std::string & type) : ModBase(op, type.c_str()) {}
	~ModStr() { Clear(); }

	void		Clear(void);
	bool		Append(const char*, size_t);
	void		Append(const char*);

	std::string	GetStringValue(void) const;
	std::vector<std::string>
			GetStringValues(void) const;
	std::list<std::string>
			GetStringList(void) const;

	std::vector<char>
			GetBinaryValue(void) const;
	std::vector< std::vector<char> >
			GetBinaryValues(void) const;
	std::list< std::vector<char> >
			GetBinaryList(void) const;
    };

    class ModBin : public ModBase
    {
	friend std::ostream & operator<< (std::ostream &, const ModBin &);

    public:
	ModBin(int op, const std::string & type) : ModBase(op | LDAP_MOD_BVALUES, type.c_str()) {}
	~ModBin() { Clear(); }

	void		Clear(void);
	bool		Append(const char*, size_t);

	std::string	GetStringValue(void) const;
	std::vector<std::string>
			GetStringValues(void) const;
	std::list<std::string>
			GetStringList(void) const;

	std::vector<char>
			GetBinaryValue(void) const;
	std::vector< std::vector<char> >
			GetBinaryValues(void) const;
	std::list< std::vector<char> >
			GetBinaryList(void) const;
    };

    std::ostream & operator<< (std::ostream &, const ModStr &);
    std::ostream & operator<< (std::ostream &, const ModBin &);
}

#endif
