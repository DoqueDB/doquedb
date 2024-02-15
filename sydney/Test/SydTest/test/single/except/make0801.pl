# コマンドのテストをするスクリプトを生成する。

use lib "../..";
use MakeScript;
@column = @MakeScript::column;

%dispatcher = ("insert"=>$MakeScript::ins_mode,
	       "update"=>$MakeScript::upd_mode,
	       "pkey"  =>$MakeScript::pkey_mode,
	       "delete"=>$MakeScript::del_mode,
	       "select"=>undef);

$MakeScript::cmdnum = ""; #念のため宣言

#=====

sub make_tupleset($$;$)
{
    my ($schema, $nullmode,$filetype) = @_;
    my $col, $count=0, @tupleset=();

    #nullにする方法について見直しが必要
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

    if ($mode ne "select") {
	selectall_command(1);
	&{$dispatcher{$mode}}(@tupleset);
	selectall_command(1);
    } else {
	insert_command(1, $tupleset[0]);
	if ($filetype != 10) {
	    insert_command(1, $tupleset[0]);
        }
	insert_command(1, $tupleset[1]);
	insert_command(2, $tupleset[2]);
        selectall_command(1);
	comm("select count(*) from T1");
	select_join_command(1, 2, $tupleset[0], $tupleset[2]);
	select_cond(1, $tupleset[0]);
    }
}
#↑↓合併可能
sub testblock($$$$@)
{
    my ($mode, $filetype, $heapp, $nullmode, @tableschema) = @_;

    create_table(1, $filetype, $heapp, \@tableschema);
    if ($mode eq "insert" || $mode eq "select") {
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
    print "# $mode\[$colnum\]"
	.get_filedesc($filetype, $heapp, 0)."\n";
    header();
    print qq(InitializeSession "TESTDB";\n);
    @nullmodes = (0);
    @columntypes = (0..(@column-1));
    foreach $nullmode (@nullmodes) {
	foreach $i (@columntypes) {
	    testblock($mode, $filetype, $heapp, $nullmode, ([$i]));
	    testblock($mode, $filetype, $heapp, $nullmode, ($i)) if $i==6;
	}
    }

    print qq(TerminateSession;\n);
    trailer();
    close OUT;
    select STDOUT;
}

write_script("select", 1, 801, 0, 0);

