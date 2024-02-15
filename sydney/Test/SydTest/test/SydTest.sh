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

# recovery後の整合性確認
# regression_mode=1 の時 Core ファイル確認
# Intel inspxer対応
############
#定数の定義
############
. env.sh
export INSTALL_PATH=$installpath
export MYTOP_PATH=$mytop
###############
# ローカル関数
###############

instrument()
{
    if [ -z $regression_mode ] ; then
        regression_mode=0      # regression.sh から実行は1

        if [ -n "$analysis_mode" ] ; then
            if [ $isLinux -eq 1 ] ; then
                cat $sydtest | sed -e '$d' | sed -e '$d' > sydtest_env.sh
                . sydtest_env.sh
            fi
            . analysis_env.sh
        fi
        
    fi

    if [ -z $purify_sh_mode ] ; then
            purify_sh_mode=0      # purify.sh から実行は1
    fi

    if [ -z $bc_sh_mode ] ; then
            bc_sh_mode=0
    fi

}

palling()
{
    #数秒おきにpallingを実行し、coveragew.exeがなくなっていたら先に進むようにする
    if [ "$analysis_mode" = "coverage_mode" ] || [ "$analysis_mode" = "purify_mode" ] ; then

        while [ `tasklist | gawk 'BEGIN{a=0} /coveragew/{a++} END{print a}'` -gt 0 ] ; do
            sleep 10
            echo -n .
        done

        while [ `tasklist | gawk 'BEGIN{a=0} /purifyW/{a++} END{print a}'` -gt 0 ] ; do
            sleep 10
            taskkill /F /FI "IMAGENAME eq purify*" >/dev/null 2>&1
        done
    fi
}

bccheck()
{
    sleep 180
    if [ `tasklist|grep -i SydTest* | wc -w` -eq 0 ] && [ `tasklist|grep -i BC.exe | wc -w` -gt 0 ] ; then
        taskkill /F /FI "IMAGENAME eq BC.exe" >/dev/null 2>&1
    fi
}

# unset と共に使うこと
set_timeout()
{
    if [ $timeout -ne 0 ] ; then
        watch_timeout &
        timeout_job=`jobs -r|grep "watch_timeout"|perl -p -e "s/\[(\d+)\].*/\1/"`
    fi
}

# set と共に使うこと
unset_timeout()
{
    if [ $timeout -ne 0 ] ; then
        # テスト自体は終了したので sleep を kill する
        kill_sleep

        # タイムアウト監視ジョブがまだ残っているなら終了を待つ
        if [ "`jobs -r|grep watch_timeout|perl -p -e 's/\[(\d+)\].*/\1/'`" = "$timeout_job" ] ; then
            wait %$timeout_job
        fi
    fi
}

# SydTest を監視し、一定時間プロセスのメモリ使用量に変化がなければタイムアウトを実行する
# タイムアウトして終了なら 1 を返し、それ以外なら 0 を返す
watch_timeout()
{
    local status=0
    local user=`whoami`

    # -T オプションで指定されたタイムアウト時間だけ sleep
    sleep $timeout
    while :
    do
        if [ $isWindows -eq 1 ] ; then
            MEM1=`tasklist|grep -i SydTest|gawk '{printf "%s", $5}'`
        else
#            MEM1=`ps alx | grep $user/opt/RICOHtrm/bin/SydTest | grep -v grep | awk '{printf "%s", $8}'`
            MEM1=`ps alx | grep $installpath/bin/SydTest | grep -v grep | awk '{printf "%s", $8}'`
        fi

        # タイムアウト前にテストが終了
        if [ "$MEM1" = '' ] ; then
            break
        fi

        # 一度タイムアウトの指定時間を経過した後は10秒毎にメモリ使用量の変化を監視する
        sleep 10

        if [ $isWindows -eq 1 ] ; then
            MEM2=`tasklist|grep -i SydTest|gawk '{printf "%s", $5}'`
        else
            MEM2=`ps alx | grep $user/opt/RICOHtrm/bin/SydTest | grep -v grep | awk '{printf "%s", $8}'`
        fi

        # タイムアウト前にテストが終了
        if [ "$MEM2" = '' ] ; then
            break
        fi

        # メモリ使用量に変化がなければ固まったと判断してタイムアウト
        if [ "$MEM1" = "$MEM2" ] ; then
            if [ $isWindows -eq 1 ] ; then
              taskkill /F /FI "IMAGENAME eq SydTest*" > /dev/null 2>&1
            else
              pkill -INT -f $user/opt/RICOHtrm/bin/SydTest > /dev/null 2>&1
            fi
            # 分散テストは子サーバも停止させる
            if [ ${dir##*_} = "dist" ]; then
                stop_cascade_server
            fi
            echo 'timeout by SydTest.' >> ./result/${name}${paratest}.out
            break
        fi
    done
    return $status
}

kill_sleep()
{
    #残存しているsleep+killプロセスの抹消
    if [ ${isWindows} -eq 1 ] ; then
        taskkill /F /FI "IMAGENAME eq sleep*" >/dev/null 2>&1
    else
        # -INT で kill するのはシェルに子プロセスの終了メッセージを表示させないようにするため
        pkill -INT -f sleep > /dev/null 2>&1
    fi
}

# singleとmulti系テストのオプション設定
check_file()
{
    case $name in
        *[0-4]???)
            defaultoptions="/c /p /b"
            diff="singlediff";;
        *[5-9]???)
            defaultoptions="/c /p /b /s"
            diff="multidiff";;
        *)
            echo "$file is unexpected."
            exit 1;;
    esac

    if [ $reg ] ; then
        defaultoptions="$defaultoptions /P"
    fi
}

