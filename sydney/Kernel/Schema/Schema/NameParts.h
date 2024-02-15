// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// NameParts.h -- オブジェクトの名前関連のクラス定義、関数宣言
// 
// Copyright (c) 2000, 2004, 2005, 2006, 2007, 2008, 2009, 2011, 2012, 2013, 2014, 2017, 2023 Ricoh Company, Ltd.
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

#ifndef	__SYDNEY_SCHEMA_NAMEPARTS_H
#define	__SYDNEY_SCHEMA_NAMEPARTS_H

#include "Schema/Module.h"
#include "Schema/PathParts.h"

#include "Common/UnicodeString.h"

_SYDNEY_BEGIN
_SYDNEY_SCHEMA_BEGIN

// オブジェクト名に用いる文字列定義
// ★注意★
// 以下の定義中にはASCII文字以外を用いてはいけない

namespace NameParts
{
	namespace Database
	{
		//	CONST
		//	Schema::NameParts::Database::Default --
		//		デフォルトのデータベースの名前
		//
		//	NOTES
		//		以前は'%DB%'だったが、データベース名がディレクトリ名に
		//		使われること、%DB%はWindowsでは「環境変数DBの値」を
		//		意味することから、普通の名前にした。

		const char* const Default = "DefaultDB";

		//	CONST
		//	Schema::NameParts::Database::Temporary --
		//		一時表を格納するデータベースの名前
		//
		//	NOTES
		//		一時表は常にこのデータベースに属するものとする
		//		この形式にセッションIDを当てはめたものを使う

		const char* const Temporary = "#%08X";

		//	CONST
		//	Schema::NameParts::Database::System --
		//		メタデータベースの名前
		//
		//	NOTES
		//		メタデータベースは固定スキーマの
		//		メモリ上にあるデータベースである

		const char* const System = "$$SystemDB";
	}

	namespace Table
	{
		//	CONST
		//	Schema::NameParts::Table::Temporary --
		//		一時表を表す表名のプレフィックス
		//
		//	NOTES
		//		一時表は名前の先頭が'#'で始まるものである

		const ModUnicodeChar Temporary = Common::UnicodeChar::usSharp;

		//	CONST
		//	Schema::NameParts::Table::SystemDatabase --
		//		メタデータベースのデータベース表の名前
		//
		//	NOTES

		const char* const SystemDatabase = "System_Database";

		//	CONST
		//	Schema::NameParts::Table::SystemTable --
		//		メタデータベースの表表の名前
		//
		//	NOTES
		//		対象のデータベースに属する表になる

		const char* const SystemTable = "System_Table";

		//	CONST
		//	Schema::NameParts::Table::SystemIndex --
		//		メタデータベースの索引表の名前
		//
		//	NOTES
		//		対象のデータベースに属する表になる

		const char* const SystemIndex = "System_Index";

		//	CONST
		//	Schema::NameParts::Table::SystemColumn --
		//		メタデータベースの列表の名前
		//
		//	NOTES
		//		対象のデータベースに属する表になる

		const char* const SystemColumn = "System_Column";

		//	CONST
		//	Schema::NameParts::Table::SystemKey --
		//		メタデータベースのキー表の名前
		//
		//	NOTES
		//		対象のデータベースに属する表になる

		const char* const SystemKey = "System_Key";

		//	CONST
		//	Schema::NameParts::Table::SystemFile --
		//		メタデータベースのファイル表の名前
		//
		//	NOTES
		//		対象のデータベースに属する表になる

		const char* const SystemFile = "System_File";

		//	CONST
		//	Schema::NameParts::Table::SystemField --
		//		メタデータベースのフィールド表の名前
		//
		//	NOTES
		//		対象のデータベースに属する表になる

		const char* const SystemField = "System_Field";

		//	CONST
		//	Schema::NameParts::Table::SystemConstraint --
		//		メタデータベースの制約表の名前
		//
		//	NOTES
		//		対象のデータベースに属する表になる

		const char* const SystemConstraint = "System_Constraint";

