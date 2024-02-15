#
# Copyright (c) 2001, 2023 Ricoh Company, Ltd.
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
# SydTest用スクリプトを自動生成するための関数群を提供する。

package MakeScript;
use Exporter;
@ISA = (Exporter);
@EXPORT = (header, trailer, comm, commp, commn, commnp, 
rset, rsetp, rsetn, rsetnp,
make_parameter_string, column_type, array_column_type,
light_header, light_trailer, start_transaction, isfts,
insert_command, insert_select_command, 
updateall_command, update_command, delete_command,
select_command, select_alias_command, select_join_command, 
selectall_command, select_cond, select_like_command, 
get_ftshint, get_filedesc, create_table, drop_table,
@column);

#=====

BEGIN {

@filetypes = ("record", "btree(index)", 
"fulltext(plain)", "fulltext(sectionized)", 
"fulltext(delayed)", "fulltext(sectionized, delayed)", 
"fulltext(normalized)", "fulltext(normalized, sectionized)", 
"fulltext(normalized, delayed)", 
"fulltext(normalized, sectionized, delayed)", "btree(primary key)");

@column = (
    {name=>"nvarchar(32)", mask=>1,
      value=>[qq("ほげ"), qq("ホゲ"),qq("ぴよ"), qq("ピヨ"), qq("へめれれ")]
    },
    {name=>"ntext", mask=>1,
      value=>[qq(textsjisfile "..\\\\..\\\\doc\\\\hello.txt"),
	      qq(textsjisfile "..\\\\..\\\\doc\\\\ricoh.txt"),
	      qq(textsjisfile "..\\\\..\\\\doc\\\\rfc3092.txt"),
	      qq(textsjisfile "..\\\\..\\\\doc\\\\rfc822.txt"),
	      qq(textsjisfile "..\\\\..\\\\doc\\\\ls.txt")]
    },
    {name=>"fulltext", mask=>1,
      value=>[qq(textsjisfile "..\\\\..\\\\doc\\\\kenpou.txt"),
	      qq(textsjisfile "..\\\\..\\\\doc\\\\rasyoumon.txt"),
	      qq(textsjisfile "..\\\\..\\\\doc\\\\yakyuu_faq.txt"),
	      qq(textsjisfile "..\\\\..\\\\doc\\\\dir.txt"),
	      qq(textsjisfile "..\\\\..\\\\doc\\\\100001.txt")]
    },
    {name=>"int", mask=>0,
      value=>[qq(777), qq(555), qq(333), qq(99999999), qq(-1)]
    },
    {name=>"float", mask=>0,
      value=>[qq(3.141592), qq(2.718281), qq(1.414213),
	      qq(-0.789878), qq(8888.8888)]
    },
    {name=>"datetime", mask=>0,
      value=>[qq(time "2001-07-07 12:34:56.789"), 
	      qq(time "2001-09-09 00:00:00.000"),
	      qq(time "2002-10-11 17:34:51.000"),
	      qq(time "1999-07-21 13:20:00.600"),
	      qq(time "2038-01-19 01:02:03.000")]
    },
    {name=>"image", mask=>0,
      value=>[qq(binaryfile "..\\\\..\\\\doc\\\\rnd10k"),
	      qq(binaryfile "..\\\\..\\\\doc\\\\rnd20k"),
	      qq(binaryfile "..\\\\..\\\\doc\\\\rnd50k"),
	      qq(binaryfile "..\\\\..\\\\doc\\\\rnd100k"),
	      qq(binaryfile "..\\\\..\\\\doc\\\\rnd10k")]
    }
);

#別管理なのは美しくないのだが……
%column_inv = ("nvarchar"=>0, "ntext"=>1, "fulltext"=>2, "int"=>3,
	       "float"=>4, "datetime"=>5, "image"=>6);
@tables = ();

}

#===== 型に関する関数

sub column_type($)
{
    return $column_inv{$_[0]};
}

sub array_column_type($)
{
    return [$column_inv{$_[0]}];
}

# <>などで比較可能な型か否かを返す
sub comparable($)
{
    return ((!ref $_[0]) && ($_[0] != column_type("image")));
}

