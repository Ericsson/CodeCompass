// $Id$
// Created by Aron Barath, 2013

#ifndef MODEL_JAVA_JAVAMODIFIERS_H
#define MODEL_JAVA_JAVAMODIFIERS_H

namespace cc
{
namespace model
{

struct JavaModifiers
{
  static const int Public       = 0x00000001;
  static const int Protected    = 0x00000002;
  static const int Private      = 0x00000004;
  static const int Static       = 0x00000008;
  static const int Abstract     = 0x00000010;
  static const int Final        = 0x00000020;
  static const int Native       = 0x00000040;
  static const int Synchronized = 0x00000080;
  static const int Transient    = 0x00000100;
  static const int Volatile     = 0x00000200;
  static const int Strictfp     = 0x00000400;
};

} // model
} // cc

#endif // MODEL_JAVA_JAVAMODIFIERS_H

