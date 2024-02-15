// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// PathParts.h -- ファイルのパス名関連のクラス定義、関数宣言
// 
// Copyright (c) 2000, 2005, 2006, 2007, 2012, 2013, 2023 Ricoh Company, Ltd.
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// 
//     http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// 

#ifndef	__SYDNEY_SCHEMA_PATHPARTS_H
#define	__SYDNEY_SCHEMA_PATHPARTS_H

#include "Schema/Module.h"

_SYDNEY_BEGIN
_SYDNEY_SCHEMA_BEGIN

// パス名に用いる文字列定義
// ★注意★
// 以下の定義中にはASCII文字以外を用いてはいけない

namespace PathParts
{
	namespace File
	{
		//	CONST
		//	Schema::PathParts::File::Record --
		//		あるレコードファイルに関するファイルを格納する
		//		ディレクトリーの名前
		//
		//	NOTES
		//		この文字列に表名を追加することにより生成する
		//		レコードファイルは表ごとにひとつなので名前は固定
		//		★注意★
		//		水平分割、垂直分割が行われるようになるとこれではいけない

		const char* const Record = "RCD_";

		//	CONST
		//	Schema::PathParts::File::Heap --
		//		ヒープとして使われるレコードファイルに関するファイルを格納する
		//		ディレクトリーの名前
		//
		//	NOTES
		//		この文字列にヒープに対応する列名を追加することにより生成する

		const char* const Heap = "RHP_";

		//	CONST
		//	Schema::PathParts::File::Btree --
		//		ある B+ 木ファイルに関するファイルを格納する
		//		ディレクトリーの名前
		//
		//	NOTES
		//		この文字列にB+木ファイルを使用する索引名を追加することにより生成する

		const char* const Btree = "BTR_";

		//	CONST
		//	Schema::PathParts::File::FullText --
		//		ある全文ファイルに関するファイルを格納する
		//		ディレクトリーの名前
		//
		//	NOTES
		//		この文字列に全文ファイルを使用する索引名を追加することにより生成する

		const char* const FullText = "FTS_";

		//	CONST
		//	Schema::PathParts::File::Vector --
		//		あるベクターファイルに関するファイルを格納する
		//		ディレクトリーの名前
		//
		//	NOTES
		//		この文字列にベクターファイルを使用する索引名を追加することにより生成する

		const char* const Vector = "VCT_";

		//	CONST
		//	Schema::PathParts::File::Conversion --
		//		ROWID-OID変換を行うファイルに使用する名前
		//
		//	NOTES
		//		表名にこの文字列を付加することによりファイル名にする
		//		ファイルの種別に応じた接頭辞がさらにつく

		const char* const Conversion = "_$$Conversion";

		//	CONST
		//	Schema::PathParts::File::Lob --
		//		Lobファイルに使用する名前
		//
		//	NOTES
		//		この文字列にLOBファイルを使用する列名を追加することにより生成する

		const char* const Lob = "LOB_";

		//	CONST
		//	Schema::PathParts::File::Bitmap --
		//		ある bitmap ファイルに関するファイルを格納する
		//		ディレクトリーの名前
		//
		//	NOTES
		//		この文字列にbitmapファイルを使用する索引名を追加することにより生成する

		const char* const Bitmap = "BMP_";

		//	CONST
		//	Schema::PathParts::File::Array --
		//		ある array ファイルに関するファイルを格納する
		//		ディレクトリーの名前
		//
		//	NOTES
		//		この文字列にarrayファイルを使用する索引名を追加することにより生成する

		const char* const Array = "ARY_";

		//	CONST
		//	Schema::PathParts::File::KdTree --
		//		あるKdTreeファイルに関するファイルを格納する
		//		ディレクトリーの名前
		//
		//	NOTES
		//		この文字列にKdTreepファイルを使用する索引名を追加することにより生成する

		const char* const KdTree = "KTR_";

		//	CONST
		//	Schema::PathParts::File::Log --
		//		論理ログファイルの名前
		//
		//	NOTES

		const char* const Log = "LOGICALLOG.SYD";
	}

	namespace Sequence
	{
		//	CONST
		//	Schema::PathParts::Sequence::ObjectID --
		//		スキーマオブジェクト ID 用のシーケンス用の物理ファイルを
		//		格納するディレクトリーの名前
		//
		//	NOTES

		const char* const ObjectID = "OBJECTID";

		//	CONST
		//	Schema::PathParts::Sequence::TupleID --
		//		ある表のタプル ID 用のシーケンス用の物理ファイルを
		//		格納するディレクトリーの名前
		//
		//	NOTES

		const char* const TupleID = "ROWID";

		//	CONST
		//	Schema::PathParts::Sequence::Identity --
		//		ある表の Identity column 用のシーケンス用の物理ファイルを
		//		格納するディレクトリーの名前
		//
		//	NOTES

		const char* const Identity = "IDENTITY";
	}

	namespace SystemTable
	{
		//	CONST
		//	Schema::PathParts::SystemTable::Schema --
		//		メタデータベースのトップディレクトリ名
		//
		//	NOTES

