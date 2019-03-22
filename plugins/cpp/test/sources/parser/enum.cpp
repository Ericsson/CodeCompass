enum Enumeration : short;
enum Enumeration : short
{
  First,
  Second,
  Third
};

Enumeration globalEnum;

Enumeration enumUserFunction(
  Enumeration funcParamEnumeration)
{
  Enumeration localEnumeration = First;
  enumUserFunction(Enumeration());
  typedef Enumeration Enumeration2;

  try
  {
    throw localEnumeration;
  }
  catch (Enumeration e)
  {
  }

  return localEnumeration;
}
