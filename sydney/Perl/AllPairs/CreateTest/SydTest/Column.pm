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
######################################################################################################
# AllPairs::CreateTest::SydTest::Column -- column class
# member:
#	name		column name
#	type		column type
#	table		table
#	constraint	column constraint
#	hint		column hint
######################################################################################################

package AllPairs::CreateTest::SydTest::Column;
use AllPairs::CreateTest;
use AllPairs::CreateTest::SydTest;
use AllPairs::CreateTest::SydTest::Condition;
use AllPairs::CreateTest::SydTest::Type;
use AllPairs::CreateTest::SydTest::Value;

my $ColumnDataMap;
my $ColumnResultMap;
my $Max = {
    int => $AllPairs::CreateTest::INTMAX,
    bigint => $AllPairs::CreateTest::BIGINTMAX,
};
my $Min = {
    int => $AllPairs::CreateTest::INTMIN,
    bigint => $AllPairs::CreateTest::BIGINTMIN,
};

#####################
# constructor
# args:
#	cls		class name
#	name		column name
#	type		column type
#	constraint	column constraint
#	hint		column hint
#	table		table

sub new ($$$;$$$)
{
    my $cls = shift;
    my $name = shift;
    my $type = shift;
    my $constraint = shift;
    my $hint = shift;
    my $table = shift;

    my $this = {name=>$name, type=>$type, constraint=>$constraint, hint=>$hint, table=>$table};
    bless $this;
    $this;
}

#########################################################
# AllPairs::CreateTest::SydTest::Column::copy -- copy the column object
# args:
#	obj	column object
# return:
#	copied object
sub copy($)
{
    my $obj = shift;
    new AllPairs::CreateTest::SydTest::Column($obj->{name}, $obj->{table}, $obj->{type}, $obj->{constraint}, $obj->{hint});
}

#########################################################
# AllPairs::CreateTest::SydTest::Column::getDefinition -- get column definition
# args:
#	<none>
# return:
#	character string of column definition
sub getDefinition($)
{
    my $obj = shift;
    my @elements;
    push(@elements, $obj->{name});
    push(@elements, $obj->{type}->getDescription);
    push(@elements, $obj->{constraint}) if $obj->{constraint};
    push(@elements, 'hint', $obj->{hint}) if $obj->{hint};

    join(' ', @elements);
}

