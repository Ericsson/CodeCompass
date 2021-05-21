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

#include <iostream>
#include <cstring>
#include <sstream>
#include <iomanip>
#include <iterator>
#include <functional>
#include <algorithm>
#include "cldap_mod.h"

char* StrDup(const char* str)
{
    size_t len = strlen(str);
    char*  res = new char [len + 1];
    std::copy(str, str + len, res);
    res[len] = 0;
    return res;
}

berval* BervalNew(const char* val, size_t len)
{
    berval* bv = NULL;

    if(len && val)
    {
	bv = new berval();

        bv->bv_len = len;
        bv->bv_val = new char [len];
        std::memcpy(bv->bv_val, val, len);
    }

    return bv;
}

void BervalDelete(berval* bv)
{
    if(bv)
    {
	if(bv->bv_val)
	{
	    delete [] bv->bv_val;
	    bv->bv_val = NULL;
	}
        bv->bv_len = 0;
	delete bv;
    }
}

Ldap::ModBase::ModBase(int op, const char* type)
{
    val.mod_op = op;
    val.mod_type = type ? StrDup(type) : NULL;
    val.mod_bvalues = NULL;

    mod_vals_size = 0;
}

bool Ldap::ModBase::IsBinary(void) const
{
    return val.mod_op | LDAP_MOD_BVALUES;
}

bool Ldap::ModBase::IsOperation(int op) const
{
    return (val.mod_op & LDAP_MOD_OP) == op;
}

bool Ldap::ModBase::IsType(const std::string & str) const
{
    return 0 == str.compare(val.mod_type);
}

const LDAPMod* Ldap::ModBase::toLDAPMod(void) const
{
    return & val;
}

bool Ldap::ModStr::Append(const char* str, size_t len)
{
    // create
    if(! val.mod_values)
    {
	mod_vals_size = 4;
	val.mod_values = new char* [mod_vals_size];
	std::memset(val.mod_values, 0, sizeof(char*) * mod_vals_size);
    }

    // find
    char** itbeg = val.mod_values;
    char** itend = val.mod_values + mod_vals_size - 1; // skip last: always is NULL
    char** itcur = std::find(itbeg, itend, static_cast<char*>(0));

    // resize
    if(itcur == itend)
    {
	size_t new_vals_size = mod_vals_size * 2;
	char** new_values = new char* [new_vals_size];
	std::memset(new_values, 0, sizeof(char*) * new_vals_size);
	std::copy(itbeg, itend, new_values);
	delete [] val.mod_values;
	val.mod_values = new_values;
	mod_vals_size = new_vals_size;

	return Append(str, len);
    }

    // add
    *itcur = StrDup(str);
    return true;
}

void Ldap::ModStr::Append(const char* str)
{
    if(str) Append(str, strlen(str));
}

void Ldap::ModStr::Clear(void)
{
    if(val.mod_values)
    {
	for(char** sval = val.mod_values; sval && *sval; ++sval) delete [] *sval;
	delete [] val.mod_values;
	val.mod_values = NULL;
    }

    if(val.mod_type)
    {
	delete [] val.mod_type;
	val.mod_type = NULL;
    }

    val.mod_op = 0;
}

std::string Ldap::ModStr::GetStringValue(void) const
{
    std::string res;

    if(val.mod_values && val.mod_values[0])
	res.assign(val.mod_values[0]);

    return res;
}

std::vector<std::string> Ldap::ModStr::GetStringValues(void) const
{
    std::vector<std::string> res;
    const char* const* vals = val.mod_values;

    while(vals && *vals)
    {
        res.push_back(std::string(*vals));
        ++vals;
    }

    return res;
}

std::list<std::string> Ldap::ModStr::GetStringList(void) const
{
    std::list<std::string> res;
    const char* const* vals = val.mod_values;

    while(vals && *vals)
    {
        res.push_back(std::string(*vals));
        ++vals;
    }

    return res;
}

std::vector<char> Ldap::ModStr::GetBinaryValue(void) const
{
    std::vector<char> res;

    if(val.mod_values && val.mod_values[0])
        res.assign(val.mod_values[0], val.mod_values[0] + strlen(val.mod_values[0]));

    return res;
}

std::vector< std::vector<char> > Ldap::ModStr::GetBinaryValues(void) const
{
    std::vector< std::vector<char> > res;
    const char* const* vals = val.mod_values;

    while(vals && *vals)
    {
        res.push_back(std::vector<char>(*vals, *vals + strlen(*vals)));
        ++vals;
    }

    return res;
}

std::list< std::vector<char> > Ldap::ModStr::GetBinaryList(void) const
{
    std::list< std::vector<char> > res;
    const char* const* vals = val.mod_values;

    while(vals && *vals)
    {
        res.push_back(std::vector<char>(*vals, *vals + strlen(*vals)));
        ++vals;
    }

    return res;
}

