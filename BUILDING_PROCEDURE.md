## Building DoqueDB from source code

This document describes the building procedure of DoqueDB from source code.

## Requirements

We assume that you are building DoqueDB on CentOS7 using gcc 4.8.  
In addition, the following development tools are required in the process.  
zlib-devel may be libz-dev, depending on the environment.
* JDK 8 or later
* ant
* libuuid
* libuuid-devel
* zlib-devel

To install the above develepment tools, do the following steps.
```
$ sudo yum install java-1.8.0-openjdk-devel
$ export JAVA_HOME=<directory where you installed JDK>
$ sudo yum install ant
$ export ANT_HOME=<directory where you installed ant>
$ sudo yum install libuuid libuuid-devel
$ sudo yum install zlib-devel
```
Also you need some Unicode data files to build MOD libary and UNA library.  
Get the files from Unicode.org as follows.
```
$ cd mod/1.0/tools/src
$ wget https://www.unicode.org/Public/1.1-Update/UnicodeData-1.1.5.txt
$ cd ../../../../una/1.0/resource/src-data/norm
$ wget https://www.unicode.org/Public/1.1-Update/UnicodeData-1.1.5.txt
$ wget https://www.unicode.org/Public/MAPPINGS/OBSOLETE/EASTASIA/JIS/JIS0201.TXT
$ wget https://www.unicode.org/Public/MAPPINGS/OBSOLETE/EASTASIA/JIS/JIS0208.TXT
$ wget https://www.unicode.org/Public/MAPPINGS/OBSOLETE/EASTASIA/JIS/JIS0212.TXT
$ cd ../../../../..
```

## Building procedure

To build DoqueDB from source code, you need to build and install MOD library,  
UNA library and DoqueDB in that order.

### MOD library (C++ helper library)
```
$ export OSTYPE=linux
$ cd mod/1.0
$ ../../common/tools/build/mkconfdir O48-64
$ cd c.O48-64
$ make conf-r
$ make buildall
$ make install-r
$ make package
$ cd ../../..
```

### UNA library (Japanese morphological anlyzer)
```
$ cd una/1.0
$ ../../common/tools/build/mkconfdir O48-64
$ cd c.O48-64
$ make conf-r
$ make buildall
$ make install-r
$ make package
$ cd ../../..
```

### UNA resource (dictionaries for UNA)
```
$ cd una/1.0/resource
$ mkdir work
$ cd work
$ make -f ../tools/make/make-tools install
$ export LD_LIBRARY_PATH=`pwd`/../tools/bin:$LD_LIBRARY_PATH
$ make -f ../tools/make/make-stem install
$ make -f ../tools/make/make-norm install
$ make -f ../tools/make/make-una install
$ make -f ../tools/make/make-una package
$ cd ../../../..
```

### DoqueDB
```
$ cd sydney
$ ../common/tools/build/mkconfdir O48-64
$ cd c.O48-64
$ make conf-r
$ make buildall
$ make package
```
Then, install the package as the root user.  
> [!CAUTION]
> Your existing database will be deleted during the process.
```
# cd doquedb
# ./unsetup.sh
# ./uninstall.sh
# ./install.sh
# ./setup.sh
# ./dump.sh
```