# regression.shから指定されたスクリプト番号(from - to)の範囲チェック
check_range()
{
    if [ ! $from ] || [ ! $to ] || [ $from -gt $to ]; then
        echo 'error: arg of -f or -t.'
        exit 1
    fi
}

# テストスクリプトが存在するフォルダをチェック、及びresultフォルダを作成
check_directory()
{
    if [ $dir ] ; then
        dir=`basename $dir`
    else
        echo directory is not specified.
        exit 1;
    fi

    if [ ! -d $dir/$testdir ] ; then
        echo $dir/$testdir not exist.
        exit 1;
    fi

    if [ ! -d $dir/$testdir/result ] ; then
        mkdir $dir/$testdir/result
    fi
}

# ....
dump_cfy()
{
    if [ "$analysis_mode" = "coverage_mode" ] ; then
        pushd $cvr_dst >/dev/null 2>&1
        #  cp merged_result.cfy $testdir/s_$from-$to.cfy
        gzip -f $testdir/s_$from-$to.cfy
        popd >/dev/null 2>&1
    fi
}

# ....
dump_registry()
{
    if [ ${isWindows} -eq 1 ]; then
        # 2000 Server は相対パスはだめ？(2005/06/17)
        $REGCMD DELETE "$regpath_sydney" /f > /dev/null 2>&1
        #regedit /s $testtop/delete.reg
        $REGEDITCMD /s $dumpfile.reg
    else
        cp $dumpfile.conf ${systemconffile}
    fi
}

set_cascade_registry()
{
    if [ ! $cascade_conf ]; then
        if [ $isWindows -eq 1 ]; then
            cascade_conf="$testtop/cascade_for_windows.conf"
        else
            cascade_conf="$testtop/cascade_for_linux.conf"
        fi
    fi

    if [ ! -e $cascade_conf ]; then
        echo "${cascade_conf} not found."
        exit 1
    fi

    distoption="/distfile ${cascade_conf}"

    # ファイル中のバックスラッシュはエスケープされていないので perl でエスケープしてからシェルスクリプトで処理する。
    # そのためのエスケープテンプレート。perlの引数に渡すコード(正規表現)部分に直接記述すると意図した動作にならなかった。
    # シェルスクリプトの文字列処理のためにはバックスラッシュをエスケープする必要があるが最終的に regedit の引数に
    # 渡すときはエスケープはいらないので元に戻す。
    # (エスケープ) → (文字列加工) → (アンエスケープ)

    rexp_escape_backslash="s/\\\\/\\\\\\\\/g"
    rexp_unescape_backslash="s/\\\\\\\\/\\\\/g"
    rexp_replace_backslash="s/\\\\/\//g"

    # CascadeID カウンタ
    cascade_id=0

    cascade_conf_path_array=()
    cascade_conf_file_array=()
    cascade_service_name_array=()
    cascade_install_path_array=()

    # シェルの区切り文字を変更する。デフォルトではスペースも区切り文字なので改行のみにする。
    IFS_BACKUP=$IFS
    IFS=$'\r\n'
    for line in `perl -p -e $rexp_escape_backslash $cascade_conf`
    do
        # TODO: 最適化
        # 設定ファイルからレジストリのパスを抽出する
        cascade_conf_path=`echo $line | perl -p -e "s/^-?\s*//g;" | grep -e "^ConfPath:" | perl -p -e "s/^ConfPath:\s*//" | perl -p -e $rexp_unescape_backslash`
        cascade_service_name=`echo $line | perl -p -e "s/^-?\s*//g;" | grep -e "^ServiceName:" | perl -p -e "s/^ServiceName:\s*//" | perl -p -e $rexp_unescape_backslash`
        cascade_install_path=`echo $line | perl -p -e $rexp_unescape_backslash | perl -p -e $rexp_replace_backslash | perl -p -e "s/^-?\s*//g;" | grep -e "^InstallPath:" | perl -p -e "s/^InstallPath:\s*//;s/\"//g;"`

        if [ ! "$cascade_conf_path" = ""  ]; then
            cascade_conf_path_array=(${cascade_conf_path_array[@]} $cascade_conf_path)

            if [ $isWindows -eq 1 ]; then
                reg_filename=cascade${cascade_id}.reg
                $REGEDITCMD /e $reg_filename $cascade_conf_path
            else
                reg_filename=cascade${cascade_id}.conf
                cp ${cascade_conf_path} ${testtop}/${reg_filename}
            fi
            cascade_conf_file_array=(${cascade_conf_file_array[@]} $reg_filename)

            let cascade_id=${cascade_id}+1
        fi

        if [ ! "$cascade_service_name" = "" ]; then
            if [ $isWindows -eq 1 ]; then
                cascade_service_name_array=(${cascade_service_name_array[@]} $cascade_service_name)
            fi
        fi

        if [ ! "$cascade_install_path" = "" ]; then
            cascade_install_path_array=(${cascade_install_path_array[@]} $cascade_install_path)
        fi
    done
    IFS=$IFS_BACKUP
}

