:exclamation::exclamation::exclamation: **Warning** :exclamation::exclamation::exclamation:
=================================================================

Major changes happened in CodeCompass architecture which caused a huge
refactoring of the tool. The previous working version of CodeCompass is
available in the branch `Earhart` and the current version is being
developed on the `master` branch. The new version doesn't contain all
the features but we are continuously porting them.

**It is important that the Git hashes changed which breaks your local
repository. Please fetch the new repository again.**

You should use the `Earhart` version, but that won't be developed in the
future except for minor fixes. Bugfixes and further developments will
happen on `master` branch. User and development documentation will come
soon.

CodeCompass
===========

CodeCompass is a pluginable code comprehension tool.

<img src="https://raw.githubusercontent.com/Ericsson/codecompass/master/webgui/images/logo.png" width="250px;"/>

# :bulb: Live demo on Xerces source code available [here](http://modelserver.inf.elte.hu:34540/#wsid=xerces)

## Screenshots

<img src="https://raw.githubusercontent.com/Ericsson/codecompass/Earhart/img/screenshot1.jpg" height="100px" />
<img src="https://raw.githubusercontent.com/Ericsson/codecompass/Earhart/img/screenshot2.jpg" height="100px" />
<img src="https://raw.githubusercontent.com/Ericsson/codecompass/Earhart/img/screenshot3.jpg" height="100px" />

Features
--------

- User friendly web UI
- Fast navigation among source code elements
- Several languages supported
- Deep parsing for C, C++, Java and more is coming
- Many diagrams: call path, inheritance, aggregation, CodeBites, etc.
- Scalable: Quick response time even for large (100Mb) source code base
