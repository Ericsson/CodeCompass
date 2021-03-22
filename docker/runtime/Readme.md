# CodeCompass runtime image

## Sqlite
If no build argument is passed the image will build with sqlite support, omitting postgreSQL dependencies.
The standard build command is as it follows:
```bash
developer@computer:CodeCompass/docker/runtime$ docker build --tag modelcpp/codecompass:runtime-sqlite .
```
##postgreSQL
In order to compile CodeCompass with postgreSQL support and install postgreSQL dependencies add the `--build-arg CC_DATABASE=pgsql` to the build command, and change the name of the image created.

```bash
developer@computer:CodeCompass/docker/runtime$ docker build --tag modelcpp/codecompass:runtime-pgsql \
  --build-arg CC_DATABASE=pgsql .
```
