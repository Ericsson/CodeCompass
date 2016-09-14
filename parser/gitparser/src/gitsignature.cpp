/*
 * gitsignature.cpp
 *
 *  Created on: Feb 16, 2014
 *      Author: cseri
 */

#include "gitparser/gitsignature.h"

#include "git2.h"

namespace cc
{
namespace parser
{

GitSignature::GitSignature(const git_signature* sig) : name(sig->name), email(sig->email), when(sig->when) { }

} /* namespace parser */
} /* namespace cc */
