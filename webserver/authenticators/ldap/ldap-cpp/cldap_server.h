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

#ifndef CLDAP_SERVER_H
#define CLDAP_SERVER_H

#include "cldap_types.h"

namespace Ldap
{
    int getVersion(void);

    class Info
    {
	protected:
    	    std::string			ldap_uri;
    	    std::string			bind_dn;
    	    std::string			bind_pw;

	public:
	    Info(const std::string & uri, bool ssl = false) : ldap_uri(CreateURI(uri, ssl)) {}
	    Info(const std::string & uri, const std::string & dn, const std::string & pw) : ldap_uri(CreateURI(uri, false)), bind_dn(dn), bind_pw(pw) {}
	    Info() {}

	    const std::string &		URI(void) const { return ldap_uri; }
    	    const std::string &		BindDN(void) const { return bind_dn; }
    	    const std::string &		BindPW(void) const { return bind_pw; }

	    static std::string 		CreateURI(const std::string & uri, bool ssl);
    };

    class Server : public Info
    {
	public:
            Server();
	    Server(const Info &);
            Server(const std::string & uri, bool ssl = false);
	    Server(const Server &);
            ~Server();

	    Server & operator= (const Server &);

            std::string BaseDN(void);

            bool Connect(const std::string & uri = "", bool ssl = false);
	    bool IsConnected(void) const;
            void Disconnect(void);

            bool Bind(void);
            bool Bind(const std::string & bind_dn, const std::string & bind_pw);
	    bool IsBinded(void) const;
            void Unbind(void);

            bool Ping(void) const;

            ListEntries Search(const std::string & base, const Scope & scope = ScopeBase, const std::string & filter = "");
            ListEntries Search(const std::string & base, const Scope & scope, const std::string & filter, const ListAttrs & attrs);

            bool Add(const Entry & entry);
            bool Modify(const Entry & entry);
            bool Compare(const std::string & dn, const std::string & attr, const std::string & val) const;
            bool Delete(const std::string & dn);
            bool ModDN(const std::string & dn, const std::string & newdn);

            int Error(void) const;
            const char* Message(void) const;

	protected:

            LDAP*			ldap_object;
	    int				ldap_errno;

	    bool			binded;
	    bool			connected;
    };

    std::ostream & operator<< (std::ostream &, const ListEntries &);

    class Pools : public std::list<Ldap::Server>
    {
	public:
	    Pools();
	    Pools(const Info &, const Info &);
	    Pools(const std::list<Info> &);

	    void AddPools(const std::list<Info> &);
	    void AddServer(const Info &);

            ListEntries Search(const std::string & base, const Scope & scope = ScopeBase, const std::string & filter = "");
            ListEntries Search(const std::string & base, const Scope & scope, const std::string & filter, const ListAttrs & attrs);

	protected:
	    std::list<Ldap::Server>::iterator current;
    };
}

#endif