		//	CONST
		//	Schema::NameParts::Table::SystemArea --
		//		メタデータベースのエリア表の名前
		//
		//	NOTES
		//		対象のデータベースに属する表になる

		const char* const SystemArea = "System_Area";

		//	CONST
		//	Schema::NameParts::Table::SystemAreaContent --
		//		メタデータベースのエリア格納関係表の名前
		//
		//	NOTES
		//		対象のデータベースに属する表になる

		const char* const SystemAreaContent = "System_AreaContent";

		//	CONST
		//	Schema::NameParts::Table::SystemPrivilege --
		//		Privilege System table's name
		//
		//	NOTES

		const char* const SystemPrivilege = "System_Privilege";

		//	CONST
		//	Schema::NameParts::Table::SystemCascade --
		//		メタデータベースの関数表の名前
		//
		//	NOTES
		//		対象のデータベースに属する表になる

		const char* const SystemCascade = "System_Cascade";

		//	CONST
		//	Schema::NameParts::Table::SystemFunction --
		//		メタデータベースの関数表の名前
		//
		//	NOTES
		//		対象のデータベースに属する表になる

		const char* const SystemFunction = "System_Function";

		//	CONST
		//	Schema::NameParts::Table::SystemPartition --
		//		メタデータベースの関数表の名前
		//
		//	NOTES
		//		対象のデータベースに属する表になる

		const char* const SystemPartition = "System_Partition";
	}

	namespace Column
	{
		//	CONST
		//	Schema::NameParts::Column::TupleID -- タプル ID を格納する列の名前
		//
		//	NOTES
		//		他の商品で同様の列に対してROWIDという名前が使われているので
		//		ここでもROWIDという名前にする

		const char* const TupleID = "ROWID";

		namespace System
		{
			//	CONST
			//	Schema::NameParts::Column::System::TupleID -- システム表に属する列名
			//
			//	NOTES

			const char* const TupleID = NameParts::Column::TupleID;

			//	CONST
			//	Schema::NameParts::Column::System::Name -- システム表に属する列名
			//
			//	NOTES

			const char* const Name = "Name";

			//	CONST
			//	Schema::NameParts::Column::System::Flag -- システム表に属する列名
			//
			//	NOTES

			const char* const Flag = "Flag";

			//	CONST
			//	Schema::NameParts::Column::System::Path -- システム表に属する列名
			//
			//	NOTES

			const char* const Path = "Paths";

			//	CONST
			//	Schema::NameParts::Column::System::ParentID -- システム表に属する列名
			//
			//	NOTES

			const char* const ParentID = "ParentID";

			//	CONST
			//	Schema::NameParts::Column::System::Category -- システム表に属する列名
			//
			//	NOTES

			const char* const Category = "Type"; // 関連文書検索機能対応のときにCategoryが予約語になった

			//	CONST
			//	Schema::NameParts::Column::System::Position -- システム表に属する列名
			//
			//	NOTES

			const char* const Position = "Position";

			//	CONST
			//	Schema::NameParts::Column::System::Permission -- システム表に属する列名
			//
			//	NOTES

			const char* const Permission = "Permission";

			//	CONST
			//	Schema::NameParts::Column::System::ColumnID -- システム表に属する列名
			//
			//	NOTES

			const char* const ColumnID = "ColumnID";

			//	CONST
			//	Schema::NameParts::Column::System::KeyID -- システム表に属する列名
			//
			//	NOTES

			const char* const KeyID = "KeyID";

			//	CONST
			//	Schema::NameParts::Column::System::FieldID -- システム表に属する列名
			//
			//	NOTES

			const char* const FieldID = "FieldID";

			//	CONST
			//	Schema::NameParts::Column::System::SourceID -- システム表に属する列名
			//
			//	NOTES

			const char* const SourceID = "SourceID";

			//	CONST
			//	Schema::NameParts::Column::System::FileID -- システム表に属する列名
			//
			//	NOTES

			const char* const FileID = "FileID";

			//	CONST
			//	Schema::NameParts::Column::System::AreaID -- システム表に属する列名
			//
			//	NOTES

			const char* const AreaID = "AreaID";