# ....
set_registry()
{
    if [ ${isWindows} -eq 1 ]; then
        $REGEDITCMD /e sydtest.reg "$regpath_sydney"
        if [ $lang ]; then
            $REGEDITCMD /s env${lang}.reg
        fi
        if [ $Message -eq 1 ]; then
            cat env_sydney.reg > env_bad_alloc_Fake_err.reg
            echo \"Common_MessageOutputInfo\"= \"1\" >> env_bad_alloc_Fake_err.reg
            $REGEDITCMD /s env_bad_alloc_Fake_err.reg
        fi
        $REGEDITCMD /e tmp.reg "$regpath_sydney"
    else
        cp ${systemconffile} sydtest.conf
        if [ $lang ]; then
            cat env${lang}.conf >> ${systemconffile}
        fi
        if [ $Message -eq 1 ]; then
            cat env_bad_alloc_Fake_err.conf >> ${systemconffile}
        fi
        cp ${systemconffile} $testtop/tmp.conf

        # テスト環境
        # default値との差分を表示
        if [ -e ./sydtest_default.conf ]; then
            diff tmp.conf sydtest_default.conf | grep -v '^[0-9]' | perl -p -e "s/^</System Parameter/;"
        fi
    fi

    # 分散テストの場合は子サーバのレジストリも設定
    if [ ${dir##*_} = "dist" ]; then
        stop_cascade_server
        set_cascade_registry
        if [ $restore_mode -eq 0 ] ; then
            start_cascade_server
        fi
    fi
}

#....
dump_cascade_db()
{
    # シェルの区切り文字を変更する。デフォルトではスペースも区切り文字なので改行のみにする。
    # デフォルトでは C:\Program Files\... などのインストールパスがスペースで区切られてしまう
    IFS_BACKUP=$IFS
    IFS=$'\r\n'
    for cascade_install_path in ${cascade_install_path_array[@]}; do
        cascade_dm="${cascade_install_path}/db"
        for testdb in `ls "${cascade_dm}"` ; do
            rm -rf "${cascade_install_path}/dumpdm/${dir}_${testdir}_$1"
            mkdir -p "${cascade_install_path}/dumpdm/${dir}_${testdir}_$1"
            cp -Rf "${cascade_dm}" "${cascade_install_path}/dumpdm/${dir}_${testdir}_$1/d_dm"
        done
    done
    IFS=$IFS_BACKUP
}

dumpdm()
{
    if [ $dump -eq 1 ]; then
        echo dumping...
        rm -rf $dumpdst/${dir}_${testdir}_$1
        mkdir -p $dumpdst/${dir}_${testdir}_$1
        cp -Rf "${dm}" $dumpdst/${dir}_${testdir}_$1/d_dm
        rm -rf $dumpdst/${dir}_${testdir}_$1/d_dm/save
        echo done.
    fi
}

# DBデータの復元
restore ()
{
    if [ $restore_mode -eq 1 ] ; then
        # 分散テストの場合は子サーバのDBデータも復元
        if [ ${dir##*_} = "dist" ]; then
            stop_cascade_server
            if [ $dump -eq 1 ]; then
                dump_cascade_db ${name}
            fi
            restore_cascade_db
        fi

        dumpfile=../../tmp
        if [ $isWindows -eq 0 ] ; then
            dumpfile=$testtop/tmp
        fi
        dump_registry

        # DB領域のrestore
        # single/normal/2860等でdata2ディレクトリ等を使っている。
        for testdb in `ls "${dm}"` ; do
            if [ $testdb != save ] ; then
                rm -rf "${dm}/$testdb"
            fi
        done

        cp -Rf "${save}/data" "${dm}"
        cp -Rf "${save}/system" "${dm}"
    fi

    kill_sleep
}

start_cascade_server()
{
    if [ -e $cascade_conf ]; then
    if [ $isWindows -eq 1 ]; then
        for cascade_service_name in ${cascade_service_name_array[@]}; do
        net start $cascade_service_name > nul 2>&1
        done
    else
        for cascade_install_path in ${cascade_install_path_array[@]}; do
        ${cascade_install_path}/bin/doquedb start > /dev/null 2>&1
        done
    fi
    fi
}

stop_cascade_server()
{
    if [ -e $cascade_conf ]; then
        if [ $isWindows -eq 1 ]; then
            for cascade_service_name in ${cascade_service_name_array[@]}; do
                net stop $cascade_service_name > nul 2>&1
            done
        else
            for cascade_install_path in ${cascade_install_path_array[@]}; do
                ${cascade_install_path}/bin/doquedb stop > /dev/null 2>&1
            done
        fi
    fi
}

restore_cascade_db()
{
    if [ ! $cascade_conf ]; then
        if [ $isWindows -eq 1 ]; then
            cascade_conf="$testtop/cascade_for_windows.conf"
        else
            cascade_conf="$testtop/cascade_for_linux.conf"
        fi
    fi

    if [ ! -e $cascade_conf ]; then
        echo "${cascade_conf} not found."
        exit 1
    fi

    if [ $isWindows -eq 1 ]; then
        for cascade_conf_path in ${cascade_conf_path_array[@]}; do
            $REGCMD DELETE $cascade_conf_path /f > nul 2>&1
        done

        for cascade_conf_file in ${cascade_conf_file_array[@]}; do
            $REGEDITCMD /s ${testtop}/${cascade_conf_file}
        done
    else
        cascade_id=0
        for cascade_conf_path in ${cascade_conf_path_array[@]}; do
            cp ${testtop}/cascade${cascade_id}.conf ${cascade_conf_path}
            let cascade_id=${cascade_id}+1
        done
    fi


    # シェルの区切り文字を変更する。デフォルトではスペースも区切り文字なので改行のみにする。
    # デフォルトでは C:\Program Files\... などのインストールパスがスペースで区切られてしまう
    IFS_BACKUP=$IFS
    IFS=$'\r\n'
    for cascade_install_path in ${cascade_install_path_array[@]}; do
        cascade_dm="${cascade_install_path}/db"
        if [ $isWindows -eq 1 ]; then
            cascade_save="${cascade_dm}/save"
        else
            cascade_save="${cascade_install_path}/save"
        fi

        for testdb in `ls "${cascade_dm}"` ; do
            if [ $testdb != save ] ; then
                rm -rf "${cascade_dm}/$testdb"
            fi
        done

        if [ $isWindows -eq 1 ]; then
            xcopy /s /q /e /k /x /i "${cascade_save}/data" "${cascade_dm}/data" > nul
            xcopy /s /q /e /k /x /i "${cascade_save}/system" "${cascade_dm}/system" > nul
        else
            cp -Rf "${cascade_save}/data" "${cascade_dm}"
            cp -Rf "${cascade_save}/system" "${cascade_dm}"
        fi
    done
    IFS=$IFS_BACKUP
}

# 正解LOGの基準はWindowsであり、plで対応しきれないSolaris, Linuxに対応
expectlog_os ()
{
    if [ `uname` = Linux ] && [ -e ./expect/L${name}.log ] ; then
        nameos=L${name}
    elif [ `uname` = SunOS ] && [ -e ./expect/S${name}.log ] ; then
        nameos=S${name}
    else
        nameos=${name}
    fi
}

check_core()
{
    sydtest=${installpath}/bin/SydTest
    corefile=`find core*`

    # 保存フォルダーを作成
    if [ ! -d CoreFile ] ; then
        mkdir CoreFile
    fi

    case $osname in
        linux)
            gdb $sydtest $corefile -x ../../core.B -batch > ./result/${name}.core
            if [ `grep -i 'Buffer::DaemonThread::runnable' ./result/${name}.core | wc -w` -ne 0 ] ; then
                perl -i".tmp" -p -e 's/^pure virtual method called\n//;s/^terminate called without an active exception\n//;\
                s/.+\(core dumped\) \$\{INSTALLDIR\}\\bin\\SydTest \$\*\n//;' ./result/${name}${paratest}.dat
                rm -f $corefile
                #rm -f ./result/${name}.core
            else
                mv $corefile CoreFile/core.${name}
            fi
            ;;
        solaris)
            #echo "Check CoreFile"
            dbx -c "where; quit" $sydtest $corefile > ./result/${name}.core
            mv $corefile CoreFile/core.${name}
            ;;
        *)
            echo "OS `uname` is unexpected."
            exit 1 ;;
    esac
}
# compare()から呼ばれる。multi系テストのoutファイルからtmpファイル(logファイル)を作成してdifを作成
# recoveryの場合はオプションを付加してtmp.outを作成した後にdifを作成する。
multidiff () {
    rm -f ./result/tmp.out
    s=0
    while [ $s -lt 51 ] ; do
        # "<"を二つつなげると、Meadowのshモードが解析に失敗する
        grep "<""<$s>>" ./result/${name}.out
        s=$(expr $s + 1)
    done > ./result/tmp.out
    grep "<""<100>>" ./result/${name}.out >> ./result/tmp.out

    # 外部コマンドが出力するログは正解としてそのまま残す (CriticalSectionログ用)
    if [ $inspect_raw_output -eq 1 ]; then
        grep -v "([0-9]\+) SydTest::" ./result/${name}.out >> ./result/tmp.out
    fi

    if [ \( ${dir%%_*} = "recovery" \) -a \( -e ./result/recovery_${name}.out \) ] ;then
        # 復旧のテスト
        if [ \( `grep -i 'ERR' ./result/${name}${paratest}.out | wc -w` -eq 0 \) -a \
         \( ${name} -ge 25520 \) -a \( ${name} -le 25529 \) ] ; then
            cat  ./result/recovery_${name}.out | perl -p -e 's/<<0>> \{\d+}/<<0>> {***}/' > ./result/tmp.out
        else
            cat  ./result/recovery_${name}.out >> ./result/tmp.out
        fi
    fi

    if [ $isWindows -eq 1 ]; then
        perl $testtop/preedit_expectlog.pl ./expect/${name}.log \
        > tmpexpectlog.log
    else
        expectlog_os
        perl $testtop/preedit_expectlog_for_${osname}.pl ./expect/${nameos}.log \
        | perl $testtop/preedit_expectlog.pl > tmpexpectlog.log
    fi

    if [ ${dir##*_} = "dist" ]; then
        perl $testtop/preedit_resultout_for_${osname}.pl ./result/tmp.out \
        | perl $testtop/log_normalizer.pl \
        | perl $testtop/dist_log_normalizer.pl > ./result/${name}.dat
    else
        perl $testtop/preedit_resultout_for_${osname}.pl ./result/tmp.out \
        | perl $testtop/log_normalizer.pl > ./result/${name}.dat
    fi

    if [ $isWindows -eq 1 ]; then
        diff -bi ./result/${name}.dat ./tmpexpectlog.log >./result/${name}.dif
    else
        if [ $regression_mode -eq 1 -a \
         `grep -i '\(core dumped\)' ./result/${name}${paratest}.dat | wc -w` -ne 0 ] ; then
            check_core
        fi
        diff -bi ./result/${name}.dat ./tmpexpectlog.log >./result/${name}.dif
    fi

}

# compare()から呼ばれる。single系テストのoutとlogからdifを作成,
# recoveryの場合はオプションを付加するがtmp.outを作成しない。
singlediff ()
{
    if [ \( ${dir%%_*} = "recovery" \) -a \( -e ./result/recovery_${name}${paratest}.out \) ] ;then
        cat  ./result/recovery_${name}${paratest}.out >> ./result/${name}${paratest}.out
    fi

    if [ $isWindows -eq 1 ]; then
        perl $testtop/preedit_expectlog.pl ./expect/${name}${paratest}.log \
        > tmpexpectlog.log
    else
        perl $testtop/preedit_expectlog_for_${osname}.pl ./expect/${nameos}${paratest}.log \
        | perl $testtop/preedit_expectlog.pl > tmpexpectlog.log
    fi

    if [ ${dir##*_} = "dist" ]; then
        perl $testtop/preedit_resultout_for_${osname}.pl ./result/${name}${paratest}.out \
        | perl $testtop/log_normalizer.pl \
        | perl $testtop/dist_log_normalizer.pl > ./result/${name}${paratest}.dat
    else
        perl $testtop/preedit_resultout_for_${osname}.pl ./result/${name}${paratest}.out \
        | perl $testtop/log_normalizer.pl > ./result/${name}${paratest}.dat
    fi

    if [ $isWindows -eq 1 ]; then
        diff -bi ./result/${name}${paratest}.dat ./tmpexpectlog.log >./result/${name}${paratest}.dif
    else
        if [ $regression_mode -eq 1 -a \
         `grep -i '\(core dumped\)' ./result/${name}${paratest}.dat | wc -w` -ne 0 ] ; then
            check_core
        fi
        if [ "$osname" = solaris ]; then
            diff -bi ./result/${name}${paratest}.dat ./tmpexpectlog.log >./result/${name}${paratest}.dif
        else
            diff -Bbi ./result/${name}${paratest}.dat ./tmpexpectlog.log >./result/${name}${paratest}.dif
        fi
    fi
}

# difファイルのエラー有無でsuccess/error/No logを出力
# difファイルが0の時difファイルを削除する。
compare()
{
    expectlog_os
    if [ $compare_mode -eq 1 ] ; then
        if [ -e ./expect/${nameos}${paratest}.log ] ; then
            if $diff; then
                echo "${name}${paratest}: Success."
                rm -f ./result/${name}${paratest}.dif ./result/${name}${paratest}.dat*
            else
                if [ $outdiff -eq 0 ] ; then
                    cat ./result/${name}${paratest}.dif
                fi

                # dif_normalizerはorder by構文句に対応していない
                if [ "$diff" = "singlediff" ] && [ `grep -i 'Main End' ./result/${name}${paratest}.dat | wc -w` -ne 0 ] \
                    &&  [ `grep -i '\(core dumped\)' ./result/${name}${paratest}.dat | wc -w` -eq 0 ] ; then
                    perl $testtop/dif_normalizer.pl ${name}${paratest} >./result/${name}${paratest}.dif.out
                    mv -f ./result/${name}${paratest}.dif.out ./result/${name}${paratest}.dif

                    if [ ! -s ./result/${name}${paratest}.dif ] ; then
                        echo "$name${paratest}: Success."
                        rm -f ./result/${name}${paratest}.dif ./result/${name}${paratest}.dat
                    else
                        echo "$name${paratest}: Error Occured."
                        if [ $retry -gt 0 ] ; then
                            retry_flag=1
                        fi
                    fi

                # multidiff及び、dif_normalizerに対応していないスクリプト
                else
                    echo "${name}${paratest}: Error Occured."
                    # 異常終了（コアダンプ）の場合はリトライモードであっても繰り返さない（ログを残すため）
                    if [ $retry -gt 0 ] \
                        && [ `grep -i 'Main End' ./result/${name}${paratest}.out | wc -w` -ne 0 ] \
                        && [ `grep -i '\(core dumped\)' ./result/${name}${paratest}.out | wc -w` -eq 0 ]; then
                        if [ "$testdir" = "normal" ] \
                            && [ `grep -i 'Connection Ran Out' ./result/${name}${paratest}.out | wc -w` -eq 0 ] \
                            && [ `grep -i 'Server is not available' ./result/${name}${paratest}.out | wc -w` -eq 0 ]; then
                            retry_flag=1
                        fi
                    fi
                fi
            fi
            else
                echo "No Log." > ./result/${name}${paratest}.dif
                echo "${name}${paratest}: No Log."
        fi
        else
        echo $name
    fi
}

preedittxt()
{
    rm -f ./result/${name}*.out ./result/recovery_${name}*.out
    perl $testtop/preedit_txt_for_${osname}.pl $name.txt > ${name}_.txt

    Rvalue=$?

    if [ ${dir%%_*} = "recovery" ] ;then
        if [ -e ./${name}r.txt ]; then
            perl $testtop/preedit_txt_for_${osname}.pl ${name}r.txt > ${name}r_.txt
            Rvalue=$?
        else
            cp -f recovery.txt ${name}r_.txt
        fi
    fi
}

envreg()
{
    if [ $likereg -gt 0 ] && [ $likereg -lt 4 ] ; then
        if [ ${isWindows} -eq 1 ] ; then
            cat ../../env_sydney.reg > env_Execution_LikeNormalizedString.reg
            echo \"Execution_LikeNormalizedString\"=dword:0000000$likereg >> env_Execution_LikeNormalizedString.reg
            $REGEDITCMD /s env_Execution_LikeNormalizedString.reg
            rm -f env_Execution_LikeNormalizedString.reg
        else
            echo Execution_LikeNormalizedString  $likereg >> ${systemconffile}
        fi
    fi
}

outputreg()
{
    if [ $1 -eq 1 ] ; then
        outputm=$testtop/$dir/$testdir/result/recovery_${name}.out
    else
        outputm=$testtop/$dir/$testdir/result/${name}.out
    fi
    cat $testtop/env_sydney.reg > $testtop/env_BoundsChecker.tmp
    cat >> $testtop/env_BoundsChecker.tmp <<_EOF_
"Common_MessageOutputError"= "$outputm"
"SydTest_MessageOutputError"= "$outputm"
"SydTest_MessageOutputInfo"= "$outputm"
"SydTest_MessageOutputTime"= "$outputm"
_EOF_
    sed -i "s/\//\\\\\\\\/g" $testtop/env_BoundsChecker.tmp
    perl -p -e "s/\x0D\x0A|\x0D|\x0A/\n/g" $testtop/env_BoundsChecker.tmp > $testtop/env_BoundsChecker.reg
    $REGEDITCMD /s $testtop/env_BoundsChecker.reg
}

sydneytest()
{
    # テスト実行開始
    if [ ${dir##*_} = "dist" ] && [ $restore_mode -eq 1 ] ; then
        start_cascade_server
    fi

    if [ ${dir%%_*} = "recovery" ] || [ -z "$analysis_mode" ] ; then
        # タイムアウトは -T オプションを指定した場合のみ有効
        set_timeout
        "$sydtest" $defaultoptions $sydtestoptions $distoption $userpasswd_op ${name}_.txt >> ./result/${name}${paratest}.out 2>&1
        unset_timeout
    else
        if [ "$analysis_mode" = "bc_mode" ] ; then
            bccheck &
            outputreg 0
            "$exeprefix" $cov_option "$sydtest" $defaultoptions $sydtestoptions $distoption $userpasswd_op ${name}_.txt 2>&1

       else
           if [ "$analysis_mode" = "inspxer_mode" ] ; then
                rm -rf $cvr_dst/$dir/$testdir/${name}
                mkdir -p $cvr_dst/$dir/$testdir/${name}
                cp -rf $cvr_dst/suppressions $cvr_dst/$dir/$testdir/${name}/
                cov_option="$intel_option -r $cvr_dst/$dir/$testdir/${name}"
                if [ $isLinux -eq 1 ] ; then
                    sydtest=${installpath}/bin/SydTest
                fi
            fi
           "$exeprefix" $cov_option "$sydtest" $defaultoptions $sydtestoptions $distoption $userpasswd_op ${name}_.txt >> ./result/${name}.out 2>&1
           Purify_End_Code=$?
           palling
       fi
   fi

}

recoverytest()
{
    echo RECOVERYSTART >> ./result/${name}.out
    dumpdm ${name}r

    if [ -z "$analysis_mode" ] ; then
        "$sydtest" $defaultoptions $sydtestoptions $distoption $userpasswd_op ${name}r_.txt >> ./result/recovery_${name}${paratest}.out 2>&1
    else
        if [ "$analysis_mode" = "bc_mode" ] ; then
             bccheck &
             outputreg 1
             "$exeprefix" $cov_option "$sydtest" $defaultoptions $sydtestoptions $distoption $userpasswd_op ${name}r_.txt 2>&1
        elif [ "$analysis_mode" = "inspxer_mode" ] ; then
            rm -rf $cvr_dst/$dir/$testdir/${name}
            mkdir -p $cvr_dst/$dir/$testdir/${name}
            cp -rf $cvr_dst/suppressions $cvr_dst/$dir/$testdir/${name}/
            cov_option="$intel_option -r $cvr_dst/$dir/$testdir/${name}"
            if [ $isLinux -eq 1 ] ; then
                sydtest=${installpath}/bin/SydTest
            fi
            "$exeprefix" $cov_option "$sydtest" $defaultoptions $sydtestoptions $distoption $userpasswd_op ${name}r_.txt >> ./result/recovery_${name}.out 2>&1
        else
            "$exeprefix" $cov_option "$sydtest" $defaultoptions $sydtestoptions $distoption $userpasswd_op ${name}r_.txt >> ./result/recovery_${name}.out 2>&1
            Purify_End_Code=$?
            palling
        fi
    fi
}

executionliketest()
{
    # 1
    likereg=1
    envreg
    paratest=L$likereg
    sydneytest

    # recoveryの場合、このテストも付加
    if [ ${dir%%_*} = "recovery" ] ;then
        recoverytest
    fi
    compare
    
    # 3
    likereg=3
    envreg
    paratest=L$likereg
    sydneytest

    # recoveryの場合、このテストも付加
    if [ ${dir%%_*} = "recovery" ] ;then
        recoverytest
    fi
    compare

    #初期化
    likereg=
    paratest=
}

# 1スクリプトのテスト実行と実行内容をoutファイルに出力
dotest()
{
    check_file
    if [ $analysis_mode ] ; then
        # テスト結果が存在したらスキップ
        if [ "$analysis_mode" = "purify_mode" ] || [ "$analysis_mode" = "bc_mode" ] ; then
            if [ -e $pur_dst/$dir/$testdir/${name}.pfy ] && [ $purify_sh_mode -eq 1 ] ; then
                echo "Skip $dir/$testdir/${name}"
                Purify_End_Code=0
                return
            fi
            if [ -e $pur_dst/$dir/$testdir/${name}.DPbcl ] && [ "$bc_sh_mode" -eq 1 ] ; then
                echo "Skip $dir/$testdir/${name}"
                return
            fi
        elif [ $analysis_mode = coverage ] ; then
            echo coverage $file
        fi
    fi

    # スクリプトのパス名変換
    preedittxt

    if [ $Rvalue -ne 0 ] ;then
        # Rオプションをつける
        if [ $Rvalue -eq 1 ] ;then
            defaultoptions="$defaultoptions /R"
        elif [ "$analysis_mode" = "purify_mode" ] || [ "$analysis_mode" = "bc_mode" ] ; then
            if [ $Rvalue -eq 6 ] ;then
                if [ "$purify_sh_mode" -eq 1 ] || [ "$bc_sh_mode" -eq 1 ] ; then
                    echo "Skip $dir/$testdir/${name}" >> $pur_dst/Skiplist.log
                    return
                fi
            elif [ $Rvalue -eq 7 ] && [ $purify_sh_mode -eq 1 ] ; then
                echo "Skip $dir/$testdir/${name}" >> $pur_dst/Skiplist.log
                return
            elif [ $Rvalue -eq 8 ] && [ "$bc_sh_mode" -eq 1 ] ; then
                echo "Skip $dir/$testdir/${name}" >> $dp_dst/Skiplist.log
                return
            fi
        fi
    fi

    # テスト
    sydneytest

    # recoveryの場合、このテストも付加
    if [ ${dir%%_*} = "recovery" ] ;then
        recoverytest
    fi

    compare

    # Execution_LikeNormalizedStringの値を変更してのテスト0 ->
    # カバレージテストとPurifyテスト は除外
    if [ "$analysis_mode" != "coverage_mode" ] && [ "$analysis_mode" != "purify_mode" ] && [ "$optest" = "L" ] ; then
        if [ $diff = singlediff ] && [ ! `grep "like" ${name}.txt | wc -l` -le 0 ] ;then
            if [ `perl ../../Execution_LikeNormalizedString.pl < ${name}.txt | wc -w` -ge 1 ] ;then
                executionliketest
            fi
        fi
    fi

    # purifyテストで出来たresult.pfy(txt)を${name}.pfy(txt)に変更
    if [ "$analysis_mode" = "purify_mode" ] && [ $Purify_End_Code -eq 0 ] ; then
        mv -f $cov_dst/$dir/$testdir/$PTF $cov_dst/$dir/$testdir/${name}.pfy
        mv -f $cov_dst/$dir/$testdir/result.txt $cov_dst/$dir/$testdir/${name}.txt
    fi

    # BoundsCheckerテストで出来たresult.DPbcl(xml)を${name}.DPbcl(xml)に変更
    if [ "$analysis_mode" = "bc_mode" ] ; then
        mv -f $testtop/$dir/$testdir/result.DPbcl $cov_dst/$dir/$testdir/${name}.DPbcl
        mv -f $testtop/$dir/$testdir/result.xml $cov_dst/$dir/$testdir/${name}.xml
    fi

    rm -f ${name}_.txt ${name}r_.txt
    dumpdm $name
    restore
    palling
}

# retry モード
retry_dotest()
{
    retry_counter=0
    while [ $retry_counter -ne $retry ]
    do
        retry_flag=0
        dotest
        retry_counter=`expr $retry_counter + 1`

        # 成功または異常終了の場合は抜ける
        if [ $retry_flag -eq 0 ] ; then
            break
        fi
    done
}

#########
# メイン
#########

# パラメータの初期値
from=0000
to=9999
timeout=0
testdir=normal
dump=0
compare_mode=1
restore_mode=1
outdiff=0
analysis_mode=
lang=
Message=0
userpasswd_op="/user root /password doqadmin"
cascade_conf=
retry=0
inspect_raw_output=0
# 引数より実行するディレクトリと内容を得る
while getopts 'neNdf:t:s:rlo:pcbmT:JEZ:MLjuC:R:i' opt ; do
    case $opt in
    f) from=$OPTARG ;;
    t) to=$OPTARG ;;
    s) from=$OPTARG; to=$OPTARG ;;
    T) timeout=$OPTARG ;;
    n) testdir=normal ;;
    e) testdir=except ;;
    N) restore_mode=0 ;;
    d) dump=1 ;;
    r) compare_mode=0 ;;
    l) outdiff=1 ;;
    o) sydtestoptions=$OPTARG ;;
    u) userpasswd_op= ;;
    p) analysis_mode=purify_mode ;;
    c) analysis_mode=coverage_mode ;;
    b) analysis_mode=bc_mode ;;
    m) analysis_mode=inspxer_mode ;;
    J) lang=_ja ;;
    E) lang=_eu ;;
    Z) lang=_zh ;;
    M) Message=1 ;;
    L) optest=L ;;
    u) userpasswd_op= ;;
    j) analysis_mode=jdbc_mode ;;
    C) cascade_conf=$OPTARG ;;
    R) retry=$OPTARG ;;
    i) inspect_raw_output=1 ;;
    *) cat << _EOF_
