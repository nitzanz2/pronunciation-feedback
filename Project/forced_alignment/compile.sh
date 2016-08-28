#!/bin/bash
cd code
make clean
make
cd ../bin
rm -fr ForcedAlignment*
ln -s ../code/_Linux_Release/* .
rm -fr *.o

