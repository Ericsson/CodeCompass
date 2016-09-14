#ifndef CPPPROVIDERELATION_H
#define	CPPPROVIDERELATION_H

#include <model/file.h>
#include <odb/lazy-ptr.hxx>
#include <odb/nullable.hxx>

namespace cc
{
namespace model
{

#pragma db object
struct CppProvideRelation
{
  #pragma db id auto
  int id;
  
  #pragma db not_null
  odb::lazy_shared_ptr<model::File> interface;
  
  #pragma db not_null
  odb::lazy_shared_ptr<model::File> implementation;
  
  bool operator<(const CppProvideRelation& other) const
  {
    return std::make_pair(interface->id, implementation->id)
         < std::make_pair(other.interface->id, other.implementation->id);
  }
};

}
}

#endif