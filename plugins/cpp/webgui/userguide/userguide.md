\section userguide_cpp Cpp Plugin

\subsection userguide_cpp_diagrams_functioncall Function Call Diagram
Function call diagram shows the called and caller functions of the current one
(yellow node). Called functions are drawn in blue and connected with blue
arrows, like caller functions and their arrows are red.

In C++ it is possible to call virtual functions which may have several
overridings. In this case the virtual function has diamond shape from which
overrider functions point out with dashed arrows. (Callers of a virtual
function are also connected with dashed arrow).

![Diagram view](images/cpp_function_call_diagram.png)

\subsection userguide_cpp_diagrams_detailt_uml_class Detailed Class Diagram

This is a classical UML class diagram for the selected class and its direct
children and parents. The nodes contain the methods and member variables
with their visibility.

![Diagram view](images/cpp_detailed_class_diagram.png)

Visibilities:
 - <span style="color:green">+</span> public
 - <span style="color:blue">#</span> protected
 - <span style="color:red">-</span> private

The meaning of special text format:
 - <b>static methods and variables</b>
 - <i>virtual methods</i>

\subsection userguide_cpp_diagrams_collaboration Class Collaboration Diagram

This returns a class collaboration diagram which shows the individual class
members and their inheritance hierarchy.

![Diagram view](images/cpp_collaboration_diagram.png)

\subsection userguide_cpp_cluster Navigation aid based on linkage resolution

Sometimes projects contain ambiguity when viewed as a whole, e.g. there can be
multiple <i>f()</i> functions defined.

![Jump to definition window with nagivation aid](images/cpp_cluster_refpage.png)

The above view shows how CodeCompass aids navigation from a call to <i>f()</i>
to the possible (multiple) definitions.

 - A file indicated in <span style="color: #4F7942; font-weight: bold">bold,
green</span> is a file which contains a definition that has been built into the
same binary as the call you are jumping from.
 - A file indicated in <span style="color: #696969; font-style: italic">grey,
italic</span> is a file which contains a definition matching the call you are
jumping from, but a common binary for the two has not been found.

In most cases, the actual definition you want to see, that is, the definition
which contains the executed code to the call, is in the
<span style="color: #4F7942; font-weight: bold">bold, green</span> files.

![InfoTree window with navigation aid](images/cpp_cluster_infotree.png)

The same highlighting rules apply to the Info Tree as well.
