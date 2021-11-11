package service.srcjava.enums;

public enum DiagramType {
  METHOD_CALL, /*!< In the method call diagram the nodes are methods and
      the edges are the method calls between them. The diagram also displays
      some dynamic information such as virtual method calls. */

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
}
