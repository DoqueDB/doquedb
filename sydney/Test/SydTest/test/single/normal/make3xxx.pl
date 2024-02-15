# コマンドのcommitやrollbackのテストをするスクリプトを生成する。
use lib "../..";
use MakeScript;
#↓納得いかないが必要っぽい
@column = @MakeScript::column;

$MakeScript::cmdnum = ""; #念のため宣言

#=====

#=====

@cr = ("commit", "rollback");
$mode_0 = sub()
{
    my ($trm, $trl, $setp, $rbp) = @_;
    print qq(# 空のトランザクションの動作確認\n);
    start_transaction($trm, $trl, $setp);
    comm($cr[$rbp]);
};

$mode_123 = sub()
{
    my ($trm, $trl, $setp, $rbp) = @_;
    print qq(# 1つのtransactionで1つだけのタプルを操作する\n);
    print qq(# タプルの準備\n);
    start_transaction($trm, $trl, $setp);
    insert_command(1, [[{list=>0}]]);
    comm("select * from T1");
    comm("commit");
    if ($rbp) {
	print qq(# insert->rollbackの動作確認\n);
	start_transaction($trm, $trl, $setp);
	insert_command(1, [[{list=>1}]]);
	comm("select * from T1");
	comm("rollback");
    }
    comm("select * from T1");
    print qq(# update->$cr[$rbp]の動作確認\n);
    start_transaction($trm, $trl, $setp);
    update_command(1, [[{list=>1}]], [[{list=>0}]]);
    comm("select * from T1");
    comm($cr[$rbp]);
    comm("select * from T1");
    print qq(# delete->$cr[$rbp]の動作確認\n);
    start_transaction($trm, $trl, $setp);
    delete_command(1, [[{list=>(1-$rbp)}]]);
    comm("select * from T1");
    comm($cr[$rbp]);
    comm("select * from T1");
};

$mode_45 = sub()
{
    my ($trm, $trl, $setp, $rbp) = @_;

    print qq(# 一度のtransactionで複数のタプルを操作する\n);
    start_transaction($trm, $trl, $setp);
    insert_command(1, [[{list=>0}]]);
    insert_command(1, [[{list=>1}]]);
    comm("select * from T1");
    comm("commit");
    if ($rbp) {
	print qq(# insert->rollbackの動作確認\n);
	start_transaction($trm, $trl, $setp);
	insert_command(1, [[{list=>2}]]);
	insert_command(1, [[{list=>3}]]);
	comm("select * from T1");
	comm("rollback");
    }
    comm("select * from T1");
    print "\n";
    print qq(# update->$cr[$rbp]の動作確認\n);
    start_transaction($trm, $trl, $setp);
    update_command(1, [[{list=>2}]], [[{list=>0}]]);
    update_command(1, [[{list=>3}]], [[{list=>1}]]);
    comm("select * from T1");
    comm($cr[$rbp]);
    comm("select * from T1");
    print "\n";
    print qq(# delete->$cr[$rbp]の動作確認\n);
    start_transaction($trm, $trl, $setp);
    delete_command(1, [[{list=>2-$rbp*2}]]);
    delete_command(1, [[{list=>3-$rbp*2}]]);
    comm("select * from T1");
    comm($cr[$rbp]);
    comm("select * from T1");
    print "\n";
};

$mode_6 = sub()
{
    my ($trm, $trl, $setp) = @_;
    my $rbp = 1;
    
    print qq(# 補償コマンドの動作確認\n\n);
    print qq(# insert->delete->$cr[$rbp]の動作確認\n);
    insert_command(1, [[{list=>0}]]);
    start_transaction($trm, $trl, $setp);
    insert_command(1, [[{list=>1}]]);
    delete_command(1, [[{list=>1}]]);
    comm("select * from T1");
    comm($cr[$rbp]);
    comm("select * from T1");
    print "\n";

    print qq(# 打ち消しあうupdateの対->$cr[$rbp]の動作確認\n);
    start_transaction($trm, $trl, $setp);
    update_command(1, [[{list=>1}]], [[{list=>0}]]);
    update_command(1, [[{list=>0}]], [[{list=>1}]]);
    comm("select * from T1");
    comm($cr[$rbp]);
    comm("select * from T1");
    print "\n";

    print qq(# delete->insert->$cr[$rbp]の動作確認\n);
    start_transaction($trm, $trl, $setp);
    delete_command(1, [[{list=>0}]]);
    insert_command(1, [[{list=>0}]]);
    comm("select * from T1");
    comm($cr[$rbp]);
    comm("select * from T1");
    print "\n";

    print qq(# もう1つタプルを追加\n);
    insert_command(1, [[{list=>1}]]);
    print qq(# insert->delete->$cr[$rbp]の動作確認\n);
    start_transaction($trm, $trl, $setp);
    insert_command(1, [[{list=>2}]]);
    insert_command(1, [[{list=>3}]]);
    delete_command(1, [[{list=>2}]]);
    delete_command(1, [[{list=>3}]]);
    comm("select * from T1");
    comm($cr[$rbp]);
    comm("select * from T1");
    print "\n";

    print qq(# 打ち消しあうupdateの対->$cr[$rbp]の動作確認\n);
    start_transaction($trm, $trl, $setp);
    update_command(1, [[{list=>2}]], [[{list=>0}]]);
    update_command(1, [[{list=>3}]], [[{list=>1}]]);
    update_command(1, [[{list=>0}]], [[{list=>2}]]);
    update_command(1, [[{list=>1}]], [[{list=>3}]]);
    comm("select * from T1");
    comm($cr[$rbp]);
    comm("select * from T1");
    print "\n";

    print qq(# delete->insert->$cr[$rbp]の動作確認\n);
    start_transaction($trm, $trl, $setp);
    delete_command(1, [[{list=>0}]]);
    delete_command(1, [[{list=>1}]]);
    insert_command(1, [[{list=>0}]]);
    insert_command(1, [[{list=>1}]]);
    comm("select * from T1");
    comm($cr[$rbp]);
    comm("select * from T1");
    print "\n";

    print qq(# 後始末\n);
    delete_command(1, [[{list=>0}]]);
    delete_command(1, [[{list=>1}]]);
    print "\n";
};

sub table_block($$$$)
{
    my ($mode, $args, $filetype, $heapp) = @_;
    my $i;
    #表番号、filetype, heapp, 型
    create_table(1, $filetype, $heapp, [array_column_type("ntext")]);

    &$mode(@$args);

    drop_table(1);
    print "\n";
}

sub write_script($$)
{
    my ($mode, $args)=@_;
    my $i;

    light_header();
    my @filetypes = (0, 2..4, 6); #btreeが欠けているが勘弁
    foreach $filetype (@filetypes) {
	$MakeScript::PlainFTS = ($filetype==2);
	foreach $heapp (0, 1) {
	    table_block($mode, $args, $filetype, $heapp);
	}
    }
    light_trailer();
}

#===== main =====

@tr_mode = ("read only, using snapshot", "read write");
@tr_ilvl = ("", #"read uncommitted", 
	    "read committed", "repeatable read", "serializable");
@modes = ($mode_0, $mode_123, $mode_45, $mode_6);
@modedesc = ("null", "update", "updates", "compensation");
@setxaction = ("", "set transaction");

#ループが五重になってしまった。
foreach $trl (0..3) {
  foreach $setp (0, 1) {
    $offset1 = 3010+$trl*80+$setp*40;
    foreach $mode (0..3) {
      foreach $trm (0, 1) {
        foreach $rbp (0, 1) {
	    $offset2 = $mode*8 + $trm*4 + $rbp*2;
	    next if grep($offset2 == $_, (16, 18, 24, 26, 28));
	    if ($offset2 == 8 || $offset2 == 10) {$offset2 += 1;}
	    if ($offset2 == 30) {$offset2 -= 6;}
    $args = [$tr_mode[$trm], $tr_ilvl[$trl], $setp, $rbp];
    open (OUT, ">".sprintf("%04d.txt", $offset1+$offset2));
    select OUT;
    print "# transaction $tr_mode[$trm] $tr_ilvl[$trl]".
	  "$setxaction[$setp] $cr[$rbp] $modedesc[$mode]\n";
    print "# commitあるいはrollbackの動作確認をする。\n";
    print "# 各々のスクリプトごとに各種の論理ファイルを試す。\n";
    write_script($modes[$mode], $args);
    close OUT;
    select STDOUT;
	}
      }
    }
  }
}


