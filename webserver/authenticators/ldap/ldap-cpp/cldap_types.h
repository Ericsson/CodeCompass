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

#ifndef CLDAP_TYPES_H
#define CLDAP_TYPES_H

#include <string>
#include <list>
#include <vector>
#include <iostream>
#include <ldap.h>

#define CLDAP_VERSION 20180427

namespace Ldap
{
    class Entry;

    enum Action { ActionAdd = LDAP_MOD_ADD, ActionDelete = LDAP_MOD_DELETE, ActionReplace = LDAP_MOD_REPLACE };
    enum Scope { ScopeBase = LDAP_SCOPE_BASE, ScopeOne = LDAP_SCOPE_ONELEVEL, ScopeTree = LDAP_SCOPE_SUBTREE };

    typedef std::list<Entry>            ListEntries;
    typedef std::list<std::string>      ListAttrs;
}

namespace Base64
{
    std::vector<char> encode(const char*, size_t);
    std::vector<char> decode(const char*, size_t);

    std::string StringWrap(const std::string &, const std::string &);
    void SetBinaryOnly(bool);
}

#endif
