#!/bin/bash

rm tinyFSDisk
gcc libDisk.c tinyfsTest.c libTinyFS.c
./a.out
