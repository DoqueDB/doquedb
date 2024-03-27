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

modparam () {
	echo "#########################################" > $regfile
	echo "# for MOD" >> $regfile
	echo "MemoryPoolFreeAreaPercent	20" >> $regfile
	echo "MemoryPoolTotalLimit		2147483647" >> $regfile
	echo "ModMessageFacility		0" >> $regfile
	echo "ModMessageOutputDebug		3" >> $regfile
	echo "ModMessageOutputError		1" >> $regfile
	echo "ModMessageOutputNormal		0" >> $regfile
	echo "SelfMemoryManagement		FALSE" >> $regfile
	echo "UseMemoryDebugList		FALSE" >> $regfile
	echo "" >> $regfile
}

doquedbparam () {
	echo "#########################################" > $regfile
	echo "# for DoqueDB" >> $regfile
	echo "Schema_DefaultAreaPath		\"$databasepath/data\"" >> $regfile
	echo "Schema_SystemAreaPath		\"$databasepath/system\"" >> $regfile
	echo "Trans_TimeStampPath		\"$databasepath/system\"" >> $regfile
	echo "Common_UnaResource_1		\"$installpath/data/unadic/\"" >> $regfile
	echo "Common_UnaResource_2		\"\"" >> $regfile
	echo "Inverted_TermResource_0		\"$installpath/data/term/\"" >> $regfile
	echo "Inverted_TermResource_1		\"$installpath/data/termrsc7/\"" >> $regfile
	echo "Inverted_TermResource_2		\"\"" >> $regfile
	echo "Server_PasswordFilePath		\"$installpath/etc/user.pswd\"" >> $regfile
#	echo "LogicalFile_BtreeFileDriver	\"SyDrvBtr2\"" >> $regfile
#	echo "LogicalFile_VectorFileDriver	\"SyDrvVct2\"" >> $regfile
	if [ $normalize == 1 ]; then
		echo "Common_LikeUnaResource		2" >> $regfile
		echo "Execution_LikeNormalizedString	2" >> $regfile
	fi
	if [ "$buffersize" != "" ]; then
		echo "Buffer_NormalPoolSize		\"$buffersize\"" >> $regfile
	fi
	if [ "$batchsize" != "" ]; then
		echo "Inverted_BatchSizeMax		$batchsize" >> $regfile
	fi
	if [ "$checkpointperiod" != "" ]; then
		echo "Checkpoint_Period		$checkpointperiod" >> $regfile
	fi
}

sydserverparam () {
	echo "Communication_PortNumber		$portnumber" >> $regfile
	echo "Common_MessageOutputInfo		\"$installpath/log/doquedb.log\"" >> $regfile
	echo "Common_MessageOutputError		\"$installpath/log/doquedb.log\"" >> $regfile
	echo "Common_MessageIncludeDate		TRUE" >> $regfile
	echo "SydServer_ProcessIdFile		\"$installpath/log/doquedb.pid\"" >> $regfile
#	echo "TRBackup_MessageOutput		\"$installpath/log/trbackup.log\"" >> $regfile
#	echo "TRBackup_ProcessIdFile		\"$installpath/log/trbackup.pid\"" >> $regfile
	echo "Os_PrintCriticalSection		\"$installpath/log/lock.log\"" >> $regfile
	echo "" >> $regfile
}

regfile=mod.conf
modparam

if [ ! -f $installpath/etc/$regfile ]; then
    echo "Install Mod Parameter"
    install -m 0644 $regfile $installpath/etc
fi

regfile=default.conf
doquedbparam
sydserverparam

if [ ! -f $installpath/etc/$regfile ]; then
    echo "Install Default Parameter"
    install -m 0644 $regfile $installpath/etc
fi

regfile=system.conf

echo "" > $regfile

if [ ! -f $installpath/etc/$regfile ]; then
    echo "Install System Parameter"
    install -m 0644 $regfile $installpath/etc
fi

LD_LIBRARY_PATH=$installpath/lib
export LD_LIBRARY_PATH
ModParameterPath=$installpath/etc/mod.conf
export ModParameterPath
SYDPARAM=$installpath/etc/default.conf
SYDSYSPARAM=$installpath/etc/system.conf
export SYDPARAM
export SYDSYSPARAM

if [ ! -d $databasepath/system ]; then
	echo "Initialize Database"
	if [ ! -d $databasepath ]; then
	    mkdir -m 0755 $databasepath
	fi
	if [ ! -d $databasepath/data ]; then
	    mkdir -m 0755 $databasepath/data
	fi
	if [ ! -d $databasepath/system ]; then
	    mkdir -m 0755 $databasepath/system
	fi
	$installpath/bin/SydServer -install
fi

#
# Copyright (c) 2023, 2024 Ricoh Company, Ltd.
# All rights reserverd.
#
