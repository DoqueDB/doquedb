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

conffile=$1
if [ -z "$conffile" ]; then conffile=./conf.sh; fi
. $conffile

echo "Install Objects"

MKDIR="mkdir -p -m 0755"
USERPSWD=user.pswd

if [ ! -d $installpath ]; then $MKDIR $installpath; fi
if [ ! -d $installpath/bin ]; then $MKDIR $installpath/bin; fi
if [ ! -d $installpath/bin/java ]; then $MKDIR $installpath/bin/java; fi
if [ ! -d $installpath/etc ]; then $MKDIR $installpath/etc; fi
if [ ! -d $installpath/lib ]; then $MKDIR $installpath/lib; fi
if [ ! -d $installpath/log ]; then $MKDIR $installpath/log; fi
if [ ! -d $installpath/data ]; then $MKDIR $installpath/data; fi

install -m 0644 notice.txt $installpath
install -m 0755 obj/bin/* $installpath/bin
install -m 0644 obj/java/* $installpath/bin/java
install -m 0644 jdbc/doquedb.jar $installpath/bin/java
install -m 0755 obj/lib/*.so $installpath/lib
install -m 0755 obj/lib/*.so.* $installpath/lib > /dev/null 2>&1
if [ ! -f $installpath/etc/$USERPSWD ]; then install -m 0600 obj/etc/$USERPSWD $installpath/etc; fi
cp -r obj/data/* $installpath/data
chmod -R go-w $installpath/data

sed -e "s!%INSTALL_PATH%!$installpath!g" -e "s!%RUN_USER%!$user!g" < doquedb.sh > doquedb
chmod 0755 doquedb
install -m 0755 doquedb $installpath/bin

sed -e "s!%INSTALL_PATH%!$installpath!g" < doquedb-logrotate.sh > doquedb-logrotate
chmod 0644 doquedb-logrotate
install -m 0644 doquedb-logrotate $installpath/etc

sed -e "s!%INSTALL_PATH%!$installpath!g" < Sqli.sh > sqli
chmod 0755 sqli
install -m 0755 sqli $installpath/bin

sed -e "s!%INSTALL_PATH%!$installpath!g" < UserAdd.sh > useradd
chmod 0755 useradd
install -m 0755 useradd $installpath/bin

sed -e "s!%INSTALL_PATH%!$installpath!g" < UserDel.sh > userdel
chmod 0755 userdel
install -m 0755 userdel $installpath/bin

sed -e "s!%INSTALL_PATH%!$installpath!g" < UserPassword.sh > userpassword
chmod 0755 userpassword
install -m 0755 userpassword $installpath/bin

#sed -e "s!%INSTALL_PATH%!$installpath!g" < admin.sh > dqadmin
#chmod 0755 dqadmin
#install -m 0755 dqadmin $installpath/bin

sed -e "s!%INSTALL_PATH%!$installpath!g" < load.sh > dqload
chmod 0755 dqload
install -m 0755 dqload $installpath/bin

sed -e "s!%INSTALL_PATH%!$installpath!g" < unload.sh > dqunload
chmod 0755 dqunload
install -m 0755 dqunload $installpath/bin

#sed -e "s!%INSTALL_PATH%!$installpath!g" < trbackup.sh > trbackup
#chmod 0755 trbackup
#install -m 0755 trbackup $installpath/bin

#
# Copyright (c) 2023, 2024 Ricoh Company, Ltd.
# All rights reserved.
#