		const char* const Schema = "SCHEMA";

		//	CONST
		//	Schema::PathParts::SystemTable::Temporary --
		//		一時データベースのトップディレクトリ名
		//
		//	NOTES

		const char* const Temporary = "$$TempDB";

		//	CONST
		//	Schema::PathParts::SystemTable::Database --
		//		メタデータベースの「データベース」表を格納する
		//		ディレクトリ名
		//
		//	NOTES

		const char* const Database = "Database";

		//	CONST
		//	Schema::PathParts::SystemTable::Area --
		//		メタデータベースの「エリア」表を格納する
		//		ディレクトリ名
		//
		//	NOTES

		const char* const Area = "Area";

		//	CONST
		//	Schema::PathParts::SystemTable::AreaContent --
		//		メタデータベースの「エリア格納関係」表を格納する
		//		ディレクトリ名
		//
		//	NOTES

		const char* const AreaContent = "AreaContent";

		//	CONST
		//	Schema::PathParts::SystemTable::Table --
		//		メタデータベースの「表」表を格納する
		//		ディレクトリ名
		//
		//	NOTES
		//		「表」表はデータベースごとに別である
		//		この名称とデータベースの名称を組み合わせた
		//		ディレクトリが作成される

		const char* const Table = "Table";

		//	CONST
		//	Schema::PathParts::SystemTable::Column --
		//		メタデータベースの「列」表を格納する
		//		ディレクトリ名
		//
		//	NOTES
		//		「列」表はデータベースごとに別である
		//		この名称とデータベースの名称を組み合わせた
		//		ディレクトリが作成される

		const char* const Column = "Column";

		//	CONST
		//	Schema::PathParts::SystemTable::Constraint --
		//		メタデータベースの「制約」表を格納する
		//		ディレクトリ名
		//
		//	NOTES
		//		「制約」表はデータベースごとに別である
		//		この名称とデータベースの名称を組み合わせた
		//		ディレクトリが作成される

		const char* const Constraint = "Constraint";

		//	CONST
		//	Schema::PathParts::SystemTable::Index --
		//		メタデータベースの「索引」表を格納する
		//		ディレクトリ名
		//
		//	NOTES
		//		「索引」表はデータベースごとに別である
		//		この名称とデータベースの名称を組み合わせた
		//		ディレクトリが作成される

		const char* const Index = "Index";

		//	CONST
		//	Schema::PathParts::SystemTable::Key --
		//		メタデータベースの「キー」表を格納する
		//		ディレクトリ名
		//
		//	NOTES
		//		「キー」表はデータベースごとに別である
		//		この名称とデータベースの名称を組み合わせた
		//		ディレクトリが作成される

		const char* const Key = "Key";

		//	CONST
		//	Schema::PathParts::SystemTable::File --
		//		メタデータベースの「ファイル」表を格納する
		//		ディレクトリ名
		//
		//	NOTES
		//		「ファイル」表はデータベースごとに別である
		//		この名称とデータベースの名称を組み合わせた
		//		ディレクトリが作成される

		const char* const File = "File";

		//	CONST
		//	Schema::PathParts::SystemTable::Field --
		//		メタデータベースの「フィールド」表を格納する
		//		ディレクトリ名
		//
		//	NOTES
		//		「フィールド」表はデータベースごとに別である
		//		この名称とデータベースの名称を組み合わせた
		//		ディレクトリが作成される

		const char* const Field = "Field";

		//	CONST
		//	Schema::PathParts::SystemTable::Cascade --
		//		メタデータベースの「子サーバー」表を格納する
		//		ディレクトリ名
		//
		//	NOTES

		const char* const Cascade = "Cascade";

		//	CONST
		//	Schema::PathParts::SystemTable::Partition --
		//		メタデータベースの「ルール」表を格納する
		//		ディレクトリ名
		//
		//	NOTES

		const char* const Partition = "Partition";

		//	CONST
		//	Schema::PathParts::SystemTable::Function --
		//		メタデータベースの「関数」表を格納する
		//		ディレクトリ名
		//
		//	NOTES

		const char* const Function = "Function";

		//	CONST
		//	Schema::PathParts::SystemTable::Previlege --
		//		Directory name for system table "privilege"
		//
		//	NOTES

		const char* const Privilege = "Privilege";

		//	CONST
		//	Schema::PathParts::SystemTable::Record --
		//		システム表を構成するファイルのうちレコードファイルのパス名
		//
		//	NOTES
		//		レコードファイルはすべてのシステム表に共通に1つだけあるので
		//		ここでパス名を定義しておく
		//		システム表を構成するファイルのうち索引ファイルは
		//		種類があるのでここでは定義せずに実装ファイルで定義する

		const char* const Record = "RCD_SYSTEMFILE";
	}
}

_SYDNEY_SCHEMA_END
_SYDNEY_END

#endif	// __SYDNEY_SCHEMA_PATHPARTS_H

//
// Copyright (c) 2000, 2005, 2006, 2007, 2012, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
