# Using Docker in CodeCompass development

Why docker necessary? Because producing a development environment of CodeCompass
is a very long task with many traps. Some software component could not be
reached by usual way (installing them from packages.) For example LLVM with
compiled in RTTI support. (See [Dependencies](deps.md))

We created a build system that can be run on any linux distribution with less
effort. We hope that the developers can be productive in a shorter time in
CodeCompass development. However the build system can be run on any
distribution, the executables are compiled for ubuntu. The result of the build
may or may not run on another distribution. This solution contains a runtime
environment too to try the compiled CodeCompass. This runtime environment based
on ubuntu too, so the CodeCompass can be tried without problem with it. (See
[Set up CodeCompass runtime environment](#set-up-codecompass-runtime-environment))

Additionally, the image creation is an exact definition how the build system
have to set up.

Using this way of CodeCompass development not requires full understanding of
Docker. The steps of development wrapped with shell scripts which hides the
dirty work.

## Installing Docker

On the developer host the Docker software should be installed. (See
[Install Docker](https://docs.docker.com/install/))

## Create CodeCompass development environment

All tools that necessary to build CodeCompass, is encapsulated in a Docker
image. So, the developer image should be created first on the developer's
machine.

To create docker image that contains all necessary tools to make a CodeCompass
binary:

```bash
git clone https://github.com/Ericsson/CodeCompass <cc_source_dir>
docker build --tag compass-devel <cc_source_dir>/scripts/docker/CC-Devel
```

This is a long running task, because a complete LLVM is necessary to compile
(among others). When it is built, the image consumes about 4GB on the
developer's machine.

Using "compass-devel" as image name is mandatory, because the scripts find the
image by this name.

Now, the build system is ready to make development cycles (modify-build-try) of
the CodeCompass.

## Compiling CodeCompass

There are four wrapper script in the
``<cc_source_dir>/scripts/docker/CC-Devel`` directory beside the ``Dockerfile``.
These scripts performs the development steps. They should be run in order.
(Explained later.)

The builder container mount two directories while it is running. The first one
which contains CodeCompass source. The second one where the result of the
compilation (executables) will be placed. All scripts uses options to specify
them. But to simplify developer's life, these scripts find environment variables
as defaults to command line parameters. So the developer can set the following
environment variables in his/her shell.

- CC_SOURCE where the CodeCompass source resides
- CC_BUILD where the cmake generates the CodeCompass executables

Additionally the ``configurecc.sh`` uses the

- CC_BUILD_TYPE what defines build type as the cmake requires, for example
``Debug``
- CC_DATABASE_TYPE what defines the underlined database type. It can be sqlite or pgsql.

Additionally the ``fetchcc.sh`` and ``create_base_images.sh``uses the

- CC_URL points to the CodeCompass git repository

### 1. Get CodeCompass source

If the latest master version is necessary to build then nothing to do here,
because that was downloaded to compile the ``compass-devel`` image before. If
other branch or other repository of the CodeCompass necessary to build then the
``fetchcc.sh`` script can be used. The ``fetchcc.sh`` script creates a directory
on the filesystem of the developer's machine and get the CodeCompass source into
it.

```bash
<cc_source_dir>/scripts/docker/CC-devel/fetchcc.sh \
  [-u <other_compass_repository_url>] [-s <cc_source_dir>] \
  [-b <cc_branch>]
```

This command simply clones the CodeCompass source from a repository that the
``<url>`` determines. (This action can be performed on the developer's machine
with native git commands too.)

### 2. Configure CodeCompass source to compile

The build system of the CodeCompass is ``cmake``. When the source is available,
``cmake`` requires a configuration step what create makefiles. This task
performed by ``configurecc.sh`` script.

```bash
<cc_source_dir>/scripts/docker/CC-devel/configurecc.sh \
  -o <output_dir> -s <cc_source_dir> [-d <database_type>] \
  [-t <build_type>]
```

The directory pointed by ``CC_BUILD`` (or -o option) will be contain a ``build``
subdirectory that populated with files that cmake generates.

### 3. Build CodeCompass

The ``buildcc.sh`` script performs the build and install steps of CodeCompass.

```bash
<cc_source_dir>/scripts/docker/CC-devel/buildcc.sh \
  [-s <source directory>] [-o <output directory>]
```

If something went wrong during build then the developer can edit the source of
CodeCompass on the developer's machine as him/her could be done in an usual
development environment.

After successful build the directory pointed by ``CC_BUILD`` (or -o option)
will be contain an ``install`` subdirectory that contains CodeCompass binaries.

### 4. Troubleshooting the builder container

The ``shellcc.sh`` script can be used to try/troubleshoot/develop the builder image. It starts a shell in a new container that based on compass-devel image. Normally not necessary to use.

```bash
<cc_source_dir>/scripts/docker/CC-devel/shellcc.sh \
  -o <output_dir> -s <cc_source_dir> [-u]
```

### Example to compile CodeCompass builder image

```bash
cc_src="~/compass/zomen2"
git checkout https://github.com/zomen2/CodeCompass ${cc_src}

export CC_SOURCE="~/compass/source"
${cc_src}/scripts/docker/CC-devel/fetchcc.sh

export CC_BUILD="~/compass/build"
${cc_src}/scripts/docker/CC-devel/configurecc.sh
${cc_src}/scripts/docker/CC-devel/buildcc.sh
```

## Installing docker-compose

The runtime environment is based on ``docker-compose``. Applications that are
parts of the environment run in their containers. The ``docker-compose`` by
``docker-compose.yaml`` file glue them together.

[Install Docker Compose](https://docs.docker.com/compose/install/)

## Set up CodeCompass runtime environment

The purpose of this environment that the developer can quickly try
CodeCompass on his/her developer's machine. CodeCompass is a complex networked
application, it is not trivial to set-up a well established environment for it.
The different roles in a host based network are realized by different
containers. Script below controls assembling of necessary images of the whole
demo environment.

```bash
<cc_source_dir>/scripts/docker/CC-Runtime/create_base_images.sh \
  [-u <other_compass_repository_url>] [-b <cc_branch>]
```

It uses the previously built compass-devel image, build a new CodeCompass from
the specified source and pulls the additional necessary images from their docker
repositories. (After calling ``create_base_images.sh`` run the
``docker images`` command that shows the created/pulled images.)

This script depends on the ``compass-devel`` image, so that should be created
first as described in chapter
[Create CodeCompass development environment](#create-codecompass-development-environment)

The CodeCompass repository contains a demonstration parser which parses
``xerces`` project. The xerces source should be downloaded by hand into the
``CC-Runtime/xercessrc`` directory before the parser started (up). (Your  
parser for your project can download its project automatically. See
implementation of ``CC-Runtime/xerces/bin/project_specific.sh``)

## Running CodeCompass

Starting the whole environment needs human control (at this moment).

```bash
cd CC-Runtime
./start_cluster.sh
```

In contrast by its name this script (now) only starts the backend database. But
writes the further necessary commands to the console. The commands should be
started in an other shell, because the first are used by the database server:

```bash
docker-compose -f <cc_source_dir>/scripts/docker/CC-Runtime/docker-compose.yaml up xercesparser
```

The application above analyses the source of the xerces project and stores the
result in the backend database. The next command (if the parsing was
successfully ended) starts the CodeCompass webserver.

```bash
docker-compose -f <cc_source_dir>/scripts/docker/CC-Runtime/docker-compose.yaml up webserver
```

Finally a browser can show the xerces project. Start a browser and connect to
the ``http://localhost:6251`` url. In the left side of the page the
``File Manager`` will be appeared and the xerces source will be shown under the
``/mnt/ws`` directory.