bool Ldap::ModBin::Append(const char* ptr, size_t len)
{
    // create
    if(! val.mod_bvalues)
    {
	mod_vals_size = 4;
	val.mod_bvalues = new berval* [mod_vals_size];
	std::memset(val.mod_bvalues, 0, sizeof(berval*) * mod_vals_size);
    }

    // find
    berval** itbeg = val.mod_bvalues;
    berval** itend = val.mod_bvalues + mod_vals_size - 1; // skip last: always is NULL
    berval** itcur = std::find(itbeg, itend, static_cast<berval*>(0));

    // resize
    if(itcur == itend)
    {
	size_t new_vals_size = mod_vals_size * 2;
	berval** new_bvalues = new berval* [new_vals_size];
	std::memset(new_bvalues, 0, sizeof(berval*) * new_vals_size);
	std::copy(itbeg, itend, new_bvalues);
	delete [] val.mod_bvalues;
	val.mod_bvalues = new_bvalues;
	mod_vals_size = new_vals_size;

	return Append(ptr, len);
    }

    // add
    *itcur = BervalNew(ptr, len);
    return true;
}

void Ldap::ModBin::Clear(void)
{
    if(val.mod_bvalues)
    {
	for(berval** bval = val.mod_bvalues; bval && *bval; ++bval) BervalDelete(*bval);
	delete [] val.mod_bvalues;
	val.mod_bvalues = NULL;
    }

    if(val.mod_type)
    {
	delete [] val.mod_type;
	val.mod_type = NULL;
    }

    val.mod_op = 0;
}

std::string Ldap::ModBin::GetStringValue(void) const
{
    std::string res;

    if(val.mod_bvalues && val.mod_bvalues[0])
    {
	const berval* ptr = val.mod_bvalues[0];
	if(ptr->bv_len && ptr->bv_val) res.assign(ptr->bv_val, ptr->bv_len);
    }

    return res;
}

std::vector<std::string> Ldap::ModBin::GetStringValues(void) const
{
    std::vector<std::string> res;
    const berval* const* bvals = val.mod_bvalues;

    while(bvals && *bvals)
    {
        res.push_back(std::string((*bvals)->bv_val, (*bvals)->bv_len));
        ++bvals;
    }

    return res;
}

std::list<std::string> Ldap::ModBin::GetStringList(void) const
{
    std::list<std::string> res;
    const berval* const* bvals = val.mod_bvalues;

    while(bvals && *bvals)
    {
        res.push_back(std::string((*bvals)->bv_val, (*bvals)->bv_len));
        ++bvals;
    }

    return res;
}

std::vector<char> Ldap::ModBin::GetBinaryValue(void) const
{
    std::vector<char> res;

    if(val.mod_bvalues && val.mod_bvalues[0])
    {
	const berval* ptr = val.mod_bvalues[0];
	if(ptr->bv_len && ptr->bv_val) res.assign(ptr->bv_val, ptr->bv_val + ptr->bv_len);
    }

    return res;
}

std::vector< std::vector<char> > Ldap::ModBin::GetBinaryValues(void) const
{
    std::vector< std::vector<char> > res;
    const berval* const* bvals = val.mod_bvalues;

    while(bvals && *bvals)
    {
        res.push_back(std::vector<char>((*bvals)->bv_val, (*bvals)->bv_val + (*bvals)->bv_len));
        ++bvals;
    }

    return res;
}

std::list< std::vector<char> > Ldap::ModBin::GetBinaryList(void) const
{
    std::list< std::vector<char> > res;
    const berval* const* bvals = val.mod_bvalues;

    while(bvals && *bvals)
    {
        res.push_back(std::vector<char>((*bvals)->bv_val, (*bvals)->bv_val + (*bvals)->bv_len));
        ++bvals;
    }

    return res;
}

std::ostream & Ldap::operator<< (std::ostream & os, const ModStr & mod)
{
    if(mod.val.mod_type)
    {
	const char* const* vals = mod.val.mod_values;

	while(vals && *vals)
	{
	    os << mod.val.mod_type << ": " << *vals << std::endl;
    	    ++vals;
	}
    }

    return os;
}

std::ostream & Ldap::operator<< (std::ostream & os, const ModBin & mod)
{
    if(mod.val.mod_type)
    {
	const berval* const* bvals = mod.val.mod_bvalues;

	while(bvals && *bvals)
	{
	    os << Base64::StringWrap(mod.val.mod_type, std::string((*bvals)->bv_val, (*bvals)->bv_len)) << std::endl;
    	    ++bvals;
	}
    }

    return os;
}

namespace Base64
{
    bool opt_binary_only = false;

    void SetBinaryOnly(bool f)
    {
	opt_binary_only = f;
    }