#image型の列か否かを返す
sub isimage($)
{
    return ($_[0] == column_type("image")
	|| ($_[0] == array_column_type("image")));
}

sub trim_uncomparable($$)
{
    my ($schema, $tuple) = @_;
    my $i;
    my @tmp = ();
    for($i=0; $i<0+@$schema; $i++) {
	push (@tmp, comparable($schema->[$i]) ? $tuple->[$i] : undef);
    }
    return \@tmp;
}

sub trim_imagecolumn($$)
{
    my ($schema, $tuple) = @_;
    my $i;
    my @tmp = ();
    for($i=0; $i<0+@$schema; $i++) {
	push (@tmp, isimage($schema->[$i]) ? undef : $tuple->[$i]);
    }
    return \@tmp;
}

#=====

# 定型のheaderを作る
sub header() 
{
print <<_EOM_;
Begin;
Initialize;
# テスト用のDBを作る
InitializeSession "";
Command "create database TESTDB";
TerminateSession;

_EOM_
}

# 定型のtrailerを作る
sub trailer()
{
print <<_EOM_;
# DBの後始末
InitializeSession "";
Command "drop database TESTDB";
TerminateSession;
Terminate;
End;
_EOM_
}

sub light_header()
{
print <<_EOF_;
Begin;
Initialize;
InitializeSession "TESTDB";

_EOF_
}

sub light_trailer()
{
print <<_EOF_;
TerminateSession "TESTDB";
Terminate;
End;
_EOF_
}

#パラメータなしのコマンドを生成する
sub comm($){
    my ($command) = @_;
    print qq(Command $cmdnum "$command";\n);
}

#パラメータつきのコマンドを生成する
#perlでは単純なoverloadは不可能
sub commp($$){
    my ($command, $parameter) = @_;
    print qq(Command $cmdnum "$command" [$parameter];\n);
}

#番号つきのコマンドを生成する
sub commn($$){
    my ($n, $command) = @_;
    print qq(Command $n "$command";\n);
}

#番号およびパラメータつきのコマンドを生成する
sub commnp($$){
    my ($n, $command, $parameter) = @_;
    print qq(Command $n "$command" [$parameter];\n);
}

#以上の各々のRowset版
sub rset($){
    my ($rowset) = @_;
    print qq(Rowset $cmdnum "$rowset";\n);
}

sub rsetp($$){
    my ($rowset, $parameter) = @_;
    print qq(Rowset $cmdnum "$rowset" [$parameter];\n);
}

sub rsetn($$){
    my ($n, $rowset) = @_;
    print qq(Rowset $n "$rowset";\n);
}

sub rsetnp($$){
    my ($n, $rowset, $parameter) = @_;
    print qq(Rowset $n "$rowset" [$parameter];\n);
}

#start transaction文を生成する
#$trm: r/w, r/oの区別
#$trl: isolation level
#$setp: 0->明示的, 1->set transactionを併用, 2->set transactionのみ
sub start_transaction($$$)
{
    my ($trm, $trl, $setp) = @_;
    if ($trl ne "") {
	$trl = " isolation level ".$trl;
	$trl = ",".$trl if $trm ne "";
    }
    $trm = " ".$trm if $trm ne "";
    if ($setp > 0) {
	comm("set transaction$trm$trl");
	if ($setp==1) {
	    if ($trm ne "") {
		comm("start transaction$trm");
	    } elsif ($trl ne "") {
		comm("start transaction$trl");
	    } else {
		comm("start transaction read write");
	    }
	}
    } else {
	comm("start transaction$trm$trl");
    }
}

#=====

# $columnから適切な値を取得するか$valueそのものを返す
sub get_column_value($$)
{
    my ($type, $value) = @_;
    if (defined $value->{list}){
	return $column[$type]->{value}->[$value->{list}];
    } else {
	return $value;
    }
}

