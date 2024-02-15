version=1.55.0
libs=*.${version}
for lib in ${libs}
do
  mv $lib ${lib%.${version}}
done
