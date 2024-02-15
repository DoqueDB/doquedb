# 一時表のテストをするスクリプトを生成する。

use lib "../..";
#use lib "../../..";
use MakeScript;
@column = @MakeScript::column;

$MakeScript::cmdnum = ""; #念のため宣言

#=====

#table番号を固定にしてしまった。

$ins_mode = sub()
{
    print qq(# insertのテスト\n);
    insert_command(1, $_[0]);
    insert_command(1, $_[1]);
    print qq(# 複数タプルをまとめてinsert\n);
    insert_command(2, $_[2], $_[3]);
    print qq(# 普通の表から一時表にinsert...select\n);
    insert_select_command(1, 2, $_[2]);
    print qq(# 一時表自身にinsert...select\n);
    insert_select_command(1, 1, $_[0]);
    print qq(# 一時表から普通の表にinsert...select\n);
    insert_select_command(1, 2, $_[1]);
    print qq(# T2をselectしてみる\n);
    selectall_command(2);
};

$upd_mode = sub()
{
    my (@tuples) = @_;
    print qq(# SQLコマンドupdateをテストする\n);
    print qq(# update対象のタプルをまずinsert\n);
    insert_command(1, $tuples[0]);
    print qq(# 自分自身にupdate\n);
    updateall_command(1, $tuples[0]);
    print qq(# 別の内容にupdate\n);
    updateall_command(1, $tuples[1]);
    print qq(# updateされない(はずの)タプルをinsert\n);
    insert_command(1, $tuples[3]);
    print qq(# 存在しないタプルをwhere句で指定\n);
    update_command(1, $tuples[1], $tuples[0]);
    print qq(# 存在するタプルをwhere句で指定。片方のみ更新される\n);
    update_command(1, $tuples[2], $tuples[1]);
};

$del_mode = sub()
{
    print qq(# 2タプル挿入する\n);
    insert_command(1, $_[0], $_[1]);
    print qq(# 1つずつ消す\n);
    delete_command(1, $_[0]);
    delete_command(1, $_[1]);
    print qq(# 正常にいけば表は空になっているはず\n);
};

$pkey_mode = sub () {
    print qq(# primary key指定で得たBtreeファイルをテストする\n);
    insert_command(1, $_[0]);
    insert_command(1, $_[1]);
    print qq(# 重複したタプルはinsertできないはず\n);
    insert_command(1, $_[1]);
    print qq(# 既存のタプルとかぶる内容にupdateすることもできないはず\n);
    update_command(1, $_[0], $_[1]);
};

%dispatcher = ("insert"=>$ins_mode,
	       "update"=>$upd_mode,
	       "pkey"  =>$pkey_mode,
	       "delete"=>$del_mode,
	       "select"=>undef);
#=====

sub make_tupleset($$;$)
{
    my ($schema, $nullmode, $filetype) = @_;
    my $col, $count=0, @tupleset=();

    # array型か否か、nullを含めるか否かによって
    # ins_mode等に与えるタプルの集合を違える
    foreach $col (@$schema) {
	if (ref $col) {
	    push (@{$tupleset[0]}, ($nullmode&1 ? ["null", {list=>0}] : 
				    [{list=>0}, {list=>1}]));
	    push (@{$tupleset[1]}, ($nullmode&2 ? [{list=>0}, "null"] :
				    [{list=>1}, {list=>0}]));
	    push (@{$tupleset[2]}, ($nullmode&1 ? ["null", {list=>1}] :
				    [{list=>0}, {list=>2}]));
	    push (@{$tupleset[3]}, ($nullmode&2 ? [{list=>1}, "null"] :
				    [{list=>2}, {list=>0}]));
	} else {
	    push (@{$tupleset[0]}, ($nullmode&1 ? "null" : {list=>0}));
	    push (@{$tupleset[1]}, ($nullmode&2 ? "null" : {list=>1}));
	    push (@{$tupleset[2]}, ($nullmode&1 ? "null" : {list=>2}));
	    push (@{$tupleset[3]}, ($nullmode&2 ? "null" : {list=>3}));
	}
	$count = ($count+1)%2;
    }
    return @tupleset;
}

sub write_block($$$;$)
{
    my ($mode, $nullmode, $schema, $filetype) = @_;
    my @tupleset = make_tupleset($schema, $nullmode, $filetype);

    # モードによって選択
    if ($mode ne "select") {
	selectall_command(1);
	&{$dispatcher{$mode}}(@tupleset);
	selectall_command(1);
    } else {
	print qq(# selectの対象となるタプルをinsertする\n);
	insert_command(1, $tupleset[0]);
	if ($filetype != 10) {
	    insert_command(1, $tupleset[0]);
        }
	insert_command(1, $tupleset[1]);
	print qq(# 2つ目のtableにもinsertする\n);
	insert_command(2, $tupleset[2]);
	print qq(# 全体が取れるかどうか、何通りかの方法で試す\n);
        selectall_command(1);
	comm("select * from T1");
	comm("select count(*) from T1");
	print qq(# 複数tableのjoinをselectする\n);
	select_join_command(1, 2, $tupleset[0], $tupleset[2]);
	if ($nullmode == 0 && grep($schema->[0]==$_, (0..5))) {
	    print qq(# さまざまな条件を課しつつselectする\n);
	    select_cond(1, $tupleset[0]);
	}
    }
}

sub testblock($$$$@)
{
    my ($mode, $filetype, $heapp, $nullmode, @tableschema) = @_;

    create_table(1, $filetype, $heapp, \@tableschema);
    if ($mode eq "insert" || $mode eq "select") {
	# insertやselectのテストでは2つ目のtableも使用する
	create_table(2, $filetype, $heapp, \@tableschema);
    }
    write_block($mode, $nullmode, \@tableschema, $filetype);
    drop_table(1);
    if ($mode eq "insert" || $mode eq "select") {
	drop_table(2);
    }
    print "\n";
}

sub write_script($$$$$)
{
    my ($mode, $colnum, $number, $filetype, $heapp)=@_;
    my $i, $j, $nullmode;

    open (OUT, ">".sprintf("%04d.txt", $number));
    select OUT;
    print "# temporary table\n";
    print "# $mode\[$colnum\]".get_filedesc($filetype, $heapp, 0)."\n";
    header();
    print qq(InitializeSession "TESTDB";\n);
    if ($filetype<2 || $number%2==1) {
	@nullmodes = (0..2);
    } else {
	@nullmodes = (0);
    }
    if ($number == 901 || $number == 905) {
	@nullmodes = (1, 2);
    }
    # 対象ファイルがftsのときはntextのみ、それ以外のときは全ての型について試す
    @columntypes = (isfts($filetype)) ? (1) : (0..(@column-1));

    foreach $nullmode (@nullmodes) {
	print qq(\n# nullを含むタプルも試す\n\n) if $#nullmodes==3 && $nullmode==1;
	foreach $i (@columntypes) {
	    #ここでスキーマを生成
	    my $j, @tableschema=(), $addcol;
	    for($j=0; $j<$colnum; $j++) {
		$addcol = ($i+$j) % (0+@column);
		if (($filetype < 3 || $column[$addcol]->{mask}) && 
		    !(($filetype==1 || $filetype==10) && 
			$column[$addcol]->{name} eq "image")) {
		    push (@tableschema, $addcol);
		}
	    }
	    if (@tableschema > 0) {
	#sectionizedオプションをarrayでない列につけることはできない
		if (!grep($filetype==$_, (3, 5, 7, 9))) {
		    testblock($mode, $filetype, $heapp, $nullmode, 
			      @tableschema);
		}
		if ($filetype != 1 && $filetype != 10) {
		    testblock($mode, $filetype, $heapp, $nullmode, 
			       map([$_], @tableschema));
		}
	    }
	}
    }

    print qq(TerminateSession;\n);
    trailer();
    close OUT;
    select STDOUT;
}

@modetable = ("insert", "update", "delete", "select");

sub make_scripts($$$)
{
    my ($filetype, $colnum, $offset) = @_;
    foreach $modenum (0..3) {
	$mode=$modetable[$modenum];
#	$MakeScript::PlainFTS = ($filetype==2);
	foreach $heapp (0, 1) {
	    $number= $offset+$heapp*10+$modenum*2;
	    write_script($mode, $colnum, $number, $filetype, $heapp);
#	    if ($filetype == 1 && $mode eq "update") {
#		# primary keyの異常系テストも行う
#		$number += 1;
#		write_script("pkey", $colnum, $number, 10, $heapp);
#	    }
	}
    }
}

#===== main =====

#(0..9)にする意味、あまりなさそう。
foreach $filetype (0..2) {
    foreach $colnum (1..2) {
	make_scripts ($filetype, $colnum, 980+$filetype*50+$colnum*20)
    }
}
