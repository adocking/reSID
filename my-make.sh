#!/bin/sh

cd src

echo "One off setup with autotools, `cd src` `autoreconf -fi`"

mkdir build
cd build

echo "Plus `../configure` with whatever options and then `make install`."

make
