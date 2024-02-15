BEGIN{$dif = shift;}
s/(\d{5,})/$1+$dif/e if /makepadding\.pl/;
s/\r+//;
