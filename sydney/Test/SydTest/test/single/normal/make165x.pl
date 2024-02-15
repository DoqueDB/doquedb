# 各種全文ファイルへのスコア検索を試すスクリプトを生成する
use lib "../..";
use MakeScript;
@column = @MakeScript::column;

$MakeScript::cmdnum = ""; #念のため宣言

#=====

sub insert_sequence($@)
{
    my ($arrayp, @tuples) = @_;
    my $count = 1;
    foreach $i (@tuples) {
	$i = qq("$i");
	$i = [$i] if $arrayp;
	insert_command(1, [$count, $i]);
	$count++;
    }
}

sub like_sequence(@)
{
    my (@tuples) = @_;
    foreach $i (@tuples) {
	$i = qq("$i");
	commp("select C1, score(C2) from T1 where C2 like ?", $i);
    }
}

sub liketestblock($$$)
{
    my ($filetype, $heapp, $arrayp) = @_;

    if ($arrayp) {
	$ftscol = array_column_type("ntext");
    } else {
	$ftscol = column_type("ntext");
    }

    print qq(# Tableを準備する\n);
    create_table(1, $filetype, $heapp, [column_type("int"), $ftscol]);

#    print qq(# \n);
    print qq(# 検索対象となるタプルを準備する\n);
    insert_sequence($arrayp, "ほげ", "ほげほげ", 
		    "げほげほ", "はらほげ", "ほえ", "");
    #具体的な単語を含んでいなければならない
    print qq(# スコア検索を行う\n);
    like_sequence("ほげ", "%ほげ", "ほげ%", "%ほげ%");

    print qq(# tableの後始末\n);
    drop_table(1);
    print "\n";
}

sub write_script($$$)
{
    my ($number, $filetype, $heapp)=@_;
    my $i, $j, $nullmode;

    open (OUT, ">".sprintf("%04d.txt", $number));
    select OUT;
    print "# score_search ".get_filedesc($filetype, $heapp, "")."\n";
    print "# 各種全文ファイルへのスコア検索を試すスクリプトを生成する\n";
    header();
    
    print qq(InitializeSession "TESTDB";\n\n);

    if (!grep($filetype==$_, (3, 5, 7, 9))) {
	print qq(# 列がスカラ型であるtableのテスト\n);
	liketestblock($filetype, $heapp, 0);
    }
    print qq(# 列が配列型であるtableのテスト\n);
    liketestblock($filetype, $heapp, 1);

    print qq(TerminateSession;\n);

    trailer();
    close OUT;
    select STDOUT;
}

foreach $filetype (2..9) {
    foreach $heapp (0, 1) {
	write_script(1642+4*$filetype+2*$heapp, $filetype, $heapp);
    }
}

