#
# Useguide documentation for CodeCompass
# Note: start every section id with 'userguide_<pluginname>' for uniqueness.
#

\page userguide User Guide
\tableofcontents

CodeCompass is a static code analyzer tool which is planned to understand large
code bases. In this document we introduce the usage of CodeCompass.

\section userguide_ui_structure User interface structure
The user interface has three main parts:

- Header
- Accordion modules on the left side
- Center modules

![Home](images/home.png)

\subsection userguide_accordion_modules Accordion Modules

\subsubsection userguide_filemanager_module File manager
The [filemanager](#userguide_filemanager) shows the directory hierarchy and
files of the project.

![File Manager](images/filemanager.png)

\subsubsection useguide_infotree_module Info Tree
Info Tree shows language specific data about different language elements such as
macros, callees/caller functions of a function, class hierarchy, function pointer
analysis, variable reads/writes, etc.

![Info Tree](images/infotree.png)

\subsubsection userguide_queryresults Query Results
In Query Results panel one can see the results of text search and other kind of
reference search (definition, usage, etc.).

\subsection userguide_center_modules Center Modules

\subsubsection userguide_textview Text view
The text view shows the source code of an opened file.
It supports context sensitive popup menu for different language elements.
This means that the content of this menu depends on the clicked token.
For example if the token belongs to a function call, then in the context menu
you can see function related diagrams and other. On a node which belongs
to a user type, you can display class diagrams.

The text view can be opened by clicking on a file in the 
[filemanager](#userguide_filemanager). The text view has a header and a body
part. The header contains the opened file's name and path. The elements of this
path can be clicked which helps to jump to the corresponding node in the
[filemanager](#userguide_filemanager).

![Text view](images/textmodule.png)

\subsection userguide_infotree Info Tree in Source Code

Info Tree shows different information about given language elements such as
its name, qualified name, and place of definition. Further information are
described below depending on the element's type.

\subsubsection userguide_infotree_functions Functions
The following information are available in the info tree which belongs to a
function:
- Signature
- Return type
- Place of definition
- Places of declaraions
- Parameters
- Local variables
- Callers and callees
- Assignments to function parameters
- Overrides and Overriden by
- Usages
- Virtual call

![Info Tree](images/infotree_functions.png)

The subtree of function callers can be opened recursively thus building a call
chain of the given function.

\subsubsection userguide_infotree_variables Variables
Besides of the basic information about a variable (place of definition,
qualified name, etc.) you can list the places of reading and writing of the
given variable.

\subsubsection userguide_infotree_types Types
Info tree contains type specific information if it is called on a class/struct
name or enums.

In case of a class, one can find information in the Info Tree about inheritance
(from which classes this inherits and which classes inherit from this). The type
of inheritance (public, protected, private) is also shown.

Through Info tree one can query the types which are aliases of the selected one,
i.e. declared as a typedef.

One can also see the friends, methods and attributes of the given class.

Info tree also shows information about enumeration types. Choosing an enum, one
can see its enum constants, their numeric values, and the place of its
definition. Enum constants can also be selected, thus querying its numeric value
and places of usage and definition.

\subsection cppdiagramscodebites CodeBites
By CodeBites diagram one can inspect the source code without loosing the visited
path. CodeBites can be called on functions, variables, classes or macros. In
each case the definition of the element is placed in a graph (tree) node. Since
an ordinary source code viewer is placed in the graph node, the parts of it are
also clickable. On click event the definition of the clicked element is opened
under the current one. The nodes of CodeBites graph are resizable and closable.

![Info Tree](images/codebites.png)