# タプルを表すデータ構造からCommand文のパラメータとなる文字列を作る
# $tupletype: 表の構造を表したリストへの参照
# $tablevalue: パラメータの要素を含んだリストへの参照
# $leavenull: 真の場合、nullをパラメータに含めない
sub make_parameter_string($$;$)
{
    my ($tupletype, $taplevalue, $leavenull) = @_;
    my $type, $value, $element, $count=0; @result=();

    foreach $type (@$tupletype) {    
	$value = $taplevalue->[$count];
	if (!$leavenull && $value eq "null") {
	    #skip;
	} elsif (ref $value eq 'ARRAY') {
	    my @li = ();
	    foreach $element (@$value) {
		push (@li, get_column_value($type->[0], $element));
	    }
	    push (@result, "[".join(", ", @li)."]");
	} elsif (defined $value) {
	    push (@result, get_column_value($type, $value));
	}
	$count++;
    }
    return join(", ", @result);
}

# make_join_columnlistの特別なケース
sub make_columnlist($$;$)
{
    make_join_columnlist(", ", @_);
}

# $templateとリストから文字列を構成する
sub make_join_columnlist($$$;$)
{
    my ($joiner, $template, $columns, $leavenull) = @_;
    my $i, $count=1, @nums=(), $element;
    foreach $i (@$columns){
	$element = $template;
	if (!$leavenull && $i eq "null"){
	    $element =~ s/=\?/ is null/;
	}
	$element =~ s/\*/$count/g;
	if (defined $i) {
	    push(@nums, $element);
        }
	$count++;
    }
    eval("\$template=".$template);
    # 変数展開を遅延するには?
    return join($joiner, @nums);
}

#$tupletypeは$tables[$tnum]に入れるものとする

#table value constructor対応版
#一部の列のみ指定することを可能にせねば。
sub insert_command($@)
{
    my ($tnum, @rtuples) = @_;
    my $clist = make_columnlist("C*", $rtuples[0]);
    my $dmys1 = "(".make_columnlist("?", $rtuples[0]).")";
    my $dmys = join(", ", map($dmys1, (1..0+@rtuples)));
    my $tuplevalue;
    
    my @prm = ();
    foreach $tuplevalue (@rtuples) {
	my $prm1 = make_parameter_string($tables[$tnum], $tuplevalue, 1);
	push(@prm, $prm1);
    }
    commp("insert into T$tnum ($clist) values $dmys", join(", ", @prm));
}


sub insert_select_command($$$)
{
    my ($t1num, $t2num, $tuplevalue) = @_;
    $tuplevalue = trim_imagecolumn($tables[$t1num], $tuplevalue);
    return if (grep(defined $_, @$tuplevalue)==0);

    my $cols  = make_columnlist("C*",   $tuplevalue);
    my $clist = make_columnlist("C*=?", $tuplevalue);
    my $prm = make_parameter_string($tables[$t2num], $tuplevalue);
    $clist =~ s/, / and /g;
    my $cmd;
    $cmdvar = ($cmdvar+1)%4 if $t1num != $t2num;
    #いずれも意味的には同じだが、Optimizer上は全てpathが異なる
    if ($cmdvar == 2 || grep (!defined $_, @{$tuplevalue})) {
	$cmd = 
	    "insert into T$t1num ($cols) select ($cols) from T$t2num where $clist";
    } elsif ($cmdvar == 0) {
	$cmd = "insert into T$t1num select ($cols) from T$t2num where $clist";
    } elsif ($cmdvar == 1) {
	$cmd = "insert into T$t1num select $cols from T$t2num where $clist";
    } else { #if ($cmdvar == 3)
	$cmd = 
	    "insert into T$t1num ($cols) select $cols from T$t2num where $clist";
    }
    commp($cmd, $prm);
}

# table内のタプルを全て無条件で同じ物に改める
sub updateall_command($$)
{
    my ($tnum, $tuplevalue) = @_;
    my $clist = make_columnlist("C*=?", $tuplevalue, 1);
    commp("update T$tnum set $clist",
	  make_parameter_string($tables[$tnum], $tuplevalue, 1));
}