			//	CONST
			//	Schema::NameParts::Column::System::ObjectID -- システム表に属する列名
			//
			//	NOTES

			const char* const ObjectID = "ObjectID";

			//	CONST
			//	Schema::NameParts::Column::System::Hint -- システム表に属する列名
			//
			//	NOTES

			const char* const Hint = "Hint";

			//	CONST
			//	Schema::NameParts::Column::System::Option -- システム表に属する列名
			//
			//	NOTES

			const char* const Option = "Option";

			//	CONST
			//	Schema::NameParts::Column::System::Function -- システム表に属する列名
			//
			//	NOTES

			const char* const Function = "Function";

			//	CONST
			//	Schema::NameParts::Column::System::Source -- システム表に属する列名
			//
			//	NOTES

			const char* const Source = "Source";

			//	CONST
			//	Schema::NameParts::Column::System::Default -- システム表に属する列名
			//
			//	NOTES

			const char* const Default = "Default";

			//	CONST
			//	Schema::NameParts::Column::System::DataType -- システム表に属する列名
			//
			//	NOTES

			const char* const DataType = "DataType";

			//	CONST
			//	Schema::NameParts::Column::System::MetaData -- システム表に属する列名
			//
			//	NOTES

			const char* const MetaData = "MetaData";

			//	CONST
			//	Schema::NameParts::Column::System::FileSize -- システム表に属する列名
			//
			//	NOTES

			const char* const FileSize = "FileSize";

			//	CONST
			//	Schema::NameParts::Column::System::HintString -- システム表に属する列名
			//
			//	NOTES

			const char* const HintString = "HintString";

			//	CONST
			//	Schema::NameParts::Column::System::UserID -- システム表に属する列名
			//
			//	NOTES

			const char* const UserID = "UserID";

			//	CONST
			//	Schema::NameParts::Column::System::Privilege -- システム表に属する列名
			//
			//	NOTES

			const char* const Privilege = "Privilege";

			//	CONST
			//	Schema::NameParts::Column::System::ObjectType -- システム表に属する列名
			//
			//	NOTES

			const char* const ObjectType = "ObjectType";

			//	CONST
			//	Schema::NameParts::Column::System::ClientID -- システム表に属する列名
			//
			//	NOTES

			const char* const ClientID = "ClientID";

			//	CONST
			//	Schema::NameParts::Column::System::HostName -- システム表に属する列名
			//
			//	NOTES

			const char* const HostName = "HostName";

			//	CONST
			//	Schema::NameParts::Column::System::ConnectedTime -- システム表に属する列名
			//
			//	NOTES

			const char* const ConnectedTime = "ConnectedTime";

			//	CONST
			//	Schema::NameParts::Column::System::ProtocolVersion -- システム表に属する列名
			//
			//	NOTES

			const char* const ProtocolVersion = "ProtocolVersion";

			//	CONST
			//	Schema::NameParts::Column::System::CryptMode -- システム表に属する列名
			//
			//	NOTES

			const char* const CryptMode = "CryptMode";

			//	CONST
			//	Schema::NameParts::Column::System::SessionID -- システム表に属する列名
			//
			//	NOTES

			const char* const SessionID = "SessionID";

			//	CONST
			//	Schema::NameParts::Column::System::DatabaseName -- システム表に属する列名
			//
			//	NOTES

			const char* const DatabaseName = "DatabaseName";
			
			//	CONST
			//	Schema::NameParts::Column::System::UserName -- システム表に属する列名
			//
			//	NOTES

			const char* const UserName = "UserName";
			
			//	CONST
			//	Schema::NameParts::Column::System::SessionStartTime -- システム表に属する列名
			//
			//	NOTES

			const char* const SessionStartTime = "SessionStartTime";

			//	CONST
			//	Schema::NameParts::Column::System::StatementType -- システム表に属する列名
			//
			//	NOTES

			const char* const StatementType = "StatementType";
			
			//	CONST
			//	Schema::NameParts::Column::System::TransactionState -- システム表に属する列名
			//
			//	NOTES
			const char* const TransactionState = "TransactionState";
			
