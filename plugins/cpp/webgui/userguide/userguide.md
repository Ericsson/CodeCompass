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