    int encodeChar(int v)
    {
	// 0 <=> 25
	if(v <= ('Z' - 'A'))
    	    return v + 'A';

	// 26 <=> 51
	if(26 <= v && v <= 26 + ('z' - 'a'))
    	    return v + 'a' - 26;

	// 52 <=> 61
	if(52 <= v && v <= 52 + ('9' - '0'))
    	    return v + '0' - 52;

	if(v == 62)
    	    return '+';

	if(v == 63)
	    return '/';

	return 0;
    }

    int decodeChar(int v)
    {
	if(v == '+')
    	    return 62;

	if(v == '/')
    	    return 63;

	if('0' <= v && v <= '9')
    	    return v - '0' + 52;

	if('A' <= v && v <= 'Z')
    	    return v - 'A';

	if('a' <= v && v <= 'z')
    	    return v - 'a' + 26;

	return 0;
    }

    std::vector<char> encode(const char* src, size_t srcsz)
    {
	std::vector<char> dst;

	size_t dstsz = 4 * srcsz / 3 + 1;
	dst.reserve(dstsz);

	for(size_t ii = 0; ii < srcsz; ii += 3)
	{
    	    int b1 = static_cast<unsigned char>(src[ii]);
    	    int b2 = static_cast<unsigned char>(ii + 1 < srcsz ? src[ii + 1] : 0);
    	    int b3 = static_cast<unsigned char>(ii + 2 < srcsz ? src[ii + 2] : 0);

    	    int triple = (b1 << 16) | (b2 << 8) | b3;

    	    dst.push_back(encodeChar(0x3F & (triple >> 18)));
    	    dst.push_back(encodeChar(0x3F & (triple >> 12)));
    	    dst.push_back(ii + 1 < srcsz ? encodeChar(0x3F & (triple >> 6)) : '=');
    	    dst.push_back(ii + 2 < srcsz ? encodeChar(0x3F & triple) : '=');
	}

	return dst;
    }

    std::vector<char> decode(const char* src, size_t srcsz)
    {
	std::vector<char> dst;

	if(0 < srcsz && srcsz % 4 == 0)
	{
    	    size_t dstsz = 3 * srcsz / 4;

    	    if(src[srcsz - 1] == '=') dstsz--;
    	    if(src[srcsz - 2] == '=') dstsz--;

    	    dst.reserve(dstsz);

    	    for(size_t ii = 0; ii < srcsz; ii += 4)
    	    {
        	int sextet_a = decodeChar(src[ii]);
        	int sextet_b = decodeChar(src[ii + 1]);
        	int sextet_c = decodeChar(src[ii + 2]);
        	int sextet_d = decodeChar(src[ii + 3]);

        	int triple = (sextet_a << 18) + (sextet_b << 12) + (sextet_c << 6) + sextet_d;

        	if(dst.size() < dstsz) dst.push_back((triple >> 16) & 0xFF);
        	if(dst.size() < dstsz) dst.push_back((triple >> 8) & 0xFF);
        	if(dst.size() < dstsz) dst.push_back(triple & 0xFF);
    	    }
	}
	else
	{
    	    std::cerr << "Tools::base64Decode: " << "incorrect size buf: " << srcsz;
	}

	return dst;
    }

    std::string StringWrap(const std::string & type, const std::string & value)
    {
	std::ostringstream os;

	unsigned int wrap = 76;
        bool binary = std::string::npos != type.find(";binary");
	bool base64 = binary;

    	os << type << ":";

	// greater wrap
	if(!opt_binary_only && !base64)
	    base64 = os.str().size() + 1 + value.size() > wrap;

	// find not ascii
	if(!base64)
	{
	    auto it = value.begin();
	    if(opt_binary_only)
	    {
		for(; it != value.end(); ++it) if(0 <= *it && std::iscntrl(*it)) break;
	    }
	    else
	    {
		for(; it != value.end(); ++it) if(0 == std::isgraph(*it)) break;
	    }
	    if(it != value.end()) base64 = true;
	}

	if(base64)
	{
	    // double ::
	    os << ":" << " ";
    	    std::vector<char> base64 = Base64::encode(value.c_str(), value.size());
    	    std::copy(base64.begin(), base64.end(), std::ostream_iterator<char>(os, ""));
	}
	else
    	    os << " " << value;

	std::string str = os.str();

	if(wrap && base64 && str.size() > wrap)
	{
    	    size_t pos = 0;

	    os.str("");
    	    os << str.substr(pos, wrap) << std::endl;
            pos += wrap;

            while(pos < str.size())
            {
        	os << " " << str.substr(pos, wrap - 1);
                pos += wrap - 1;
		if(pos < str.size()) os << std::endl;
            }
	}

	return os.str();
    }
}
