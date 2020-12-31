# Test images

[![Docker](../../doc/images/docker.jpg)](https://www.docker.com/)

This Dockerfile and the `install_dependencies.sh` script serves the purpose of local multi-OS testing.
## Build
To build the testing image navigate to the root of the CodeCompass source and the command below. By default the user and group properties inside and outside the container do not match. If one would prefer them to match, for convenience reasons, they can add 
`--build-arg CC_UID=$(id -u) --build-arg CC_GUI=$(id -g)` to the command below.

```bash
developer@computer:CodeCompass$ docker build --tag cc-test:<version> \
--build-arg UBUNTU_VERSION=<version> --file docker/test/Dockerfile .
```
Version can be 18.04 or 20.04 since these are the supported Ubuntu versions. Naturally the name of the image does not matter, one can change it however they like.

## How to test with the images
These images, just like the `codecompass:dev` image, are shipped with the `codecompass-build.sh` convenience script inside.
To test one's code it has to be mounted to the container at startup.
```bash
developer@computer:~$ docker run --tty --interactive --rm \
--volume <path_to_source>:/CodeCompass/CodeCompass \
cc-test:<version> /bin/bash
```
The inside the container one can build and test their source by running the following commands in order:
```bash
developer@container:/$ codecompass-build.sh -j $(nproc)
developer@container:/$ codecompass-build.sh install
developer@container:/$ codecompass-build.sh test
```