usage:
bash SydTest_sh.txt [options] <test_dir>
-n
execute normal test
-e
execute except test
-f <num> -t <num>
set the range of executing scripts
-s <num>
set only single script
-N
do not restore after each script
-d
dump database before each recovery and restore
-T
set timeout for each script
-l
write a diff file for every script
-r
do not compare the outputs with *.log
-o <string>
set options for SydTest.exe
-p
execute in Purify mode
-c
execute in PureCoverage mode
-b
execute in BoundsChecker mode
-m
execute in IntelInspector mode
-J
use UNA of dmja, which is for Japanese
-E
use UNA of dmeu, which is for European language
-Z
use UNA of dmzh, which is for Chinese
-j
use JDBC version of SydTest
-R <num>
retry test if it fails
-i
inspect raw output executed by external command by SydTest command 'System'
_EOF_
exit 1 ;;
    esac
done
shift `expr $OPTIND - 1`
dir=$*

#################### メインループ ####################

check_directory
check_range
instrument
set_registry

echo "start testing $dir/$testdir"
pushd $dir/$testdir >/dev/null 2>&1

for ((number=10#$from; $number<=10#$to; number++)) ; do
    length=${#number};
    zero="00000";
    if [ $number -lt 10000 ] ; then
        prefix=${zero:0:4-$length};
    elif [ $number -lt 100000 ]; then
        prefix=${zero:0:5-$length};
    else
        prefix=${zero:0:6-$length};
    fi
    file="$prefix$number.txt";
    if [ -f $file ] ; then
        # name=`basename $file .txt` basenameがうまく動いていない？
        name=`echo $file | sed 's/\.txt//'`

        if [ $name -lt $from ] ; then
            continue
        elif [ $name -gt $to ] ; then
            break
        fi

        if [ $retry -gt 0 ] ; then
          retry_dotest
        else
          dotest
        fi
    fi
done

echo "end testing $dir/$testdir"
popd
if [ $restore_mode -eq 0 ] ; then
    restore_mode=1
    restore
fi

dumpfile=sydtest
dump_registry
if [ ${dir##*_} = "dist" ]; then
    stop_cascade_server
fi
#dump_cfy

#################### ############ ####################

#
# Copyright (c) 2001, 2023, 2024 Ricoh Company, Ltd.
# All rights reserved.
#
