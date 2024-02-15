use lib "../..";
use MakeScript;
@column = @MakeScript::column;

$MakeScript::cmdnum = ""; #念のため宣言

#=====

sub create_table_only($$$$)
{
    my ($tnum, $filetype, $heapp, $schema) = @_;
    my $col, $type, $coldef="", $colcount=1;
    my $array, $hint, $ctype, $i;

    # @tablesに登録
    $MakeScript::tables[$tnum] = $schema;

    foreach $col (@$schema){
	if (ref $col) {
	    $array = " array [no limit]"; 
	    $ctype = $col->[0];
	} else {
	    $array = "";
	    $ctype = $col;
	}
	$hint = $heapp  ? " hint heap" : "";
	$coldef .= "C$colcount $column[$ctype]->{name}$array$hint, ";
	$colcount++;
    }
    $coldef =~ s/, $//;
    $colcount--;
    if ($filetype == 10) {
	$coldef .= ", primary key(".
	    MakeScript::make_columnlist("C*", $schema) .")";
    }
    comm("create table T$tnum($coldef)");
}

sub create_index($$$$)
{
    my ($tnum, $filetype, $heapp, $schema) = @_;
    if ($filetype == 1) { 
	# 複合索引
	my $cols = make_columnlist("C*", $schema);
        comm("create index I$tnum on T$tnum($cols)"); 

	if ($colcount >= 2) {
	    for($i=1; $i<=$colcount; $i++) {
		comm("create index I${tnum}_$i on T$tnum(C$i)");
	    }
	}
    } elsif (isfts($filetype)) { 
	for($i=1; $i<=$colcount; $i++) {
	    my $hint = get_ftshint($filetype);
	    $hint = $hint ? " hint '$hint'" : "";
	    my $type = $schema->[$i-1];
	    if ((ref $type) && $column[$type->[0]]->{mask}
		|| $column[$type]->{mask}) {
		comm("create fulltext index I${tnum}_$i on T$tnum(C$i)$hint");
	    }
	}
    }
}

sub insert_sequence($)
{
    my ($arrayp) = @_;
    foreach $i (0..4) {
	if ($arrayp) {
	    $i = [{list=>$i}];
	} else {
	    $i = {list=>$i};
	}
	insert_command(1, [$i]);
    }
}

sub inscreateblock($$$)
{
    my ($filetype, $heapp, $arrayp) = @_;

    if ($arrayp) {
	$ftscol = array_column_type("ntext");
    } else {
	$ftscol = column_type("ntext");
    }

    create_table_only(1, $filetype, $heapp, [$ftscol]);
    insert_sequence($arrayp);
    print "# Indexが正常につけられれば成功\n";
    create_index(1, $filetype, $heapp, [$ftscol]);
    print "# 念のためselect\n";
    comm("select * from T1");
    drop_table(1);
    print "\n";
}

sub write_script($$$)
{
    my ($number, $filetype, $heapp)=@_;
    my $i, $j, $nullmode;

    open (OUT, ">".sprintf("%04d.txt", $number));
    select OUT;
    print "# insert->create index ".get_filedesc($filetype, $heapp, "")."\n";
    print "# 先に索引のない状態で何件か入っているtableに後から索引をつけるテストを行う。\n\n";
    header();
    
    print qq(InitializeSession "TESTDB";\n);
    print "\n";
    if (!grep($filetype==$_, (3, 5, 7, 9))) {
	print qq(# 列がスカラ型であるtableのテスト\n);
	inscreateblock($filetype, $heapp, 0);
	print "\n";
    }
    print qq(# 列が配列型であるtableのテスト\n);
    inscreateblock($filetype, $heapp, 1);
    print "\n";
    print qq(TerminateSession;\n);

    trailer();
    close OUT;
    select STDOUT;
}

foreach $filetype (2..9) {
    foreach $heapp (0, 1) {
	write_script(2592+4*$filetype+2*$heapp, $filetype, $heapp);
    }
};