			//	CONST
			//	Schema::NameParts::Column::System::TransactionStartTime -- システム表に属する列名
			//
			//	NOTES

			const char* const TransactionStartTime = "TransactioStartTime";

			//	CONST
			//	Schema::NameParts::Column::System::SqlStatement -- システム用に属する列名
			//
			//	NOTES

			const char* const SqlStatement = "SqlStatement";
			
			//	CONST
			//	Schema::NameParts::Column::System::Target -- システム表に属する列名
			//
			//	NOTES

			const char* const Target = "Target";
			
			//	CONST
			//	Schema::NameParts::Column::System::FunctionName -- システム表に属する列名
			//
			//	NOTES

			const char* const FunctionName = "FunctionName";
			
			//	CONST
			//	Schema::NameParts::Column::System::Routine -- システム表に属する列名
			//
			//	NOTES

			const char* const Routine = "Routine";
			
			//	CONST
			//	Schema::NameParts::Column::System::MasterURL -- システム表に属する列名
			//
			//	NOTES

			const char* const MasterURL = "MasterURL";
		}
	}

	namespace Constraint
	{
		//	CONST
		//	Schema::NameParts::Constraint::PrimaryKey --
		//		主キーの制約につける名前
		//
		//	NOTES
		//		表名にこの文字列を付加することで主キーの制約の名前にする

		const char* const PrimaryKey = "_$$PrimaryKey";

		//	CONST
		//	Schema::NameParts::Constraint::Unique --
		//		ユニーク制約につける名前
		//
		//	NOTES
		//		この文字列に列名を付加することでユニーク制約の名前にする

		const char* const Unique = "UNIQUE";

		//	CONST
		//	Schema::NameParts::Constraint::ForeignKey --
		//		外部キー制約につける名前
		//
		//	NOTES
		//		この文字列に参照する列名を付加することで外部キー制約の名前にする

		const char* const ForeignKey = "FOREIGN";

		//	CONST
		//	Schema::NameParts::Constraint::ReferedKey --
		//		外部キー制約の対になる制約につける名前
		//
		//	NOTES
		//		この文字列に参照する索引名を付加することで外部キー制約の対になる制約の名前にする

		const char* const ReferedKey = "REFER";
	}

	namespace File
	{
		//	CONST
		//	Schema::NameParts::File::Record -- あるレコードファイルの名前
		//
		//	NOTES

		const char* const Record = PathParts::File::Record;

		//	CONST
		//	Schema::NameParts::File::Heap -- あるヒープファイルの名前
		//
		//	NOTES

		const char* const Heap = PathParts::File::Heap;

		//	CONST
		//	Schema::NameParts::File::Btree -- ある B+ 木ファイルの名前
		//
		//	NOTES

		const char* const Btree = PathParts::File::Btree;

		//	CONST
		//	Schema::NameParts::File::FullText -- ある全文ファイルの名前
		//
		//	NOTES

		const char* const FullText = PathParts::File::FullText;

		//	CONST
		//	Schema::NameParts::File::Vector -- あるベクターファイルの名前
		//
		//	NOTES

		const char* const Vector = PathParts::File::Vector;

		//	CONST
		//	Schema::NameParts::File::Conversion -- ROWID-OID変換の名前
		//
		//	NOTES

		const char* const Conversion = PathParts::File::Conversion;

		//	CONST
		//	Schema::NameParts::File::Lob -- LOBファイルの名前
		//
		//	NOTES

		const char* const Lob = PathParts::File::Lob;

		//	CONST
		//	Schema::NameParts::File::Bitmap -- ある bitmap ファイルの名前
		//
		//	NOTES

		const char* const Bitmap = PathParts::File::Bitmap;

		//	CONST
		//	Schema::NameParts::File::Array -- ある array ファイルの名前
		//
		//	NOTES

		const char* const Array = PathParts::File::Array;

		//	CONST
		//	Schema::NameParts::File::KdTree -- あるKdTreeファイルの名前
		//
		//	NOTES

		const char* const KdTree = PathParts::File::KdTree;
	}

