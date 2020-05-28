# Docker

[![Docker](/doc/images/docker.jpg)](https://www.docker.com/)

Docker related scripts and config files are under [CodeCompass/docker](/docker).
After navigating to this directory you can use the build scripts with the
commands below.

:warning: When using Docker to build CodeCompass it is very important not to modify the following filesystem layout:

```
...
 `-CodeCompass
   |-CodeCompass   # Source code from Git.
   | `-docker      # Docker related files.
   |  `-dev        # Docker files for development.
   |  `-web        # Docker files for webserver.
```

The scripts assume this layout.

Table of Contents
=================
* [Development](#development)
  * [Build image for development](#build-image-for-development)
  * [How to use docker to develop CodeCompass](#how-to-use-docker-to-develop-codecompass)
    * [Build, install and test CodeCompass](#build-install-and-test-codecompass)
    * [How to parse a project](#how-to-parse-a-project)
    * [How to start a webserver](#how-to-start-a-webserver)
* [Deployment](#deployment)
  * [Build image for web](#build-image-for-web)
  * [How to run CodeCompass webserver in docker](#how-to-run-codecompass-webserver-in-docker)

# Development
## Build image for development
This container will be used in the next subsections to build CodeCompass, parse a project and start a webserver. To build the development environment image use the building script. The image's name will be codecompass:dev.
```bash
user@computer:~ cd CodeCompass
user@computer:CodeCompass docker/builder.sh -d
```
One can build it manually with the following command.
```bash
docker build -t <image_name> <path_to_dev_dockerfile>
```
When building manually keep in mind that the webserver's image originates from the development one, meaning, that while building it, one will have to pass the name of the developer image as a build argument. This is discussed at length [here](#build-image-for-web).

See more information on how to use this image to develop CodeCompass [below](#how-to-use-docker-to-develop-codecompass).

## How to use docker to develop CodeCompass
You can use the `codecompass:dev` image created [above](#build-image-for-development) to develop CodeCompass. First, you have to start a docker container from this image and mount your CodeCompass directory from your host.
```bash
user@computer:~ docker run --rm -ti \
  --env DATABASE=<database_type> \
  --mount type=bind,source=<path_to_your_cc_source>,target=/CodeCompass
  --mount type=bind,surce=<path_to_src_project>.target=/Project
  -p 8001:8080 \
  codecompass:dev \
  /bin/bash
```
The `<database_type>` can either be sqlite or postgres. *Note*: you do not have to give the `--publish` option and set the `DATABASE`
environment variable if you do not want to run a webserver. Also you do not
have to mount a project directory if you do not want to parse it later.

### Build, install and test CodeCompass
You can use the `codecompass-build.sh` script inside the container to build,
install and test CodeCompass:
```bash
# Build CodeCompass.
user@container:~ codecompass-build.sh -j$(nproc)

# Install CodeCompass.
user@container:~ codecompass-build.sh install

# Run tests.
user@container:~ codecompass-build.sh test
```

### How to parse a project
If you want to parse a project in the codecompass:dev image keep it in mind that you have to build the CodeCompass binaries inside the container first as described [above](#how-to-use-docker-to-develop-codecompass).
```bash
user@computer:~ CodeCompass_parser \
  -d "sqlite:database=/CodeCompass/workspace/myproject/data.sqlite" \
  -w /CodeCompass/workspace \
  -n myproject \
  -i /projects/myproject
```


### How to start a webserver
You can start a webserver inside the container by using the following command:
```bash
# Create a workspace directory.
user@container:~ mkdir -p /CodeCompass/workspace

# Run the web server.
user@container:~ CodeCompass_webserver -w /CodeCompass/workspace
```

# Deployment

## Build image for web
The webserver's image uses the development one as base. If you want to build the webserver and you haven't built the codecompass:dev before simply run the following commands. The script will build both the developer and the webserver image.
```bash
user@computer:~ cd CodeCompass
user@computer:CodeCompass docker/builder.sh
```
If you have previously built the developer image, there is no need to erase it. Run the builder script with `-w` like this:
```bash
user@computer:~ cd CodeCompass
user@computer:CodeCompass docker/builder.sh -w <dev_image_name>
```
Or one can build it manually.
```bash
user@computer:~ CodeCompass
user@computer:CodeCompass docker build -t --build-arg DEV_IMAGE=<dev_image_name> codecompass:web 
```

## How to run CodeCompass webserver in docker
You can use the `codecompass:web` image created
[above](#build-image-for-web) to start a CodeCompass webserver.
For this run the following command:
```bash
user@computer:~ docker run \
  --mount type=bind,source=<path_to_host_workspace>,target=/Workspace \
  codecompass:web
```
*Note*: The standard port of the CodeCompass webserver is 8080. To change that you can add `-p <port>` to the command above.
