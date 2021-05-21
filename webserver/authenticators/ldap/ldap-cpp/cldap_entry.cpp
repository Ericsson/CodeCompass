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

#include <algorithm>
#include <iterator>
#include "cldap_entry.h"

Ldap::Entry::Entry(const std::string & str) : dn(str)
{
    values.reserve(32);
}

Ldap::Entry::~Entry(void)
{
}

void Ldap::Entry::SetDN(const std::string & str)
{
    dn = str;
}

const std::string & Ldap::Entry::DN(void) const
{
    return dn;
}

void Ldap::Entry::Append(int op, const std::string & attr, const std::string & value)
{
    if(attr.size() && value.size())
    {
	auto it = PushBack(attr, op, false);

	if(it == values.end())
	    (*it)->Append(value);
    }
}

void Ldap::Entry::Append(int op, const std::string & attr, const std::vector<std::string> & vals)
{
    if(attr.size() && vals.size())
    {
	auto it1 = PushBack(attr, op, false);

	if(it1 == values.end())
	    for(auto it2 = vals.begin(); it2 != vals.end(); ++it2)
		(*it1)->Append(*it2);
    }
}

void Ldap::Entry::Append(int op, const std::string & attr, const std::list<std::string> & vals)
{
    if(attr.size() && vals.size())
    {
	auto it1 = PushBack(attr, op, false);

	if(it1 == values.end())
	    for(auto it2 = vals.begin(); it2 != vals.end(); ++it2)
		(*it1)->Append(*it2);
    }
}

void Ldap::Entry::Append(int op, const std::string & attr, const std::vector<char> & value)
{
    if(attr.size() && value.size())
    {
	auto it = PushBack(attr, op, true);

	if(it == values.end())
	    (*it)->Append(value);
    }
}

void Ldap::Entry::Append(int op, const std::string & attr, const std::vector< std::vector<char> > & vals)
{
    if(attr.size() && vals.size())
    {
	auto it1 = PushBack(attr, op, true);

	if(it1 == values.end())
	    for(auto it2 = vals.begin(); it2 != vals.end(); ++it2)
		(*it1)->Append(*it2);
    }
}

void Ldap::Entry::Append(int op, const std::string & attr, const std::list< std::vector<char> > & vals)
{
    if(attr.size() && vals.size())
    {
	auto it1 = PushBack(attr, op, true);

	if(it1 == values.end())
	    for(auto it2 = vals.begin(); it2 != vals.end(); ++it2)
		(*it1)->Append(*it2);
    }
}

Ldap::Entry::const_iterator Ldap::Entry::FindType(const std::string & type) const
{
    for(auto it = values.begin(); it != values.end(); ++it)
	if((*it).get() && (*it).get()->IsType(type)) return it;
    return values.end();
}

std::string Ldap::Entry::GetStringValue(const std::string & attr) const
{
    if(attr.size())
    {
	auto it = FindType(attr);

	if(it != values.end())
	    return (*it)->GetStringValue();
    }

    return std::string();
}

std::vector<std::string>
Ldap::Entry::GetStringValues(const std::string & attr) const
{
    std::vector<std::string> res;

    if(attr.size())
    {
	auto it = FindType(attr);

	if(it != values.end())
	    return (*it)->GetStringValues();
    }

    return res;
}

std::list<std::string>
Ldap::Entry::GetStringList(const std::string & attr) const
{
    std::list<std::string> res;

    if(attr.size())
    {
	auto it = FindType(attr);

	if(it != values.end())
	    return (*it)->GetStringList();
    }

    return res;
}

std::vector<char> Ldap::Entry::GetBinaryValue(const std::string & attr) const
{
    if(attr.size())
    {
	auto it = FindType(attr);

	if(it != values.end())
	    return (*it)->GetBinaryValue();
    }

    return std::vector<char>();
}

std::vector< std::vector<char> >
Ldap::Entry::GetBinaryValues(const std::string & attr) const
{
    std::vector< std::vector<char> > res;

    if(attr.size())
    {
	auto it = FindType(attr);

	if(it != values.end())
	    return (*it)->GetBinaryValues();
    }

    return res;
}

std::list< std::vector<char> >
Ldap::Entry::GetBinaryList(const std::string & attr) const
{
    std::list< std::vector<char> > res;

    if(attr.size())
    {
	auto it = FindType(attr);

	if(it != values.end())
	    return (*it)->GetBinaryList();
    }

    return res;
}

Ldap::Entry::iterator Ldap::Entry::PushBack(const std::string & type, int op, bool binary)
{
    for(auto it = values.begin(); it != values.end(); ++it)
	if((*it).get() && ((*it).get()->IsBinary() == binary) &&
	    (*it).get()->IsType(type) && (*it)->IsOperation(op)) return it;

    if(binary)
    {
	values.push_back(std::shared_ptr<ModBase>(new ModBin(op, type)));
	return values.end() - 1;
    }
    else
    {
	values.push_back(std::shared_ptr<ModBase>(new ModStr(op, type)));
	return values.end() - 1;
    }

    return values.end();
}

std::vector<LDAPMod*> Ldap::Entry::toLDAPMods(void) const
{
    std::vector<LDAPMod*> v;

    v.reserve(values.size() + 1);

    for(auto it = values.begin(); it != values.end(); ++it)
	if((*it).get())
	    v.push_back(const_cast<LDAPMod*>((*it).get()->toLDAPMod()));

    return v;
}

std::ostream & Ldap::operator<< (std::ostream & os, const Entry & entry)
{
    os << Base64::StringWrap("dn", entry.dn) << std::endl;

    for(auto it = entry.values.begin(); it != entry.values.end(); ++it)
    {
	if((*it)->IsBinary())
	{
	    const ModBin* mod = dynamic_cast<const ModBin*>((*it).get());
	    if(mod) os << *mod;
	}
	else
	{
	    const ModStr* mod = dynamic_cast<const ModStr*>((*it).get());
	    if(mod) os << *mod;
	}
    }

    return os;
}
