namespace cpp cc.service.core
namespace java cc.service.core

/********************
 * Identifier types *
 ********************/

// On the server side we use a hash function to generate IDs to files, AST nodes
// and other entities. This hash function generates a 64bit integer hash.
// Unfortunately JavaScript has only one Number type which is for 64bit floating
// point numbers which cannot represent all 64bit integers. This is the reason
// why we transfer IDs as strings through the interface.

typedef string AstNodeId
typedef string FileId

/**********************
 * General Exceptions *
 **********************/

exception InvalidId
{
  1:string msg,
  2:optional FileId fid,
  3:optional AstNodeId nodeid
}

exception InvalidInput
{
  1:string msg
}

exception Timeout
{
  1:string msg
}

/******************************
 * File positions & locations *
 ******************************/

struct Position
{
  1:i32 line   = -1,
  2:i32 column = -1
}

struct Range
{
  1:Position startpos,
  2:Position endpos
}

struct FilePosition
{
  1:FileId file,
  2:Position pos
}

struct FileRange
{
  1:FileId file,
  2:Range range
}

/***************
 * Description *
 ***************/

struct Description
{
  1:i32 id;      /** Identifier of an option. */
  2:string name; /** Human readable name of an option. */
}
