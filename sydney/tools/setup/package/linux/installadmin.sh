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

. ./dist/conf.sh

echo "Install TRAdmin Objects"

MKDIR="mkdir -p -m 0755"
if [ ! -d $installpath/bin ]; then $MKDIR $installpath/bin; fi
if [ ! -d $installpath/bin/java ]; then $MKDIR $installpath/bin/java; fi

install -m 0644 obj/java/* $installpath/bin/java
install -m 0644 jdbc/doquedb.jar $installpath/bin/java

#sed -e "s!%INSTALL_PATH%!$installpath!g" < admin.sh > dqadmin
#chmod 0755 dqadmin
#install -m 0755 dqadmin $installpath/bin

sed -e "s!%INSTALL_PATH%!$installpath!g" < load.sh > dqload
chmod 0755 dqload
install -m 0755 dqload $installpath/bin

sed -e "s!%INSTALL_PATH%!$installpath!g" < unload.sh > dqunload
chmod 0755 dqunload
install -m 0755 dqunload $installpath/bin

#
# Copyright (c) 2023, 2024 Ricoh Company, Ltd.
# All rights reserved.
#
