#!/bin/sh
mkdir m4
if autoreconf -i -v -f ; then
	echo
	echo "autoreconf done."
	echo
else
	echo
	echo "autoreconf failed."
	echo
	exit 1
fi
