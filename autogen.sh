#!/bin/sh
mkdir config 2> /dev/null
aclocal
automake --add-missing
autoreconf
