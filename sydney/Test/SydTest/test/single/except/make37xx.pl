use lib "../..";
use MakeScript;
@column = @MakeScript::column;

@command=("dummy",
	  "insert into T1 values (?)",
	  "insert into T1 values (?)",
	  "update T1 set C1=? where C1=?");
@param=("dummy",
	qq(textsjisfile "..\\\\..\\\\doc\\\\rfc3092.txt"),
	qq(textsjisfile "..\\\\..\\\\doc\\\\rfc822.txt"),
	qq(textsjisfile "..\\\\..\\\\doc\\\\kenpou.txt", 
	   textsjisfile "..\\\\..\\\\doc\\\\rfc3092.txt"));
@array_param=("dummy",
	qq([textsjisfile "..\\\\..\\\\doc\\\\rfc3092.txt"]),
	qq([textsjisfile "..\\\\..\\\\doc\\\\rfc822.txt"]),
	qq([textsjisfile "..\\\\..\\\\doc\\\\kenpou.txt"], 
	   [textsjisfile "..\\\\..\\\\doc\\\\rfc3092.txt"]));
@desc=("create", "insert1", "insert2", "update");
%pad=(
3701=>9155000, 3703=>9015000, 3705=>8945000, 3707=>8855000,
3711=>8800000, 3713=>8640000, 3715=>8505000, 3717=>8385000,

3721=>8605000, 3723=>8140000, 3725=>7395000, 3727=>6520000,
3731=>8195000, 3733=>7680000, 3735=>6890000, 3737=>5945000,
3741=>7405000, 3743=>7000000, 3745=>6210000, 3747=>5345000,
3751=>6995000, 3753=>6530000, 3755=>5815000, 3757=>4880000,

3761=>8605000, 3763=>8170000, 3765=>7555000, 3767=>6805000,
3771=>8195000, 3773=>7710000, 3775=>7045000, 3777=>6275000,
3781=>7400000, 3783=>7050000, 3785=>6530000, 3787=>5715000,
3791=>6990000, 3793=>6580000, 3795=>5945000, 3797=>5210000,

3801=>9105000, 3803=>8965000, 3805=>8895000, 3807=>8805000,
3811=>8750000, 3813=>8580000, 3815=>8435000, 3817=>8285000,

3821=>8555000, 3823=>8065000, 3825=>7315000, 3827=>6440000,
3831=>8145000, 3833=>7610000, 3835=>6810000, 3837=>5895000,
3841=>7350000, 3843=>6950000, 3845=>6135000, 3847=>5270000,
3851=>6940000, 3853=>6490000, 3855=>5735000, 3857=>4830000,

3861=>8555000, 3863=>8105000, 3865=>7480000, 3867=>6730000,
3871=>8145000, 3873=>7645000, 3875=>6970000, 3877=>6225000,
3881=>7350000, 3883=>6975000, 3885=>6455000, 3887=>5665000,
3891=>6940000, 3893=>6520000, 3895=>5885000, 3897=>5160000,
);

#=====

sub writedown($$$) {
my ($heap, $ftype, $ncomm)=@_;
$strheap=$heap?", heap":"";
$num=3701+($heap*100)+($ftype*10)+($ncomm*2);

my $schema, $p, $p_i;
if (grep($ftype==$_, (3, 5, 7, 9))) {
    $schema=[[1]];
    $p=$array_param[$i];
} else {
    $schema=[1];
    $p=$param[$i];
}

open OUT, ">${num}.txt";
select OUT;

print <<_EOB_;
#$num: diskfull ($MakeScript::filetypes[$ftype]$strheap, $desc[$ncomm])

Begin;
System "perl ..\\\\..\\\\makepadding.pl $pad{$num} > h:\\\\padding1";
Initialize;
InitializeSession "";
Command "create database DISKFULLTEST
	path 'h:\\\\diskfulltest\\\\data'
	logicallog 'h:\\\\diskfulltest\\\\log'
	system 'h:\\\\diskfulltest\\\\system'";
TerminateSession;

InitializeSession "DISKFULLTEST";
_EOB_

print qq(System "dir h:|tail -1";\n);
create_table(1, $ftype, $heap, $schema);
print qq(System "dir h:|tail -1";\n);
for($i=1; $i<=$ncomm; $i++) {
    if (grep($ftype==$_, (3, 5, 7, 9))) {
	$p_i=$array_param[$i];
    } else {
	$p_i=$param[$i];
    }
    commp($command[$i], $p_i);
    print qq(System "dir h:|tail -1";\n);
}
print <<_EOB_;
Command "select * from T1";
TerminateSession;
 Terminate;

System "rm h:\\\\padding1"; #paddingを外す

Initialize;
InitializeSession "DISKFULLTEST";
#妨げられていたコマンドを繰り返す
_EOB_
if ($ncomm==0) {
    create_table(1, $ftype, $heapp, $schema);
} else {
    commp($command[$ncomm], $p);
}
print <<_EOB_;
#中身を確かめる
Command "select * from T1";
Command "drop table T1";
TerminateSession;

InitializeSession "";
Command "drop database DISKFULLTEST";
TerminateSession;

Terminate;
End;
_EOB_

close OUT;
select STDOUT;
}

#=====

foreach $heap(0, 1){
    foreach $ftype(0..9){
	foreach $ncomm(0..3){
	    writedown($heap, $ftype, $ncomm);
	}
    }
}

