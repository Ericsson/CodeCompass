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
   |  `-web        # Docker files for deployment.  
   |-build         # CMake runs here.
   |-install       # CodeCompass goes here.
   `-workspace     # Parsed projects' workspace directory.
```

The scripts assume this layout. The `build` and `install` directories will be
generated to the parent directory of `CodeCompass` directory containing the
source code. The `workspace` directory should be created manually.

Table of Contents
=================
* [Development](#development)
  * [Build image for development](#build-image-for-development)
  * [How to use docker to develop CodeCompass](#how-to-use-docker-to-develop-codecompass)
    * [Build, install and test CodeCompass](#build-install-and-test-codecompass)
    * [Make a Postgresql container](#make-a-postgresql-container)
    * [Create a network and connect CodeCompass and Postgres](#create-a-network-and-connect-codecompass-and-postgres)
    * [Create a Postgres user](#create-a-postgres-user)
    * [How to parse a project](#how-to-parse-a-project)
    * [How to start a webserver](#how-to-start-a-webserver)
* [Deployment](#deployment)
  * [Build image for runtime](#build-image-for-runtime)
  * [Build image for webserver](#build-image-for-webserver)
    * [How to use the webserver executing container](#how-to-use-the-webserver-executing-container)
* [Official DockerHub images](#official-dockerhub-images)

# Development
## Build image from development
Build the development environment image. The tag name is important!
**It is very important to give this command from the top level directory of the
CodeCompass source.**

```
cd CodeCompass
docker build -t codecompass:dev --file docker/dev/Dockerfile .
```

See more information [below](#how-to-use-docker-to-develop-codecompass) how to
use this image to develop CodeCompass.

## How to use docker to develop CodeCompass
You can use the `codecompass:dev` image created
[above](#build-image-for-development) to develop CodeCompass.
First, you have to start a docker container from this image, which will mount
your CodeCompass directory from your host and starts a shell:
```bash
docker run --rm -ti \
  --env DATABASE=pgsql --env BUILD_TYPE=Release \
  --volume /path/to/host/CodeCompass:/CodeCompass \
  --volume /path/to/your/host/project:/projects/myproject \
  -p 8001:8080 \
  codecompass:dev \
  /bin/bash
```
This container will be used in the next subsections to build CodeCompass,
parse a project and start a webserver.

*Note*: you do not have to give the `--publish` option and set the `DATABASE`
environment variable if you do not want to run a webserver. Also you do not
have to mount a project directory if you do not want to parse it later.

### Build, install and test CodeCompass
You can use the `codecompass-build.sh` script inside the container to build,
install and test CodeCompass:
(If you get a timeout error when downloading, put this command before install: NODE_OPTIONS="--max-old-space-size=4096" NEXT_PRIVATE_BUILD_WORKER_TIMEOUT=600)
```bash
# Build CodeCompass.
codecompass-build.sh -j8

# Install CodeCompass.
codecompass-build.sh install

# Run tests.
codecompass-build.sh test
```

### Make a Postgresql container
You need to create a PostgreSQL container that CodeCompass can communicate with.
(postgres is the image name and :15 is the specific version)
```bash
docker run \
  --name postgres_container \
  -e POSTGRES_PASSWORD=root_password \
  -d \
  -p 5432:5432 \
  postgres:15
```

### Create a network and connect CodeCompass and Postgres
Run the containers and connect them to a nework
```bash
docker network create my_network
docker network connect my_network postgres_name
docker network connect my_network codecompass_container
```
### Create a Postgres user
You need to create a Postgres user and give it admin rights.
(Run these commands from the host computer)
```bash
# Create user "your_user" with password "your_password" and give Superuser rights
docker exec -it postgres_container psql -U postgres -d postgres -c "CREATE USER your_user WITH PASSWORD 'your_password' SUPERUSER;"
```

### How to parse a project
You can parse a project inside a docker container by using the following
command:
(The project must be built before parsing.)
```bash
CodeCompass_parser \
  -d "pgsql:database=myproject;host=codecompass-postgres;port=5432;user=your_user;password=your_password" \
  -w /CodeCompass/workspace \
  -n myproject \
  -i /projects/myproject
  -b /projects/myproject/projectname/bin/Debug \
  -f
```

*Note*: the project directory should be mounted inside the container.

### How to start a webserver
You can start a webserver inside the container by using the following command:
```bash
# Run the web server.
CodeCompass_webserver \
  -w /CodeCompass/workspace
```

# Deployment

## Build image for runtime
For a production environment you can build and use the runtime environment image, 
which contains the built CodeCompass binaries and their dependencies:
```bash
docker build -t codecompass:runtime --no-cache --file docker/runtime/Dockerfile .
```

By default this image download the `master` branch of the CodeCompass GitHub 
repository and build it in `Release` mode with `sqlite` database configuration.
You can override these default values through the following build-time
variables:

|       Variable       |                  Meaning                 |
| -------------------- | ---------------------------------------- |
| `CC_REPO_URL` | The URL of the CodeCompass repository to use. |
| `CC_VERSION` |  The branch, version hash or tag of the CodeCompass repository to use. |
| `CC_DATABASE`|  Database type. Possible values are **sqlite**, **pgsql**. |
| `CC_BUILD_TYPE` | Specifies the build type. Supported values are **`Debug`** and **`Release`**. |

The below example builds the `codecompass:runtime` image with *pgsql* configuration:
```bash
docker build -t codecompass:runtime --build-arg CC_DATABASE=pgsql \
  --no-cache --file docker/runtime/Dockerfile .
```

*Note*: the `codecompass:dev` is a prerequisite to build the `codecompass:runtime` image.

## Build image for webserver
You can use the `codecompass:runtime` image created
[above](#build-image-for-runtime) to build an executing container for the webserver:
```bash
docker build -t codecompass:web --no-cache --file docker/web/Dockerfile .
```

See more information [below](#how-to-run-codecompass-webserver-in-docker) how
to use this image to start a CodeCompass webserver.

### How to use the webserver executing container
You can use the `codecompass:web` image to start a CodeCompass webserver.
For this run the following command:
```bash
docker run \
  --volume /path/to/host/workspace/:/workspace \
  -p 8010:8080 \
  codecompass:web \
  CodeCompass_webserver -w /workspace
```

# Official DockerHub images

Prebuilt images can be downloaded from DockerHub, from the 
[modelcpp/codecompass](https://hub.docker.com/r/modelcpp/codecompass) 
repository.

The following image tags are available:

| Name | Description |
| ---- | ------------|
| `dev` | Development image |
| `runtime-sqlite` | Runtime image containing CodeCompass binaries built against SQLite |
| `runtime-pgsql` | Runtime image containing CodeCompass binaries built against PostrgreSQL |
| `web-sqlite` | Webserver executing container image built against SQLite |
| `web-pgsql` | Webserver executing container image built against PostgreSQL |

The default `latest` is an alias to `:runtime-pgsql`.
To download (or update) an image from DockerHub, issue the command 
`docker pull modelcpp/codecompass:latest`. (Replace `latest` with the desired tag.)
