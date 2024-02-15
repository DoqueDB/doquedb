#!/bin/sh
#
# Copyright (c) 2001, 2023, 2024 Ricoh Company, Ltd.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
#
# doquedb	DoqueDB Server
# chkconfig: 345 80 20
# description: DoqueDB Server.
# processname: SydServer
#

PS=/bin/ps
OP=p
GREP=/bin/grep
if [ -x /bin/awk ]; then
	AWK=/bin/awk
elif [ -x /bin/mawk ]; then
	AWK=/bin/mawk
fi
ECHO=echo
SU=/bin/su
WHOAMI=/usr/bin/whoami

# thread stack size 1MB
ulimit -s 1024

# for debug
ulimit -c unlimited

INSTALLDIR=%INSTALL_PATH%
SYDUSER=%RUN_USER%

BASE=SydServer
SERVER=$INSTALLDIR/bin/$BASE
LD_LIBRARY_PATH=$INSTALLDIR/lib
export LD_LIBRARY_PATH
ModParameterPath=$INSTALLDIR/etc/mod.conf
export ModParameterPath
SYDPARAM=$INSTALLDIR/etc/default.conf
SYDSYSPARAM=$INSTALLDIR/etc/system.conf
export SYDPARAM
export SYDSYSPARAM
PIDFILE=$INSTALLDIR/log/doquedb.pid

if [ -f "$INSTALLDIR/bin/option.sh" ]; then
	. $INSTALLDIR/bin/option.sh
fi

start () {
	if [ -f "$SYDPARAM" -a -x "$SERVER" ]; then
		if [ -f "$PIDFILE" ]; then
			pid=`cat $PIDFILE`
			pp=`$PS $OP $pid | $GREP $BASE | $AWK '{print $1}'`
			if [ -n "$pp" ]; then
				$ECHO "$BASE (pid $pp) is already running."
				exit 1
			fi
		fi
		rm -f $PIDFILE
		$ECHO -n "$BASE starting ($SYDUSER) ..."
		cd $INSTALLDIR/bin
		if [ `$WHOAMI` = $SYDUSER ]; then
		    $SERVER
		else
		    $SU $SYDUSER -m -f -c $SERVER
		fi
		$ECHO " done."
	else
		$ECHO "$BASE is not installed."
	fi
}

stop () {
	if [ -f "$PIDFILE" ]; then
		pid=`cat $PIDFILE`
		pp=`$PS $OP $pid | $GREP $BASE | $AWK '{print $1}'`
		if [ -z "$pp" ]
		then
			$ECHO "$BASE is not started."
			rm -f $PIDFILE
			return 0
		fi
		$ECHO -n "$BASE stopping ($SYDUSER) ..."
		if [ `$WHOAMI` = $SYDUSER ]; then
		    kill -TERM $pid
		else
		    $SU $SYDUSER -m -f -c "kill -TERM $pid"
		fi
		pp=`$PS $OP $pid | $GREP $BASE | $AWK '{print $1}'`
		sleep 1
		while [ -n "$pp" ]
		do
			$ECHO -n "."
			sleep 1
			pp=`$PS $OP $pid | $GREP $BASE | $AWK '{print $1}'`
		done
		$ECHO " done."
		rm -f $PIDFILE
	else
		$ECHO "$BASE is not started."
	fi
}

status () {
	if [ -f "$PIDFILE" ]
	then
		pid=`cat $PIDFILE`
		pp=`$PS $OP $pid | $GREP $BASE | $AWK '{print $1}'`
		if [ -n "$pp" ]
		then
			$ECHO "$BASE (pid $pp) is running."
			return 0	
		fi

		$ECHO "$BASE dead, but $PIDFILE exists."
		return 1
	fi

	$ECHO "$BASE is not started."
	return 3
}

reload () {
	if [ -f "$PIDFILE" ]; then
		pid=`cat $PIDFILE`
		pp=`$PS $OP $pid | $GREP $BASE | $AWK '{print $1}'`
		if [ -z "$pp" ]
		then
			$ECHO "$BASE is not started."
			rm -f $PIDFILE
			return 0
		fi
		if [ `$WHOAMI` = $SYDUSER ]; then
		    kill -HUP $pid
		else
		    $SU $SYDUSER -m -f -c "kill -HUP $pid"
		fi
		$ECHO "$BASE reload ($SYDUSER)"
	else
		$ECHO "$BASE is not started."
	fi
}

clean_temporary () {
	status
	if [ $? -eq 0 ]
	then
		return 1
	fi
	rm -f $PIDFILE
	return 0
}

case "$1" in
'start')
	start
	;;
'stop')
	stop
	;;
'restart')
	stop
	sleep 1
	start
	;;
'status')
	status
	exit $?
	;;
'reload')
	reload
	;;
'clean_temporary')
	clean_temporary
	exit $?
	;;
*)
	$ECHO "Usage: $0 { start | stop | restart | status | reload | clean_temporary }"
	exit 1
	;;
esac
exit 0

#
# Copyright (c) 2023, 2024 Ricoh Company, Ltd.
# All rights reserved.
#