#tuplebeforeのタプルをtupleafterに改める
sub update_command($$$)
{
    my ($tnum, $tupleafter, $tuplebefore) = @_;
    $tuplebefore = trim_imagecolumn($tables[$tnum], $tuplebefore);
    return if (grep(defined $_, @$tuplebefore)==0);

    my $clist = make_columnlist("C*=?", $tupleafter, 1);
    my $clist2 = make_join_columnlist(" and ", "C*=?", $tuplebefore);
    my $prmbefore = make_parameter_string($tables[$tnum], $tuplebefore);
    my $prmafter = make_parameter_string($tables[$tnum], $tupleafter, 1);
    my $prm;
    if ($prmafter eq "") {
	$prm = $prmbefore;
    } elsif ($prmbefore eq "") {
	$prm = $prmafter;
    } else {
	$prm = $prmafter.", ".$prmbefore;
    }
    $clist2 =~ s/, / and /g;
    commp("update T$tnum set $clist where $clist2", $prm);
}

# $tuplevalueと同じ条件のタプルを削除する
sub delete_command($$)
{
    my ($tnum, $tuplevalue) = @_;
    $tuplevalue = trim_imagecolumn($tables[$tnum], $tuplevalue);
    return if (grep(defined $_, @$tuplevalue)==0);

    my $clist = make_join_columnlist(" and ", "C*=?", $tuplevalue);
    $clist =~ s/, / and /g;
    my $prm = make_parameter_string($tables[$tnum], $tuplevalue);
#    if ($prm =~ /null/) {return;}
    commp("delete from T$tnum where $clist", $prm);
}

# like検索を行う
# $orderby: 真ならばorder by句を付加する
sub select_like_command($$;$)
{
    my ($tnum, $tuplevalue, $orderby) = @_;

    my $cols  = make_columnlist("C*",        $tuplevalue);
    my $clist = make_columnlist("C* like ?", $tuplevalue);
    $clist =~ s/, / and /g;
    my $ord = $orderby ? " order by $cols" : "";
    commp("select $cols from T$tnum where $clist$ord", 
	  make_parameter_string($tables[$tnum], $tuplevalue));
}

#特定のtupleを選択する
sub select_command($$;$)
{
    my ($tnum, $tuplevalue, $orderby) = @_;
    $tuplevalue = trim_imagecolumn($tables[$tnum], $tuplevalue);
    return if (grep(defined $_, @$tuplevalue)==0);

    my $cols  = make_columnlist("C*",   $tuplevalue);
    my $clist = make_columnlist("C*=?", $tuplevalue);
    $clist =~ s/, / and /g;
    my $prm = make_parameter_string($tables[$tnum], $tuplevalue);
    my $ord = $orderby ? " order by $cols" : "";
#    if ($prm =~ /null/) {return;}
    commp("select $cols from T$tnum where $clist$ord", $prm);
}

#aliasをつけて特定のtupleを選択する
sub select_alias_command($$)
{
    my ($tnum, $tuplevalue) = @_;
    $tuplevalue = trim_imagecolumn($tables[$tnum], $tuplevalue);
    return if (grep(defined $_, @$tuplevalue)==0);

    my $cols  = make_columnlist("C* alias*", $tuplevalue);
    my $clist = make_columnlist("alias*=?", $tuplevalue);
    $clist =~ s/, / and /g;
    my $prm = make_parameter_string($tables[$tnum], $tuplevalue);
#    if ($prm =~ /null/) {return;}
    commp("select $cols from T$tnum where $clist", $prm);
}

# 複数の表に検索をかける
sub select_join_command($$$$)
{
    my ($t1num, $t2num, $tuplevalue1, $tuplevalue2) = @_;
    $tuplevalue1 = trim_imagecolumn($tables[$t1num], $tuplevalue1);
    return if (grep(defined $_, @$tuplevalue1)==0);
    $tuplevalue2 = trim_imagecolumn($tables[$t2num], $tuplevalue2);
    return if (grep(defined $_, @$tuplevalue2)==0);

    my $clist = join(" and ",  
		     make_columnlist("T$t1num.C*=?", $tuplevalue1),
		     make_columnlist("T$t2num.C*=?", $tuplevalue2));
    $clist =~ s/, / and /g;
    $prm =  make_parameter_string($tables[$t1num], $tuplevalue1).", ".
	    make_parameter_string($tables[$t2num], $tuplevalue2);
#    if ($prm =~ /null/) {return;}
    commp("select * from T$t1num, T$t2num where $clist", $prm);
}

