// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// FullTextIndex.cpp -- 全文索引関連の関数定義
// 
// Copyright (c) 2002, 2004, 2005, 2006, 2011, 2015, 2023 Ricoh Company, Ltd.
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

namespace {
const char srcFile[] = __FILE__;
const char moduleName[] = "Schema";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Schema/FullTextIndex.h"
#include "Schema/AreaCategory.h"
#include "Schema/FullTextFile.h"
#include "Schema/Table.h"
#include "Schema/Column.h"
#include "Schema/Key.h"
#include "Schema/Field.h"
#include "Schema/Message.h"
#include "Schema/LogData.h"

#include "Statement/IndexDefinition.h"
#include "Statement/ColumnNameList.h"
#include "Statement/ColumnName.h"

#include "Common/Assert.h"

#include "Exception/InvalidIndexKey.h"
#include "Exception/NotSupported.h"

_SYDNEY_USING
_SYDNEY_SCHEMA_USING

//	FUNCTION public
//	Schema::FullTextIndex::FullTextIndex -- 全文索引を表すクラスのデフォルトコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

FullTextIndex::
FullTextIndex()
	: Index(Category::FullText)
{ }

//	FUNCTION public
//	Schema::FullTextIndex::FullTextIndex -- 全文索引を表すクラスのコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		const Schema::Database& database
//			索引が属するデータベース
//		Schema::Table& table
//			索引が属する表
//		const Schema::Object::Name& cName_
//			索引の名称
//		Schema::Hint* pHint_
//			索引のヒント
//
//	RETURN
//		なし
//
//	EXCEPTIONS

FullTextIndex::
FullTextIndex(const Database& database, Table& table, const Name& cName_, Hint* pHint_)
	: Index(Category::FullText, database, table, cName_, pHint_)
{ }

//	FUNCTION public
//	Schema::FullTextIndex::FullTextIndex -- 制約からの索引を表すクラスのコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		const Schema::Database& database
//			索引が属するデータベース
//		Schema::Table&	table
//			索引が存在する表を表すクラス
//		const Schema::Constraint&	constraint
//			索引に対応する制約を表すクラス
//
//	RETURN
//		なし
//
//	EXCEPTIONS

FullTextIndex::
FullTextIndex(const Database& database, Table& table, const Constraint& constraint)
	: Index(Category::FullText, database, table, constraint)
{ }

//	FUNCTION public
//	Schema::FullTextIndex::FullTextIndex -- データベースからの索引を表すクラスのコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Database&	database
//			索引が存在するデータベースを表すクラス
//		const Schema::LogData& cLogData_
//			索引作成のログ
//
//	RETURN
//		なし
//
//	EXCEPTIONS

FullTextIndex::
FullTextIndex(const Database& database, const LogData& cLogData_)
	: Index(Category::FullText, database, cLogData_)
{ }

//	FUNCTION public
//	Schema::FullTextIndex::createKey -- 索引定義にしたがってキーオブジェクトを作成する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		Schema::Table&	table
//			索引を定義する表を表すクラス
//		const Statement::IndexDefinition& cStatement_
//			索引の定義を表すオブジェクト
//		Common::DataArrayData& cLogData_
//			ログデータに格納するデータ
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
FullTextIndex::
createKey(Trans::Transaction& cTrans_,
		  Table& cTable_,
		  const Statement::IndexDefinition& cStatement_,
		  Common::DataArrayData& cLogData_)
{
	// キー列指定があるときのみ処理する
	Statement::ColumnNameList* names = cStatement_.getColumnNameList();
	; _SYDNEY_ASSERT(names);

	// 言語列指定
	Statement::ColumnName* langColumn = cStatement_.getLanguageColumnName();

	// スコア列指定
	Statement::ColumnName* scoreColumn = cStatement_.getScoreColumnName();

	// キーにする列名の総数を得る
	int n = names->getCount();

	// 16.5から複合索引がサポートされた

	cLogData_.clear();
	cLogData_.reserve(n + ((langColumn) ? 1 : 0) + ((scoreColumn) ? 1 : 0));

	// キーにする列名の指定に対応してキーを追加する
	int i = 0;
	while (i < n) {
		Statement::ColumnName* name = names->getColumnNameAt(i);
		; _SYDNEY_ASSERT(name);

		Key::Pointer key = Key::create(*this, i++, cTable_, *name, cTrans_);
		; _SYDNEY_ASSERT(key.get());

		// キーの状態は「生成」である
		; _SYDNEY_ASSERT(key->getStatus() == Status::Created);

		Field* pKeyField = key->getColumn(cTrans_)->getField(cTrans_);
		; _SYDNEY_ASSERT(pKeyField);

		// 文字列型でなければならない
		if (pKeyField->getType() != Common::DataType::String
			&& pKeyField->getElementType() != Common::DataType::String) {
			_SYDNEY_THROW2(Exception::InvalidIndexKey, getName(), *name->getIdentifierString());
		}

		// 索引に追加する
		// ★注意★
		// 永続化は索引単位で行うので索引にキーが登録されている必要がある
		// キーの新規作成は索引の新規作成と常にセットなので
		// ここで追加しても他のセッションに影響はない
		// キャッシュへの登録は永続化後に行う
		(void) addKey(key, cTrans_);

		// ログデータに追加する
		ModAutoPointer<Common::DataArrayData> pData = new Common::DataArrayData();
		key->makeLogData(*pData);
		cLogData_.pushBack(pData.release());
	}

	if (langColumn) {
		// 言語列もキーに追加する
		Key::Pointer langKey = Key::create(*this, i++, cTable_, *langColumn, cTrans_);
		; _SYDNEY_ASSERT(langKey.get());

		// 全文が配列なら言語も配列でなければならない
		// -> という仕様は複合索引実装時からなくした

		Field* pLangField = langKey->getColumn(cTrans_)->getField(cTrans_);

		// 言語型でなければならない
		if (pLangField->getType() != Common::DataType::Language
			&& pLangField->getElementType() != Common::DataType::Language) {
			_SYDNEY_THROW2(Exception::InvalidIndexKey, getName(), *langColumn->getIdentifierString());
		}

		// キーの状態は「生成」である
		; _SYDNEY_ASSERT(langKey->getStatus() == Status::Created);

		// 索引に追加する
		// ★注意★
		// 永続化は索引単位で行うので索引にキーが登録されている必要がある
		// キーの新規作成は索引の新規作成と常にセットなので
		// ここで追加しても他のセッションに影響はない
		// キャッシュへの登録は永続化後に行う
		(void) addKey(langKey, cTrans_);

		// ログデータに追加する
		ModAutoPointer<Common::DataArrayData> pData = new Common::DataArrayData();
		langKey->makeLogData(*pData);
		cLogData_.pushBack(pData.release());
	}

	if (scoreColumn) {
		// スコア指定列もキーに追加する
		Key::Pointer scoreKey = Key::create(*this, i++, cTable_, *scoreColumn, cTrans_);
		; _SYDNEY_ASSERT(scoreKey.get());

		// スコア指定列はFloat型でなければならない
		// 第一キー以外はKey.cppのisAllowedではなくここでチェックする
		Column* pScoreColumn = scoreKey->getColumn(cTrans_);
		; _SYDNEY_ASSERT(pScoreColumn);

		if (pScoreColumn->getType().getType() != Common::SQLData::Type::Float) {
			_SYDNEY_THROW2(Exception::InvalidIndexKey, getName(), *scoreColumn->getIdentifierString());
		}

		// キーの状態は「生成」である
		; _SYDNEY_ASSERT(scoreKey->getStatus() == Status::Created);

		// 索引に追加する
		// ★注意★
		// 永続化は索引単位で行うので索引にキーが登録されている必要がある
		// キーの新規作成は索引の新規作成と常にセットなので
		// ここで追加しても他のセッションに影響はない
		// キャッシュへの登録は永続化後に行う
		(void) addKey(scoreKey, cTrans_);

		// ログデータに追加する
		ModAutoPointer<Common::DataArrayData> pData = new Common::DataArrayData();
		scoreKey->makeLogData(*pData);
		cLogData_.pushBack(pData.release());
	}
}

//	FUNCTION protected
//	Schema::FullTextIndex::createFile --
//		索引を構成するファイルのクラスを生成する
//
//	NOTES
//		オブジェクトを作成するだけで実際のファイルはまだ作成されない
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作をするトランザクション記述子
//
//	RETURN
//		作成したファイルオブジェクト
//
//	EXCEPTIONS

File::Pointer
FullTextIndex::
createFile(Trans::Transaction& cTrans_)
{
	return FullTextFile::create(cTrans_,
								*getDatabase(cTrans_),
								*getTable(cTrans_), *this,
								getHint(), getAreaHint());
}

//	FUNCTION protected
//	Schema::FullTextIndex::createField --
//		索引を構成するファイルのフィールドクラスを生成する
//
//	NOTES
//		オブジェクトを作成するだけで実際のファイルはまだ作成されない
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作をするトランザクション記述子
//		File& cFile_
//			索引を構成するファイルオブジェクト
//		const Common::DataArrayData* pLogData_ /* = 0 */
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
FullTextIndex::
createField(Trans::Transaction& cTrans_, File& cFile_,
			const Common::DataArrayData* pLogData_ /* = 0 */)
{
	// ファイルの状態は「生成」である
	; _SYDNEY_ASSERT(cFile_.getStatus() == Status::Created
					 || cFile_.getStatus() == Status::Mounted);

	// 索引を構成するキーの値を格納するフィールドをファイルに加えていく
	// ★注意★
	// キーに対応するフィールドとTIDのフィールドはどの索引でも加えられる
	// 索引ごとに固有のフィールドはこの後で設定される

	const ModVector<Key*>& keys = getKey(cTrans_);

	ModSize n = keys.getSize();
	for (ModSize i = 0; i < n; ++i) {
		; _SYDNEY_ASSERT(keys[i]);

		// キーとファイルにフィールドを登録する
		// ★注意★
		// source->addDestinationは永続化後の処理で行う

		(void) keys[i]->setField(*(cFile_.addField(*keys[i], Field::Permission::Putable, cTrans_,
												   cFile_.getNextFieldID(cTrans_, pLogData_))));
	}

	// 索引で探索した結果得られる
	// ある表のタプル ID の値を格納するフィールドを、
	// ファイルに加える
	{
		; _SYDNEY_ASSERT(getTable(cTrans_));

		Column* column = getTable(cTrans_)->getTupleID(cTrans_);
		; _SYDNEY_ASSERT(column);

		Field* source = column->getField(cTrans_);
		; _SYDNEY_ASSERT(source);

		// ★注意★
		// source->addDestinationは永続化後に行う
		cFile_.addField(Field::Category::Data, Field::Permission::All, *source, *column, cTrans_,
						cFile_.getNextFieldID(cTrans_, pLogData_));
	}
}

//	FUNCTION public
//	Schema::FullTextIndex::getClassID -- このクラスのクラス ID を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

int
FullTextIndex::
getClassID() const
{
	return Externalizable::Category::FullTextIndex +
		Common::Externalizable::SchemaClasses;
}

//	FUNCTION public
//	Schema::FullTextIndex::getAreaCategory --
//		索引を実現するファイルを格納するエリアの種別を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		Schema::AreaCategory::Value
//			索引を実現するファイルを格納するエリアの種別を表す値
//
//	EXCEPTIONS
//

AreaCategory::Value
FullTextIndex::
getAreaCategory() const
{
	return AreaCategory::FullText;
}

//
// Copyright (c) 2002, 2004, 2005, 2006, 2011, 2015, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
