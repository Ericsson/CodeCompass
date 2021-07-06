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
#include "cldap_server.h"

std::string Ldap::Info::CreateURI(const std::string & uri, bool ssl)
{
    const char* ldap1 = "ldaps://";
    const char* ldap2 = "ldap://";
    std::string res;

    if(strlen(ldap2) < uri.size() &&
	(0 == uri.substr(0, strlen(ldap1)).compare(ldap1) || 0 == uri.substr(0, strlen(ldap2)).compare(ldap2)))
	res = uri;
    else
	res = std::string(ssl ? ldap1 : ldap2) + uri;

    return res;
}

Ldap::Server::Server() : ldap_object(NULL), ldap_errno(0), binded(false), connected(false)
{
}

Ldap::Server::Server(const std::string & uri, bool ssl) : Info(uri, ssl), ldap_object(NULL), ldap_errno(0)
{
}

Ldap::Server::Server(const Info & info) : Info(info), ldap_object(NULL), ldap_errno(0)
{
}

Ldap::Server::~Server()
{
    if(ldap_object) Disconnect();
}

Ldap::Server::Server(const Server & srv) : Info(srv), ldap_object(NULL), ldap_errno(0)
{
    binded = srv.binded;
    connected = srv.connected;

    if(connected)
    {
	Connect();

	if(binded)
	{
	    Bind();
	    ldap_errno = srv.ldap_errno;
	}
    }
}

Ldap::Server & Ldap::Server::operator=(const Server & srv)
{
    Unbind();
    Disconnect();

    Info::ldap_uri = srv.ldap_uri;
    Info::bind_dn = srv.bind_dn;
    Info::bind_pw = srv.bind_pw;

    binded = srv.binded;
    connected = srv.connected;

    if(connected)
    {
	Connect();

	if(binded)
	{
	    Bind();
	    ldap_errno = srv.ldap_errno;
	}
    }

    return *this;
}

bool Ldap::Server::IsBinded(void) const
{
    return binded;
}

bool Ldap::Server::IsConnected(void) const
{
    return connected;
}

std::string Ldap::Server::BaseDN(void)
{
    std::string res;

    if(ldap_object)
    {
        ListEntries entries = Search("");
        res = entries.size() ? entries.front().DN() : "";
    }

    return res; 
}

bool Ldap::Server::Connect(const std::string & uri, bool ssl)
{
    if(ldap_object) Disconnect();

    if(uri.size())
	Info::ldap_uri = CreateURI(uri, ssl);

    const int protocol_version = 3;
    ldap_errno = ldap_initialize(&ldap_object, URI().c_str());
    if(ldap_object) ldap_set_option(ldap_object, LDAP_OPT_PROTOCOL_VERSION, &protocol_version);

    connected = LDAP_SUCCESS == ldap_errno;
    return connected;
}

void Ldap::Server::Disconnect(void)
{
    if(ldap_object) Unbind();
    connected = false;
}

bool Ldap::Server::Bind(const std::string & bdn, const std::string & bpw)
{
    Info::bind_dn = bdn;
    Info::bind_pw = bpw;

    return Bind();
}

bool Ldap::Server::Bind(void)
{
    if(!ldap_object) Connect();

    struct berval cred;

    cred.bv_val = const_cast<char*>(BindPW().c_str());
    cred.bv_len = BindPW().size();

    ldap_errno = ldap_sasl_bind_s(ldap_object, BindDN().c_str(), NULL, &cred, NULL, NULL, NULL);

    binded = LDAP_SUCCESS == ldap_errno;
    return binded;
}

void Ldap::Server::Unbind(void)
{
    if(ldap_object) ldap_errno = ldap_unbind_ext_s(ldap_object, NULL, NULL);

    binded = false;
    ldap_object = NULL;
}

bool Ldap::Server::Add(const Entry & entry)
{
    if(ldap_object)
    {
	std::vector<LDAPMod*> v = entry.toLDAPMods();
	v.push_back(NULL);
	LDAPMod** mod = reinterpret_cast<LDAPMod**>(&v[0]);
	ldap_errno = ldap_add_ext_s(ldap_object, entry.DN().c_str(), mod, NULL, NULL);
	return LDAP_SUCCESS == ldap_errno;
    }

    return false;
}

bool Ldap::Server::Modify(const Entry & entry)
{
    if(ldap_object)
    {
	std::vector<LDAPMod*> v = entry.toLDAPMods();
	v.push_back(NULL);
	LDAPMod** mod = reinterpret_cast<LDAPMod**>(&v[0]);
	ldap_errno = ldap_modify_ext_s(ldap_object, entry.DN().c_str(), mod, NULL, NULL);
	return LDAP_SUCCESS == ldap_errno;
    }

    return false;
}

bool Ldap::Server::Compare(const std::string & dn, const std::string & attr, const std::string & val) const
{
    struct berval bv;

    bv.bv_val = const_cast<char*>(val.c_str());
    bv.bv_len = val.size();

    return ldap_object &&
	LDAP_COMPARE_TRUE == ldap_compare_ext_s(ldap_object, dn.c_str(), attr.c_str(), &bv, NULL, NULL);
}

const char* Ldap::Server::Message(void) const
{
    return ldap_object ? ldap_err2string(ldap_errno) : NULL;
}

bool Ldap::Server::ModDN(const std::string & dn, const std::string & newdn)
{
    return ldap_object &&
	LDAP_SUCCESS == (ldap_errno = ldap_rename_s(ldap_object, dn.c_str(), newdn.c_str(), NULL, 1, NULL, NULL));
}

