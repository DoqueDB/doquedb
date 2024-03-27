## DoqueDBをソースコードからビルドする

この文書ではDoqueDBをソースコードからビルドする手順を説明しています。

## 必要条件

ここではCentOS Stream 9上でgcc 11.4を使ってビルドすることを想定しています。  
gcc 4.8を使ってビルドする場合は、O114-64をO48-64と読み替えてください。  
作業には、それに加えて以下の開発ツールが必要です。  
zlib-develは、環境によってはlibz-devのこともあります。  
CentOS 7ではperl-openをインストールする必要はありません。
* JDK 8 以降
* ant
* libuuid
* libuuid-devel
* zlib-devel
* perl-open

上記開発ツールをインストールするため、以下の手順を実行してください。
```
$ sudo yum install java-1.8.0-openjdk-devel
$ export JAVA_HOME=<directory where you installed JDK>
$ sudo yum install ant
$ export ANT_HOME=<directory where you installed ant>
$ sudo yum install libuuid libuuid-devel
$ sudo yum install zlib-devel
$ sudo yum install perl-open.noarch
```
さらに、MODライブラリ、UNAライブラリをビルドするためには  
いくつかのUnicodeデータファイルが必要です。  
Unicode.orgから以下のようにファイルを取得してください。
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

## ビルド手順

DoqueDBをソースコードからビルドするには、MODライブラリ、UNAライブラリ、  
DoqueDBをその順にビルドする必要があります。

### MODライブラリ (C++ヘルパーライブラリ)

```
$ export OSTYPE=linux
$ cd mod/1.0
$ ../../common/tools/build/mkconfdir O114-64
$ cd c.O114-64
$ make conf-r
$ make buildall
$ make install-r
$ make package
$ make installh-r
$ cd ../../..
```

### UNAライブラリ (日本語形態素解析器)

```
$ cd una/1.0
$ ../../common/tools/build/mkconfdir O114-64
$ cd c.O114-64
$ make conf-r
$ make buildall
$ make install-r
$ make package
$ make installh-r
$ cd ../../..
```

### UNAリソース (UNA辞書)

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
$ ../common/tools/build/mkconfdir O114-64
$ cd c.O114-64
$ make conf-r
$ make buildall
$ make package
```
次に、パッケージをrootユーザーでインストールします。
> [!CAUTION]
> 既存のデータベースはこの手順で削除されます。
```
# cd doquedb
# ./unsetup.sh
# ./uninstall.sh
# ./install.sh
# ./setup.sh
# ./dump.sh
```

### 再ビルド

make buildallで生成されたファイルは、make clean-rで削除できます。  
各モジュールを再ビルドするには、make clean-rを実行してから  
make buildallをもう一度実行してください。

```
$ cd c.O114-64
$ make clean-r
$ make buildall
...
```

./conf以下の設定ファイルを変更した場合はmake reconf-rも必要です。

```
$ cd c.O114-64
$ make clean-r
$ make reconf-r
$ make buildall
...
```
