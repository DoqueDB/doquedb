Overview

DoqueDB is a relational database featuring powerful Japanese full-text search.
Links to the documents are provided at the end of this document.

Operating environments

DoqueDB can be used in the following environments:
  - OS: Linux (64bit) *1
  - Architecture: x86_64
  - Memory: 2GB or more *2
  - Disk space: 1GB or more *3

*1 Operation has been verified under:
  - RedHat Enterprise Linux 7
  - CentOS 7

*2 Required memory varies depending on data size.
*3 Required disk space varies depending on the size of database and logs.

Quick start

(1) Install and start server

Installation must be performed by a root user.
Execution as a root user is indicated by the prompt "#".

  # tar xvf doquedb-<version>.tar.gz
  # cd doquedb-<version>
  # ./install.sh
  # ./setup.sh
  # cd /var/lib/DoqueDB/bin
  # ./doquedb start

(2) Execute samples

The samples can be run as a non-root user.
Execution as a non-root user is indicated by the prompt "$".

  $ mkdir ~/doquedb
  $ cp -rp /var/lib/DoqueDB/doc/sample ~/doquedb
  $ cd ~/doquedb/sample/sqli

Create a database and a table, load data and create an index.
Then perform various search samples.
See the shell scripts for details on the searches.

  $ ./setup.sh
  $ ./likeSearch.sh
  $ ./rankSearch.sh
  $ ./sentenceSearch.sh

(3) Stop server

  # cd /var/lib/DoqueDB/bin
  # ./doquedb stop

(4) Uninstall

  # cd <directory where you extracted the package>
  # ./unsetup.sh
  # ./uninstall.sh
  # rm -fr /var/lib/DoqueDB

4. Documents

The following documents are available.
At this time, only Japanese-language documents are available.

  * 使ってみよう(How to use) https://github.com/doquedb/doquedb/tree/master/docs/howtouse.html
  * ユーザーズマニュアル(Users Manual) https://github.com/doquedb/doquedb/tree/master/docs/users.html
  * ナレッジ(Knowledge) https://github.com/doquedb/doquedb/tree/master/docs/knowledge.html
  * JDBC Driver https://github.com/doquedb/doquedb/tree/master/docs/javadoc/index.html
  * Hibernate Dialect https://github.com/doquedb/doquedb/tree/master/docs/dialect/index.html

5. Community and supports

Use GitHub Issues and Pull Request for any requests and bug reports.

6. Licenses

The source code of DooqueDB is released under the Apache License 2.0.
Please see LICENSES/NOTICE.txt for third-party software licenses.
