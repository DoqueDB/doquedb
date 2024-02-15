## Overview

DoqueDB is a relational database featuring powerful Japanese full-text search.  
Links to the documents are provided at the end of this document.

## Operating environments

DoqueDB can be used in the following environments:
  - OS: Linux (64-bit) \*1
  - Architecture: x86\_64
  - Memory: 2 GB or more \*2
  - Disk space: 1 GB or more \*3

\*1 Operation has been verified under:
  - RedHat Enterprise Linux 7
  - CentOS 7

\*2 Required memory varies depending on data size.  
\*3 Required disk space varies depending on the size of database and logs.

## Quick start

Please obtain the binary package from Releases.  
The name of the binary package is doquedb-\<version\>.\<architecture\>.tar.gz.

### Install and start server

Installation must be performed by the root user.  
Execution as the root user is indicated by the prompt "#".
```
# tar xvf doquedb-<version>.<architecture>.tar.gz
# cd doquedb-<version>.<architecture>
# ./install.sh
# ./setup.sh
# cd /var/lib/DoqueDB/bin
# ./doquedb start
```

### Execute examples

The examples can be run as a non-root user.  
Execution as a non-root user is indicated by the prompt "$".
```
$ mkdir ~/doquedb
$ cp -rp /var/lib/DoqueDB/doc/sample ~/doquedb
$ cd ~/doquedb/sample/sqli
```
Create a database and a table, load data and create an index.  
Then perform various search samples.  
See the shell scripts for details on the searches.
```
$ ./setup.sh
$ ./likeSearch.sh
$ ./rankSearch.sh
$ ./sentenceSearch.sh
```

### Stop server

```
# cd /var/lib/DoqueDB/bin
# ./doquedb stop
```

### Uninstall

```
# cd <directory where you extracted the package>
# ./unsetup.sh
# ./uninstall.sh
# rm -fr /var/lib/DoqueDB
```

## Building DoqueDB from source code

See [BUILDING_PROCEDURE.md](./BUILDING_PROCEDURE.md).

## Documents

The following documents are available.  
At this time, only Japanese-language documents are available.
* [使ってみよう(How to use)](https://doquedb.github.io/doquedb/howtouse.html)
* [ユーザーズマニュアル(Users Manual)](https://doquedb.github.io/doquedb/users.html)
* [ナレッジ(Knowledge)](https://doquedb.github.io/doquedb/knowledge.html)
* [JDBC Driver](https://doquedb.github.io/doquedb-javadoc/javadoc/index.html)
* [Hibernate Dialect](https://doquedb.github.io/doquedb-javadoc/dialect/index.html)

## Community and supports

Use GitHub Issues and Pull Request for any requests and bug reports.  
If you want to contribute code with Pull Request,  
you'll need to agree to the Contributor License Agreement  
(for individuals: [INDIV\_CLA.txt](https://github.com/doquedb/doquedb/INDIV_CLA.txt), for corporations: [CORP\_CLA.txt](https://github.com/doquedb/doquedb/CORP_CLA.txt)).  
Please fill out the form and send it to the address listed.

## Licenses

The source code of DooqueDB is released under the Apache License 2.0.  
Please see LICENSE/NOTICE.txt for third-party software licenses.
