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

#ifndef CLDAP_ENTRY_H
#define CLDAP_ENTRY_H

#include <memory>
#include "cldap_mod.h"

namespace Ldap
{
    class Server;

    class Entry
    {
	public:
	    Entry(const std::string &);
	    ~Entry();

	    void		SetDN(const std::string &);
	    const std::string &	DN(void) const;

    	    void		Append(int, const std::string &, const std::string &);
    	    void		Append(int, const std::string &, const std::vector<std::string> &);
    	    void		Append(int, const std::string &, const std::list<std::string> &);

    	    void		Append(int, const std::string &, const std::vector<char> &);
    	    void		Append(int, const std::string &, const std::vector< std::vector<char> > &);
    	    void		Append(int, const std::string &, const std::list< std::vector<char> > &);

	    std::string		GetStringValue(const std::string &) const;
	    std::vector<std::string>
				GetStringValues(const std::string &) const;
	    std::list<std::string>
				GetStringList(const std::string &) const;

	    std::vector<char>	GetBinaryValue(const std::string &) const;
	    std::vector< std::vector<char> >
				GetBinaryValues(const std::string &) const;
	    std::list< std::vector<char> >
				GetBinaryList(const std::string &) const;

	    int			Size(void) const { return values.size(); }

	protected:
	    friend class Server;
	    friend std::ostream & operator<< (std::ostream &, const Entry &);

	    typedef std::vector< std::shared_ptr<ModBase> >::iterator iterator;
	    typedef std::vector< std::shared_ptr<ModBase> >::const_iterator const_iterator;

	    std::vector<LDAPMod*>
				toLDAPMods(void) const;
	    iterator		PushBack(const std::string & type, int op, bool binary);
	    const_iterator	FindType(const std::string &) const;

	    std::string		dn;
	    std::vector< std::shared_ptr<ModBase> >
				values;
	};

    std::ostream & operator<< (std::ostream &, const Entry &);
}

#endif
