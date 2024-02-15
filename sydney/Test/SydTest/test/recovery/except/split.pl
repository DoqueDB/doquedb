$num=$ARGV[0];
open IN, "<${num}.txt";
open OUT, ">${num}b.txt";
while(<IN>)
{
    s/\r+//;

    if (/^ Terminate;$/) {
	print OUT "End;\n";
	close OUT;
	open OUT, ">${num}r.txt";
	print OUT "Begin;\n";
    } else {
	print OUT;
    }
}
close IN;
close OUT;
