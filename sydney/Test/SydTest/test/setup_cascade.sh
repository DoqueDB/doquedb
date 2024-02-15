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

# 子サーバの sydtest.conf を作る。
# sydtest-setup.sh の子サーバ版。
# 20120802 

conf_file=cascade_for_linux.conf
normalized_conf_file=cascade_for_linux_normalized.conf
pid_file=/log/doquedb.pid

regfile=default.conf

# 設定ファイルの存在チェック
check_file() {
	if [ "$1" = "" ]; then
		if [ ! -f $normalized_conf_file ]; then
			echo default conf file not found: ${normalized_conf_file}
			echo Usage: $0 [cascade conf file]
			exit 1
		fi
    else
		if [ ! -f $1 ]; then
			echo file not found: $1
			exit 1
		fi
		normalized_conf_file=$1
	fi
	make_personalized_conf
}

# 実行ユーザによるパスの違いを吸収した正規化パスから、実行環境に応じた絶対パスを生成
make_personalized_conf() {
	if [ -f $conf_file ]; then
		mv $conf_file ${conf_file}.temp
	fi

	parent_path=/proj/sydney/work/`whoami`
	while read line; do
		line=`echo $line | perl -p -e "s! (/opt*)! ${parent_path}\1!"`
		echo $line >> $conf_file
	done < $normalized_conf_file
}

# 子サーバのユーザ認証解除
cancel_certification() {
	if [ ! -f "${conf_dir_path}${regfile}.back" ]; then
		cp ${conf_dir_path}${regfile} ${conf_dir_path}${regfile}.back
    fi
    cat ${conf_dir_path}${regfile}.back | sed "s/\(Server_PasswordFilePath\s*\"\).*\(\"\)/\1Sydney\2/" > ${conf_dir_path}${regfile}
}

# 子サーバにSydTestのメッセージストリームの出力先を設定する
# 再起動までデフォルトで５分からの変更
set_sydtest_message_stream() {
    echo -e "SydTest_MessageOutputInfo\\t\\t\"1\"" >>  ${conf_dir_path}${regfile}
    echo -e "SydTest_MessageOutputError\\t\\t\"1\"" >>  ${conf_dir_path}${regfile}
    echo -e "SydTest_MessageOutputTime\\t\\t\"1\"" >>  ${conf_dir_path}${regfile}
    echo -e "Admin_ReplicatorWaitTime\\t\\t\"1000\"" >>  ${conf_dir_path}${regfile}
}

# 子サーバが起動していたら終了
stop_cascade() {
    if [ -f "$pid_path" ]; then
		${cascade_install_path}/bin/doquedb stop
    fi
}

##########
# Main
##########

check_file

cascade_id=0

while read line; do
	cascade_install_path=`echo $line | grep -e "^-\?\s*InstallPath:" | perl -p -e "s/^-?\s*InstallPath:\s*\"?//" | perl -p -e "s/\"?\s*$//"`
	if [ ! "$cascade_install_path" = "" ]; then
		conf_dir_path="${cascade_install_path}/etc/"
		pid_path="${cascade_install_path}${pid_file}"
		stop_cascade
		cancel_certification
		set_sydtest_message_stream
		chmod 0644 ${conf_dir_path}${regfile}
		install -m 0644 ${conf_dir_path}${regfile} cascade${cascade_id}_default.conf
		let cascade_id=${cascade_id}+1
	fi
done < $conf_file
#
# Copyright (c) 2001, 2023, 2024 Ricoh Company, Ltd.
# All rights reserved.
#
