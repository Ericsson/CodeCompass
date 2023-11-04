#!/bin/sh

wget "http://www.apache.org/dyn/mirrors/mirrors.cgi?action=download&filename=thrift/0.13.0/thrift-0.13.0.tar.gz" \
-O thrift-0.13.0.tar.gz
tar -xvf ./thrift-0.13.0.tar.gz
cd thrift-0.13.0
# Configure
./configure --prefix=/thrift_install --silent --without-python    \
  --enable-libtool-lock --enable-tutorial=no --enable-tests=no    \
  --with-libevent --with-zlib --without-nodejs --without-lua      \
  --without-ruby --without-csharp --without-erlang --without-perl \
  --without-php --without-php_extension --without-dart            \
  --without-haskell --without-go --without-rs --without-haxe      \
  --without-dotnetcore --without-d --without-qt4 --without-qt5    \
  --without-java
make install -j $(nproc)
# Copy to /usr
cp -rn /thrift_install/* /usr
# Clean up
cd /
rm -rf /thrift_install thrift-0.13.0.tar.gz thrift-0.13.0