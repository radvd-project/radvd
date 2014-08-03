#!/bin/bash

autoreconf -vif
./configure
make -j 10

