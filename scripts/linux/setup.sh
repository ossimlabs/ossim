#!/bin/sh
echo Working Directory: $PWD
mkdir -p build
pushd build
echo New working dir: $PWD
popd
exit 0

