package service.srcjava.enums;

public enum ReferenceType {
  DEFINITION, /*!< By this option the definition(s) of the AST node can be
    queried. However according to the "one definition rule" a named entity
    can have only one definition, in a parsing several definitions might be
    available. This is the case when the project is built for several targets
    and in the different builds different definitions are defined for an
    entity (e.g. because of an #ifdef section). */

  DECLARATION, /*!< By this options the declaration(s) of the AST node can be
    queried. */

  USAGE, /*!< By this option the usages of the AST node can be queried, i.e.
    the nodes of which the entity hash is identical to the queried one. */

  THIS_CALLS, /*!< Get function calls in a function. WARNING: If the
    definition of the AST node is not unique then it returns the callees of
    one of them. */

  CALLEE, /*!< Get called functions definitions. WARNING: If the definition of
    the AST node is not unique then it returns the callees of one of them. */

  CALLER, /*!< Get caller functions. */

  PARAMETER, /*!< This option returns the parameters of a function. */

  LOCAL_VAR, /*!< This option returns the local variables of a function. */

  RETURN_TYPE, /*!< This option returns the return type of a function. */

  OVERRIDE, /*!< This option returns the functions which the given function
    overrides. */

  OVERRIDDEN_BY, /*!< This option returns the overrides of a function. */

  IMPLEMENT, /*!< This option returns the functions which the given function
    implements. */

  IMPLEMENTED_BY, /*!< This option returns the implementations of a function. */

  READ, /*!< This option returns the places where a variable is read. */

  WRITE, /*!< This option returns the places where a variable is written. */

  TYPE, /*!< This option returns the type of a variable. */

  INHERIT_FROM, /*!< Types from which the queried type inherits. */

  INHERIT_BY, /*!< Types by which the queried type is inherited. */

  DATA_MEMBER, /*!< Data members of a class. */

  INNER_TYPE, /*!< Inner classes of a class. */

  INITIALIZER, /*!< Initializers of a class. */

  CONSTRUCTOR, /*!< Constructors of a class. */

  METHOD, /*!< Members of a class. */

  ENUM_CONSTANTS, /*!< Enum constants. */
}
