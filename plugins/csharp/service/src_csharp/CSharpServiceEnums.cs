enum ReferenceType
  {
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

    CALLS_OF_THIS, /*!< Get calls of a function. */

    CALLEE, /*!< Get called functions definitions. WARNING: If the definition of
      the AST node is not unique then it returns the callees of one of them. */

    CALLER, /*!< Get caller functions. */

    VIRTUAL_CALL, /*!< A function may be used virtually on a base type object.
      The exact type of the object is based on dynamic information, which can't
      be determined statically. Weak usage returns these possible calls. */

    FUNC_PTR_CALL, /*!< Functions can be assigned to function pointers which
      can be invoked later. This option returns these invocations. */

    PARAMETER, /*!< This option returns the parameters of a function. */

    LOCAL_VAR, /*!< This option returns the local variables of a function. */

    RETURN_TYPE, /*!< This option returns the return type of a function. */

    OVERRIDE, /*!< This option returns the functions which the given function
      overrides. */

    OVERRIDDEN_BY, /*!< This option returns the overrides of a function. */

    USAGEREAD, /*!< This option returns the places where a variable is read. */

    WRITE, /*!< This option returns the places where a variable is written. */

    READ,
    TYPE, /*!< This option returns the type of a variable. */

    ALIAS, /*!< Types may have aliases, e.g. by typedefs. */

    INHERIT_FROM, /*!< Types from which the queried type inherits. */

    INHERIT_BY, /*!< Types by which the queried type is inherited. */

    DATA_MEMBER, /*!< Data members of a class. */

    METHOD, /*!< Members of a class. */

    FRIEND, /*!< The friends of a class. */

    UNDERLYING_TYPE, /*!< Underlying type of a typedef. */

    ENUM_CONSTANTS, /*!< Enum constants. */

    EXPANSION, /*!< Macro expansion. */

    UNDEFINITION, /*!< Macro undefinition. */

    EVALUATION, // LINQ evaluation
    
    DATA_MODIFICATION, // LINQ underlying datadtruct is modified

    CONSTRUCTOR,

    DESTRUCTOR,

    OPERATOR,

    ACCESSOR,

    DELEGATE,

    EVENT
  };

  enum FileReferenceType
  {
    INCLUDES, /*!< Included source files in the current source file after the
      inclusion directive. */

    TYPES, /*!< User defined data types such as classes, structs etc. */

    FUNCTIONS, /*!< Functions in the current source file. */

    MACROS, /*!< Macros in the current source file. */
  };

  enum DiagramType
  {
    FUNCTION_CALL, /*!< In the function call diagram the nodes are functions and
      the edges are the function calls between them. The diagram also displays
      some dynamic information such as virtual function calls. */

    DETAILED_CLASS, /*!< This is a classical UML class diagram for the selected
      class and its direct children and parents. The nodes contain the methods
      and member variables with their visibility. */

    CLASS_OVERVIEW, /*!< This is a class diagram which contains all classes
      which inherit from the current one, and all parents from which the
      current one inherits. The methods and member variables are node included
      in the nodes, but the type of the member variables are indicated as
      aggregation relationship. */

    CLASS_COLLABORATION, /*!< This returns a class collaboration diagram
      which shows the individual class members and their inheritance
      hierarchy. */

    COMPONENT_USERS, /*!< Component users diagram for source file S shows which
      source files depend on S through the interfaces S provides. */

    EXTERNAL_DEPENDENCY, /*!< This diagram shows the module which directory
      depends on. The "depends on" diagram on module A traverses the
      subdirectories of module A and shows all directories that contain files
      that any of the source files in A includes. */

    EXTERNAL_USERS, /*!< This diagram shows directories (modules) that are
      users of the queried module. */

    INCLUDE_DEPENDENCY, /*!< This diagram shows of the `#include` file
      dependencies. */

    INTERFACE, /*!< Interface diagram shows the used and provided interfaces of
      a source code file and shows linking information. */

    SUBSYSTEM_DEPENDENCY, /*!< This diagram shows the directories relationship
      between the subdirectories of the queried module. This diagram is useful
      to understand the relationships of the subdirectories (submodules)
      of a module. */
  };