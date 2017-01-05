\mainpage Python data model overview
\tableofcontents


Python Parser data model provides Python specific data in a coherent way
about the parsed project using ODB ORM tool to Python service. 

The main design goal was create such a data structure, which easy to integrate
into both of PySonar2 data model and CodeCompass service based system. We
striving to create a flexible, expandable model which allows to define
simple and fast queries in service.



\section tables Tables

In one of the last steps of PySonar2`s running, all of the important information
are send to persist through Thrift API. These informations are organized into
ODB tables named with the "Python" prefix.

According to the main concept of design, the most common tables is
[PythonAstNode](#pythonastnode) which stores the all of the used ast node in the
project with its most general properties. The remaining tables extend these
properties with additional information and relationships between nodes. 


\subsection pythonastnode PythonAstNode

The table contains a set of the useful ast nodes with its general properties.

PythonAstNode table stores the elements marked with type Name in the Python
abstract syntactic tree of the project. Because of all of the tables refer these
nodes through their ids the table has a core role in making relationship between
tables.


#### Table schema ####

The columns of the table are:
  - id
  - name
    \n Stores the identifier of the given ast node which is used in the Python
    source code.
  - abv_qname
    \n Abbreviation qualified name contains the qname without calculated project
    directory prefix.
  - ast_type
    \n Shows the role of the ast node. It also indicates in which table has an
    extender record of the ast node.
  - location
    \n The exact position of the ast node in the parsed source code.
  - base_binding
    \n If there is an extender binding for the given ast node, this value contains
    its binding id, otherwise its null. (See below more about [bindings](#pythonbinding).)
  - container_binding
    \n Refers that binding via its id which encloses the given ast node.
  - global_write
    \n Its a logical value, which is only used when reads/writes determination
    is necessary in case of global variables.


#### Common queries and usages ####

- In practice PythonAstNode table is used most. With this table easy to determine
  on which ast node was clicked in the UI.
- After getting the appropriate ast node examine its `ast_type` we can specify
  the next action(s).
- `abv_qname` is only used in the context menu, if more than one definitions
  should be listed.
- `global_write` is only used when reads/writes of a global variable should be
  determine.


#### Limitations and future plans ####

`global_write` field may violate the loose coupling, however it was inevitable
because PySonar2 handles differently the references (see below in
[PythonReference](#pythonreference) paragraph) of global variables. In this case
only the first assignment of the global variable wrapped into a binding.
Moreover we can not delegate this filed into PythonVariable table, because its
id field should refer a binding id.




\subsection pythonbinding PythonBinding

A binding extends a PythonAstNode with addition information. These information
typically concern to a definition or an assignment of an entity in the Python
source code.


#### Table schema ####

The columns of the table are:
  - id
  - name
    \n Same as name in [PythonAstNode](#pythonastnode) table.
  - formatted_qname
    \n It is a name qualified by replaced the path prefix of mangled_name with
    `libDir` or `projectDir`. It is used in context menu.
  - mangled_name
    \n It is a full qualified name using by PySonar2, which identifies the given
    binding.
  - kind
    \n It refers the type of the binding. (It is also used by PySonar2.)
  - type
    \n A string indicates the type of the entity in the Python source code. For
    example it can be the type signature of a Python function or the possible
    type of a variable.
  - location
    \n Since typically binding has body (e.x.: in case of a method), the binding
    location indicate the full length of the entity with its body in the Python 
    source code.
  - documentation
    \n Python doc string.


####  Common queries and usages ####

PythonBinding table is mostly use for determine the definition of an entity and
its properties, like location.


#### Limitations and future plans ####

The binding as definition is too low lever construction in the view of
CodeCompass services. In the future it should be replaced such a constructions
like [PythonVariableRef](#pythonvariableref) table implements.



\subsection pythonattribure PythonAttribute

The table contains information about which attribute belongs to which class.


#### Table schema ####

The columns of the table are:
  - id
    \n Auto incremented id.
  - attribute
    \n Stores an attribute binding id.
  - target
    \n It holds the id of the owner class binding.
    (Note: Since target has to be a class definition, we assume that its id also
    take place in [PythonClassDef](#pythonclassdef) table.)



\subsection pythonclassdef PythonClassDef

It holds additional information about class definitions.


#### Table schema ####

The columns of the table are:
  - id
    \n Auto incremented id.
  - target
    \n Class definition binding id.
  - constructor
    \n Method binding id which refers to the constructor.



\subsection pythondecorator PythonDecorator

It associates decorators to its owner entities.


#### Table schema ####

The columns of the table are:
  - id
  \n Owner ast node id.
  - target
  \n The id of decorated ast node.
  - value
  \n Textual name of the decorator.



\subsection pythonfunctioncall PythonFunctionCall

The table indicates how many arguments are used in a function call.


#### Table schema ####

The columns of the table are:
  - id
    \n Call ast node id.
  - arg_num
    \n Number of used arguments.


####  Common queries and usages ####

The table is good for when we would like to determine the possible unresolved
usages of a function definition. The table allows us to list only such function
calls which matches with the signature of function definition.



\subsection pythonfunctiondef PythonFunctionDef

The table stores the functions definition with its special properties.


#### Table schema ####

The columns of the table are:
  - id
    \n Binding id of function definition.
  - member_of
    \n If the function is a method, the field refers a class definition ast node.
  - min_param_num
    \n The minimum number with the function is callable.
  - max_param_num
    \n The maximum number with the function is callable.



\subsection pythonfunctionparam PythonFunctionParam

This table contains the parameter list of function definitions.


#### Table schema ####

The columns of the table are:
  - id
    \n Parameter binding id.
  - target
    \n Function definition binding id.



\subsection pythoninheritance PythonInheritance

The table contains inheritance hierarchy.


#### Table schema ####

The columns of the table are:
  - id
    \n Auto incremented id.
  - target
    \n Class definition binding id which represents the subclass.
  - base
    \n Class definition binding id which represents the superclass.
  - kind
    \n The kind shows extra information about the remaining record of the table.
    It defines the type of the inheritance relationship:
      - Simple
        \n It indicates single inheritance.
      - Multiple
        \n It indicates multiple inheritance.
      - Missing
        \n An inheritance kind is `Missing`, if there is no found PythonBinding
        for the base class. (It can happen if the base class is unresolved or
        is a native C class.)



\subsection pythonreference PythonReference

The table contains usage information of bindings. If in the Python source code
there is an usage of an another entity (for example a printing of a variable),
then that usAge is registered as reference to the appropriate biding.

To understand the basic concept behind bindings and their references, lets take
a look to the example below:

~~~~~~~~~~~~~{.py}
def foo():
  pass

var = 42
print(var)

var = True
if var:
  foo()
~~~~~~~~~~~~~

As the example shows, first we define a function named `foo`. For this ast node
a binding is created extending the node with appropriate information. Then we
create a variable `var` assigning an integer value to it. This assignment results
a new binding. When the `var` is given to the `print` function as its argument,
the argument will be a reference of the binding, which created by the assignment.
In the same way the `var` variable taking place in the if condition is a reference
of the appropriate binding, which is now created in the 7. line. The `foo()` call
is also a reference of the its previous function definition.

#### Table schema ####

The columns of the table are:
  - id
    \n Auto incremented id.
  - node
    \n Id of reference node.
  - binding
    \n Binding id.
  

####  Common queries and usages ####

This table is good for determining usage relationship between bindings and ast
nodes. For getting the correct result some ODB views should be used, since there
are some cases when a bunch of different usage handled together, however they
belong to different bindings.
For example in the above example, selecting `var` variable should result
highlighting all the `var` occurrences.

#### Limitations and future plans ####

This binding-reference construction is heavily used by PySonar2 during the
parsing process. This design is unnecessary complicated for CodeCompass services,
and because of its low level design, it results inconvenient usage of ODB ORM
tool. In the future it will nice to have if it would be replaced such a
constructions like [PythonVariableRef](#pythonvariableref) implements.



\subsection pythonunknown PythonUnknown

This table contains unresolved attributes and method calls, e.g.: `x.y` where
`y` is unresolved.

During the parsing process there can be some nodes, which can not be resolved
because of several reasons. The most common failure appears when an object of
which some member is accessed via dot operator, is ambiguous because in this 
case the member can not be associated to a binding. These members are stored in
the PythonUnknown table.

#### Table schema ####

The columns of the table are:
  - id
    \n Id of PythonAstNode, which refers the member.
  - target
    \n The object, on which the member is accessed.
  - name
    \n Name of the accessed member.
  - kind
    It shows the member kind:
      - Attribute
        \n The accessed member is an attribute.
      - Function
        \n The accessing is a function call.
      - Unknown
        \n There is no information about the member kind.



\subsection pythonvariable PythonVariable

The table contains information about global and local variables.

#### Table schema ####

The columns of the table are:
  - id
    \n Variable binding id.
  - target
    \n Id of the container binding, for which the variable is local or global.
  - is_global
    \n A boolean value indicating the variable is global or not.



\subsection pythonvariableref PythonVariableRef

This table encapsulates the attributes and provides a full view of its usages.
It distinguishes attributes definition, read and write from each other. For
identifying attributes and its related usages, a unique mangled name is used.

The definition is the first assignment according to the running flow, 
which is takes place in the topmost scope in the class definition or its
constructor. Moreover we assume that there is only one definition of every 
attributes.


#### Table schema ####

The columns of the table are:
  - id
    \n Auto incremented id.
  - mangledName
    \n Unique qualified name.
  - astNode
    \n Id of the attribute ast node.
  - refType
    \n Indicates the type of attribute occurrence. It can take value of:
      - Unset
        \n Indicates error.
      - Definition
      - Read
      - Write



#### Limitations and future plans ####

The used design of PythonVariableRef table should be taken as a model for
replacing low level [binding-reference](#pythonreference) construction.