#!/bin/sh -f
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
# useradd

INSTALLDIR=%INSTALL_PATH%
LD_LIBRARY_PATH=$INSTALLDIR/lib
export LD_LIBRARY_PATH
ModParameterPath=$INSTALLDIR/etc/mod.conf
export ModParameterPath
SYDPARAM=$INSTALLDIR/etc/default.conf
SYDSYSPARAM=$INSTALLDIR/etc/system.conf
export SYDPARAM
export SYDSYSPARAM

$INSTALLDIR/bin/UserAdd "$@"

#
# Copyright (c) 2023, 2024 Ricoh Company, Ltd.
# All rights reserverd.
#
