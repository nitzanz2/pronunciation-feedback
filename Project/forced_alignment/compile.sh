#!/bin/bash
cd code
make clean
make
cd ../bin
rm -fr ForcedAlignment*
ln -s ~/OurProject/Project/forced_alignment/code/_Linux_Release/* .
rm -fr *.o