#########################################################
# AllPairs::CreateTest::SydTest::Column::createData -- create column data for test
# args:
#	obj		column object
#	type		creating type
#	count		the number of tuples (optional)
# return:
#	array reference for data
sub createData($$;$$)
{
    my $obj = shift;
    my $generatetype = shift;
    my $count = shift;
    my $offset = shift;

    my $result;

    if ($generatetype eq 'null') {
	return ['null'];
    }

    &initializeDataMap;

    my $checksum = $obj->getUniqueNum();
    my $datatype = $obj->getTypeDescription() or die "Unknown type for createData: " . $obj->{type}->{notation};

    my $data = $ColumnDataMap->{$generatetype}->{$datatype};
    $data or die "Illegal generator($generatetype) and type($datatype)";

    my $idx = $checksum % ($#$data + 1);
    $data->[$idx] or die "Can't create data";
    $#{$data->[$idx]} >= 0 or die "Can't create data: datatype=$datatype, generatetype=$generatetype";

    if ($obj->{unique} && $count) {
	#generate value
	my $getUniqueValue;
	if ($datatype =~ /int/) {
	    $getUniqueValue =
		sub ($) {
		    my $i = shift;
		    my $d = int($i / 4);
		    my $r = $i % 4;

		    if ($r == 0) {
			return $d;
		    } elsif ($r == 1) {
			return -$d-1;
		    } elsif ($r == 2) {
			$Max->{$datatype} or die "Max can't be gotten for $datatype";
			return $Max->{$datatype} - $d;
		    } elsif ($r == 3) {
			$Min->{$datatype} or die "Min can't be gotten for $datatype";
			return $Min->{$datatype} + $d;
		    }
		};
	} elsif ($datatype eq 'decimal') {
	    $getUniqueValue =
		sub ($) {
		    my $i = shift;
		    my $d = int($i / 4);
		    my $r = $i % 4;
		    my $max = '99999';

		    if ($r == 0) {
			return $d;
		    } elsif ($r == 1) {
			return -$d-1;
		    } elsif ($r == 2) {
			return ($max - $d) . '.' . ($max - $d);
		    } elsif ($r == 3) {
			return '-' . ($max - $d) . '.' . ($max - $d);
		    }
		};
	} elsif ($datatype eq 'float') {
	    $getUniqueValue =
		sub ($) {
		    my $i = shift;
		    my $d = int($i / 3);
		    my $r = $i % 3;

		    if ($r == 0) {
			return $d;
		    } elsif ($r == 1) {
			return ($DBLMAX_value * ($d+1)) . 'E' . ($DBLMAX_exp - $d);
		    } elsif ($r == 2) {
			return ($DBLMIN_value * ($d+1)) . 'E' . ($DBLMIN_exp + $d);
		    }
		};
	} elsif ($datatype =~ /char/) {
	    my $n = $#{$data->[$idx]}+1;
	    $getUniqueValue =
		sub ($) {
		    my $i = shift;

		    return $i . $data->[$idx]->[($checksum + $offset + $i) % $n];
		};
	}
	$getUniqueValue or die "Can't set getUniqueValue for " . $datatype;
	my $c = 0;
	for (my $i = 0; $i < $count; ++$i) {
	    if ($generatetype ne 'halfnull' || ($i % 2 == 0)) {
		push(@$result, &$getUniqueValue($c));
		++$c;
	    } else {
		push(@$result, 'null');
	    }
	}
    } else {
	$result = $data->[$idx];
    }
    die "Create data failed. generatetype=$generatetype, unique=" . $obj->{unique} . ", count=$count" if $#$result < 0;
    $result;
}

#########################################################
# AllPairs::CreateTest::SydTest::Column::createResult -- create result data for condition
# args:
#	obj		column object
#	type		creating type of data
#	conditiontype	creating type of condition
# return:
#	array reference for hash holding result data
sub createResult($$)
{
    my $obj = shift;
    my $generatetype = shift;
    my $conditiontype = shift;
    my $result;

    if ($generatetype eq 'null') {
	return [];
    }

    &initializeDataMap;

    my $checksum = $obj->getUniqueNum();
    my $datatype = $obj->getTypeDescription() or die "Unknown type for createResult: " . $obj->{type}->{notation};

    my $data = $ColumnResultMap->{$generatetype}->{$conditiontype}->{$datatype};
    if ($data) {
	my $idx = $checksum % ($#$data + 1);
	return $data->[$idx] or die "Can't create data";
    }
    $data;
}

#########################################################
# AllPairs::CreateTest::SydTest::Column::createCondition -- create Condition object
# args:
#	obj		column object
#	type		creating type
#	option		[optional] additional option
# return:
#	condition object
sub createCondition($$;$)
{
    my $obj = shift;
    my $generatetype = shift;
    my $option = shift;
    my $result;

    if ($generatetype eq 'in') {
	# in needs more than one value
	my $data = $obj->createData('=');
	$#$data >= 0 or die "Empty data";
	my $count = ($option > 0) ? $option : 5;
	my $operand = [$obj];
	my $resultdata;
	for (my $i = 0; $i < $count; ++$i) {
	    my $value = $data->[$i % ($#$data + 1)];
	    my $data = new AllPairs::CreateTest::SydTest::Value($obj->{type}, $value);
	    push (@$operand, $data);
	    $resultdata->{$data->{value}} = 1;
	}
	$result = new AllPairs::CreateTest::SydTest::Condition($generatetype, $operand);
	$result->setResult($resultdata);

	return $result;
    }

    # normal cases
    my $offset = ($option > 0) ? $option : 0;
    my $data = $obj->createData($generatetype, 1, $offset);
    my $ndata = $#$data + 1;
    $ndata > 0 or die "Empty data";
    if ($generatetype eq 'between') {
	# between needs two data
	my @operands = split(/@@@/, $data->[$offset % $ndata]);
	my $operand0 = new AllPairs::CreateTest::SydTest::Value($obj->{type}, $operands[0]);
	my $operand1 = new AllPairs::CreateTest::SydTest::Value($obj->{type}, $operands[1]);
	$result = new AllPairs::CreateTest::SydTest::Condition($generatetype, [$obj, $operand0, $operand1]);
    } elsif ($generatetype =~ /freetext|contains/) {
	my $type = $obj->{type}->copy;
	if ($type->isArray) {
	    $type = $type->getElementType;
	}
	my $operand = new AllPairs::CreateTest::SydTest::Value($type, $data->[$offset % $ndata]);
	$result = new AllPairs::CreateTest::SydTest::Condition($generatetype, [$obj, $operand]);
    } elsif ($generatetype eq 'wordlist') {
	my $type = $obj->{type}->copy;
	if ($type->isArray) {
	    $type = $type->getElementType;
	}
	$result = new AllPairs::CreateTest::SydTest::Condition(
			     'wordlist',
			     [$obj,
			      map {new AllPairs::CreateTest::SydTest::Value($type, $_)} split(/@@@/, $data->[$offset % $ndata])]);
    } elsif ($generatetype =~ /<|>|=|like/) {
	my $operand = new AllPairs::CreateTest::SydTest::Value($obj->{type}, $data->[$offset % $ndata]);
	$result = new AllPairs::CreateTest::SydTest::Condition($generatetype, [$obj, $operand]);
    } else {
	die "Unknown geterate type: $generatetype"; 
    }

    my $resultdata = $obj->createResult('auto', $generatetype);
    if ($#$resultdata >= 0) {
	my $castresult;
	map {
	    my $data = new AllPairs::CreateTest::SydTest::Value($obj->{type}, $_);
	    $castresult->{$data->{value}} = 1;
	} keys %{$resultdata->[0]};

	$result->setResult($castresult);
    }

    $result;
}

#########################################################
# AllPairs::CreateTest::SydTest::Column::getUniqueNum -- generate unique number to column
# args:
#	obj		column object
# return:
#	number
sub getUniqueNum($)
{
    my $obj = shift;

    unpack("%32C*", $obj->{name} . $obj->{table}->{name});
}

#########################################################
# AllPairs::CreateTest::SydTest::Column::getTypeDescription -- generate type description to generate data
# args:
#	obj		column object
# return:
#	type description
sub getTypeDescription($)
{
    my $obj = shift;
    my $result;
    if ($obj->{type}->isAsciiCharacterString) {
	$result = 'char';
    } elsif ($obj->{type}->isNationalCharacterString) {
	$result = 'nchar';
    } elsif ($obj->{type}->isInt) {
	$result = 'int';
    } elsif ($obj->{type}->isBigInt) {
	$result = 'bigint';
    } elsif ($obj->{type}->isFloat) {
	$result = 'float';
    } elsif ($obj->{type}->isDecimal) {
	$result = 'decimal';
    }
    return $result;
}

#########################################################
# AllPairs::CreateTest::SydTest::Column::getSQL -- get SQL representation of an column
# args:
#	column
# return:
#	character string representing column reference
sub getSQL($;$)
{
    my $obj = shift;
    my $includeTableName = shift;

    return $includeTableName == 0 ? $obj->{name} : $obj->{table}->{name} . '.' . $obj->{name};
}

#########################################################
# AllPairs::CreateTest::SydTest::Column::initializeDataMap -- initialize data
# args:
#	<none>
# return:
#	<none>
sub initializeDataMap()
{
    unless ($ColumnDataMap) {
	# create data map
	my $generate;
	my $type;
	my $result;
	while (<DATA>) {
	    chomp;
	    if (/^generate:(.*)/) {
		$generate = $1;
		undef $result;
		next;
	    }
	    if (/^type:(.*)/) {
		$type = $1;
		undef $result;
		next;
	    }
	    if (/^result:(.*)/) {
		$result = $1;
		next;
	    }
	    my $data;
	    @$data = split(/:::/);
	    if ($result) {
		my $resultnum = $#{$ColumnResultMap->{$result}->{$generate}->{$type}} + 1;
		my $datanum = $#{$ColumnDataMap->{$result}->{$type}} + 1;
		my $idx = $resultnum % $datanum;
		my $source = $ColumnDataMap->{$result}->{$type}->[$idx];
		my $resultData;
		@$resultData =
		    map {
			my $t;
			map {$t->{$source->[$_]} = 1} split(/@@@/, $_);
			$t;
		    } @$data;
		push(@{$ColumnResultMap->{$result}->{$generate}->{$type}}, $resultData);
	    } else {
		push(@{$ColumnDataMap->{$generate}->{$type}}, $data);
	    }
	}
    }
}

1;

#
# Copyright (c) 2001, 2023 Ricoh Company, Ltd.
# All rights reserved.
#

# following section are source for data
__DATA__
generate:auto
type:char
What is TRMeister?:::TRMeister is a proprietary full-text search engine of Ricoh and provides an advanced information retrieval system which is a prime requirement today. In addition, it also has RDBMS function.:::Main feature of full-text search engine:::TRMeister is characterized by full-text search as below.:::Ranking search :::    The results can be listed up by ranking score value with unique algorithm of Ricoh. :::Search by sentence :::    Sentence can be used as terms and adequate terms are extracted automatically. :::Expansion of query terms :::    It can automatically adds related terms through feedback of relevant documents. :::Multi language support :::    It can support Japanese, Chinese(traditional, simplified), English, French, Italian, German, Spanish and Dutch. :::Main feature of RDBMS:::TRMeister is characterized by very important function as below.:::Online backup :::    It can backup without suspending system operations. :::Automatic recovery :::    Data can be verified automatically when the server restart by an unforeseen sutdown. :::TRMeister can connect to global standard DB servers or through JDBC and also can create a high-speed information retrieval system without costly DBMS. 
Full text search From Wikipedia, the free encyclopedia In text retrieval, full text search (also called free search text [citation needed]) refers to a technique for searching a computer-stored document or database; in a full text search, the search engine examines all of the words in every stored document as it tries to match search words supplied by the user. :::Full-text searching techniques became common in online bibliographic databases in the 1970s. :::Most Web sites and application programs (such as word processing software) provide full text search capabilities. :::Some Web search engines, such as AltaVista employ full text search techniques, while others index only a portion of the Web pages examined by its indexing system.[1]:::The most common approach to full text search is to generate a complete index or concordance for all of the searchable documents. :::For each word (excepting stop words which are too common to be useful) an entry is made which lists the exact position of every occurrence of it within the database of documents. :::From such a list it is relatively simple to retrieve all the documents that match a query, without having to scan each document. :::Although for very small document collections full-text searching can be done by serial scanning, indexing is the preferred method for almost all full-text searching.
Database From Wikipedia, the free encyclopedia The term or expression database originated within the computer industry.::: Although its meaning has been broadened by popular use, even to include non-electronic databases, this article takes a more technical perspective towards the topic.::: A possible definition is that a database is a structured collection of records or data which is stored in a computer so that a program can consult it to answer queries.::: The records retrieved in answer to queries become information that can be used to make decisions.::: The computer program used to manage and query a database is known as a database management system (DBMS).::: The properties and design of database systems are included in the study of information science.:::The central concept of a database is that of a collection of records, or pieces of knowledge.::: Typically, for a given database, there is a structural description of the type of facts held in that database: this description is known as a schema.::: The schema describes the objects that are represented in the database, and the relationships among them.::: There are a number of different ways of organizing a schema, that is, of modelling the database structure: these are known as database models (or data models).::: The model in most common use today is the relational model, which in layman's terms represents all information in the form of multiple related tables each consisting of rows and columns (the true definition uses mathematical terminology).::: This model represents relationships by the use of values common to more than one table.::: Other models such as the hierarchical model and the network model use a more explicit representation of relationships.:::The term database refers to the collection of related records, and the software should be referred to as the database management system or DBMS.::: When the context is unambiguous, however, many database administrators and programmers use the term database to cover both meanings.:::Many professionals would consider a collection of data to constitute a database only if it has certain properties: for example, if the data is managed to ensure its integrity and quality, if it allows shared access by a community of users, if it has a schema, or if it supports a query language.::: However, there is no agreed definition of these properties.:::Database management systems are usually categorized according to the data model that they support: relational, object-relational, network, and so on.::: The data model will tend to determine the query languages that are available to access the database.::: A great deal of the internal engineering of a DBMS, however, is independent of the data model, and is concerned with managing factors such as performance, concurrency, integrity, and recovery from hardware failures.::: In these areas there are large differences between products'.
type:nchar
TRMeisterとは TRMeisterはリコー独自の全文検索エンジンであり、今日最も要求されている情報検索システムにおいて必要とされる優れた検索性能を提供します。また、RDBMSとしての機能も有しています。:::全文検索エンジンとしての主な特徴 TRMeisterの全文検索エンジンとしての特徴は以下の通りです。:::ランキング検索    リコー独自のランキングアルゴリズムを用い、検索結果を適合度の順に並べることができます:::自然文検索    検索語として文章を入れることができ、システムが自動的に最適な検索語を抽出します :::関連語拡張     適合文書のフィードバックにより自動的に関連性の高い検索語を追加することができます :::多言語サポート     日本語、中国語(繁体字、簡体字)、英語、フランス語、イタリア語、ドイツ語、スペイン語、オランダ語の9言語に対応することができます :::RDBMSとしての主な特徴 TRMeisterはRDBMSとして非常に重要な以下の機能を有しています。:::オンラインバックアップ     システムの運用を中断せずにバックアップが可能です :::自動リカバリー     不慮の電源断などのために正常にシステムが終了しなかった場合でも、システムの再起動時に自動的にデータベースの整合性を回復します :::また、TRMeisterは業界標準であるSQLやJDBCを使用したアクセスが可能であり、他の高価なDBMSを使わずに高速に全文検索可能な情報検索アプリケーションを用意に構築できます。 
全文検索 出典: フリー百科事典『ウィキペディア（Wikipedia）』 全文検索（ぜんぶんけんさく、Full text search）とは、コンピュータにおいて、複数の文書(ファイル)から特定の文字列を検索すること。「ファイル名検索」や「ファイル内文字列検索」と異なり、「複数文書にまたがって、文書に含まれる全文を検索する」という意味で使用される。:::全文検索技術 grep型 順次走査検索、逐次検索とも。「grep」とはUnixにおける文字列検索コマンドであり、複数のテキストファイルの内容を順次走査していくことで、検索対象となる文字列を探し出す。一般に「grep型」と呼ばれる検索手法は、事前に索引ファイル（インデックス）を作成せず、ファイルを順次走査していくために、検索対象の増加に伴って検索速度が低下するのが特徴である。ちなみに「grep型」とは実際にgrepコマンドを使っているという意味ではないので注意のこと。:::索引（インデックス）型 検索対象となる文書数が膨大な場合、grep型では検索を行うたびに1つ1つの文書にアクセスし、該当データを逐次検索するので、検索対象文書の増加に比例して、検索にかかる時間も長くなっていってしまう。そこであらかじめ検索対象となる文書群を走査しておき、高速な検索が可能になるような索引データを準備することで、検索時のパフォーマンスを向上させる手法が取られている。事前に索引ファイルを作成することをインデクシング(indexing)と呼ぶ。インデクシングにより生成されるデータはインデックス（インデクス）と呼ばれ、その構造は多くの場合、「文字列 | ファイルの場所 | ファイルの更新日 | 出現頻度・・・」といったようなリスト形式（テーブル構造)を取り、文字列が検索キーとなっている。検索時にはこのインデックスにアクセスすることで、劇的に高速な検索が可能となる。:::索引文字列の抽出手法 形態素解析 英文の場合は単語と単語の間にスペースが入るため、自然、スペースで区切られた文字列を抽出していけば、索引データの作成は容易となる。しかし日本語の場合は、単語をスペースで区切る「わかち書き」の習慣がないため、形態素解析技術を用いて、文脈の解析、単語分解を行い、それをもとにインデックスを作成する必要がある。形態素解析を行うためには解析用の辞書が必須であり、検索結果は辞書の品質に少なからず影響を受ける。また、辞書に登録されていないひらがな単語の抽出に難があるなど、技術的障壁も多く、検索漏れが生じることが欠点とされる。:::N-Gram 「N文字インデックス法」「Nグラム法」などともいう。検索対象を単語単位ではなく一定のN文字単位で分解し、それの出現頻度を求める方法。Nの値が1なら「ユニグラム(uni-gram)」、2なら「バイグラム(bi-gram)」、3なら「トライグラム(tri-gram)」と呼ばれる。たとえば「全文検索技術」という文字列の場合、「全文」「文検」「検索」「索技」「技術」と2文字ずつ分割して索引化を行ってやれば、検索漏れが生じず、辞書の必要も無い。しかし形態素解析によるわかち書きに比べると、意図したものとは異なる検索結果（検索ノイズ＝「京都」で検索すると「東京都庁」がヒットするなど）が生じることが多く、インデックスのサイズも肥大化しがちであることが欠点とされる。
データベース 出典: フリー百科事典『ウィキペディア（Wikipedia）』 データベース (Database) は、特定のテーマに沿ったデータを集めて管理し、容易に検索・抽出などの再利用をできるようにしたもの。 狭義には、コンピュータによって実現されたものを言う。OSが提供するファイルシステム上に直接構築されるものや、後述するデータベースマネージメントシステム (DBMS) を用いて構築されるものを含む。:::コンピュータ上では、データの再利用を高速かつ安定に実現するため、データを格納するための構造について様々な工夫が払われており、このデータ構造とアルゴリズムは情報工学において重要な研究分野のひとつである。:::単純なファイルシステムには、ファイルシステム自体に「データ」を統一的手法で操作する機能はない。ファイルシステムでデータ管理をするためには、データの操作機能を「応用プログラム側」に持つしかない。データベースは、それを自ら持つことにより、応用プログラム側でデータの物理的格納状態を知らずとも操作でき、かつ、データの物理的格納状態に変更があった場合にも応用プログラム側の処理に影響が及ばないことを保障することがデータベースの前提条件となっている。(プログラムとデータの独立性):::データベースをコンピュータ上で管理するためのシステム（Oracle Database、SQL Server、PostgreSQL、MySQL、SQLiteなど）をデータベースマネージメントシステム（DBMS）という。
type:int
0:::1:::-1:::6543210:::-34567890:::2147483647:::-2147483648
type:bigint
0:::1:::-1:::876543210:::-5678901234567890:::9223372036854775807:::-9223372036854775808
type:float
0:::3.141592:::3.776E3:::1.79769313486231E308:::-1.79769313486231E308:::2.22507385850721E-308:::-2.22507385850721E-308
type:decimal
0:::1:::-1:::987654.3210987654321:::0.01234567890123456789:::-987654.3210987654321:::-0.01234567890123456789
generate:halfnull
type:char
What is TRMeister?:::null:::TRMeister is a proprietary full-text search engine of Ricoh and provides an advanced information retrieval system which is a prime requirement today. In addition, it also has RDBMS function.:::null:::Main feature of full-text search engine:::null:::TRMeister is characterized by full-text search as below.:::null:::Ranking search :::null:::    The results can be listed up by ranking score value with unique algorithm of Ricoh. :::null:::Search by sentence :::null:::    Sentence can be used as terms and adequate terms are extracted automatically. :::null:::Expansion of query terms :::null:::    It can automatically adds related terms through feedback of relevant documents. :::null:::Multi language support :::null:::    It can support Japanese, Chinese(traditional, simplified), English, French, Italian, German, Spanish and Dutch. :::null:::Main feature of RDBMS:::null:::TRMeister is characterized by very important function as below.:::null:::Online backup :::null:::    It can backup without suspending system operations. :::null:::Automatic recovery :::null:::    Data can be verified automatically when the server restart by an unforeseen sutdown. :::null:::TRMeister can connect to global standard DB servers or through JDBC and also can create a high-speed information retrieval system without costly DBMS. 
Full text search From Wikipedia, the free encyclopedia In text retrieval, full text search (also called free search text [citation needed]) refers to a technique for searching a computer-stored document or database; in a full text search, the search engine examines all of the words in every stored document as it tries to match search words supplied by the user. :::null:::Full-text searching techniques became common in online bibliographic databases in the 1970s. :::null:::Most Web sites and application programs (such as word processing software) provide full text search capabilities. :::null:::Some Web search engines, such as AltaVista employ full text search techniques, while others index only a portion of the Web pages examined by its indexing system.[1]:::null:::The most common approach to full text search is to generate a complete index or concordance for all of the searchable documents. :::null:::For each word (excepting stop words which are too common to be useful) an entry is made which lists the exact position of every occurrence of it within the database of documents. :::null:::From such a list it is relatively simple to retrieve all the documents that match a query, without having to scan each document. :::null:::Although for very small document collections full-text searching can be done by serial scanning, indexing is the preferred method for almost all full-text searching.
Database From Wikipedia, the free encyclopedia The term or expression database originated within the computer industry.:::null::: Although its meaning has been broadened by popular use, even to include non-electronic databases, this article takes a more technical perspective towards the topic.:::null::: A possible definition is that a database is a structured collection of records or data which is stored in a computer so that a program can consult it to answer queries.:::null::: The records retrieved in answer to queries become information that can be used to make decisions.:::null::: The computer program used to manage and query a database is known as a database management system (DBMS).:::null::: The properties and design of database systems are included in the study of information science.:::null:::The central concept of a database is that of a collection of records, or pieces of knowledge.:::null::: Typically, for a given database, there is a structural description of the type of facts held in that database: this description is known as a schema.:::null::: The schema describes the objects that are represented in the database, and the relationships among them.:::null::: There are a number of different ways of organizing a schema, that is, of modelling the database structure: these are known as database models (or data models).:::null::: The model in most common use today is the relational model, which in layman's terms represents all information in the form of multiple related tables each consisting of rows and columns (the true definition uses mathematical terminology).:::null::: This model represents relationships by the use of values common to more than one table.:::null::: Other models such as the hierarchical model and the network model use a more explicit representation of relationships.:::null:::The term database refers to the collection of related records, and the software should be referred to as the database management system or DBMS.:::null::: When the context is unambiguous, however, many database administrators and programmers use the term database to cover both meanings.:::null:::Many professionals would consider a collection of data to constitute a database only if it has certain properties: for example, if the data is managed to ensure its integrity and quality, if it allows shared access by a community of users, if it has a schema, or if it supports a query language.:::null::: However, there is no agreed definition of these properties.:::null:::Database management systems are usually categorized according to the data model that they support: relational, object-relational, network, and so on.:::null::: The data model will tend to determine the query languages that are available to access the database.:::null::: A great deal of the internal engineering of a DBMS, however, is independent of the data model, and is concerned with managing factors such as performance, concurrency, integrity, and recovery from hardware failures.:::null::: In these areas there are large differences between products'.
type:nchar
TRMeisterとは TRMeisterはリコー独自の全文検索エンジンであり、今日最も要求されている情報検索システムにおいて必要とされる優れた検索性能を提供します。また、RDBMSとしての機能も有しています。:::null:::全文検索エンジンとしての主な特徴 TRMeisterの全文検索エンジンとしての特徴は以下の通りです。:::null:::ランキング検索    リコー独自のランキングアルゴリズムを用い、検索結果を適合度の順に並べることができます:::null:::自然文検索    検索語として文章を入れることができ、システムが自動的に最適な検索語を抽出します :::null:::関連語拡張     適合文書のフィードバックにより自動的に関連性の高い検索語を追加することができます :::null:::多言語サポート     日本語、中国語(繁体字、簡体字)、英語、フランス語、イタリア語、ドイツ語、スペイン語、オランダ語の9言語に対応することができます :::null:::RDBMSとしての主な特徴 TRMeisterはRDBMSとして非常に重要な以下の機能を有しています。:::null:::オンラインバックアップ     システムの運用を中断せずにバックアップが可能です :::null:::自動リカバリー     不慮の電源断などのために正常にシステムが終了しなかった場合でも、システムの再起動時に自動的にデータベースの整合性を回復します :::null:::また、TRMeisterは業界標準であるSQLやJDBCを使用したアクセスが可能であり、他の高価なDBMSを使わずに高速に全文検索可能な情報検索アプリケーションを用意に構築できます。 
全文検索 出典: フリー百科事典『ウィキペディア（Wikipedia）』 全文検索（ぜんぶんけんさく、Full text search）とは、コンピュータにおいて、複数の文書(ファイル)から特定の文字列を検索すること。「ファイル名検索」や「ファイル内文字列検索」と異なり、「複数文書にまたがって、文書に含まれる全文を検索する」という意味で使用される。:::null:::全文検索技術 grep型 順次走査検索、逐次検索とも。「grep」とはUnixにおける文字列検索コマンドであり、複数のテキストファイルの内容を順次走査していくことで、検索対象となる文字列を探し出す。一般に「grep型」と呼ばれる検索手法は、事前に索引ファイル（インデックス）を作成せず、ファイルを順次走査していくために、検索対象の増加に伴って検索速度が低下するのが特徴である。ちなみに「grep型」とは実際にgrepコマンドを使っているという意味ではないので注意のこと。:::null:::索引（インデックス）型 検索対象となる文書数が膨大な場合、grep型では検索を行うたびに1つ1つの文書にアクセスし、該当データを逐次検索するので、検索対象文書の増加に比例して、検索にかかる時間も長くなっていってしまう。そこであらかじめ検索対象となる文書群を走査しておき、高速な検索が可能になるような索引データを準備することで、検索時のパフォーマンスを向上させる手法が取られている。事前に索引ファイルを作成することをインデクシング(indexing)と呼ぶ。インデクシングにより生成されるデータはインデックス（インデクス）と呼ばれ、その構造は多くの場合、「文字列 | ファイルの場所 | ファイルの更新日 | 出現頻度・・・」といったようなリスト形式（テーブル構造)を取り、文字列が検索キーとなっている。検索時にはこのインデックスにアクセスすることで、劇的に高速な検索が可能となる。:::null:::索引文字列の抽出手法 形態素解析 英文の場合は単語と単語の間にスペースが入るため、自然、スペースで区切られた文字列を抽出していけば、索引データの作成は容易となる。しかし日本語の場合は、単語をスペースで区切る「わかち書き」の習慣がないため、形態素解析技術を用いて、文脈の解析、単語分解を行い、それをもとにインデックスを作成する必要がある。形態素解析を行うためには解析用の辞書が必須であり、検索結果は辞書の品質に少なからず影響を受ける。また、辞書に登録されていないひらがな単語の抽出に難があるなど、技術的障壁も多く、検索漏れが生じることが欠点とされる。:::null:::N-Gram 「N文字インデックス法」「Nグラム法」などともいう。検索対象を単語単位ではなく一定のN文字単位で分解し、それの出現頻度を求める方法。Nの値が1なら「ユニグラム(uni-gram)」、2なら「バイグラム(bi-gram)」、3なら「トライグラム(tri-gram)」と呼ばれる。たとえば「全文検索技術」という文字列の場合、「全文」「文検」「検索」「索技」「技術」と2文字ずつ分割して索引化を行ってやれば、検索漏れが生じず、辞書の必要も無い。しかし形態素解析によるわかち書きに比べると、意図したものとは異なる検索結果（検索ノイズ＝「京都」で検索すると「東京都庁」がヒットするなど）が生じることが多く、インデックスのサイズも肥大化しがちであることが欠点とされる。
データベース 出典: フリー百科事典『ウィキペディア（Wikipedia）』 データベース (Database) は、特定のテーマに沿ったデータを集めて管理し、容易に検索・抽出などの再利用をできるようにしたもの。 狭義には、コンピュータによって実現されたものを言う。OSが提供するファイルシステム上に直接構築されるものや、後述するデータベースマネージメントシステム (DBMS) を用いて構築されるものを含む。:::null:::コンピュータ上では、データの再利用を高速かつ安定に実現するため、データを格納するための構造について様々な工夫が払われており、このデータ構造とアルゴリズムは情報工学において重要な研究分野のひとつである。:::null:::単純なファイルシステムには、ファイルシステム自体に「データ」を統一的手法で操作する機能はない。ファイルシステムでデータ管理をするためには、データの操作機能を「応用プログラム側」に持つしかない。データベースは、それを自ら持つことにより、応用プログラム側でデータの物理的格納状態を知らずとも操作でき、かつ、データの物理的格納状態に変更があった場合にも応用プログラム側の処理に影響が及ばないことを保障することがデータベースの前提条件となっている。(プログラムとデータの独立性):::null:::データベースをコンピュータ上で管理するためのシステム（Oracle Database、SQL Server、PostgreSQL、MySQL、SQLiteなど）をデータベースマネージメントシステム（DBMS）という。
type:int
6543210:::null:::0:::null:::1:::null:::-1:::null:::-34567890:::null:::2147483647:::null:::-2147483648
type:bigint
876543210:::null:::0:::null:::1:::null:::-1:::null:::-5678901234567890:::null:::9223372036854775807:::null:::-9223372036854775808
type:float
3.776E3:::null:::0:::null:::3.141592:::null:::1.79769313486231E308:::null:::-1.79769313486231E308:::null:::2.22507385850721E-308:::null:::-2.22507385850721E-308
type:decimal
987654.3210987654321:::null:::0:::null:::1:::null:::-1:::null:::0.01234567890123456789:::null:::-987654.3210987654321:::null:::-0.01234567890123456789
generate:freetext
type:char
Full-text search engine developed by Ricoh
Most important thing for full text search
Famous database management system
type:nchar
リコー製のDBMS機能つき全文検索エンジン
全文検索の最新動向
データベースマネージメントシステムの研究
result:auto
0
0@@@1@@@2@@@3@@@4
0
generate:wordlist
type:char
text@@@search@@@Ricoh
full@@@text@@@search
database@@@management@@@system
type:nchar
全文検索@@@データベース@@@リコー
索引@@@ファイル
データベース@@@マネージメント@@@システム
result:auto
0
0@@@1@@@2@@@3@@@4
0
generate:contains
type:char
TRMeister
text
database
type:nchar
TRMeister:::全文検索:::関連
全文検索:::索引:::形態素
DBMS:::SQL:::ファイル
result:auto
0@@@1@@@6@@@9
0@@@1@@@4
0@@@2@@@3
generate:likehead
type:char
TRMeister%
It%
Although%
type:nchar
TRMeister%:::全文検索%:::関連%
全文検索%:::索引%:::形態素%
DBMS%:::SQL%:::ファイル%
result:auto
0
0@@@1
0@@@3
generate:likemiddle
type:char
%TRMeister%
%text%
%database%
type:nchar
%TRMeister%:::%全文検索%:::%関連%
%全文検索%:::%索引%:::%形態素%
%DBMS%:::%SQL%:::%ファイル%
result:auto
0@@@1@@@6@@@9
0@@@1@@@4
0@@@2@@@3
generate:=
type:int
6543210:::-34567890:::2147483647:::-2147483648
-34567890:::2147483647:::-2147483648:::6543210
2147483647:::-2147483648:::6543210:::-34567890
-2147483648:::6543210:::-34567890:::2147483647
result:auto
3:::4:::5:::6
4:::5:::6:::3
5:::6:::3:::4
6:::3:::4:::5
type:bigint
876543210:::-5678901234567890:::9223372036854775807:::-9223372036854775808
-5678901234567890:::9223372036854775807:::-9223372036854775808:::876543210
9223372036854775807:::-9223372036854775808:::876543210:::-5678901234567890
-9223372036854775808:::876543210:::-5678901234567890:::9223372036854775807
result:auto
3:::4:::5:::6
4:::5:::6:::3
5:::6:::3:::4
6:::3:::4:::5
type:float
3.776E3:::1.79769313486231E308:::-2.22507385850721E-308
1.79769313486231E308:::-2.22507385850721E-308:::3.776E3
-2.22507385850721E-308:::3.776E3:::1.79769313486231E308
result:auto
2:::3:::6
3:::6:::2
6:::2:::3
type:decimal
987654.3210987654321:::0.01234567890123456789
0.01234567890123456789:::987654.3210987654321
result:auto
3:::4
4:::3
generate:>
type:int
-5000000:::0
0:::-5000000
result:auto
0@@@1@@@2@@@3@@@5:::1@@@3@@@5
1@@@3@@@5:::0@@@1@@@2@@@3@@@5
type:bigint
-700000000:::0
0:::-700000000
result:auto
0@@@1@@@2@@@3@@@5:::1@@@3@@@5
1@@@3@@@5:::0@@@1@@@2@@@3@@@5
type:float
-3000:::0
0:::-3000
result:auto
0@@@1@@@2@@@3@@@5@@@6:::1@@@2@@@3@@@5
1@@@2@@@3@@@5:::0@@@1@@@2@@@3@@@5@@@6
type:decimal
-6543.2109:::0
0:::-6543.2109
result:auto
0@@@1@@@2@@@3@@@4:::1@@@3@@@4
1@@@3@@@4:::0@@@1@@@2@@@3@@@4
generate:<
type:int
20000000:::0
0:::20000000
result:auto
0@@@1@@@2@@@3@@@4@@@6:::2@@@4@@@6
2@@@4@@@6:::0@@@1@@@2@@@3@@@4@@@6
type:bigint
4000000000000000:::0
0:::4000000000000000
result:auto
0@@@1@@@2@@@3@@@4@@@6:::2@@@4@@@6
2@@@4@@@6:::0@@@1@@@2@@@3@@@4@@@6
type:float
5.32E10:::0
0:::5.32E10
result:auto
0@@@1@@@2@@@4@@@5@@@6:::4@@@6
4@@@6:::0@@@1@@@2@@@4@@@5@@@6
type:decimal
9876.543:::0
0:::9876.543
result:auto
0@@@1@@@2@@@4@@@5@@@6:::2@@@5@@@6
2@@@5@@@6:::0@@@1@@@2@@@4@@@5@@@6
generate:between
type:int
-5000000@@@0:::0@@@20000000
0@@@20000000:::-5000000@@@0
result:auto
0@@@2:::0@@@1@@@3
0@@@1@@@3:::0@@@2
type:bigint
-700000000@@@0:::0@@@4000000000000000
0@@@4000000000000000:::-700000000@@@0
result:auto
0@@@2:::0@@@1@@@3
0@@@1@@@3:::0@@@2
type:float
-3000@@@0:::0@@@5.32E10
0@@@5.32E10:::-3000@@@0
result:auto
0@@@6:::0@@@1@@@2@@@5
0@@@1@@@2@@@5:::0@@@6
type:decimal
-6543.2109@@@0:::0@@@9876.543
0@@@9876.543:::-6543.2109@@@0
result:auto
0@@@2:::0@@@1@@@4
0@@@1@@@4:::0@@@2