bool Ldap::Server::Delete(const std::string & dn)
{
    return ldap_object &&
	LDAP_SUCCESS == (ldap_errno = ldap_delete_ext_s(ldap_object, dn.c_str(), NULL, NULL));
}

Ldap::ListEntries Ldap::Server::Search(const std::string & base, const Scope & scope, const std::string & filter)
{
    ListAttrs attrs;
    return Search(base, scope, filter, attrs);
}

Ldap::ListEntries Ldap::Server::Search(const std::string & base, const Scope & scope, const std::string & filter, const ListAttrs & attrs)
{
    ListEntries result;

    // prepare ldap attrs
    char** ldap_attrs = attrs.size() ? new char* [ attrs.size() + 1 ] : NULL;

    if(ldap_attrs)
    {
        char** ptr = ldap_attrs;

        for(ListAttrs::const_iterator
            it = attrs.begin(); it != attrs.end(); ++it)
            *ptr++ = const_cast<char*>((*it).c_str());

        *ptr = NULL;
    }

    LDAPMessage* res = NULL;

    // search
    ldap_errno = ldap_search_ext_s(ldap_object, (base.empty() ? NULL : base.c_str()), scope,
			(filter.empty() ? NULL : filter.c_str()), ldap_attrs, 0, NULL, NULL, NULL, 0, & res);

    // insert entries
    if(LDAP_SUCCESS == ldap_errno &&
	0 != ldap_count_entries(ldap_object, res))
    {
	for(LDAPMessage* ldap_entry = ldap_first_entry(ldap_object, res);
		    ldap_entry; ldap_entry = ldap_next_entry(ldap_object, ldap_entry))
	{
	    char* dn = ldap_get_dn(ldap_object, ldap_entry);

    	    result.push_back(Entry(dn));
    	    Entry & current_entry = result.back();

	    BerElement* ber = NULL;

    	    for(char* ldap_attr = ldap_first_attribute(ldap_object, ldap_entry, &ber);
			ldap_attr; ldap_attr = ldap_next_attribute(ldap_object, ldap_entry, ber))
    	    {
		if(berval** vals = ldap_get_values_len(ldap_object, ldap_entry, ldap_attr))
		{
		    size_t count = ldap_count_values_len(vals);
		    if(count)
		    {
			auto it = current_entry.PushBack(ldap_attr, ActionAdd, true);
			if(it != current_entry.values.end())
			    for(size_t ii = 0; ii < count; ++ii)
				if(vals[ii]) (*it)->Append(vals[ii]->bv_val, vals[ii]->bv_len);
		    }
    		    ldap_value_free_len(vals);
		}
#ifdef LDAP_DEPRECATED
		else
		if(char** vals = ldap_get_values(ldap_object, ldap_entry, ldap_attr))
		{
		    size_t count = ldap_count_values(vals);
		    if(count)
		    {
			auto it = current_entry.PushBack(ldap_attr, ActionAdd, false);
			if(it != current_entry.values.end())
			    for(size_t ii = 0; ii < count; ++ii)
				(*it)->Append(vals[ii]);
		    }
    		    ldap_value_free(vals);
		}
#endif
    		ldap_memfree(ldap_attr);
    	    }

    	    if(ber) ber_free(ber, 0);
	    if(dn) ldap_memfree(dn);
	}
    }

    if(res) ldap_msgfree(res);
    if(ldap_attrs) delete [] ldap_attrs;

    return result;
}

bool Ldap::Server::Ping(void) const
{
    if(ldap_object)
    {
	LDAPMessage* res = NULL;
        int err = ldap_search_ext_s(ldap_object, NULL, ScopeBase, NULL, NULL, 0, NULL, NULL, NULL, 0, & res);
	if(res) ldap_msgfree(res);

	return LDAP_SERVER_DOWN != err;
    }

    return false;
}

int Ldap::Server::Error(void) const
{
    return ldap_errno;
}

std::ostream & Ldap::operator<< (std::ostream & os, const ListEntries & entries)
{
    std::copy(entries.begin(), entries.end(), std::ostream_iterator<Entry>(os, "\n"));
    return os;
}

int Ldap::getVersion(void)
{
    return CLDAP_VERSION;
}

Ldap::Pools::Pools()
{
    current = begin();
}

Ldap::Pools::Pools(const Info & bi1, const Info & bi2)
{
    push_back(Ldap::Server(bi1));
    push_back(Ldap::Server(bi2));
    current = begin();
}

Ldap::Pools::Pools(const std::list<Info> & bis)
{
    AddPools(bis);
}

void Ldap::Pools::AddServer(const Info & bi)
{
    push_back(Ldap::Server(bi));
    current = begin();
}

void Ldap::Pools::AddPools(const std::list<Info> & bis)
{
    for(auto it = bis.begin(); it != bis.end(); ++it)
	push_back(Ldap::Server(*it));
    current = begin();
}

Ldap::ListEntries Ldap::Pools::Search(const std::string & base, const Scope & scope, const std::string & filter)
{
    ListAttrs attrs;
    return Search(base, scope, filter, attrs);
}

Ldap::ListEntries Ldap::Pools::Search(const std::string & base, const Scope & scope, const std::string & filter, const ListAttrs & attrs)
{
    Ldap::ListEntries res;
    if(empty()) return res;

    if(current == end()) current = begin();
    size_t loop = 1;

    while(! (*current).Ping())
    {
	current++;

	if(current == end())
	    current = begin();

	if((*current).Connect() && 
	    (*current).Bind()) break;

	if(loop++ >= size()) break;
    }

    if((*current).IsBinded())
    {
	res = (*current).Search(base, scope, filter);
    }

    return res;
}