	namespace Field
	{
		//	CONST
		//	Schema::NameParts::Field::ObjectID --
		//		あるオブジェクト ID フィールドの名前
		//
		//	NOTES
		//		この文字列に名前を生成するオブジェクト ID フィールドを
		//		保持するファイルの名称を追加することにより生成する

		const char* const ObjectID = "OID:";

		//	CONST
		//	Schema::NameParts::Field::Key -- あるキーフィールドの名前
		//
		//	NOTES
		//		この形式に名前を生成するキーフィールドに対応する列の名称を
		//		追加することにより生成する

		const char* const Key = "KEY:";

		//	CONST
		//	Schema::NameParts::Field::Data -- あるデータフィールドの名前
		//
		//	NOTES
		//		この形式に名前を生成するデータフィールドに対応する列の名称を
		//		追加することにより生成する

		const char* const Data = "DAT:";

		//	CONST
		//	Schema::NameParts::Field::Score -- スコア関数の名前
		//
		//	NOTES

		const char* const Score = "SCR:";

		//	CONST
		//	Schema::NameParts::Field::Section -- スコア関数の名前
		//
		//	NOTES

		const char* const Section = "SEC:";

		//	CONST
		//	Schema::NameParts::Field::Word -- Word関数の名前
		//
		//	NOTES

		const char* const Word = "WRD:";

		//	CONST
		//	Schema::NameParts::Field::WordDf -- WordDf関数の名前
		//
		//	NOTES

		const char* const WordDf = "WDF:";

		//	CONST
		//	Schema::NameParts::Field::WordScale -- WordScale関数の名前
		//
		//	NOTES

		const char* const WordScale = "WSC:";

		//	CONST
		//	Schema::NameParts::Field::AverageLength -- 平均文書長/平均単語数関数の名前
		//
		//	NOTES

		const char* const AverageLength = "AVL:";

		//	CONST
		//	Schema::NameParts::Field::AverageCharLength -- 平均文書長関数の名前
		//
		//	NOTES

		const char* const AverageCharLength = "AVC:";

		//	CONST
		//	Schema::NameParts::Field::AverageWordCount -- 平均単語数関数の名前
		//
		//	NOTES

		const char* const AverageWordCount = "AVW:";

		//	CONST
		//	Schema::NameParts::Field::Count -- 文書数関数の名前
		//
		//	NOTES

		const char* const Count = "CNT:";

		//	CONST
		//	Schema::NameParts::Field::Tf -- 単語出現数関数の名前
		//
		//	NOTES

		const char* const Tf = "TF:";

		//	CONST
		//	Schema::NameParts::Field::Existence -- 単語出現関数の名前
		//
		//	NOTES

		const char* const Existence = "EXT:";

		//	CONST
		//	Schema::NameParts::Field::ClusterId -- 結果のクラスター番号
		//
		//	NOTES

		const char* const ClusterId = "CID:";

		//	CONST
		//	Schema::NameParts::Field::ClusterWord -- 結果のクラスター用特徴語配列
		//
		//	NOTES

		const char* const ClusterWord = "CWD:";

		//	CONST
		//	Schema::NameParts::Field::Kwic -- Key Word In Context
		//
		//	NOTES

		const char* const Kwic = "KWC:";

		//	CONST
		//	Schema::NameParts::Field::MinKey -- 最小値関数の名前
		//
		//	NOTES

		const char* const MinKey = "MIN:";

		//	CONST
		//	Schema::NameParts::Field::MaxKey -- 最大値関数の名前
		//
		//	NOTES

		const char* const MaxKey = "MAX:";

		//	CONST
		//	Schema::NameParts::Field::NeighborId -- 
		//
		//	NOTES

		const char* const NeighborId = "NID:";

		//	CONST
		//	Schema::NameParts::Field::NeighborDistance -- 
		//
		//	NOTES

		const char* const NeighborDistance = "NDT:";
	}
}

_SYDNEY_SCHEMA_END
_SYDNEY_END

#endif	// __SYDNEY_SCHEMA_NAMEPARTS_H

//
// Copyright (c) 2000, 2004, 2005, 2006, 2007, 2008, 2009, 2011, 2012, 2013, 2014, 2017, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
