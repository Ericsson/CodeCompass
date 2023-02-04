# Microservice plugin
## Developers' documentation

The microservice plugin serves the purpose of analyzing Helm charts and storing data about the microservices and their relations in the database.

The plugin uses the _yaml-cpp_ library to parse YAML nodes. 
The analysis is executed file-by-file.
**This plugin heavily relies on the rules and conventions of Helm and Kubernetes.**

## YAML file analysis steps

The microservice parser receives one or more root directories of Helm charts. 
The directory is traversed in a recursive manner. 
The plugin is capable of multi-threaded analysis, where each thread receives a file for analysis.

### File purpose analysis

First the parser checks the purpose of the YAML file in analysis.
The following "purposes" (i.e. file types) are checked:

- _Helm charts_: the actual chart files which contains the name and other metadata of the microservice.
  Within this category, _integration charts_ and _subcharts_ are distinguished. 
  Processing is somewhat different based on the subtype (see details below).
  Files named _Chart.yaml_ or _Chart.yml_ are classified into this category.

- _Helm values_: files which contain vital information of microservice relations and default values (e.g. for environment variables).
  Files named _values.yaml_ or _values.yml_ are classified into this category.

- _Helm templates_: files that define microservice relations and metadata, such as ConfigMaps, Secrets, Certificates etc.
  Files that are inside a directory named _templates_ are classified into this category.

- _Docker compose files_: files named _compose.yml, compose.yaml, docker_compose.yml, or docker_compose.yaml_ are classified into this category.

- _CI files_: any file that does not belong to any category above and contains the substring _"ci"_, is classified as a continuous integration file.

_Note:_ At this point, the plugin is more specifically aimed at the analysis of Helm charts.
However, it can be used for generic YAML parsing as well, so the non-Helm types are left in.

### YAML node analysis

After the file type check, the keys and values in the file are analyzed in a recursive manner.
_yaml-cpp_ distinguishes the following node types: scalar, sequence, map, and null and undefined.
These types appear within the `SymbolType` field. 
The nodes are processed according to their type.

Generally, every node is broken down to the smallest, scalar node which is persisted in the database.
However, a scalar holds the type of node in which it was included: e.g. if a scalar is originally a key in a map,
its `AstType` will be `MAP`.

In order to provide better syntax highlight, a `SymbolType` field holds the place of a node:
`Key`, `Value`, `NestedKey`, `NestedValue`, or `Other`.

### Exception handling

If any error happens during file analysis, 