sub selectall_command($)
{
    my ($tnum) = @_;
    comm("select " . make_columnlist("C*", $tables[$tnum])
	 . " from T$tnum");
}

# 条件つきのselect文をいろいろ生成する
# 結果はcountで返すものとする
sub select_cond($@)
{
    my ($tnum, $tuplevalue) = @_;
    $tuplevalue = trim_uncomparable($tables[$tnum], $tuplevalue);
    return if (grep(defined $_, @$tuplevalue)==0);

    my $joiner, $op, $cond;
    my $cols = make_columnlist("C*",   $tuplevalue);
    my $colnum = 0+@{$tables[$tnum]};
    my $vlist = make_parameter_string($tables[$tnum], $tuplevalue);

    print qq(# さまざまな等号や不等号の検索条件を試す\n);
    my @joinerlist = (@$tuplevalue >= 2) ? (" and ", " or ") : ("dummy");
    foreach $joiner (@joinerlist) {
	foreach $op ("=", "<", ">", "<=", ">=", "<>") {
	    $cond = make_join_columnlist($joiner, "C*${op}?", $tuplevalue);
	    last if $cond =~ /null/;
	    commp("select count(*) from T$tnum where $cond", $vlist);
	    if ($PlainFTS) {
		commp("select * from T$tnum where $cond", $vlist);
		commp("select $cols from T$tnum where $cond", $vlist);
		commp("select ($cols) from T$tnum where $cond", $vlist);
		if ($op eq "=") {
		    $cond =~ s/=/</;
		    commp("select count(*) from T$tnum where $cond", $vlist);
		} elsif ($op eq "<") {
		    $cond =~ s/</=/;
		    commp("select count(*) from T$tnum where $cond", $vlist);
		}
	    }
	}
	foreach $op ("", " not") {
	    print qq(# is (not) nullを試す\n);
	    $cond = make_join_columnlist($joiner, "C* is$op null", $tuplevalue);
	    comm("select count(*) from T$tnum where $cond");
	    if ($PlainFTS) {
		comm("select * from T$tnum where $cond");
		comm("select $cols from T$tnum where $cond");
		comm("select ($cols) from T$tnum where $cond");
	    }
	}
    }
}

#=====

# filetypeがFTSを表しているか否かを調べる
sub isfts($)
{
    return $_[0]>=2 && $_[0]<=9;
}

sub get_ftshint($)
{
    my ($filetype) = @_;
    my $ftshint="";
    if (isfts($filetype)) { 
	$ftshint .= "sectionized, " if ($filetype-2)&1;
	$ftshint .= "delayed, " if ($filetype-2)&2;
	$ftshint .= "inverted=(normalized=true), " if ($filetype-2)&4;
	$ftshint =~ s/, $//;
    }
    return $ftshint;
}

sub get_filedesc($$$)
{
    my ($filetype, $heapp, $arrayp) = @_;
    my $filedesc;

    $filedesc = "/".$filetypes[$filetype];
    $filedesc .= "/heap" if $heapp;
    $filedesc .= "/array" if $arrayp;
    return $filedesc;
}

sub create_table($$$$)
{
    my ($tnum, $filetype, $heapp, $schema) = @_;
    my $col, $type, $coldef="", $colcount=1;
    my $array, $hint, $ctype, $i;

    # @tablesに登録
    $tables[$tnum] = $schema;

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
	    make_columnlist("C*", $schema) .")";
    }
    comm("create table T$tnum($coldef)");
    if ($filetype == 1) { 

	my $cols = make_columnlist("C*", $schema);
        comm("create index I$tnum on T$tnum($cols)"); 

	if ($colcount >= 2) {
	    #colcountも適切ではないかな…
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

sub drop_table($)
{
    comm("drop table T$_[0]");
    undef $tables[$num];
}

#=====

1;

#
# Copyright (c) 2001, 2023 Ricoh Company, Ltd.
# All rights reserved.
#
