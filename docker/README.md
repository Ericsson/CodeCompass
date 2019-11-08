# Docker

[![Docker](/doc/images/docker.jpg)](https://www.docker.com/)

Docker related scripts and config files are under [CodeCompass/docker](/docker).
After navigating to this directory you can use the build scripts with the
commands below.

:warning: When using Docker to build CodeCompass it is very important to have
the following filesystem layout:

```
...
 `-CodeCompass
   |-CodeCompass   # Source code from Git.
   | `-docker      # Docker related files.
   |  `-dev        # Docker files for development.
   |-build         # CMake runs here.
   |-install       # CodeCompass goes here.
   `-workspace     # Parsed projects' workspace directory.
```

The scripts assume this layout. The `build` and `install` directories will be
generated to the parent directory of `CodeCompass` directory containing the
source code. The `workspace` directory should be created manually.

Table of Contents
=================
* [Build image from development](#build-image-from-development)
* [How to use docker to develop CodeCompass](#how-to-use-docker-to-develop-codecompass)
  * [Build, install and test CodeCompass](#build-install-and-test-codecompass)
  * [How to parse a project](#how-to-parse-a-project)
  * [How to start a webserver](#how-to-start-a-webserver)

## Build image from development
Build the development environment image. The tag name is important!
```
docker build -t codecompass-dev docker/dev
```

## How to use docker to develop CodeCompass
You can use the `codecompass-dev` image created above to develop CodeCompass.
First, you have to start a docker container from this image, which will mount
your CodeCompass directory from your host and starts a shell:
```bash
docker run --rm -ti \
  --env DATABASE=sqlite --env BUILD_TYPE=Release \
  --volume /path/to/host/CodeCompass:/CodeCompass \
  --volume /path/to/your/host/project:/projects/myproject \
  --port 8001:8080 \
  codecompass-dev \
  /bin/bash
```
This container will be used in the next subsections to build CodeCompass,
parse a project and start a webserver.

*Note*: you do not have to give the `--port` option and set the `DATABASE`
environment variable if you do not want to run a webserver. Also you do not
have to mount a project directory if you do not want to parse it later.

### Build, install and test CodeCompass
You can use the `codecompass-build.sh` script inside the container to build,
install and test CodeCompass:
```bash
# Build CodeCompass.
codecompass-build.sh -j8

# Install CodeCompass.
codecompass-build.sh install

# Run tests.
codecompass-build.sh test
```

### How to parse a project
You can parse a project inside a docker container by using the following
command:
```bash
/CodeCompass/install/bin/CodeCompass_parser \
  -d "sqlite:database=/CodeCompass/workspace/myproject/data.sqlite" \
  -w /CodeCompass/workspace \
  -n myproject \
  -i /projects/myproject
```

*Note*: the project directory should be mounted inside the container.

### How to start a webserver
You can start a webserver inside the container by using the following command:
```bash
# Create a workspace directory.
mkdir -p /CodeCompass/workspace

# Run the web server.
/CodeCompass/install/bin/CodeCompass_webserver \
  -w /CodeCompass/workspace
```
