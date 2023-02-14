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
  These files are collected in a container for further analysis.

- _Helm templates_: files that define microservice relations and metadata, such as ConfigMaps, Secrets, Certificates etc.
  Files that are inside a directory named _templates_ are classified into this category.
  These files are collected in a container for further analysis.

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
its `AstType` will be `MAP`. Maps and sequences need to be parsed in slightly different ways, 
because their iterators are not interchangeable. In the end, all nodes are broken down to scalars,
which are then persisted in the database with their containing type.

In order to provide better syntax highlight, a `SymbolType` field holds the place of a node:
`Key`, `Value`, `NestedKey`, `NestedValue`, or `Other`.

The location of nodes within files is calculated by the `Mark()` method of _yaml_cpp_,
and the content of a node is extracted with the `Dump()` method.

### Exception handling

If any error happens during file analysis, the error will be saved as a `BuildLog` object. 
The erroneous file will remain in _Not parsed_ status.

Errors usually happen in Helm charts if the user did not execute the `helm template` command on the charts before parsing,
or when there are syntax errors in the file.

### Chart analysis

As mentioned above, Helm charts should be handled differently if they are integration charts or subcharts.

#### Integration charts

If the file is an integration chart - a chart which describes the entire software, not just a component -,
tha parser check if there is a _dependencies_ key in the file.
If not, the service that the file describes is registered in the database.
If there is, the listed microservices are registered in the database. 
If a listed service contains an _alias_ key, its value should be persisted as the name of the service.
Otherwise, the value of the obligatory _name_ key is the name of the service.

_Note: this approach might be refined in the future to adapt to more possible architecture description formats._

#### Subcharts

If a _Chart.yaml_ file is contained by a _charts_ directory, it should be considered a subchart
(i.e. a component in a microservice-based software).
The described microservice should be persisted in the database.

_Note: in case of parsing multiple projects, keep in mind that a full-value microservice (i.e.
a microservice which is not defined in a subchart, but not necessarily an integration service)
can define a component in within another project. Thus it can be a subchart in on project, and
an integration chart on its own at the same time._

### Template file analysis

This analysis is implemented in the `TemplateAnalyzer` class.

#### Dependencies

After basic parsing, the previously collected template files are analyzed to collect
microservice dependencies.
The content of each template file is investigated.
Any template object should be persisted in the database as a `HelmTemplate` object with
the file that it is defined in.
Based on the content and its format, the analysis branches off in the following directions:

- _Service dependency:_ if the value of the `kind` key in the template file is _Service_. 
  This type defines additional services that derive from the original service.
  In this case, the new service is persisted in the database as an _EXTERNAL_ service, and an edge
  is created between the two services. The defined service depends on the defining service.
- _Mount dependencies:_ these types are usually collected within big deployment files. 
  The dependencies listed as the value of a _volumes_ key should be analyzed.
  Please note that there can be multiple _volumes_ keys in a deployment file.
  We consider two subtypes:
  - _ConfigMaps_: these contain lots of important information that is needed for K8S pod definition.
  - _Secrets_: these contain confidential information within services.
  Both types can be identified by a `kind` key in the list.
- _Certificate dependencies:_ while the previous types could be identified by a `kind` key
  within a file, a certificate can be defined by several template types.
  An approximate heuristics is to check the value of the `kind` key for containing the
  "Certificate" or the "InternalUserCA" substring. (_WARNING:_ naming conventions may vary
  in every project.) Certificate dependencies define further secrets which should also
  be persisted in the database.

#### Resources

All template files are checked to see if they contain resource usage information.
The following resources are collected:

- _CPU:_ listed with `resources` and `requests` keys. This resource is calculated in
  the number of CPU cores which can be a fraction.
- _Memory:_ listed with `resources` and `requests` keys. Calculated in gigabytes.
- _Storage:_ listed within a `volumeClaimTemplates` list, with a `storage` key. Calculated in gigabytes.

### Analysis of _values_ files