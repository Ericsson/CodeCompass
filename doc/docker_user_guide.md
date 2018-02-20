# Using Docker in CodeCompass development

Why docker necessary? Because producing a development environment of CodeCompass
is a very long task with many traps. Some software component could not be
reached by usual way (installing them from packages.) For example LLVM with
compiled in RTTI support. (See ...)

We created an ubuntu based build system that can be run on any linux
distribution with less effort. We hope that the developers can be productive in
a shorter time in CodeCompass development. However the build system can be run
on other distributions than ubuntu, the executables are compiled for ubuntu.
The result of build may or may not run on another distribution. This solution
contains a runtime environment to try the compiled CodeCompass. This runtime
environment based on ubuntu too, so the CodeCompass can try without problem with
it. (See ...)

Additionally the image creation is an
exact definition how the build system should be set up when somebody want to try
to develop in a different distribution than ubuntu.

Using this way of CodeCompass development not requires full understanding of
Docker. The build steps wrapped with shell scripts which hides the dirty work.

## Installing Docker

On the developer host the Docker software should be installed. See

https://docs.docker.com/install/

## Create CodeCompass development environment

All tools that necessary to build CodeCompass, is encapsulated in a Docker
image. First, the developer image should be created on the developer machine.

To create docker image that contains all necessary tool to make a CodeCompass
binary:

```bash
git clone https://github.com/Ericsson/CodeCompass <cc_source_dir>
docker build --tag compass-devel <cc_source_dir>/scripts/docker/CC-Devel
```

This is a long running task, because a complete LLVM is necessary to compile
(among others). When it is built, it consumes about 4GB on the developer's host
machine.

Using "compass-devel" as image name is mandatory, because the scripts find the
image by this name.

Now, the build system is ready to make development cycles (modify-build-try) of
the CodeCompass.

## Compiling CodeCompass

There are three wrapper script in the
``<cc_source_dir>/scripts/docker/CC-Devel`` directory beside the ``Dockerfile``.
These scripts performs the development steps.

The builder container mount two directories while it is running. The first one
which contains CodeCompass source. The second one where the result of the
compilation (executables) will be placed. All scripts uses options to specify
them. But to simplify developer's life these scripts find environment variables
as defaults to command line parameters. So the developer can set the following environment variables in his/her shell.

- CC_SOURCE where the CodeCompass source resides
- CC_BUILD where the cmake generates the CodeCompass executables

Additionally the ``configurecc.sh`` uses the

- CC_BUILD_TYPE what defines build type as the cmake requires, for example
``Debug``

### Get CodeCompass source

If the latest master version is necessary to build then nothing to do here,
because that was downloaded to compile the ``compass-devel`` image before. If
other branch or other repository of the CodeCompass necessary to build then the
``fetchcc.sh`` script can be used. The ``fetchcc.sh`` script creates a directory
on the filesystem of the host machine and get the CodeCompass source into it.

```bash
<cc_source_dir>/scripts/docker/CC-devel/fetchcc.sh \
  -u <other_compass_repository_url> -s <cc_source_dir>
```

This command simply clones the CodeCompass source from a repository that the
``<url>`` determines. (This action can be performed on the host with native git
commands too.)

### Configure CodeCompass

The build system of the CodeCompass is ``cmake``. When the source is available,
cmake requires a configuration step what create makefiles. This task performed
by ``configurecc.sh`` script.

The directory pointed by ``CC_BUILD`` (or -o option) will be contain a ``build``
subdirectory that populated with files that cmake generates.

### Build CodeCompass

The ``buildcc.sh`` script performs the build and install steps of CodeCompass. If
something went wrong during build then the developer can edit the source of
CodeCompass on the host machine as him/her does an usual development environment.

After successful build the directory pointed by ``CC_BUILD`` (or -o option)
will be contain an ``install`` subdirectory that contains CodeCompass binaries.

## Installing docker-compose

The runtime environment based on ``docker-compose``. Applications that are parts
of the environment run in their containers. The ``docker-compose.yaml`` file
glue them together.

https://docs.docker.com/compose/install/

## Set up CodeCompass runtime environment

CodeCompass is a complex networked application, it is not trivial to set-up a
working environment for it. The purpose of this environment that the developer
can quickly try CodeCompass on his/her developer host.

The ``create_base_images.sh`` script control the assembling of necessary images.
This script depends on the ``compass-devel`` image, so that should be created
first as described in chapter ...

The demonstration parser parses ``xerces`` project. The xerces source should be
downloaded by hand under the ``CC-Runtime/xercessrc`` directory before the parser
started (up). (A specific parser can download its project automatically, but in
this demo does not. See ``CC-Runtime/xerces/bin/project_specific.sh``)

## Running CodeCompass

Starting the whole environment needs human control (at this moment).

```bash
cd CC-Runtime
./start_cluster.sh
```

In contrast by its name this script (now) only starts the backend database. But
writes the necessary commands to the console. The next command is to start the
parser:

```bash
/usr/local/bin/docker-compose -f <cc_source_dir>/scripts/docker/CC-Runtime/docker-compose.yaml up xercesparser
```

This script analyses the source of the xerces project and stores the result in the backend database. The next command (if the parsing was successfully ended) is to
start the CodeCompass webserver.

```bash
/usr/local/bin/docker-compose -f <cc_source_dir>/scripts/docker/CC-Runtime/docker-compose.yaml up webserver
```

Finally a browser can show the xerces project. Start a browser and connect to the
``http://localhost:6251`` url. In the left side of the page the ``File Manager``
will be appeared and the xerces source will be shown under the ``/mnt/ws``
directory.