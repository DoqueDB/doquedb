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

dumppath=$1
if [ -z "$dumppath" ]; then dumppath=save; fi
conffile=$2
if [ -z "$conffile" ]; then conffile=./conf.sh; fi
. $conffile

rm -rf $installpath/$dumppath
mkdir -p -m 0755 $installpath/$dumppath
cp -r $databasepath/. $installpath/$dumppath

#
# Copyright (c) 2023, 2024 Ricoh Company, Ltd.
# All rights reserved.
#
