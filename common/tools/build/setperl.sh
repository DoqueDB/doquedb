#! /bin/sh
PATH=$PATH:/bin:/usr/ucb:/usr/etc:/etc
export PATH

case `uname` in
SunOS)
	echo perl5;;
Linux)
	echo perl;;
esac
