// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// File.cpp -- ファイル関連の関数定義
// 
// Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2023 Ricoh Company, Ltd.
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
#include "SyDynamicCast.h"

#include "Schema/Area.h"
#include "Schema/AreaContent.h"
#include "Schema/ArrayFile.h"
#include "Schema/AutoLatch.h"
#include "Schema/AutoRWLock.h"
#include "Schema/BitmapFile.h"
#include "Schema/BtreeFile.h"
#include "Schema/Database.h"
#include "Schema/FakeError.h"
#include "Schema/Field.h"
#include "Schema/FieldMap.h"
#include "Schema/File.h"
#include "Schema/FileMap.h"
#include "Schema/FileReflect.h"
#include "Schema/FullTextFile.h"
#include "Schema/Hint.h"
#include "Schema/Index.h"
#include "Schema/KdTreeFile.h"
#include "Schema/Key.h"
#include "Schema/LobFile.h"
#include "Schema/LogData.h"
#include "Schema/Manager.h"
#include "Schema/Message.h"
#include "Schema/Meta.h"
#include "Schema/ObjectTemplate.h"
#include "Schema/Parameter.h"
#include "Schema/RecordFile.h"
#include "Schema/Recovery.h"
#include "Schema/SystemTable_File.h"
#include "Schema/SystemTable_Field.h"
#include "Schema/Table.h"
#include "Schema/Utility.h"
#include "Schema/VectorFile.h"

#include "Checkpoint/Database.h"
#include "Checkpoint/FileDestroyer.h"

#include "Common/Assert.h"
#include "Common/BinaryData.h"
#include "Common/DataArrayData.h"
#include "Common/Message.h"
#include "Common/Parameter.h"
#include "Common/StringArrayData.h"
#include "Exception/AreaNotFound.h"
#include "Exception/BadArgument.h"
#include "Exception/MetaDatabaseCorrupted.h"
#include "Exception/RecoveryFailed.h"
#include "Exception/Unexpected.h"
#include "FileCommon/FileOption.h"
#include "Lock/Name.h"
#include "LogicalFile/AutoLogicalFile.h"
#include "LogicalFile/FileDriverManager.h"
#include "LogicalFile/FileDriver.h"
#include "LogicalFile/File.h"
#include "LogicalFile/FileID.h"
#include "LogicalFile/ObjectID.h"
#include "Os/Path.h"
#include "Statement/Literal.h"
#include "Trans/Transaction.h"

#include "ModAutoPointer.h"
#include "ModVector.h"
#include "ModUnicodeString.h"

_SYDNEY_USING
_SYDNEY_SCHEMA_USING

#define	_BEGIN_REORGANIZE_RECOVERY	\
								/* UNDO中の場合エラー処理をしない */ \
								if (!bUndo_ && Schema::Database::isAvailable(getDatabaseID())) { \
									try {
#define _END_REORGANIZE_RECOVERY \
									} catch (Exception::Object& e) { \
										SydErrorMessage << "Error recovery failed. FATAL. " << e << ModEndl; \
										/* データベースを使用不可能にする*/ \
										Schema::Database::setAvailability(getDatabaseID(), false); \
										/* エラー処理中に発生した例外は再送しない */ \
										/* thru. */ \
									} catch (...) { \
										SydErrorMessage << "Error recovery failed. FATAL." << ModEndl; \
										/* データベースを使用不可能にする*/ \
										Schema::Database::setAvailability(getDatabaseID(), false); \
										/* エラー処理中に発生した例外は再送しない */ \
										/* thru. */ \
									} \
								}

namespace {

// FieldMapのコンストラクターに与えるパラメーター
ModSize _fieldMapSize = 29; // 想定スキーマのフィールド最大数/2
ModBoolean _fieldMapEnableLink = ModFalse; // Iterationしない

// フィールドを出現位置の昇順でソートするための関数

ModBoolean
ascendingSortField(Field* l, Field* r)
{
	return (l->getPosition() < r->getPosition()) ? ModTrue : ModFalse;
}

// 配列のキー値がファイルに格納できるものかを得る

bool
_isArrayKeyImportable(const File& cFile_, const Common::DataArrayData& cData_)
{
	// 全文ファイルかKdTreeファイルの場合すべての要素がNullDataなら格納しない
	switch (cFile_.getCategory()) {
	case File::Category::FullText:
	case File::Category::KdTree:
		{
			int n = cData_.getCount();
			for (int i = 0; i < n; ++i) {
				// ひとつでもNull以外があれば格納できる
				if (!cData_.getElement(i)->isNull())
					return true;
			}
			// すべてNull
			return false;
		}
	default:
		{
			break;
		}
	}
	// 全文でない
	return true;
}

// キー値がファイルに格納できるものかを得る

bool
_isKeyImportable(const File& cFile_, const Common::Data::Pointer& pData_)
{
	return !( // できない条件のほうがわかりやすいので以下を反転させて返り値とする
			 (!cFile_.hasAllTuples()
			  && (pData_->isNull()
				  || (pData_->getType() == Common::DataType::Array
					  && pData_->getElementType() == Common::DataType::Data
					  && !_isArrayKeyImportable(cFile_, _SYDNEY_DYNAMIC_CAST(const Common::DataArrayData&, *pData_))))));
}

// 移動前後で重なりのないパスを破棄する
void
_sweep(const ModVector<ModUnicodeString>& vecPrevPath_, const ModVector<ModUnicodeString>& vecPostPath_)
{
	int n = ModMin(vecPrevPath_.getSize(), vecPostPath_.getSize());
	for (int i = 0; i < n; i++) {
		// 移動後のディレクトリーに同じものがなければ移動前のものを削除する
		int j = 0;
		for (; j < n; j++) {
			if (Os::Path::compare(vecPrevPath_[i], vecPostPath_[j])
				== Os::Path::CompareResult::Identical) {
				break;
			}
		}
		if (j == n) {
			// 同じものがないので削除する
			// ★注意★
			// 中身に何があっても強引にすべて削除する
			Utility::File::rmAll(vecPrevPath_[i]);
		}
	}
}

}

//	FUNCTION public
//	Schema::File::File -- ファイルを表すクラスのデフォルトコンストラクター
//
//	NOTES
//		このファイルのスキーマオブジェクト ID は採番されずに、
//		不定なままである
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

File::
File()
	: Object(Object::Category::File),
	  _category(Category::Unknown),
	  _table(0),
	  _index(0),
	  _indexID(Schema::Object::ID::Invalid),
	  _fields(0),
	  _area(0),
	  _areaID(ID::Invalid),
	  _logArea(0),
	  _logAreaID(ID::Invalid),
	  m_pHint(0),
	  m_pAreaHint(0),
	  m_pPathPart(0),
	  m_pvecSkipCheckKey(0),
	  m_pvecPutKey(0),
	  m_pFileReflect(0),
	  m_bCreated(false),
	  m_bImmediateDestroy(true),
	  m_pFileID(0)
{ }

//	FUNCTION public
//	Schema::File::File -- 表を構成するファイルを表すクラスのコンストラクター
//
//	NOTES
//		このファイルのスキーマオブジェクト ID は新たに採番される
//
//	ARGUMENTS
//		Schema::Database&	database
//			ファイルが存在するデータベースを表すクラス
//		Schema::Table&		table
//			ファイルを持つ表を表すクラス
//			これは、呼び出し中には更新されないが、
//			その後、更新される可能性がある
//		Schema::File::Category::Value	category
//			ファイルの種別
//		const Schema::Hint* pHint_
//			ファイルヒント
//		const Schema::Hint* pAreaHint_
//			エリアヒント
//
//	RETURN
//		なし
//
//	EXCEPTIONS

File::
File(const Database& database, Table& table, File::Category::Value category, const Hint* pHint_, const Hint* pAreaHint_)
	: Object(Object::Category::File, table.getScope(), table.getStatus(),
			 ID::Invalid, table.getID(), table.getDatabaseID()),
	  _category(category),
	  _table(&table),
	  _index(0),
	  _indexID(Object::ID::Invalid),
	  _fields(0),
	  _area(0),
	  _areaID(ID::Invalid),
	  _logArea(0),
	  _logAreaID(ID::Invalid),
	  m_pHint(0),
	  m_pAreaHint(0),
	  m_pPathPart(0),
	  m_pvecSkipCheckKey(0),
	  m_pvecPutKey(0),
	  m_pFileReflect(0),
	  m_bCreated(false),
	  m_bImmediateDestroy(true)
{
	if (pHint_)
		m_pHint = new Hint(*pHint_);
	if (pAreaHint_)
		m_pAreaHint = new Hint(*pAreaHint_);
	m_pFileID = new LogicalFile::FileID();	// File.hをインクルードするモジュールが
											// LogicalFileまで必要とならないように
											// ここでFileIDを設定する
}

//	FUNCTION public
//	Schema::File::File -- 索引を構成するファイルを表すクラスのコンストラクター
//
//	NOTES
//		このファイルのスキーマオブジェクト ID は新たに採番される
//
//	ARGUMENTS
//		Schema::Database&	database
//			ファイルが存在するデータベースを表すクラス
//		Schema::Table&		table
//			ファイルを持つ表を表すクラス
//			これは、呼び出し中には更新されないが、
//			その後、更新される可能性がある
//		Schema::Index&		index
//			ファイルを持つ索引を表すクラス
//			これは、呼び出し中には更新されないが、
//			その後、更新される可能性がある
//		Schema::File::Category::Value	category
//			ファイルの種別
//		const Schema::Hint* pHint_
//			ファイルヒント
//		const Schema::Hint* pAreaHint_
//			エリアヒント
//
//	RETURN
//		なし
//
//	EXCEPTIONS

File::
File(const Database& database, Table& table, Index& index,
	 File::Category::Value category, const Hint* pHint_, const Hint* pAreaHint_)
	: Object(Object::Category::File, index.getScope(), index.getStatus(),
			 ID::Invalid, table.getID(), table.getDatabaseID()),
	  _category(category),
	  _table(&table),
	  _index(&index),
	  _indexID(index.getID()),
	  _fields(0),
	  _area(0),
	  _areaID(ID::Invalid),
	  _logArea(0),
	  _logAreaID(ID::Invalid),
	  m_pHint(0),
	  m_pAreaHint(0),
	  m_pPathPart(0),
	  m_pvecSkipCheckKey(0),
	  m_pvecPutKey(0),
	  m_pFileReflect(0),
	  m_bCreated(false),
	  m_bImmediateDestroy(true)
{
	if (pHint_)
		m_pHint = new Hint(*pHint_);
	if (pAreaHint_)
		m_pAreaHint = new Hint(*pAreaHint_);
	m_pFileID = new LogicalFile::FileID();	// File.hをインクルードするモジュールが
											// LogicalFileまで必要とならないように
											// ここでFileIDを設定する
}

//	FUNCTION public
//	Schema::File::~File -- ファイルを表すクラスのデストラクター
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

File::
~File()
{
	destruct();
}

//	FUNCTION public
//		Schema::File::getNewInstance -- オブジェクトを新たに取得する
//
//	NOTES
//
//	ARGUMENTS
//		const Common::DataArrayData& cData_
//			元になるデータ
//
//	RETURN
//		新規に作成されたオブジェクト
//
//	EXCEPTIONS

// static
File*
File::
getNewInstance(const Common::DataArrayData& cData_)
{
	if (cData_.getCount() <= Meta::File::Category) {
		SydErrorMessage
			<< "File get new instance failed. Insufficient data array data."
			<< ModEndl;
		_SYDNEY_THROW0(Exception::MetaDatabaseCorrupted);
	}

	int iCategory;
	if (!unpack(cData_.getElement(Meta::File::Category).get(), iCategory)) {
		SydErrorMessage
			<< "Can't get file category."
			<< ModEndl;
		_SYDNEY_THROW0(Exception::MetaDatabaseCorrupted);
	}

	ModAutoPointer<File> pObject;

	switch (iCategory) {
	case Category::Record:
		pObject = new RecordFile();			break;
	case Category::Btree:
		pObject = new BtreeFile();			break;
	case Category::FullText:
		pObject = new FullTextFile();		break;
	case Category::Vector:
		pObject = new VectorFile();			break;
	case Category::Lob:
		pObject = new LobFile();			break;
	case Category::Bitmap:
		pObject = new BitmapFile();			break;
	case Category::Array:
		pObject = new ArrayFile();			break;
	case Category::KdTree:
		pObject = new KdTreeFile();			break;
	default:
		// 例外送出
		SydErrorMessage
			<< "File get new instance failed. Illegal file category."
			<< ModEndl;
		_SYDNEY_THROW0(Exception::MetaDatabaseCorrupted);
		break;
	}
	; _SYDNEY_ASSERT(pObject.get());

	pObject->unpack(cData_);
	return pObject.release();
}

//	FUNCTION public
//	Schema::File::clear -- ファイルを表すクラスのメンバーをすべて初期化する
//
//	NOTES
//		親クラスのメンバーは初期化しない
//		下位オブジェクトのキャッシュからの抹消を行わないので
//		キャッシュに載っているオブジェクトに対してこのメソッドを呼ぶときには
//		あらかじめキャッシュから抹消する処理を行う必要がある
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
File::
clear()
{
	_category = Category::Unknown;
	if (m_pFileID) m_pFileID->clear();
	_table = 0;
	_index = 0;
	_indexID = ID::Invalid;
	_logArea = 0;
	_logAreaID = ID::Invalid;
	_area = 0;
	_areaID = ID::Invalid;
	m_bCreated = false;
	m_bImmediateDestroy = true;

	destruct();
}

//	FUNCTION private
//	Schema::File::destruct -- ファイルを表すクラスのデストラクター下位関数
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

void
File::
destruct()
{
	// ★注意★
	// デストラクトのときは保持するオブジェクトを行儀よく片付ける必要はない
	// 必要ならばこのオブジェクトをdeleteするところでresetを呼ぶ
	// ここでは領域を開放するのみ

	delete _fields, _fields = 0;

	clearHint();
	clearAreaHint();

	delete m_pPathPart, m_pPathPart = 0;
	delete m_pFileReflect, m_pFileReflect = 0;

	delete m_pFileID, m_pFileID = 0;

	delete m_pvecSkipCheckKey, m_pvecSkipCheckKey = 0;
	delete m_pvecPutKey, m_pvecPutKey = 0;
}

// FUNCTION public
//	Schema::File::create -- ファイルのスキーマ情報を生成する
//
// NOTES
//		サブクラスのcreateで生成したスキーマ情報を表すクラスについて
//		ファイル共通の処理を行う
//
// ARGUMENTS
//		const Schema::File::Pointer& file
//			サブクラスのcreateで生成したクラス
//		Schema::Table& table
//			ファイルを追加する表を表すクラス
//	Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//	const Common::DataArrayData* pLogData_ /* = 0 */
//	
// RETURN
//		生成したファイルを表すクラス
//
// EXCEPTIONS

// static
const File::Pointer&
File::
create(Trans::Transaction& cTrans_, const Pointer& file, Table& table,
	   const Common::DataArrayData* pLogData_ /* = 0 */)
{
	; _SYDNEY_ASSERT(cTrans_.getCategory()
					 != Trans::Transaction::Category::ReadOnly);
	; _SYDNEY_ASSERT(file.get());

	// IDをふり、状態を変える
	ID::Value iID = pLogData_ ? LogData::getID(pLogData_->getElement(Log::ID)) : ID::Invalid;
	file->Object::create(cTrans_, iID);

	if (file->getScope() == Scope::Permanent) {

		// ファイルに割り当てられるエリアを調べる
		// ・まず自分のエリアカテゴリーで親オブジェクトまでさかのぼって調べる
		// ・見つからなければエリアカテゴリーの上位に当たるもので再度調べる

		file->setAreaID(table, cTrans_);
	}

	// ファイルを格納するディレクトリのデータベース名などの部分を作る
	// 実際に格納されるディレクトリーは<TOP>/ここでセットした部分
	// になる
	// <TOP>はエリアの指定、一時オブジェクトかどうかにより変わる

	file->m_pPathPart = new Os::Path(table.getPathPart(cTrans_));
	file->m_pPathPart->addPart(file->getName());

	// 表に生成したファイルを表すクラスを追加する
	// ★注意★
	// キャッシュに登録するのは永続化の後
	table.addFile(file, cTrans_);

	switch (file->getCategory()) {
	case Category::Record:
	case Category::Lob:
	{
		// レコードファイル、LOBファイルの先頭には
		// 必ずオブジェクト ID を格納するフィールドが存在するので、
		// このフィールドを表すクラスを生成する

		Field::Pointer field =
			Field::create(*file, 0, Field::Category::ObjectID,
						  Field::Permission::All,
						  LogicalFile::ObjectID().getType(), 0,
						  cTrans_,
						  file->getNextFieldID(cTrans_, pLogData_));
		; _SYDNEY_ASSERT(field.get());
		; _SYDNEY_ASSERT(field->getType() != Common::DataType::Undefined);

		// フィールドの状態は「生成」である

		; _SYDNEY_ASSERT(field->getStatus() == Status::Created);

		break;
	}
	case Category::Btree:
	{
		// B+木はオブジェクトIDが不要になったが
		// 2001年2月時点では論理ファイルドライバーが0番をオブジェクトIDと仮定してしまっているので
		// このフィールドを表すクラスを生成する
		// insertデータとして入れる必要があるのでputableにしておく

		Field::Pointer field =
			Field::create(*file, 0, Field::Category::ObjectID,
						  Field::Permission::Putable,
						  LogicalFile::ObjectID().getType(), 0,
						  cTrans_,
						  file->getNextFieldID(cTrans_, pLogData_));
		; _SYDNEY_ASSERT(field.get());
		; _SYDNEY_ASSERT(field->getType() != Common::DataType::Undefined);

		// フィールドの状態は「生成」である

		; _SYDNEY_ASSERT(field->getStatus() == Status::Created);

		break;
	}
	case Category::FullText:
	case Category::Vector:
	case Category::Bitmap:
	case Category::Array:
	case Category::KdTree:
	default:
		break;
	}

	// ファイルとエリアの間の「エリア格納関係」を作る
	// ★注意★
	// REDOのときにエリアIDの割り当てはInvalidではないのに
	// エリアオブジェクトが得られないことがある
	// この場合、必ずAlter TableなどがREDOされるはずであり、
	// そのREDOによって正しいエリア格納関係が作成される
	if (Area* pArea = file->getArea(file->getAreaCategory(), cTrans_)) {

		AreaContent::Pointer pContent =
			AreaContent::create(*pArea, *file, file->getAreaCategory(), cTrans_);

		// 状態は「生成」である

		; _SYDNEY_ASSERT(pContent->getStatus() == Status::Created);
	}

	return file;
}

// FUNCTION public
//	Schema::File::create -- ファイルのスキーマ情報を生成する
//
// NOTES
//		サブクラスのcreateで生成したスキーマ情報を表すクラスについて
//		ファイル共通の処理を行う
//
// ARGUMENTS
//		const Schema::File::Pointer& file
//			サブクラスのcreateで生成したクラス
//		Schema::Table& table
//			ファイルを追加する表を表すクラス
//		Schema::Index& index
//			ファイルを追加する索引を表すクラス
//	Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//	const Common::DataArrayData* pLogData_ /* = 0 */
//	
// RETURN
//		生成したファイルを表すクラス
//
// EXCEPTIONS

// static
const File::Pointer&
File::
create(Trans::Transaction& cTrans_,
	   const Pointer& file, Table& table, Index& index,
	   const Common::DataArrayData* pLogData_ /* = 0 */)
{
	; _SYDNEY_ASSERT(cTrans_.getCategory()
					 != Trans::Transaction::Category::ReadOnly);
	; _SYDNEY_ASSERT(file.get());

	// IDをふり、状態を変える
	ID::Value iID = pLogData_ ? LogData::getID(pLogData_->getElement(Log::ID)) : ID::Invalid;
	file->Object::create(cTrans_, iID);

	if (file->getScope() == Scope::Permanent) {

		// ファイルに割り当てられるエリアを調べる
		// ・まず自分のエリアカテゴリーで親オブジェクトまでさかのぼって調べる
		// ・見つからなければエリアカテゴリーの上位に当たるもので再度調べる

		file->setAreaID(index, cTrans_);
	}

	// 既存のファイルを表に読み込んでおく
	// ★注意★
	// doAfterPersistの中でファイルが表に追加されるので
	// ここで読み込んでおかないと追加のときに不完全なFileを
	// 読み込んでしまう

	(void) table.loadFile(cTrans_);

	// ファイルを格納するディレクトリのデータベース名などの部分を作る
	// 実際に格納されるディレクトリーは<TOP>/ここでセットした部分
	// になる
	// <TOP>はエリアの指定、一時オブジェクトかどうかにより変わる

	file->m_pPathPart = new Os::Path(Os::Path(table.getPathPart(cTrans_)).addPart(file->getName()));

	// 索引を構成するファイルとして生成したファイルを設定する
	// ★注意★
	// この時点で表に登録できるのは表が永続化後でないときのみ
	// キャッシュに登録するのは永続化の後

	if (table.getStatus() != Status::Persistent)
		table.addFile(file, cTrans_);
	index.setFile(file);

	switch (file->getCategory()) {
	case Category::Record:
	case Category::Lob:
	{
		// レコードファイル、LOBファイルの先頭には
		// 必ずオブジェクト ID を格納するフィールドが存在するので、
		// このフィールドを表すクラスを生成する

		Field::Pointer field =
			Field::create(*file, 0, Field::Category::ObjectID,
						  Field::Permission::All,
						  LogicalFile::ObjectID().getType(), 0,
						  cTrans_,
						  file->getNextFieldID(cTrans_, pLogData_));
		; _SYDNEY_ASSERT(field.get());
		; _SYDNEY_ASSERT(field->getType() != Common::DataType::Undefined);

		// フィールドの状態は「生成」である

		; _SYDNEY_ASSERT(field->getStatus() == Status::Created);
		break;
	}
	case Category::Btree:
	{
		// B+木はオブジェクトIDが不要になったが
		// 2001年2月時点では論理ファイルドライバーが0番をオブジェクトIDと仮定してしまっているので
		// このフィールドを表すクラスを生成する
		// スキーマ情報としてはオブジェクトIDにはしない
		// insertデータとして入れる必要があるのでputableにしておく

		Field::Pointer field =
			Field::create(*file, 0, Field::Category::ObjectID,
						  Field::Permission::Putable,
						  LogicalFile::ObjectID().getType(), 0,
						  cTrans_,
						  file->getNextFieldID(cTrans_, pLogData_));
		; _SYDNEY_ASSERT(field.get());
		; _SYDNEY_ASSERT(field->getType() != Common::DataType::Undefined);

		// フィールドの状態は「生成」である

		; _SYDNEY_ASSERT(field->getStatus() == Status::Created);

		break;
	}
	case Category::FullText:
	case Category::Vector:
	case Category::Bitmap:
	case Category::Array:
	case Category::KdTree:
	default:
		break;
	}

	// ファイルとエリアの間の「エリア格納関係」を作る
	// ★注意★
	// REDOのときにエリアIDの割り当てはInvalidではないのに
	// エリアオブジェクトが得られないことがある
	// この場合、必ずAlter TableなどがREDOされるはずであり、
	// そのREDOによって正しいエリア格納関係が作成される
	if (Area* pArea = file->getArea(file->getAreaCategory(), cTrans_)) {

		AreaContent::Pointer pContent =
			AreaContent::create(*pArea, *file, file->getAreaCategory(), cTrans_);

		// 状態は「生成」である

		; _SYDNEY_ASSERT(pContent->getStatus() == Status::Created);
	}

	return file;
}

//	FUNCTION public
//	Schema::File::create -- 自分自身の表すファイルを実際に生成する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			この操作を行うトランザクション記述子
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
File::create(Trans::Transaction& cTrans_)
{
	// 始める前に中断のチェックをする
	Manager::checkCanceled(cTrans_);

	// ファイルIDに示されたディレクトリーを生成する
	// AutoRmDirは最初から全体の数が分かっている必要があるので
	// まずパス名一覧を作る
	ModVector<ModUnicodeString> vecPath;
	int n = 0;
	Parameter::ValueType cValue;
	while (m_pFileID->getString(_SYDNEY_SCHEMA_FORMAT_KEY(
									FileCommon::FileOption::Area::Key, n),
								cValue)) {
		vecPath.pushBack(cValue);
		++n;
	}

	// エラー発生時に自動的にmkdirを取り消すためのクラス
	ModVector<Utility::File::AutoRmDir> vecAutoRmDir(n);
	{
		for (int i = 0; i < n; ++i) {
			vecAutoRmDir[i].setDir(vecPath[i]);
		}
	}

	// ファイルを操作するための論理ファイルドライバを取得する

	LogicalFile::FileDriver* driver =
		LogicalFile::FileDriverManager::getDriver(getDriverID());
	; _SYDNEY_ASSERT(driver);

	// ファイルを操作する
	{
		AutoLatch	latch(cTrans_, *this);
		*m_pFileID = LogicalFile::AutoLogicalFile(*driver, *m_pFileID).create(cTrans_);
	}

	m_bCreated = true;

	// すべて成功したのでエラー処理のためのクラスをdisableする
	{
		for (int i = 0; i < n; ++i) {
			vecAutoRmDir[i].disable();
		}
	}
}

// FUNCTION public
//	Schema::File::createForAlter -- create a new file for alter table
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	const File& cFile_
//	Table& cTable_
//	const Common::DataArrayData* pLogData_ /* = 0 */
//	
// RETURN
//	File::Pointer
//
// EXCEPTIONS

//static
File::Pointer
File::
createForAlter(Trans::Transaction& cTrans_, const File& cFile_, Table& cTable_,
			   const Common::DataArrayData* pLogData_ /* = 0 */)
{
	Pointer pResult;
	// create a new name
	Name cNewName("$$");
	cNewName.append(cFile_.getName());

	switch (cFile_.getCategory()) {
	case Category::Record:
		{
			pResult = File::create(cTrans_,
								   RecordFile::create(cTrans_, *cTable_.getDatabase(cTrans_), cTable_,
													  cFile_.getHint(), cFile_.getAreaHint()),
								   cTable_,
								   pLogData_);
			break;
		}
	case Category::Vector:
		{
			pResult = File::create(cTrans_,
								   VectorFile::create(cTrans_, *cTable_.getDatabase(cTrans_), cTable_,
													  cFile_.getHint(), cFile_.getAreaHint(),
													  cFile_.hasAllTuples()),
								   cTable_,
								   pLogData_);
			break;
		}
	default:
		{
			// never reach
			; _SYDNEY_ASSERT(false);
			_SYDNEY_THROW0(Exception::Unexpected);
		}
	}
	// rename to the new name
	pResult->rename(cNewName);

	return pResult;
}

//	FUNCTION public
//	Schema::File::drop -- ファイルのスキーマ定義を破棄する
//
//	NOTES
//		ファイルに属するフィールドの定義も抹消される
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		bool bRecovery_ = false
//			リカバリー処理でのDROPか
//		bool bNoUnset_ = false
//			true:削除の取り消し時、unsetStatusをしない(システム表の永続化を行う)
//		bool bImmediate = true
//			true:OSファイルの破棄を即座に行う
//			false:Checkpointモジュールに削除してもらうように登録する
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
File::
drop(Trans::Transaction& cTrans_, bool bRecovery_ /* = false */, bool bNoUnset_ /* = false */, bool bImmediate_ /* = true */)
{
	// ファイルに属するフィールドをすべて破棄する
	loadField(cTrans_).apply(ApplyFunction2<Field, bool, bool>(&Field::drop, bRecovery_, bNoUnset_),
							 BoolFunction1<Field, Field::Category::Value>(FieldMap::findByCategory, Field::Category::Function));
										// Functionフィールドは対象から除く

	// ファイルを格納するエリアとファイルの間の格納関係を破棄する
	{
		Area* pArea = getArea(getAreaCategory(), cTrans_);
		if (pArea)
			AreaContent::drop(*pArea, *this, getAreaCategory(), cTrans_, bRecovery_);
	}

	// 作成中なら表のマップから除き、実体を破棄してしまう
	// ★注意★
	// 以前の実装ではMOUNT中でもdestroyを実行していたが
	// MOUNT中は消してはいけないだろう。
	if (getStatus() == Status::Created) {
		// ファイルの削除
		destroy(cTrans_, true);
	}

	// オブジェクトに削除マークをつける、または作成をなかったことにする
	Object::drop(bRecovery_, bNoUnset_);

	// 即座に破棄するかのフラグをセットする
	// Object::dropの処理でStatusが変わるのでgetStatusで取り直す
	if (getStatus() == Status::Deleted
		|| getStatus() == Status::DeletedInRecovery)
		m_bImmediateDestroy = bImmediate_;
}

//	FUNCTION public
//	Schema::File::undoDrop -- ファイルの破棄マークをクリアする
//
//	NOTES
//		破棄は論理ログに出力する前なら破棄マークをクリアすることで
//		復旧可能である
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
File::undoDrop(Trans::Transaction& cTrans_)
{
	// ファイルに属するフィールドのすべての破棄マークをクリアする
	loadField(cTrans_).apply(ApplyFunction0<Field>(&Field::undoDrop),
							 BoolFunction1<Field, Field::Category::Value>(FieldMap::findByCategory, Field::Category::Function));
										// Functionフィールドは対象から除く

	// ファイルを格納するエリアとファイルの間の格納関係の破棄マークをクリアする
	{
		Area* pArea = getArea(getAreaCategory(), cTrans_);
		if (pArea)
			AreaContent::undoDrop(*pArea, *this, getAreaCategory(), cTrans_);
	}

	// 状態を変化を取り消す
	Object::undoDrop();
}

//	FUNCTION public
//	Schema::File::destroy -- 自分自身の表すファイルを実際に破棄する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			この操作を行うトランザクション記述子
//		bool bDestroyDirectory_ = false
//			trueの場合ファイルが格納されているディレクトリーも削除する
//			falseの場合ファイルが格納されているディレクトリーは削除しない
//		bool bForce_ = true
//			trueの場合チェックポイントを待たずにすぐに削除する
//			falseの場合システムパラメーターによる
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
File::destroy(
	Trans::Transaction& cTrans_, bool bDestroyDirectory_, bool bForce_)
{
	if (m_bCreated) {

		// ファイルを操作するための論理ファイルドライバを取得する

		LogicalFile::FileDriver* driver =
			LogicalFile::FileDriverManager::getDriver(getDriverID());
		; _SYDNEY_ASSERT(driver);

		// ファイルを操作する
		{
		AutoLatch	latch(cTrans_, *this);
		LogicalFile::AutoLogicalFile(*driver, *m_pFileID).destroy(cTrans_, getDatabaseID(),
																  bForce_ && m_bImmediateDestroy);
		}
		if (!m_bImmediateDestroy)
			// 即座に破棄されない場合は下位モジュールの管理情報を解放するためにunmountを呼ぶ
			unmount(cTrans_, true /* retain fileid */);

		m_bCreated = false;
	}

	if (bDestroyDirectory_) {
		// ファイルが格納されているディレクトリーを破棄する

		if (Area* pArea = getArea(getAreaCategory(), cTrans_))
			pArea->destroy(cTrans_, getPathPart(cTrans_), bForce_ && m_bImmediateDestroy);

		else {
			Database* pDatabase = getDatabase(cTrans_);
			; _SYDNEY_ASSERT(pDatabase);

			pDatabase->destroy(cTrans_, Database::Path::Category::Data, getPathPart(cTrans_), bForce_ && m_bImmediateDestroy);
		}
	}
}

// FUNCTION public
//	Schema::File::undoDestroy -- ファイルを抹消を取り消す
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
File::
undoDestroy(Trans::Transaction& cTrans_)
{
	// ファイルを操作するための論理ファイルドライバを取得する

	LogicalFile::FileDriver* driver =
		LogicalFile::FileDriverManager::getDriver(getDriverID());
	; _SYDNEY_ASSERT(driver);

	// ファイルを操作する
	{
		AutoLatch	latch(cTrans_, *this);
		LogicalFile::AutoLogicalFile(*driver, *m_pFileID).undoDestroy(cTrans_);
	}

	// ファイルが格納されているディレクトリーの破棄を取り消す

	if (Area* pArea = getArea(getAreaCategory(), cTrans_))
		pArea->undoDestroy(cTrans_, getPathPart(cTrans_));

	else {
		Database* pDatabase = getDatabase(cTrans_);
		; _SYDNEY_ASSERT(pDatabase);

		pDatabase->undoDestroy(cTrans_, Database::Path::Category::Data, getPathPart(cTrans_));
	}
}

//	FUNCTION public
//	Schema::File::mount --
//		ファイルを mount する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
File::mount(Trans::Transaction& cTrans_)
{
	// ファイルを操作するための論理ファイルドライバを取得する

	LogicalFile::FileDriver* driver =
		LogicalFile::FileDriverManager::getDriver(getDriverID());
	; _SYDNEY_ASSERT(driver);

	// UNDO/REDO処理でエリアが変更されている場合があるので
	// 設定しなおす
	setAreaPath(*m_pFileID, cTrans_);

	// データベースの ReadOnly 属性を反映する
	setReadOnly(getDatabase(cTrans_)->isReadOnly());

	// 新たなデータベース ID を FileID に設定する
	setDatabaseIDInFileID();

	// ファイルを操作する

	AutoLatch	latch(cTrans_, *this);

	SCHEMA_FAKE_ERROR("Schema::File", "Mount", "File");
	*m_pFileID = LogicalFile::AutoLogicalFile(*driver, *m_pFileID).mount(cTrans_);
}

//	FUNCTION public
//	Schema::File::unmount --
//		ファイルを unmount する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		bool bRetainFileID_ = false
//			trueのときFileIDを変えない
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
File::unmount(Trans::Transaction& cTrans_, bool bRetainFileID_ /* = false */)
{
	// ファイルを操作するための論理ファイルドライバを取得する

	LogicalFile::FileDriver* driver =
		LogicalFile::FileDriverManager::getDriver(getDriverID());
	; _SYDNEY_ASSERT(driver);

	// ファイルを操作する

	AutoLatch	latch(cTrans_, *this);
	*m_pFileID = LogicalFile::AutoLogicalFile(*driver, *m_pFileID).unmount(cTrans_);

	if (bRetainFileID_)
		// 論理ファイルドライバーが引数のFileIDを書き換える場合があるので
		// 変更された可能性のあるパラメーター値をここで元に戻す
		m_pFileID->setBoolean(FileCommon::FileOption::Mounted::Key, true);
}

//	FUNCTION public
//	Schema::File::flush --
//		ファイルを flush する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
File::flush(Trans::Transaction& cTrans_)
{
	// ファイルを操作するための論理ファイルドライバを取得する

	LogicalFile::FileDriver* driver =
		LogicalFile::FileDriverManager::getDriver(getDriverID());
	; _SYDNEY_ASSERT(driver);

	// ファイルを操作する

	AutoLatch	latch(cTrans_, *this);
	LogicalFile::AutoLogicalFile(*driver, *m_pFileID).flush(cTrans_);
}

//	FUNCTION public
//	Schema::File::sync -- 不要な版を破棄する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction&	trans
//			不要な版を破棄する処理を行う
//			トランザクションのトランザクション記述子
//		bool&				incomplete
//			true
//				今回の同期処理でファイルを持つ
//				オブジェクトの一部に処理し残しがある
//			false
//				今回の同期処理でファイルを持つ
//				オブジェクトを完全に処理してきている
//
//				同期処理の結果、ファイルを処理し残したかを設定する
//		bool&				modified
//			true
//				今回の同期処理でファイルを持つ
//				オブジェクトの一部が既に更新されている
//			false
//				今回の同期処理でファイルを持つ
//				オブジェクトはまだ更新されていない
//
//				同期処理の結果、ファイルが更新されたかを設定する
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
File::sync(Trans::Transaction& trans, bool& incomplete, bool& modified)
{
	// ファイルを操作するための論理ファイルドライバを取得する

	LogicalFile::FileDriver* driver =
		LogicalFile::FileDriverManager::getDriver(getDriverID());
	; _SYDNEY_ASSERT(driver);

	LogicalFile::AutoLogicalFile file(*driver, *m_pFileID);
	{
	// ファイルから不要なデータを削除する

	AutoLatch	latch(trans, *this);
	file.compact(trans, incomplete, modified);
	}
	{
	// ファイルの同期をとる

	AutoLatch	latch(trans, *this);
	file.sync(trans, incomplete, modified);
	}
}

//	FUNCTION public
//	Schema::File::startBackup -- バックアップを開始する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		bool bRestorable_ = true
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
File::startBackup(Trans::Transaction& cTrans_, bool bRestorable_)
{
	// ファイルを操作するための論理ファイルドライバを取得する

	LogicalFile::FileDriver* driver =
		LogicalFile::FileDriverManager::getDriver(getDriverID());
	; _SYDNEY_ASSERT(driver);

	// 論理ファイルのバックアップを開始する
	//
	//【注意】	版を使用するトランザクションでも
	//			バックアップの開始時に更新操作を行う可能性があるので、
	//			とにかくラッチする

	AutoLatch	latch(cTrans_, *this, true);
	LogicalFile::AutoLogicalFile(*driver, *m_pFileID).startBackup(cTrans_, bRestorable_);
}

//	FUNCTION public
//	Schema::File::endBackup -- バックアップを終了する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
File::endBackup(Trans::Transaction& cTrans_)
{
	// ファイルを操作するための論理ファイルドライバを取得する

	LogicalFile::FileDriver* driver =
		LogicalFile::FileDriverManager::getDriver(getDriverID());
	; _SYDNEY_ASSERT(driver);

	// ファイルを操作する

	AutoLatch	latch(cTrans_, *this);
	LogicalFile::AutoLogicalFile(*driver, *m_pFileID).endBackup(cTrans_);
}

//	FUNCTION public
//	Schema::File::recover -- 障害から回復する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		const Trans::TimeStamp& cPoint_
//			回復する時点を表すタイムスタンプ
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
File::recover(Trans::Transaction& cTrans_, const Trans::TimeStamp& cPoint_)
{
	// ファイルを操作するための論理ファイルドライバを取得する

	LogicalFile::FileDriver* driver =
		LogicalFile::FileDriverManager::getDriver(getDriverID());
	; _SYDNEY_ASSERT(driver);

	// UNDO/REDO処理でエリアが変更されている場合があるので
	// 設定しなおす
	setAreaPath(*m_pFileID, cTrans_);

	// ファイルを操作する

	AutoLatch	latch(cTrans_, *this);
	LogicalFile::AutoLogicalFile(*driver, *m_pFileID).recover(cTrans_, cPoint_);
}

//	FUNCTION public
//	Schema::File::restore --
//		ある時点に開始された版管理するトランザクションが
//		参照する版を最新版とする
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		const Trans::TimeStamp& cPoint_
//			回復する時点を表すタイムスタンプ
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
File::restore(Trans::Transaction& cTrans_, const Trans::TimeStamp& cPoint_)
{
	// ファイルを操作するための論理ファイルドライバを取得する

	LogicalFile::FileDriver* driver =
		LogicalFile::FileDriverManager::getDriver(getDriverID());
	; _SYDNEY_ASSERT(driver);

	// UNDO/REDO処理でエリアが変更されている場合があるので
	// 設定しなおす
	setAreaPath(*m_pFileID, cTrans_);

	// ファイルを操作する

	AutoLatch	latch(cTrans_, *this);
	LogicalFile::AutoLogicalFile(*driver, *m_pFileID).restore(cTrans_, cPoint_);
}

//	FUNCTION public
//	Schema::File::get --
//		あるスキーマオブジェクト ID のファイルを表すクラスを得る
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Object::ID::Value	id
//			ファイルのスキーマオブジェクト ID
//		Schema::Database* pDatabase_
//			ファイルが属するデータベースのスキーマオブジェクト
//			値が0ならすべてのデータベースについて調べる
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//
//	RETURN
//		0 以外の値
//			得られたファイルを格納する領域の先頭アドレス
//		0
//			指定されたスキーマオブジェクト ID のファイルは存在しない
//
//	EXCEPTIONS

// static
File*
File::
get(ID::Value id_, Database* pDatabase_, Trans::Transaction& cTrans_)
{
	// IDがシステム表のものと一致するならシステム表を表すオブジェクトを返す
	// [NOTES]
	// システム表のレコードファイルは表と同じIDを割り振られている
	Object::Category::Value eCategory = Table::getSystemTableCategory(id_);
	if (eCategory != Object::Category::Unknown
		&& eCategory != Object::Category::Database) {
		return getSystem(eCategory, id_, pDatabase_, cTrans_);
	}

	return ObjectTemplate::get<File, SystemTable::File, Object::Category::File>(id_, pDatabase_, cTrans_);
}

//	FUNCTION public
//	Schema::File::get --
//		あるスキーマオブジェクト ID のファイルを表すクラスを得る
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Object::ID::Value	id
//			ファイルのスキーマオブジェクト ID
//		Schema::Object::ID::Value	iDatabaseID_ = ID::Invalid
//			ファイルが属するデータベースのスキーマオブジェクトID
//			値がID::Invalidならすべてのデータベースについて調べる
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//
//	RETURN
//		0 以外の値
//			得られたファイルを格納する領域の先頭アドレス
//		0
//			指定されたスキーマオブジェクト ID のファイルは存在しない
//
//	EXCEPTIONS

// static
File*
File::get(ID::Value id_, ID::Value iDatabaseID_, Trans::Transaction& cTrans_)
{
	if (id_ == Object::ID::Invalid)
		return 0;

	return get(id_, Database::get(iDatabaseID_, cTrans_), cTrans_);
}

// FUNCTION public
//	Schema::File::getSystem -- システム表のレコードファイルを得る
//
// NOTES
//
// ARGUMENTS
//	Object::Category::Value eCategory_
//	ID::Value id
//	Database* pDatabase_
//	Trans::Transaction& cTrans_
//	
// RETURN
//	File*
//
// EXCEPTIONS

//static
File*
File::
getSystem(Object::Category::Value eCategory_,
		  ID::Value id,
		  Database* pDatabase_,
		  Trans::Transaction& cTrans_)
{
	Table* pTable = pDatabase_->getSystemTable(eCategory_, cTrans_);
	if (pTable == 0) {
		_SYDNEY_THROW0(Exception::Unexpected);
	}
	return pTable->getFile(id, cTrans_);
}

//	FUNCTION public
//	Schema::File::isValid -- 陳腐化していないか
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Object::ID::Value iID_
//			陳腐化していないかを調べるスキーマオブジェクトID
//		Schema::Object::ID::Value iDatabaseID_
//			このスキーマオブジェクトが属するデータベースのID
//		Schema::Object::Timestamp iTimestamp_
//			正しいスキーマオブジェクトの値とこの値が異なっていたら
//			陳腐化していると判断する
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//
//	RETURN
//		true
//			自分自身の表すスキーマオブジェクトは最新のものである
//		false
//			陳腐化している
//
//	EXCEPTIONS
//		なし

// static
bool
File::
isValid(ID::Value iID_, ID::Value iDatabaseID_, Timestamp iTimestamp_,
		Trans::Transaction& cTrans_)
{
	File* pFile = get(iID_, iDatabaseID_, cTrans_);

	return (pFile && pFile->getTimestamp() == iTimestamp_);
}

//	FUNCTION public
//	Schema::File::doBeforePersist -- 永続化前に行う処理
//
//	NOTES
//		一時オブジェクトでもこの処理は行う
//
//	ARGUMENTS
//		const Schema::File::Pointer& pFile_
//			永続化するオブジェクト
//		Schema::Object::Status::Value eStatus_
//			永続化する前の状態
//		bool bNeedToErase_
//			永続化が削除操作のとき、キャッシュから消去するかを示す
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

// static
void
File::
doBeforePersist(const Pointer& pFile_, Status::Value eStatus_, bool bNeedToErase_, Trans::Transaction& cTrans_)
{
	; _SYDNEY_ASSERT(pFile_.get());

	switch (eStatus_) {
	case Status::Created:
	case Status::Mounted:
	case Status::Changed:
	case Status::CreateCanceled:
	case Status::DeleteCanceled:
	{
		// 何もしない
		break;
	}
	case Status::Deleted:
	{
		// ファイルを実際に削除する
		// ★注意★
		// エラーが起きてもログの内容から再実行できるように
		// ファイルやディレクトリーを実際に「消す」操作は
		// システム表から消す操作を永続化する前に行う

		if ( bNeedToErase_ )
		{
			pFile_->destroy(cTrans_, true);
		}

		break;
	}
	case Status::DeletedInRecovery:
	{
		ModUnicodeString cPath;
		if (pFile_->m_pFileID->getString(_SYDNEY_SCHEMA_FORMAT_KEY(
											   FileCommon::FileOption::Area::Key, 0),
										 cPath)
			&& Manager::RecoveryUtility::Path::isUsedPath(cPath) == false) {
			// リカバリー中はファイルがすでに消去されているので、
			// キャッシュから除くためにunmountだけ行う
			// ただしファイルが使用されていない時に限る
			// (ALTERにより別のオブジェクトのファイルが同じパスにあるかもしれない)
			pFile_->unmount(cTrans_, true);
		}
		break;
	}
	default:
		// 何もしない
		break;
	}
}

//	FUNCTION public
//	Schema::File::doAfterPersist -- 永続化後に行う処理
//
//	NOTES
//		一時オブジェクトでもこの処理は行う
//
//	ARGUMENTS
//		const Schema::File::Pointer& pFile_
//			永続化したオブジェクト
//		Schema::Object::Status::Value eStatus_
//			永続化する前の状態
//		bool bNeedToErase_
//			永続化が削除操作のとき、キャッシュから消去するかを示す
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

// static
void
File::
doAfterPersist(const Pointer& pFile_, Status::Value eStatus_, bool bNeedToErase_,
			   Trans::Transaction& cTrans_)
{
	; _SYDNEY_ASSERT(pFile_.get());

	const ObjectID::Value dbId = pFile_->getDatabaseID();

	switch (eStatus_) {
	case Status::Created:
	case Status::Mounted:
	{
		// ファイルに属するフィールドにsourceがあるときは
		// sourceのdestination配列にフィールドを加え、
		// sourceとそれに関係するcolumnを陳腐化する

		pFile_->loadField(cTrans_).apply(ApplyFunction2<Field, Status::Value, Trans::Transaction&>(
											   &Field::touchRelated, eStatus_, cTrans_),
										 BoolFunction1<Field, Field::Category::Value>(FieldMap::findByCategory, Field::Category::Function));
										// Functionフィールドは対象から除く
		
		// マネージャーにこのファイルを表すクラスを
		// スキーマオブジェクトとして管理させる

		Database* pDatabase = pFile_->getDatabase(cTrans_);

		if (pFile_->getIndexID() != ID::Invalid) {

			// このファイルが索引を構成するファイルの場合、
			// ファイルが属する表への登録もここで行う

			Table* pTable = pFile_->getTable(cTrans_);
			; _SYDNEY_ASSERT(pTable);

			(void) pDatabase->addCache(pTable->addFile(pFile_, cTrans_));
		} else

			// 索引を構成するファイルでない場合はキャッシュに登録するのみ
			(void) pDatabase->addCache(pFile_);

		break;
	}
	case Status::DeleteCanceled:
	{
		if (!pFile_->m_bImmediateDestroy) {
			// 遅延削除の取り消しだった
			pFile_->undoDestroy(cTrans_);
		}

		// 作成フラグを立てる
		pFile_->m_bCreated = true;

		// フィールドに関係するフィールドに変更を伝達する
		pFile_->loadField(cTrans_).apply(ApplyFunction2<Field, Status::Value, Trans::Transaction&>(
											   &Field::touchRelated, eStatus_, cTrans_),
										 BoolFunction1<Field, Field::Category::Value>(FieldMap::findByCategory, Field::Category::Function));
										// Functionフィールドは対象から除く
		
		// データベースのキャッシュに追加する
		// ファイルが属する表への登録もここで行う

		Database* pDatabase = pFile_->getDatabase(cTrans_);
		Table* pTable = pFile_->getTable(cTrans_);
		; _SYDNEY_ASSERT(pTable);

		(void) pDatabase->addCache(pTable->addFile(pFile_, cTrans_));

		break;
	}
	case Status::CreateCanceled:
	{
		// フィールドに関係するフィールドに変更を伝達する
		pFile_->loadField(cTrans_).apply(ApplyFunction2<Field, Status::Value, Trans::Transaction&>(
											   &Field::touchRelated, eStatus_, cTrans_),
										 BoolFunction1<Field, Field::Category::Value>(FieldMap::findByCategory, Field::Category::Function));
										// Functionフィールドは対象から除く

		// 表の登録からの削除
		Table* pTable = pFile_->getTable(cTrans_);
		; _SYDNEY_ASSERT(pTable);
		pTable->eraseFile(pFile_->getID());
		break;
	}
	case Status::Changed:
		break;

	case Status::Deleted:
	case Status::DeletedInRecovery:
	{
		// 状態を「実際に削除された」にする

		pFile_->setStatus(Status::ReallyDeleted);

		if (bNeedToErase_) {

			// ファイルに属するフィールドにsourceがあるときは
			// sourceのdestination配列からフィールドを除き、
			// sourceとそれに関係するcolumnを陳腐化する

			pFile_->loadField(cTrans_).apply(ApplyFunction2<Field, Status::Value, Trans::Transaction&>(
													   &Field::touchRelated, eStatus_, cTrans_),
											 BoolFunction1<Field, Field::Category::Value>(FieldMap::findByCategory, Field::Category::Function));
										// Functionフィールドは対象から除く

			Database* pDatabase = pFile_->getDatabase(cTrans_);
			; _SYDNEY_ASSERT(pDatabase);

			if (pFile_->m_bImmediateDestroy)
				// 下位オブジェクトがあればそれを抹消してからdeleteする
				// 即座に破棄するのでなければエラー処理で使われるかもしれないのでresetは呼ばない
				pFile_->reset(*pDatabase);

			// キャッシュから抹消する
			// NeedToErase==falseのときは親オブジェクトのdeleteの中で
			// キャッシュから抹消される
			pDatabase->eraseCache(pFile_->getID());

			// 索引の登録から削除する
			if (Index* pIndex = pFile_->getIndex(cTrans_)) {
				pIndex->clearFile();
			}

			// 表の登録からの削除も行う → deleteされる
			Table* pTable = pFile_->getTable(cTrans_);
			; _SYDNEY_ASSERT(pTable);
			pTable->eraseFile(pFile_->getID());
		}
		break;
	}
	default:

		// 何もしない

		return;
	}

	// システム表の状態を変える
	SystemTable::unsetStatus(dbId, Object::Category::File);
}

//	FUNCTION public
//	Schema::File::doAfterLoad -- 読み込んだ後に行う処理
//
//	NOTES
//
//	ARGUMENTS
//		const Schema::FilePointer& pFile_
//			読み出したオブジェクト
//		Schema::Table& cTable_
//			オブジェクトが属する親オブジェクト
//		Trans::Transaction& cTrans_
//			この操作を行うトランザクション記述子
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

// static
void
File::
doAfterLoad(const Pointer& pFile_, Table& cTable_, Trans::Transaction& cTrans_)
{
	Database* pDatabase = cTable_.getDatabase(cTrans_);

	// 作成されていることを示すフラグを設定する
	pFile_->m_bCreated = true;

	// FileIDにReadOnly属性を設定する
	pFile_->setReadOnly(pDatabase->isReadOnly());
	// データベースIDが変わっているかもしれないので
	// FileIDのデータベースID情報も設定しなおす
	pFile_->setDatabaseIDInFileID();

	// NotAvailableでこの関数が呼ばれている場合、Drop database中なので以下の処理はしない
	if (Schema::Database::isAvailable(pFile_->getDatabaseID())) {

		// 索引IDに記録されている索引が存在するかを検査する
		if (pFile_->getIndexID() != Schema::Object::ID::Invalid) {
			Schema::Index* pIndex = pFile_->getIndex(cTrans_);
			if (!pIndex) {
				_SYDNEY_THROW0(Exception::MetaDatabaseCorrupted);
			}
			// UNDO情報にALTER TABLEやALTER INDEXが登録されている場合は
			// エリア割り当てが変更されている可能性があるので設定しなおす
			pFile_->checkUndo(*pDatabase, *pIndex, cTrans_);

		} else {
			// UNDO情報にALTER TABLEが登録されている場合は
			// エリア割り当てが変更されている可能性があるので設定しなおす
			pFile_->checkUndo(*pDatabase, cTable_, cTrans_);
		}
	}

	// FileID にエリアを設定する
	pFile_->setAreaPath(cTrans_, pDatabase);

	// データベースへ読み出したファイルを表すクラスを追加する
	// また、データベースにこのファイルを表すクラスを
	// スキーマオブジェクトとして管理させる

	pDatabase->addCache(cTable_.addFile(pFile_, cTrans_));
}

//	FUNCTION public
//	Schema::File::propagateDatabaseAttribute -- データベースの属性変化を反映する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		const Schema::Database::Attribute& cAttribute_
//			変更後の属性
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
File::
propagateDatabaseAttribute(Trans::Transaction& cTrans_,
						   const Database::Attribute& cAttribute_)
{
	// 論理ファイルが内部で保持している構造を破棄してもらうために
	// unmountを呼ぶ
	unmount(cTrans_, true /* retain fileID */);
	// 表に属するファイルのFileIDに記載されているReadOnlyを設定しなおす
	setReadOnly(cAttribute_.m_bReadOnly);
}

//	FUNCTION public
//	Schema::File::reflectReadOnly
//		-- 読み込み属性の反映
//
//	NOTES
//		Database の読み込み属性を設定し、FileID に反映する
//
//	ARGUMENTS
//		bool bReadOnly_
//			true  : ファイルは ReadOnly
//			false : ファイルは ReadWrite
//
//	RETURN
//		bool	設定前の値
//
//	EXCEPTIONS
void
File::
setReadOnly(bool bReadOnly_)
{
	// 自分の属性を取得(設定されていないなら ReadWrite )
	bool bThisReadOnly = false;
	m_pFileID->getBoolean(FileCommon::FileOption::ReadOnly::Key, bThisReadOnly);

	if ( bThisReadOnly != bReadOnly_ )
	{
		// 属性が異なる場合に設定する
		m_pFileID->setBoolean(FileCommon::FileOption::ReadOnly::Key, bReadOnly_);

	}
}

//	FUNCTION public
//	Schema::File::reset --
//		下位オブジェクトを抹消する
//
//	NOTES
//
//	ARGUMENTS
//		Database& cDatabase_
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
File::reset(Database& cDatabase_)
{
	AutoRWLock l(getRWLock(), Os::RWLock::Mode::Write);

	if (_fields)
		resetField(cDatabase_);
}

//	FUNCTION public
//	Schema::File::getCategory -- ファイルの種別を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたファイルの種別
//
//	EXCEPTIONS
//		なし

File::Category::Value
File::
getCategory() const
{
	return _category;
}

//	FUNCTION public
//	Schema::File::getFileID -- ファイルのファイル ID を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたファイル ID
//
//	EXCEPTIONS
//		なし

const LogicalFile::FileID&
File::
getFileID() const
{
	return *m_pFileID;
}

//	FUNCTION public
//	Schema::File::getDriverID --
//		ファイルを扱うためのファイルドライバーのファイルドライバー ID を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたファイルドライバー ID
//
//	EXCEPTIONS
//		なし

LogicalFile::FileDriverID::Value
File::
getDriverID() const
{
	return getDriverID(_category);
}

//	FUNCTION public
//	Schema::File::getTableID --
//		ファイルを使用する表のスキーマオブジェクト ID を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		Schema::Object::ID::Invalid 以外
//			ファイルを使用する表のスキーマオブジェクト ID
//		Schema::Object::ID::Invalid
//			このファイルを使用する表は存在しない
//
//	EXCEPTIONS
//		なし

Object::ID::Value
File::
getTableID() const
{
	return getParentID();
}

//	FUNCTION public
//	Schema::File::getIndexID --
//		ファイルを使用する索引のスキーマオブジェクト ID を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		Schema::Object::ID::Invalid 以外
//			ファイルを使用する索引のスキーマオブジェクト ID
//		Schema::Object::ID::Invalid
//			このファイルを使用する索引は存在しない
//
//	EXCEPTIONS
//		なし

Object::ID::Value
File::
getIndexID() const
{
	return _indexID;
}

//	FUNCTION public
//	Schema::File::setIndexID --
//		ファイルを使用する索引のスキーマオブジェクト ID を設定する
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Object::ID::Value
//			設定するID
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

inline // 内部でしか使用しない
void
File::
setIndexID(ID::Value id_)
{
	_indexID = id_;
}

//	FUNCTION public
//	Schema::File::getAreaID --
//		ファイルを格納するエリアのスキーマオブジェクト ID を得る
//
//	NOTES
//
//	ARGUMENTS
//		Schema::AreaCategory::Value eCategorey_ = Schema::AreaCategory::Default
//			得るエリアの種別(ファイル、物理ログファイルのいずれか)
//
//	RETURN
//		Schema::Object::ID::Invalid 以外
//			ファイルを格納するエリアのスキーマオブジェクト ID
//		Schema::Object::ID::Invalid
//			このファイルを格納するエリアは存在しない
//
//	EXCEPTIONS
//		なし

Object::ID::Value
File::
getAreaID(AreaCategory::Value eCategory_) const
{
	return (eCategory_ == AreaCategory::PhysicalLog) ? _logAreaID : _areaID;
}

//	FUNCTION public
//	Schema::File::getHint -- ファイルに指定されるヒントを得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		ヒントを表すオブジェクトへのポインタ
//
//	EXCEPTIONS
//		なし

const Hint*
File::
getHint() const
{
	return m_pHint;
}

//	FUNCTION public
//	Schema::File::getAreaHint -- ファイルのエリアに指定されるヒントを得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		エリアのヒントを表す文字列へのポインタ
//
//	EXCEPTIONS
//		なし

const Hint*
File::
getAreaHint() const
{
	return m_pAreaHint;
}

//	FUNCTION public
//	Schema::File::moveArea --
//		ファイルを格納するエリアを変更する
//
//	NOTES
//		エリアの割り当て変更による移動
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		Schema::Object::ID::Value iPrevAreaID_
//		Schema::Object::ID::Value iPostAreaID_
//			移動前後のエリアID配列
//		bool bUndo_ = false
//			trueの場合エラー処理中なので重ねてのエラー処理はしない
//		bool bRecovery_ = false
//			trueなら変更前のパスにファイルがなくても変更後にあればOKとする
//		bool bMount_ = false
//			trueならMOUNTでのDROP AREAに対応した処理なのでファイルの移動はしない
//
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
File::
moveArea(Trans::Transaction& cTrans_,
         ID::Value iPrevAreaID_, ID::Value iPostAreaID_,
		 bool bUndo_, bool bRecovery_, bool bMount_)
{
	// 変更がある場合のみ処理を行う
	if ((bRecovery_ && iPostAreaID_ != iPrevAreaID_)
		|| iPostAreaID_ != getAreaID()) {

		Database* pDatabase = getDatabase(cTrans_);
		Area* pPrevArea = pDatabase->getArea(iPrevAreaID_, cTrans_);
		Area* pPostArea = pDatabase->getArea(iPostAreaID_, cTrans_);
		AreaContent* pContent = 0;

		// 索引のファイルなら索引のパスの記録を破棄しておく
		if (Index* pIndex = getIndex(cTrans_)) {
			pIndex->clearPath();
		}

		enum {
			None,
			FileMoved,
			ContentMoved,
			AreaSet,
			Touched,
			ValueNum
		} eStatus = None;

		ModVector<ModUnicodeString> vecPrevAreaPath;
		ModVector<ModUnicodeString> vecPostAreaPath;

		try {

			// ファイルを移動する
			(void) getAreaPath(cTrans_, pDatabase, iPrevAreaID_, vecPrevAreaPath);
			(void) getAreaPath(cTrans_, pDatabase, iPostAreaID_, vecPostAreaPath);
			moveArea(cTrans_, vecPrevAreaPath, vecPostAreaPath,
					 bUndo_, bRecovery_, bMount_);
			eStatus = FileMoved;
			SCHEMA_FAKE_ERROR("Schema::File", "MoveArea", "FileMoved");

			// 格納関係を変更する
			pContent = AreaContent::moveArea(cTrans_, pPrevArea, pPostArea,
											 this, getAreaCategory(),
											 bUndo_, bRecovery_, bMount_);
			eStatus = ContentMoved;
			SCHEMA_FAKE_ERROR("Schema::File", "MoveArea", "ContentMoved");

			// エリアIDを変える
			setAreaID(iPostAreaID_, getAreaCategory());
			eStatus = AreaSet;
			SCHEMA_FAKE_ERROR("Schema::File", "MoveArea", "AreaSet");

			// 変更を記録する
			if (bUndo_ && getStatus() != Status::Persistent) {
				// UNDO中で永続化状態でなければ変更の記録を取り消す
				untouch();
			} else {
				touch();
			}
			eStatus = Touched;
			SCHEMA_FAKE_ERROR("Schema::File", "MoveArea", "Touched");

		} catch (...) {

			_BEGIN_REORGANIZE_RECOVERY;

			switch (eStatus) {
			case Touched:
				{
					// 状態の変更を取り消す
					untouch();
					// thru
				}
			case AreaSet:
				{
					// エリアIDを戻す
					setAreaID(iPrevAreaID_, getAreaCategory());
					// thru
				}
			case ContentMoved:
				{
					// 格納関係の変更を取り消す
					AreaContent::undoMoveArea(cTrans_, pPrevArea, pPostArea,
											  this, getAreaCategory(),
											  pContent);
					// thru
				}
			case FileMoved: 
				{
					// ファイルを元の位置に戻す
					moveArea(cTrans_, vecPostAreaPath, vecPrevAreaPath,
							 true /* undo */, bRecovery_, bMount_);
					// thru
				}
			case None:
			default:
				break;
			}

			_END_REORGANIZE_RECOVERY;

			_SYDNEY_RETHROW;
		}
	}
}

//	FUNCTION public
//	Schema::File::moveArea --
//		ファイルを格納するエリアを移動する
//
//	NOTES
//		エリアの定義変更による移動
//		およびエリアの割り当て変更による移動の下請け
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		const ModVector<ModUncodeString>& vecPrevPath_ 
//			移動前のパス配列
//		const ModVector<ModUncodeString>& vecPostPath_ 
//			移動前のパス配列
//		bool bUndo_ = false
//			trueの場合エラー処理中なので重ねてのエラー処理はしない
//		bool bRecovery_ = false
//			trueなら変更前のパスにファイルがなくても変更後にあればOKとする
//		bool bMount_ = false
//			trueならファイルの移動を行わない
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
File::
moveArea(Trans::Transaction& cTrans_,
         const ModVector<ModUnicodeString>& vecPrevPath_,
         const ModVector<ModUnicodeString>& vecPostPath_,
		 bool bUndo_, bool bRecovery_, bool bMount_)
{
	// 始める前に中断のチェックをする
	Manager::checkCanceled(cTrans_);

	// このメソッドは以下の2つのケースで呼ばれることがある
	// ・表や索引のエリア割り当てが変更されたのに伴い、ファイルのエリアを変更
	// ・エリアの定義が変更されたのに伴い、ファイルの格納位置を変更
	// 後者ではファイルと関係するエリアオブジェクト自体は変更されない

	if (bMount_ || bRecovery_) {
		// MOUNT時またはREDOの処理なら格納場所をセットするのみ
		setAreaPath(*m_pFileID, vecPostPath_, getPathPart(cTrans_));
		return;
	}

	// ファイルを操作するための論理ファイルドライバを取得する

	LogicalFile::FileDriver* driver =
		LogicalFile::FileDriverManager::getDriver(getDriverID());
	; _SYDNEY_ASSERT(driver);

	// ファイル操作の排他制御のためにラッチする

	AutoLatch	latch(cTrans_, *this);

	// ファイル ID の表す論理ファイルを得る
	LogicalFile::AutoLogicalFile pLogicalFile(*driver, *m_pFileID);
	
	// エラー処理でどこまでやったかを得るための列挙型
	enum {
		None,							// 初期値
		Moving,							// ファイル移動開始
		Moved,							// ファイル移動
		FileIDSet,						// FileID の内容変更
		Removed,						// 移動前のディレクトリ破棄済み
		ValueNum
	} eStatus = None;

	// 移動できるところまでする
	int n = ModMin(vecPrevPath_.getSize(), vecPostPath_.getSize());

	ModVector<ModUnicodeString> vecPrevDir(n);
	ModVector<ModUnicodeString> vecPostDir(n);

	try {
		// 移動前後のディレクトリーを作りながらmoveに渡すデータを作る
		int i = 0;
		for (; i < n; i++) {
			if (Os::Path::compare(vecPrevPath_[i], vecPostPath_[i])
				!= Os::Path::CompareResult::Identical) {
				Os::Path cPrevDir(vecPrevPath_[i]);
				Os::Path cPostDir(vecPostPath_[i]);

				; _SYDNEY_ASSERT(cPrevDir.getLength());
				; _SYDNEY_ASSERT(cPostDir.getLength());

				cPrevDir.addPart(getPathPart(cTrans_));
				cPostDir.addPart(getPathPart(cTrans_));

				vecPrevDir[i] = cPrevDir;
				vecPostDir[i] = cPostDir;

			} else {
				ModUnicodeString cstrPath;
				bool bResult =
					m_pFileID->getString(_SYDNEY_SCHEMA_FORMAT_KEY(FileCommon::FileOption::Area::Key, i),
										 cstrPath);
				; _SYDNEY_ASSERT(bResult);
				vecPrevDir[i].append(cstrPath);
				vecPostDir[i].append(cstrPath);
			}
		}
		SCHEMA_FAKE_ERROR("Schema::File", "MoveAreaPath", "Created");

		// 移動する
		eStatus = Moving;
		pLogicalFile.move(cTrans_, Common::StringArrayData(vecPostDir));
		eStatus = Moved;
		SCHEMA_FAKE_ERROR("Schema::File", "MoveAreaPath", "Moved");

		// FileIDの内容を変更する
		setAreaPath(*m_pFileID, vecPostPath_, getPathPart(cTrans_));
		eStatus = FileIDSet;
		SCHEMA_FAKE_ERROR("Schema::File", "MoveAreaPath", "FileIDSet");

		// FileID が変更されたので変更通知する
		// -> AreaPathの部分は上書きされるので変更通知の必要はない
		//touch();
		SCHEMA_FAKE_ERROR("Schema::File", "MoveAreaPath", "Touched");

		// 移動前のディレクトリーを破棄する
		_sweep(vecPrevDir, vecPostDir);
		eStatus = Removed;
		SCHEMA_FAKE_ERROR("Schema::File", "MoveAreaPath", "Removed");

	} catch (...) {

		_BEGIN_REORGANIZE_RECOVERY;

		switch (eStatus) {
		case Removed:
		case FileIDSet:
			{
				// ファイルIDを元に戻す
				setAreaPath(*m_pFileID, vecPrevPath_, getPathPart(cTrans_));
				// thru
			}
		case Moved:
			{
				// 移動したファイルを元の場所に戻す
				pLogicalFile.move(cTrans_, Common::StringArrayData(vecPrevDir));
				// thru
			}
		case Moving:
			{
				// 移動後のディレクトリーを破棄する
				_sweep(vecPostDir, vecPrevDir);
			}
		case None:
		default:
			break;
		}

		_END_REORGANIZE_RECOVERY;

		_SYDNEY_RETHROW;
	}
}

//	FUNCTION public
//		Schema::File::movePath -- 親ディレクトリーの変更によりファイルを移動する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction&			cTrans_
//			操作をするトランザクション記述子
//		const Os::Path& cDir_
//			移動後の親ディレクトリー(表名まで含む)
//		bool bUndo_ = false
//			trueの場合エラー処理中なので重ねてのエラー処理はしない
//		bool bRecovery_ = false
//			trueなら変更前のパスにファイルがなくても変更後にあればOKとする
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		???

void
File::
movePath(Trans::Transaction& cTrans_, const Os::Path& cDir_,
		 bool bUndo_, bool bRecovery_)
{
	// 始める前に中断のチェックをする
	Manager::checkCanceled(cTrans_);

	// ファイルにエリア指定がないときのみ処理すればよい
	if (getAreaID() == ID::Invalid) {

		// エラー処理でどこまでやったかを得るための列挙型
		enum {
			None,						// 初期値
			Moved,						// ファイル移動
			FileIDSet,					// FileIDの内容変更
			Removed,					// 移動前のディレクトリー破棄
			ValueNum
		} eStatus = None;

		ModUnicodeString cPrevPath;		// status==Moved以降ならセットされる

		Common::StringArrayData cData;

		Os::Path cNewPath(cDir_);
		cNewPath.addPart(getName());

		// REDO時の処理ならFileIDにセットするのみ
		if (bRecovery_) {
			m_pFileID->setString(_SYDNEY_SCHEMA_FORMAT_KEY(
										FileCommon::FileOption::Area::Key, 0),
								 cNewPath);
			return;
		}

		// 索引のファイルなら索引のパスの記録を破棄しておく
		if (Index* pIndex = getIndex(cTrans_)) {
			pIndex->clearPath();
		}

		// ファイルを操作するための論理ファイルドライバを取得する

		LogicalFile::FileDriver* driver =
			LogicalFile::FileDriverManager::getDriver(getDriverID());
		; _SYDNEY_ASSERT(driver);

		// ファイル操作の排他制御のためにラッチする

		AutoLatch	latch(cTrans_, *this);

		// ファイル ID の表す論理ファイルを得る
		LogicalFile::AutoLogicalFile pLogicalFile(*driver, *m_pFileID);

		// エラー時に自動的にrmDirするためのオブジェクト
		Utility::File::AutoRmDir cAutoRmDir;
		// 移動後のディレクトリーを設定する
		cAutoRmDir.setDir(cNewPath);

		try {

			SCHEMA_FAKE_ERROR("Schema::File", "MovePath", "Created");

			// エラー処理のためにここで移動前のパス名を得ておく
			bool bResult =
				m_pFileID->getString(_SYDNEY_SCHEMA_FORMAT_KEY(
										FileCommon::FileOption::Area::Key, 0),
									 cPrevPath);
			; _SYDNEY_ASSERT(bResult);

			// 移動する
			cData.setElement(0, cNewPath);
			pLogicalFile.move(cTrans_, cData);
			// 移動してしまったら自動でrmdirできない
			cAutoRmDir.disable();
			eStatus = Moved;
			SCHEMA_FAKE_ERROR("Schema::File", "MovePath", "Moved");

			// 移動前後のパスが異なる場合
			// 移動前のディレクトリーを削除する
			// ★注意★
			// 中身に何があっても強引にすべて削除する
			// 移動後なのでファイルは存在しないはずである
			// したがって即座に破棄できる
			
			if (cNewPath.compare(cPrevPath) != Os::Path::CompareResult::Identical) {
				Utility::File::rmAll(cPrevPath);
				eStatus = Removed;
			}
			SCHEMA_FAKE_ERROR("Schema::File", "MovePath", "Removed");

			// ファイルIDのエントリーを新しい場所にセットする
			m_pFileID->setString(_SYDNEY_SCHEMA_FORMAT_KEY(
										FileCommon::FileOption::Area::Key, 0),
								 cNewPath);
			eStatus = FileIDSet;
			SCHEMA_FAKE_ERROR("Schema::File", "MovePath", "FileIDSet");

			// FileID が変更されたので変更通知する
			// -> AreaPathの部分は上書きされるので変更通知の必要はない
			//touch();

		} catch (...) {

			_BEGIN_REORGANIZE_RECOVERY;

			Common::StringArrayData cData;

			switch (eStatus) {
			case FileIDSet:
				{
					// FileIDを元に戻す
					m_pFileID->setString(_SYDNEY_SCHEMA_FORMAT_KEY(
											   FileCommon::FileOption::Area::Key, 0),
										 cPrevPath);
					// thru
				}
			case Removed:
			case Moved:
				{
					// 移動したファイルをもう一度移動する
					cData.setElement(0, cPrevPath);
					pLogicalFile.move(cTrans_, cData);
					// 移動したら再び自動的に作成したディレクトリーを破棄できる
					cAutoRmDir.enable();
					// thru
				}
			case None:
			default:
				break;
			}

			_END_REORGANIZE_RECOVERY;

			_SYDNEY_RETHROW;
		}
	}
}

// FUNCTION public
//	Schema::File::rename -- 名前を変更する
//
// NOTES
//
// ARGUMENTS
//	const Name& cName_
//	
// RETURN
//	なし
//
// EXCEPTIONS

void
File::
rename(const Name& cName_)
{
	setName(cName_);
	delete m_pPathPart, m_pPathPart = 0;
}

// FUNCTION public
//	Schema::File::moveRename -- 親オブジェクトの名称変更によりファイルを移動する
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	const Name& cPrevTableName_
//	const Name& cPostTableName_
//	const Name& cPrevName_
//	const Name& cPostName_
//	bool bUndo_ /* = false */
//	bool bRecovery_ /* = false */
//	
// RETURN
//	なし
//
// EXCEPTIONS

void
File::
moveRename(Trans::Transaction& cTrans_,
		   const Name& cPrevTableName_,
		   const Name& cPostTableName_,
		   const Name& cPrevName_,
		   const Name& cPostName_,
		   bool bUndo_ /* = false */,
		   bool bRecovery_ /* = false */)
{
	// 始める前に中断のチェックをする
	Manager::checkCanceled(cTrans_);

	Database* pDatabase = getDatabase(cTrans_);
	Area* pArea = getArea(getAreaCategory(), cTrans_);

	// 索引のファイルなら索引のパスの記録を破棄しておく
	if (Index* pIndex = getIndex(cTrans_)) {
		pIndex->clearPath();
	}

	// ファイルを格納するパスのトップディレクトリーを得る
	ModVector<ModUnicodeString> vecPath;
	(void) getAreaPath(cTrans_, pDatabase, getAreaID(), vecPath);

	// 移動前後のPathPartを作る
	Os::Path cPrevPathPart(cPrevTableName_);
	cPrevPathPart.addPart(cPrevName_);
	Os::Path cPostPathPart(cPostTableName_);
	cPostPathPart.addPart(cPostName_);

	// REDO時の処理なら名前を変更してFileIDにセットするのみ
	if (bRecovery_) {
		rename(cPostName_);
		setAreaPath(*m_pFileID, vecPath, cPostPathPart);
		touch();
		return;
	}

#ifdef DEBUG
	SydSchemaDebugMessage << "File rename " << cPrevTableName_ << "/" << cPrevName_
						  << " -> " << cPostTableName_ << "/" << cPostName_ << ModEndl;
#endif

	// ファイルを操作するための論理ファイルドライバを取得する
	LogicalFile::FileDriver* driver =
		LogicalFile::FileDriverManager::getDriver(getDriverID());
	; _SYDNEY_ASSERT(driver);

	// ファイル操作の排他制御のためにラッチする
	AutoLatch	latch(cTrans_, *this);

	// ファイル ID の表す論理ファイルを得る
	LogicalFile::AutoLogicalFile pLogicalFile(*driver, *m_pFileID);

	// エラー処理でどこまでやったかを得るための列挙型
	enum {
		None,						// 初期値
		Moving,						// ファイル移動開始
		Moved,						// ファイル移動
		FileIDSet,					// FileIDの内容変更
		Renamed,					// 名前変更
		Removed,					// 移動前のディレクトリー破棄
		ValueNum
	} eStatus = None;

	// 移動できるところまでする
	int n = vecPath.getSize();

	ModVector<ModUnicodeString> vecPrevDir(n);
	ModVector<ModUnicodeString> vecPostDir(n);

	try {
		// 移動前後のディレクトリーを作りながらmoveに渡すデータを作る
		int i = 0;
		for (; i < n; i++) {
			Os::Path cPrevDir(vecPath[i]);
			Os::Path cPostDir(vecPath[i]);

			; _SYDNEY_ASSERT(cPrevDir.getLength());
			; _SYDNEY_ASSERT(cPostDir.getLength());

			cPrevDir.addPart(cPrevPathPart);
			cPostDir.addPart(cPostPathPart);

			vecPrevDir[i] = cPrevDir;
			vecPostDir[i] = cPostDir;
		}
		SCHEMA_FAKE_ERROR("Schema::File", "MoveRename", "Created");

		// 移動する
		eStatus = Moving;
		pLogicalFile.move(cTrans_, Common::StringArrayData(vecPostDir));
		eStatus = Moved;
		SCHEMA_FAKE_ERROR("Schema::File", "MoveRename", "Moved");

		// FileIDの内容を変更する
		setAreaPath(*m_pFileID, vecPath, cPostPathPart);
		eStatus = FileIDSet;
		SCHEMA_FAKE_ERROR("Schema::File", "MoveRename", "FileIDSet");

		// 名前を変更し、変更通知する
		rename(cPostName_);
		touch();
		eStatus = Renamed;
		SCHEMA_FAKE_ERROR("Schema::File", "MoveRename", "Touched");

		// 移動前のディレクトリーを破棄する
		_sweep(vecPrevDir, vecPostDir);
		eStatus = Removed;
		SCHEMA_FAKE_ERROR("Schema::File", "MoveRename", "Removed");

	} catch (...) {

		_BEGIN_REORGANIZE_RECOVERY;

		switch (eStatus) {
		case Removed:
		case Renamed:
			{
				rename(cPrevName_);
				// touchした後永続化していないのでtouch状態を戻す
				untouch();
				// thru
			}
		case FileIDSet:
			{
				// ファイルIDを元に戻す
				setAreaPath(*m_pFileID, vecPath, cPrevPathPart);
				// thru
			}
		case Moved:
			{
				// 移動したファイルを元の場所に戻す
				pLogicalFile.move(cTrans_, Common::StringArrayData(vecPrevDir));
				// thru
			}
		case Moving:
			{
				// 移動後のディレクトリーを破棄する
				_sweep(vecPostDir, vecPrevDir);
			}
		case None:
		default:
			break;
		}

		_END_REORGANIZE_RECOVERY;

		_SYDNEY_RETHROW;
	}
}

// modify fileID according to alter column
void
File::
alterField(Trans::Transaction& cTrans_,
		   Field* pField_)
{
	; _SYDNEY_ASSERT(m_pFileID);
	setFieldTypeToFileID(*m_pFileID, pField_, pField_->getPosition(), cTrans_);
}

//	FUNCTION public
//	Schema::File::getAreaPath -- ファイルを格納するパスを得る
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			トランザクション記述子
//		Schema::Database* pDatabase_
//			ファイルが属するデータベース
//		Schema::Object::ID::Value iAreaID_
//			ファイルに指定されているエリアID
//			指定がない場合はID::Invalid
//		ModVector<ModUnicodeString>& vecPath_
//			返り値を入れる
//
//	RETURN
//		パス指定の配列の要素数
//
//	EXCEPTIONS

//static
ModSize
File::
getAreaPath(Trans::Transaction& cTrans_, Database* pDatabase_, ID::Value iAreaID_, ModVector<ModUnicodeString>& vecPath_)
{
	if (iAreaID_ != ID::Invalid) {
		// REDOのときはUNDOで登録された最終的なパスを使用する
		if (Manager::RecoveryUtility::Path::getUndoAreaPath(pDatabase_->getName(), iAreaID_, vecPath_)) {
			return vecPath_.getSize();
		}
		// 登録されていない場合はエリアオブジェクトを取得してそのパス指定を使う
		// このエリアがDROP対象である場合はエリアが得られない場合がある
		Area* pArea = pDatabase_->getArea(iAreaID_, cTrans_);
		if (pArea) {
			vecPath_ = pArea->getPath();
			return vecPath_.getSize();
		}
		// エリアが見つからない場合はDROP AREAがUNDOされているときでなければならない
		if (!Manager::RecoveryUtility::Undo::isEntered(pDatabase_->getName(), iAreaID_,
													   Manager::RecoveryUtility::Undo::Type::DropArea)) {
			_SYDNEY_THROW2(Exception::AreaNotFound,
						   ModUnicodeString(ModCharString().format("ID=%d", iAreaID_)),
						   pDatabase_->getName());
		}
	}
	// エリア指定がないときはデータベースのデータ格納パスを使用する
	vecPath_.erase(vecPath_.begin(), vecPath_.end());
	vecPath_.pushBack(pDatabase_->getDataPath());
	return 1;
}

//	FUNCTION public
//	Schema::File::getPathPart -- 格納場所のファイル固有部分を得る
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//
//	RETURN
//		パス名のファイル固有部分
//
//	EXCEPTIONS

Os::Path
File::getPathPart(Trans::Transaction& cTrans_) const
{
	if (!m_pPathPart) {
		AutoRWLock l(getRWLock(), Os::RWLock::Mode::Write);
		// 書き込みロックの中で再度調べる
		if (!m_pPathPart) {
			Table* pTable = getTable(cTrans_);
			; _SYDNEY_ASSERT(pTable);
			ModAutoPointer<Os::Path> pPath = new Os::Path(pTable->getPathPart(cTrans_));
			pPath->addPart(getName());
			m_pPathPart = pPath.release();
		}
		; _SYDNEY_ASSERT(m_pPathPart);
	}

	return *m_pPathPart;
}

//	FUNCTION protected
//	Schema::File::setAreaPath -- FileIDのエリアに関する部分を設定する
//
//	NOTES
//		ファイル共通の部分なのでここで定義した
//
//	ARGUMENTS
//		LogicalFile::FileID&	fileID
//			設定するファイル ID
//		Trans::Transaction& cTrans_
//			操作をするトランザクション記述子
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
File::setAreaPath(LogicalFile::FileID& cFileID_, Trans::Transaction& cTrans_)
{
	// ファイルを格納するエリア指定を求める
	// エリア指定がなければデータベースの定義時に指定された
	// データ格納ディレクトリーが使用される

	Database* pDatabase = getDatabase(cTrans_);
	ModVector<ModUnicodeString> vecPath;
	(void) getAreaPath(cTrans_, pDatabase, getAreaID(), vecPath);

	setAreaPath(cFileID_, vecPath, getPathPart(cTrans_));
}

//	FUNCTION protected
//	Schema::File::setAreaPath -- FileIDのエリアに関する部分を設定する
//
//	NOTES
//
//	ARGUMENTS
//		LogicalFile::FileID&	fileID
//			設定するファイル ID
//		const ModVector<ModUnicodeString>& vecPath_
//			設定するパス名配列
//		const Schema::Os::Path& cPathPart_
//			パス名に追加するファイル固有部分
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
File::
setAreaPath(LogicalFile::FileID& cFileID_,
			const ModVector<ModUnicodeString>& vecPath_,
			const Os::Path& cPathPart_)
{
	ModSize n = vecPath_.getSize();
	for (ModSize i = 0; i < n; i++) {
		if (vecPath_[i].getLength()) {
			cFileID_.setString(_SYDNEY_SCHEMA_FORMAT_KEY(FileCommon::FileOption::Area::Key, i),
							   Os::Path(vecPath_[i]).addPart(cPathPart_));
		}
	}
}

//	FUNCTION protected
//	Schema::File::setAreaPath -- FileIDのエリアに関する部分を設定する
//
//	NOTES
//
//	ARGUMENTS
//		LogicalFile::FileID&	fileID
//			設定するファイル ID
//		const ModVector<ModUnicodeString>& vecPath_
//			設定するパス名配列
//		const Schema::Os::Path& cPathPart_
//			パス名に追加するファイル固有部分
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
File::
setAreaPath(Trans::Transaction& cTrans_, Database* pDatabase_)
{
	ModVector<ModUnicodeString> vecPath;
	(void) getAreaPath(cTrans_, pDatabase_, getAreaID(), vecPath);

	setAreaPath(*m_pFileID, vecPath, getPathPart(cTrans_));
}

//	FUNCTION protected
//	Schema::File::setDatabaseIDInFileID -- FileIDのデータベースID部分を設定する
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

void
File::
setDatabaseIDInFileID()
{
	m_pFileID->setInteger(_SYDNEY_SCHEMA_PARAMETER_KEY(FileCommon::FileOption::DatabaseID::Key),
						  getDatabaseID());
}

//	FUNCTION protected
//	Schema::File::setFileID -- 与えられたファイル ID を設定する
//
//	NOTES
//
//	ARGUMENTS
//		const LogicalFile::FileID&	fileID
//			設定するファイル ID
//
//	RETURN
//		設定したファイル ID
//
//	EXCEPTIONS

const LogicalFile::FileID&
File::
setFileID(const LogicalFile::FileID& fileID)
{
	return *m_pFileID = fileID;
}

//	FUNCTION public
//	Schema::File::getDriverID --
//		ある種別のファイルを扱うためのファイルドライバーの
//		ファイルドライバー ID を得る
//
//	NOTES
//
//	ARGUMENTS
//		Schema::File::Category	category
//			ファイルの種別
//
//	RETURN
//		得られたファイルドライバー ID
//
//	EXCEPTIONS
//		なし

// static
LogicalFile::FileDriverID::Value
File::
getDriverID(File::Category::Value category)
{
	// ファイルの種別から、そのファイルのドライバー ID を得るための配列

	const LogicalFile::FileDriverID::Value table[] =
	{
		LogicalFile::FileDriverID::Unknown,		// Category::Unknown
		LogicalFile::FileDriverID::Record,		// Category::Record
		LogicalFile::FileDriverID::Btree,		// Category::Btree
		LogicalFile::FileDriverID::FullText,	// Category::FullText
		LogicalFile::FileDriverID::Vector,		// Category::Vector
		LogicalFile::FileDriverID::Lob,			// Category::Lob
		LogicalFile::FileDriverID::Bitmap,		// Category::Bitmap
		LogicalFile::FileDriverID::Array,		// Category::Array
		LogicalFile::FileDriverID::KdTree,		// Category::KdTree
	};

	return
		(category < sizeof(table) / sizeof(LogicalFile::FileDriverID::Value))
		? table[category] : LogicalFile::FileDriverID::Unknown;
}

//	FUNCTION public
//	Schema::File::getTable -- ファイルを持つ表を表すクラスを得る
//
//	NOTES
//		生成前、中のファイルや、排他制御がうまく行われていない場合を除けば、
//		ファイルを持つ表または索引が必ずひとつ存在するはずである
//
//	ARGUMENTS
//		Trans::Transaction&			cTrans_
//			操作をするトランザクション記述子
//
//	RETURN
//		0 以外の値
//			得られた表を格納する領域の先頭アドレス
//		0
//			ファイルを持つ表が存在しない
//
//	EXCEPTIONS

Table*
File::getTable(Trans::Transaction& cTrans_) const
{
	if (!_table) {
		AutoRWLock l(getRWLock(), Os::RWLock::Mode::Write);
		// 書き込みロックの中でもう一度調べる

		return (!_table) ?
			_table = Table::get(getParentID(), getDatabase(cTrans_), cTrans_, true /* internal */)
			: _table;
	}
	return _table;
}

//	FUNCTION public
//	Schema::File::getIndex -- ファイルを持つ索引を表すクラスを得る
//
//	NOTES
//		生成前、中のファイルや、排他制御がうまく行われていない場合を除けば、
//		ファイルを持つ表または索引が必ずひとつ存在するはずである
//
//	ARGUMENTS
//		Trans::Transaction&			cTrans_
//			操作をするトランザクション記述子
//
//	RETURN
//		0 以外の値
//			得られた索引を格納する領域の先頭アドレス
//		0
//			ファイルを持つ索引が存在しない
//
//	EXCEPTIONS

Index*
File::getIndex(Trans::Transaction& cTrans_) const
{
	if (!_index) {
		AutoRWLock l(getRWLock(), Os::RWLock::Mode::Write);
		// 書き込みロックの中でもう一度調べる

		return (!_index) ?
			_index = getTable(cTrans_)->getIndex(getIndexID(), cTrans_)
			: _index;
	}
	return _index;
}

//	FUNCTION public
//	Schema::File::getArea -- ファイルを格納するエリアを表すクラスを得る
//
//	NOTES
//
//	ARGUMENTS
//		Schema::AreaCategory::Value eCategory_
//			得るエリアの種別(ファイル、物理ログファイルのいずれか)
//		Trans::Transaction&			cTrans_
//			操作をするトランザクション記述子
//
//	RETURN
//		0 以外の値
//			得られたエリアを格納する領域の先頭アドレス
//		0
//			ファイルを格納するエリアが存在しない
//
//	EXCEPTIONS

Area*
File::
getArea(AreaCategory::Value eCategory_, Trans::Transaction& cTrans_) const
{
	Area** pRefArea =
		(eCategory_ == AreaCategory::PhysicalLog) ? &_logArea : &_area;
	if (!*pRefArea) {
		AutoRWLock l(getRWLock(), Os::RWLock::Mode::Write);
		// 書き込みロックの中でもう一度調べる

		return (!*pRefArea) ?
			*pRefArea = Area::get(getAreaID(eCategory_), getDatabase(cTrans_), cTrans_)
			: *pRefArea;
	}
	return *pRefArea;
}

//	FUNCTION public
//	Schema::File::setAreaID --
//		ファイルを格納するエリアのスキーマオブジェクト ID を設定する
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Object::ID::Value iID_
//			設定するエリアのオブジェクトID
//		Schema::AreaCategory::Value eCategory_
//			設定するエリアの種別(ファイル、物理ログファイルのいずれか)
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

void
File::
setAreaID(ID::Value iID_, AreaCategory::Value eCategory_)
{
	AutoRWLock l(getRWLock(), Os::RWLock::Mode::Write);

	if (eCategory_ == AreaCategory::PhysicalLog) {
		_logAreaID = iID_;
		_logArea = 0;
	} else {
		_areaID = iID_;
		_area = 0;
	}
}

//	FUNCTION public
//	Schema::File::isKeyUnique --
//		ファイルに格納されるデータがキーについて一意かどうか
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		キーについて一意ならtrueを返す
//
//	EXCEPTIONS

bool
File::
isKeyUnique() const
{
	// デフォルトはFalseを返す
	return false;
}

// FUNCTION public
//	Schema::File::isKeyNotNull -- すべてのキーにNotNull制約がついているか
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
File::isKeyNotNull(Trans::Transaction& cTrans_) const
{
	bool bResult = true;
	Index* pIndex = getIndex(cTrans_);
	if (pIndex) {
		const ModVector<Key*> vecKey = pIndex->getKey(cTrans_);
		ModVector<Key*>::ConstIterator iterator = vecKey.begin();
		const ModVector<Key*>::ConstIterator last = vecKey.end();
		for (; iterator != last; ++iterator) {
			if ((*iterator)->isNullable()) {
				bResult = false;
				break;
			}
		}
	}
	return bResult;
}

//	FUNCTION public
//	Schema::File::hasAllTuples --
//		ファイルにすべてのタプルが格納されるかどうか
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		すべてのタプルを保持するならtrueを返す
//
//	EXCEPTIONS

bool
File::
hasAllTuples() const
{
	// デフォルトはFalseを返す
	return false;
}

//	FUNCTION public
//	Schema::File::isKeyGenerated --
//		ファイルのキーは挿入時に生成されるか
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		キーが挿入時に生成されるならtrueを返す
//
//	EXCEPTIONS

bool
File::
isKeyGenerated() const
{
	// デフォルトはFalseを返す
	return false;
}

//	FUNCTION public
//	Schema::File::isAbleToScan --
//		順次取得が可能か
//
//	NOTES
//
//	ARGUMENTS
//		bool bAllTuples_
//			trueのとき、hasAllTuplesの返り値に関係なくすべてのタプルを保持しているとみなしてよい
//
//	RETURN
//		順次取得が可能ならtrueを返す
//
//	EXCEPTIONS

bool
File::
isAbleToScan(bool bAllTuples_) const
{
	// デフォルトはFalseを返す
	return false;
}

//	FUNCTION public
//	Schema::File::getSkipInsertType --
//		挿入しないデータの種別を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		挿入しないデータの種別を返す
//
//	EXCEPTIONS

File::SkipInsertType::Value
File::
getSkipInsertType() const
{
	// デフォルトはすべて挿入する
	return SkipInsertType::None;
}

//	FUNCTION public
//	Schema::File::isAbleToFetch --
//		キーを指定したFetchによる取得が可能か
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		Fetchが可能ならtrueを返す
//
//	EXCEPTIONS

bool
File::
isAbleToFetch() const
{
	// デフォルトはFalseを返す
	return false;
}

//	FUNCTION public
//	Schema::File::isAbleToSearch --
//		条件を指定した検索結果の取得が可能か
//
//	NOTES
//		Schemaモジュールではオペレーターまでのチェックを行う
//		必ずできないことが分かる場合にのみfalseを返すこと
//
//	ARGUMENTS
//		const LogicalFile::TreeNodeInterface& pCond_
//			条件ノード
//
//	RETURN
//		検索が不可能であることが明らかでないならtrueを返す
//
//	EXCEPTIONS

bool
File::
isAbleToSearch(const LogicalFile::TreeNodeInterface& pCond_) const
{
	// デフォルトはFalseを返す
	return false;
}

//	FUNCTION public
//	Schema::File::isAbleToGetByBitSet --
//		取得がRowIDのみのときBitSetによる取得が可能か
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		BitSetによる取得が可能ならtrueを返す
//
//	EXCEPTIONS

bool
File::
isAbleToGetByBitSet() const
{
	// デフォルトはFalseを返す
	return false;
}

// FUNCTION public
//	Schema::File::isAbleToSeachByBitSet -- 取得対象のRowIDを渡して絞込み検索が可能か
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
File::
isAbleToSearchByBitSet() const
{
	// デフォルトはfalse
	return false;
}

// FUNCTION public
//	Schema::File::isAbleToUndo -- 削除や挿入のUndoはドライバーが行うか
//
// NOTES
//		このメソッドがtrueを返すドライバーに対してはUndoのときに
//		ValueフィールドにOIDを入れる
//
// ARGUMENTS
//	なし
//
// RETURN
//		削除や挿入のUndoはドライバーが行うならtrue
//
// EXCEPTIONS

bool
File::
isAbleToUndo() const
{
	// デフォルトはFalseを返す
	return false;
}

// FUNCTION public
//	Schema::File::isAbleToSort -- キー順の取得が可能か
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
File::
isAbleToSort() const
{
	// default is false
	return false;
}


// FUNCTION public
//	Schema::File::isAbleToGrouping -- キー順のグルーピングが可能か
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
File::
isAbleToBitSetSort() const
{
	// default is false
	return false;
}


// FUNCTION public
//	Schema::File::isAbleToGrouping -- キー順のグルーピングが可能か
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
File::
isGettable(Trans::Transaction& cTrans_,
		   const Field* pField_,
		   const LogicalFile::TreeNodeInterface* pScalarField_) const
{
	if (pScalarField_->getOptionSize() == 1
		&& pScalarField_->getOptionAt(0)->getType() == LogicalFile::TreeNodeInterface::Expand) {
		return false;
	}
	
	if (pField_) {
		return pField_->isGetable();
	} else {
		return true;
	}
}



// FUNCTION public
//	Schema::File::isAbleToVerifyTuple -- タプル単位の整合性検査が可能か
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
File::
isAbleToVerifyTuple() const
{
	// default is true
	return true;
}

// FUNCTION public
//	Schema::File::isHasFunctionField -- 関数フィールドがあるか
//
// NOTES
//
// ARGUMENTS
//	Schema::Field::Function::Value eFunction_
//
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
File::
isHasFunctionField(Schema::Field::Function::Value eFunction_) const
{
	// デフォルトはFalseを返す
	return false;
}

// FUNCTION public
//	Schema::File::isHasFunctionField -- 
//
// NOTES
//
// ARGUMENTS
//	LogicalFile::TreeNodeInterface::Type eFunction_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
File::
isHasFunctionField(LogicalFile::TreeNodeInterface::Type eFunction_) const
{
	// デフォルトはFalseを返す
	return false;
}

//	FUNCTION public
//	Schema::File::getFetchKey --
//		Fetchに使うことができるフィールドを得る
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//
//	RETURN
//		Fetchが可能ならキーとなるフィールドの配列
//
//	EXCEPTIONS

//virtual
ModVector<Field*>
File::
getFetchKey(Trans::Transaction& cTrans_) const
{
	// defaultはカテゴリーがKeyのフィールドを返す
	if (isAbleToFetch()) {
		return getField(Field::Category::Key, cTrans_);
	}
	return ModVector<Field*>();
}

// ファイルの名前を生成する
//virtual
Object::Name
File::
createName(Trans::Transaction& cTrans_, const Name& cParentName_)
{
	// サブクラスでオーバーライドする
	_SYDNEY_THROW0(Exception::Unexpected);
}

//	FUNCTION public
//	Schema::File::loadField --
//		ファイルに属するすべてのフィールドを表すクラスを読み出す
//
//	NOTES
//		参照を返すので呼び出し側で排他制御すべきである
//
//	ARGUMENTS
//		Trans::Transaction&			cTrans_
//			操作をするトランザクション記述子
//		bool bRecovery_ = false
//			trueの場合Mount後のリカバリーのための特別な動作を行う
//
//	RETURN
//		ファイルに属するフィールドをひとつづつ要素とするハッシュマップ
//
//	EXCEPTIONS

const FieldMap&
File::loadField(Trans::Transaction& cTrans_, bool bRecovery_ /* = false */)
{
	AutoRWLock l(getRWLock());

	if (!_fields) {
		l.convert(Os::RWLock::Mode::Write);

		// 書き込みロックの中で再度調べる
		if (!_fields) {
			// 「フィールド」表のうち、このファイルに関する部分を
			// 一度も読み出していないので、まず、読み出す
			// これが一時オブジェクトかもしくは作成後永続化していないなら
			// 読み出す必要はないのでベクターを初期化するのみ
			Database* pDatabase = getDatabase(cTrans_);
			; _SYDNEY_ASSERT(pDatabase);

			if ((getScope() != Scope::Permanent
				 || getStatus() == Status::Created)
				&& !bRecovery_)
				resetField(*pDatabase);
			else {
				SystemTable::Field(*pDatabase).load(cTrans_, *this, bRecovery_);

				// 読み込んだ後にフィールドの名称をセットする
				FieldMap::Iterator iterator = _fields->begin();
				const FieldMap::Iterator& end = _fields->end();

				for (; iterator != end; ++iterator)
					FieldMap::getValue(iterator)->setName(cTrans_);

				if (Schema::Database::isAvailable(getDatabaseID())) {
					// 読み込み専用の仮想列に対応するフィールドを追加する
					if (ModSize v = createVirtualField(cTrans_)) {
						ModSize n = _fields->getSize();

						// オブジェクトを構成するフィールド数を上書きする
						m_pFileID->setInteger(_SYDNEY_SCHEMA_PARAMETER_KEY(
													FileCommon::FileOption::FieldNumber::Key),
											  n - v);
					}
				}
			}

			; _SYDNEY_ASSERT(_fields);
		}
	}
	return *_fields;
}

//	FUNCTION public
//	Schema::File::getField --
//		ファイルに存在するすべてのフィールドを表すクラスを得る
//
//	NOTES
//		参照を返すので呼び出し側で排他制御すべきである
//
//	ARGUMENTS
//		Trans::Transaction&			cTrans_
//			操作をするトランザクション記述子
//
//	RETURN
//		ファイルに存在するフィールドを定義順にひとつづつ要素とするベクター
//
//	EXCEPTIONS

const ModVector<Field*>&
File::getField(Trans::Transaction& cTrans_) const
{
	return const_cast<File*>(this)->loadField(cTrans_).getView(getRWLock());
}

//	FUNCTION public
//	Schema::File::getField --
//		ファイルに属するフィールドのうち、
//		ある種別のフィールドを表すクラスを得る
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Field::Category::Value		fieldCategory
//			この種別を持つフィールドを表すクラスを得る
//		Trans::Transaction&			cTrans_
//			操作をするトランザクション記述子
//
//	RETURN
//		指定された種別のフィールドを
//		定義順にひとつづつ要素とするベクター
//
//	EXCEPTIONS

ModVector<Field*>
File::
getField(Field::Category::Value fieldCategory,
		 Trans::Transaction& cTrans_) const
{
	ModVector<Field*> v;

	const FieldMap& cMap = const_cast<File*>(this)->loadField(cTrans_);
	AutoRWLock l(getRWLock());
	cMap.extract(v,
				 BoolFunction1<Field, Field::Category::Value>(FieldMap::findByCategory, fieldCategory));
	return v;
}

//	FUNCTION public
//	Schema::File::getField --
//		ファイルに属するフィールドのうち、
//		あるフィールドを派生元とするフィールドを表すクラスを得る
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Field&		field
//			このフィールドを派生元とする
//			フィールドを表すクラスを得る
//		Trans::Transaction&			cTrans_
//			操作をするトランザクション記述子
//
//	RETURN
//		指定されたフィールドを派生元とするフィールドを
//		定義順にひとつづつ要素とするベクター
//
//	EXCEPTIONS

ModVector<Field*>
File::getField(const Field& source, Trans::Transaction& cTrans_) const
{
	ModVector<Field*>	v;

	const FieldMap& cMap = const_cast<File*>(this)->loadField(cTrans_);

	AutoRWLock l(getRWLock());
	cMap.extract(v,
				 BoolFunction1<Field, ID::Value>(FieldMap::findBySourceID, source.getID()));
	return v;
}

//	FUNCTION public
//	Schema::File::getFieldByID --
//		ファイルに属するフィールドのうち、
//		指定したオブジェクトIDを持つフィールドを表すクラスを得る
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Object::ID::Value	iFieldID_
//			このオブジェクトIDを持つフィールドを表すクラスを得る
//		Trans::Transaction&			cTrans_
//			操作をするトランザクション記述子
//
//	RETURN
//		0 以外の値
//			得られたフィールドを格納する領域の先頭アドレス
//		0
//			ファイルには指定されたオブジェクトIDのフィールドは存在しない
//
//	EXCEPTIONS

Field*
File::getFieldByID(ID::Value iFieldID_, Trans::Transaction& cTrans_) const
{
	if (iFieldID_ == ID::Invalid)
		return 0;

	const FieldMap& cMap = const_cast<File*>(this)->loadField(cTrans_);

	AutoRWLock l(getRWLock());
	return cMap.get(iFieldID_).get();
}

#ifdef OBSOLETE // フィールド名からフィールドオブジェクトを得ることはない
//	FUNCTION public
//	Schema::File::getField --
//		ファイルに属するフィールドのうち、
//		指定した名前を持つフィールドを表すクラスを得る
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Object::Name&	fieldName
//			フィールドの名前
//		Trans::Transaction&			cTrans_
//			操作をするトランザクション記述子
//
//	RETURN
//		0 以外の値
//			得られたフィールドを格納する領域の先頭アドレス
//		0
//			ファイルには指定された名前のフィールドは存在しない
//
//	EXCEPTIONS

Field*
File::getField(const Name& fieldName, Trans::Transaction& cTrans_) const
{
	const FieldMap& cMap = const_cast<File*>(this)->loadField(cTrans_);

	AutoRWLock l(getRWLock());
	cMap.find(BoolFunction1<Field, const Name&>(_Bool::_findByName, fieldName));
}
#endif

//	FUNCTION public
//	Schema::File::getFieldByPosition --
//		ファイルに属するフィールドのうち、
//		指定した位置にあるフィールドを表すクラスを得る
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Field::Position fieldPosition
//			フィールドの位置
//		Trans::Transaction&			cTrans_
//			操作をするトランザクション記述子
//
//	RETURN
//		0 以外の値
//			得られたフィールドを格納する領域の先頭アドレス
//		0
//			ファイルには指定された位置のフィールドは存在しない
//
//	EXCEPTIONS

Field*
File::
getFieldByPosition(Field::Position fieldPosition,
				   Trans::Transaction& cTrans_) const
{
	const FieldMap& cMap = const_cast<File*>(this)->loadField(cTrans_);

	AutoRWLock l(getRWLock());
	return cMap.find(BoolFunction1<Field, Field::Position>(FieldMap::findByPosition, fieldPosition));
}

//	FUNCTION public
//	Schema::File::getField --
//		ファイルに属するフィールドのうち、
//		指定した関数フィールドを表すクラスを得る
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Field::Function::Value	function
//			関数を表す列挙子の値
//		Schema::Object::ID::Value iColumnID_
//			関数の引数になる列のID
//		Trans::Transaction&			cTrans_
//			操作をするトランザクション記述子
//
//	RETURN
//		0 以外の値
//			得られたフィールドを格納する領域の先頭アドレス
//		0
//			ファイルには指定された名前のフィールドは存在しない
//
//	EXCEPTIONS

Field*
File::
getField(Field::Function::Value eFunction_, ID::Value iColumnID_,
		 Trans::Transaction& cTrans_) const
{
	const FieldMap& cMap = const_cast<File*>(this)->loadField(cTrans_);

	AutoRWLock l(getRWLock());
	return cMap.find(BoolFunction2<Field, Field::Function::Value, ID::Value>(FieldMap::findByFunction, eFunction_, iColumnID_));
}

// FUNCTION public
//	Schema::File::getSkipCheckKey -- 挿入をスキップするか検査するフィールド
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	
// RETURN
//	const ModVector<Field*>&
//
// EXCEPTIONS

const ModVector<Field*>&
File::
getSkipCheckKey(Trans::Transaction& cTrans_) const
{
	AutoRWLock l(getRWLock());

	if (!m_pvecSkipCheckKey) {
		l.convert(Os::RWLock::Mode::Write);

		// 書き込みロックの中で再度調べる
		if (!m_pvecSkipCheckKey) {
			m_pvecSkipCheckKey = new ModVector<Field*>();
			switch (getSkipInsertType()) {
			case SkipInsertType::None:
				{
					break;
				}
			case SkipInsertType::ValueIsNull:
				{
					*m_pvecSkipCheckKey = getField(Field::Category::Data, cTrans_);
					break;
				}
			case SkipInsertType::FirstKeyIsNull:
				{
					const FieldMap& cMap = const_cast<File*>(this)->loadField(cTrans_);
					m_pvecSkipCheckKey->pushBack(cMap.find(BoolMemberFunction1<Field, Trans::Transaction&>(
														 &Field::isFirstKey, cTrans_)));
					break;
				}
			case SkipInsertType::AllKeyIsNull:
				{
					*m_pvecSkipCheckKey = getField(Field::Category::Key, cTrans_);
					break;
				}
			case SkipInsertType::AllStringKeyIsNull:
				{
					const FieldMap& cMap = const_cast<File*>(this)->loadField(cTrans_);
					cMap.extract(*m_pvecSkipCheckKey,
								 BoolMemberFunction0<Field>(&Field::isStringKey));
					break;
				}
			default:
				{
					_SYDNEY_THROW0(Exception::Unexpected);
				}
			}
		}
	}
	return *m_pvecSkipCheckKey;
}

// FUNCTION public
//	Schema::File::getPutKey -- 更新/削除のキーとなるフィールド
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	
// RETURN
//	const ModVector<Field*>&
//
// EXCEPTIONS

const ModVector<Field*>&
File::
getPutKey(Trans::Transaction& cTrans_) const
{
	AutoRWLock l(getRWLock());

	if (!m_pvecPutKey) {
		l.convert(Os::RWLock::Mode::Write);

		// 書き込みロックの中で再度調べる
		if (!m_pvecPutKey) {
			m_pvecPutKey = new ModVector<Field*>();

			// add key fields
			ModVector<Field*> vecKey = getField(Field::Category::Key, cTrans_);
			m_pvecPutKey->insert(m_pvecPutKey->end(),
								 vecKey.begin(), vecKey.end());
			if (getIndexID() == ObjectID::Invalid) {
				if (Field* pObjectID = getObjectID(cTrans_)) {
					m_pvecPutKey->pushBack(pObjectID);
				}
			} else {
				// add value for index files
				ModVector<Field*> vecValue = getField(Field::Category::Data, cTrans_);
				m_pvecPutKey->insert(m_pvecPutKey->end(),
									 vecValue.begin(), vecValue.end());
			}
		}
	}
	return *m_pvecPutKey;
}

//	FUNCTION public
//	Schema::File::addField --
//		ファイルに属するフィールドとして、
//		指定されたフィールドを表すクラスを追加する
//
//	NOTES
//		「フィールド」表は更新されない
//
//	ARGUMENTS
//		const Schema::Field::Pointer&		field
//			追加するフィールドを表すクラス
//			これは、呼び出し中には更新されないが、
//			その後、更新される可能性がある
//		Trans::Transaction&			cTrans_
//			操作をするトランザクション記述子
//
//	RETURN
//		追加したフィールドを表すクラス
//
//	EXCEPTIONS

const Field::Pointer&
File::addField(const Field::Pointer& field, Trans::Transaction& cTrans_)
{
	// 「フィールド」表のうち、このファイルに属する部分を
	// 読み出していなければ、まず読み出してから、
	// 与えられたフィールドを追加する

	(void) loadField(cTrans_);

	AutoRWLock l(getRWLock(), Os::RWLock::Mode::Write);

#ifdef DEBUG
	{
		FieldMap::Iterator iterator = _fields->begin();
		const FieldMap::Iterator& end = _fields->end();

		for (; iterator != end; ++iterator) {
			Field* pField = FieldMap::getValue(iterator).get();
			if (pField && pField->getPosition() == field->getPosition()) {
				SydErrorMessage
					<< "Duplicated field's position("
					<< field->getPosition() << ") "
					<< pField->getName() << "<->" << field->getName()
					<< ModEndl;
				_SYDNEY_THROW0(Exception::BadArgument);
			}
		}
	}
#endif

	_fields->insert(field);

	return field;
}

//	FUNCTION public
//	Schema::File::addField --
//		ファイルを表すクラスのフィールドとして、
//		指定された列の値を格納するフィールドを表すクラスを追加する
//
//	NOTES
//		「フィールド」表は更新されない
//
//	ARGUMENTS
//		Schema::Column&		column
//			この列の値を格納するフィールドを表すクラスを追加する
//			これは、呼び出し中には更新されないが、
//			その後、更新される可能性がある
//		Trans::Transaction&			cTrans_
//			操作をするトランザクション記述子
//
//	RETURN
//		追加したフィールドを表すクラス
//
//	EXCEPTIONS

Field::Pointer
File::addField(Column& column, Trans::Transaction& cTrans_,
			   ObjectID::Value iID_ /* = ObjectID::Invalid */)
{
	// 「フィールド」表のうち、このファイルに属する部分を
	// 読み出していなければ、まず読み出してから、
	// 与えられたフィールドを追加する

	(void) loadField(cTrans_);

	AutoRWLock l(getRWLock(), Os::RWLock::Mode::Write);

	// 指定された列の値を格納するフィールドを表すクラスを生成する
	// ★注意★
	// ファイルへの追加はこのcreateの中で同時に行われる

	Field::Pointer field =
		Field::create(*this, _fields->getSize(),
					  Field::Category::Data,
					  Field::Permission::All,
					  column, cTrans_,
					  iID_);
	; _SYDNEY_ASSERT(field.get());
	; _SYDNEY_ASSERT(field->getType() != Common::DataType::Undefined);

	// フィールドの状態は「生成」である

	; _SYDNEY_ASSERT(field->getStatus() == Status::Created);

	return field;
}

//	FUNCTION public
//	Schema::File::addField --
//		ファイルを表すクラスのフィールドとして、
//		指定された属性を持つフィールドを表すクラスを追加する
//
//	NOTES
//		「フィールド」表は更新されない
//
//	ARGUMENTS
//		Schema::Field::Category::Value	category
//			追加するフィールドの種類
//		Schema::Field::Permission::Value	permission
//			追加するフィールドに許可される操作
//		Schema::Field&		source
//			追加するフィールドの派生元であるフィールドを表すクラス
//			これは、呼び出し中には更新されないが、
//			その後、更新される可能性がある
//		Trans::Transaction&			cTrans_
//			操作をするトランザクション記述子
//
//	RETURN
//		追加したフィールドを表すクラス
//
//	EXCEPTIONS

Field::Pointer
File::
addField(Field::Category::Value category, Field::Permission::Value permission,
		 Field& source, Trans::Transaction& cTrans_,
		 ObjectID::Value iID_ /* = ObjectID::Invalid */)
{
	// 「フィールド」表のうち、このファイルに属する部分を
	// 読み出していなければ、まず読み出してから、
	// 与えられたフィールドを追加する

	(void) loadField(cTrans_);

	AutoRWLock l(getRWLock(), Os::RWLock::Mode::Write);

	// 指定されたフィールドを派生元とする
	// フィールドを表すクラスを生成する
	// ★注意★
	// ファイルへの追加はこのcreateの中で同時に行われる

	Field::Pointer field =
		Field::create(*this, _fields->getSize(),
					  category, permission, source, cTrans_,
					  iID_);
	; _SYDNEY_ASSERT(field.get());
	; _SYDNEY_ASSERT(field->getType() != Common::DataType::Undefined);

	// フィールドの状態は「生成」である

	; _SYDNEY_ASSERT(field->getStatus() == Status::Created);

	return field;
}

//	FUNCTION public
//	Schema::File::addField --
//		ファイルを表すクラスのフィールドとして、
//		指定された属性を持つフィールドを表すクラスを追加する
//
//	NOTES
//		「フィールド」表は更新されない
//
//	ARGUMENTS
//		Schema::Field::Category::Value	category
//			追加するフィールドの種類
//		Schema::Field::Permission::Value	permission
//			追加するフィールドに許可される操作
//		Schema::Field&		source
//			追加するフィールドの派生元であるフィールドを表すクラス
//			これは、呼び出し中には更新されないが、
//			その後、更新される可能性がある
//		Schema::Column&		column
//			追加するフィールドの派生元であるフィールドが
//			値を保持する列を表すクラス
//			これは、呼び出し中には更新されないが、
//			その後、更新される可能性がある
//		Trans::Transaction&			cTrans_
//			操作をするトランザクション記述子
//
//	RETURN
//		追加したフィールドを表すクラス
//
//	EXCEPTIONS

Field::Pointer
File::
addField(Field::Category::Value category, Field::Permission::Value permission,
		 Field& source, Column& column, Trans::Transaction& cTrans_,
		 ObjectID::Value iID_ /* = ObjectID::Invalid */)
{
	// 「フィールド」表のうち、このファイルに属する部分を
	// 読み出していなければ、まず読み出してから、
	// 与えられたフィールドを追加する

	(void) loadField(cTrans_);

	AutoRWLock l(getRWLock(), Os::RWLock::Mode::Write);

	// 指定されたフィールドを派生元とする
	// フィールドを表すクラスを生成する
	// ★注意★
	// ファイルへの追加はこのcreateの中で同時に行われる

	Field::Pointer field =
		Field::create(*this, _fields->getSize(),
					  category, permission, source, column, cTrans_,
					  iID_);
	; _SYDNEY_ASSERT(field.get());
	; _SYDNEY_ASSERT(field->getType() != Common::DataType::Undefined);

	// フィールドの状態は「生成」である

	; _SYDNEY_ASSERT(field->getStatus() == Status::Created);

	return field;
}

//	FUNCTION public
//	Schema::File::addField --
//		ファイルを表すクラスのフィールドとして、
//		指定されたキーの値を保持するフィールドを表すクラスを追加する
//
//	NOTES
//		「フィールド」表は更新されない
//
//	ARGUMENTS
//		Schema::Key& cKey_
//			値を保持するキーを表すクラス
//		Schema::Field::Permission::Value	permission
//			追加するフィールドに許可される操作
//		Trans::Transaction&			cTrans_
//			操作をするトランザクション記述子
//
//	RETURN
//		追加したフィールドを表すクラス
//
//	EXCEPTIONS

Field::Pointer
File::
addField(Key& cKey_, Field::Permission::Value permission,
		 Trans::Transaction& cTrans_,
		 ObjectID::Value iID_ /* = ObjectID::Invalid */)
{
	// 「フィールド」表のうち、このファイルに属する部分を
	// 読み出していなければ、まず読み出してから、
	// 与えられたフィールドを追加する

	(void) loadField(cTrans_);

	AutoRWLock l(getRWLock(), Os::RWLock::Mode::Write);

	// 指定されたフィールドを派生元とする
	// フィールドを表すクラスを生成する
	// ★注意★
	// ファイルへの追加はこのcreateの中で同時に行われる

	Field::Pointer field =
		Field::create(*this, _fields->getSize(),
					  cKey_, permission, cTrans_,
					  iID_);
	; _SYDNEY_ASSERT(field.get());
	; _SYDNEY_ASSERT(field->getType() != Common::DataType::Undefined);

	// フィールドの状態は「生成」である

	; _SYDNEY_ASSERT(field->getStatus() == Status::Created);

	return field;
}

//	FUNCTION public
//	Schema::File::addField --
//		ファイルを表すクラスのフィールドとして、
//		指定された関数に対応するフィールドを表すクラスを追加する
//
//	NOTES
//		「フィールド」表は更新されない
//
//	ARGUMENTS
//		Schema::Field::Function::Value	function
//			追加するフィールドが表す関数の種類
//		Schema::Column&		column
//			追加するフィールドが表す関数の引数になる列を表すクラス
//		Trans::Transaction&			cTrans_
//			操作をするトランザクション記述子
//
//	RETURN
//		追加したフィールドを表すクラス
//
//	EXCEPTIONS

Field::Pointer
File::
addField(Field::Function::Value function, Column& column, Trans::Transaction& cTrans_)
{
	// 「フィールド」表のうち、このファイルに属する部分を
	// 読み出していなければ、まず読み出してから、
	// 与えられたフィールドを追加する

	(void) loadField(cTrans_);

	AutoRWLock l(getRWLock(), Os::RWLock::Mode::Write);

	// 指定されたフィールドを派生元とする
	// フィールドを表すクラスを生成する
	// ★注意★
	// ファイルへの追加はこのcreateの中で同時に行われる

	Field::Pointer field =
		Field::create(*this, _fields->getSize(), function, column, cTrans_);
	; _SYDNEY_ASSERT(field.get());
	; _SYDNEY_ASSERT(field->getType() != Common::DataType::Undefined);

	// フィールドは仮想列である

	; _SYDNEY_ASSERT(field->getScope() != Object::Scope::Permanent);

	return field;
}

// FUNCTION public
//	Schema::Field::addField -- add a field copying an original field
//
// NOTES
//
// ARGUMENTS
//	Field& field
//	
// RETURN
//	Field::Pointer
//
// EXCEPTIONS

Field::Pointer
File::
addField(Field& field, Trans::Transaction& cTrans_,
		 ObjectID::Value iID_ /* = ObjectID::Invalid */)
{
	(void) loadField(cTrans_);

	AutoRWLock l(getRWLock(), Os::RWLock::Mode::Write);

	Field::Pointer result =
		Field::create(*this, _fields->getSize(), field, cTrans_,
					  iID_);
	; _SYDNEY_ASSERT(result.get());
	; _SYDNEY_ASSERT(result->getType() != Common::DataType::Undefined);
	; _SYDNEY_ASSERT(result->getStatus() == Status::Created);

	return result;
}

#ifdef OBSOLETE // FieldのdoAfterPersistで使用しているがそれがOBSOLETEなのでこれもOBSOLETEにする
//	FUNCTION public
//	Schema::File::eraseField --
//		ファイルを表すクラスからあるフィールドを表すクラスの登録を抹消する
//
//	NOTES
//		「フィールド」表は更新されない
//
//	ARGUMENTS
//		Schema::Object::ID::Value	fieldID
//			登録を抹消するフィールドのオブジェクト ID
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
File::
eraseField(ID::Value fieldID)
{
	AutoRWLock l(getRWLock(), Os::RWLock::Mode::Write);

	if (_fields)
		(void) _fields->erase(fieldID);
}
#endif

//	FUNCTION public
//	Schema::File::resetField --
//		ファイルにはフィールドを表すクラスが登録されていないことにする
//
//	NOTES
//		「フィールド」表は更新されない
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
File::resetField()
{
	AutoRWLock l(getRWLock(), Os::RWLock::Mode::Write);

	if (_fields) {

		if (getScope() == Scope::Permanent)
			Manager::ObjectTree::Database::decrementCacheSize(_fields->getSize());

		// ベクターを空にする
		// キャッシュには入っていないので
		// 他のもののようにループしながら
		// キャッシュから削除して回る必要はない
		_fields->reset();

	} else {
		// フィールドを表すクラスを登録するハッシュマップを生成する

		_fields = new FieldMap;
		; _SYDNEY_ASSERT(_fields);
	}
}

//	FUNCTION public
//	Schema::File::resetField --
//		ファイルにはフィールドを表すクラスが登録されていないことにする
//
//	NOTES
//		「フィールド」表は更新されない
//
//	ARGUMENTS
//		Database& cDatabase_
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
File::resetField(Database& cDatabase_)
{
	AutoRWLock l(getRWLock(), Os::RWLock::Mode::Write);

	if (_fields) {
		_fields->reset(cDatabase_);

	} else {

		// フィールドを表すクラスを登録するハッシュマップを生成する

		_fields = new FieldMap;
		; _SYDNEY_ASSERT(_fields);
	}
}

//	FUNCTION public
//	Schema::File::clearField --
//		ファイルを表すクラスに登録されているフィールドを表すクラスと、
//		その管理用のベクターを破棄する
//
//	NOTES
//		「フィールド」表は更新されない
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
File::clearField(Trans::Transaction& cTrans_)
{
	if (_fields) {
		AutoRWLock l(getRWLock(), Os::RWLock::Mode::Write);
		// 書き込みロックの中でもう一度調べる

		if (_fields) {
			// ハッシュマップに登録されているフィールドを表す
			// クラスがあればすべて破棄し、ハッシュマップも破棄する

			resetField(*getDatabase(cTrans_));
			delete _fields, _fields = 0;
		}
	}
}

// FUNCTION public
//	Schema::File::checkFieldType -- Check the validity of FileID contents and modify if needed
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	
// RETURN
//	ModSize
//
// EXCEPTIONS

//virtual
void
File::
checkFieldType(Trans::Transaction& cTrans_)
{
	; // Do nothing as default
}

// FUNCTION public
//	Schema::File::createVirtualField -- 読み込み専用の仮想列を追加する
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	
// RETURN
//	ModSize
//
// EXCEPTIONS

//virtual
ModSize
File::
createVirtualField(Trans::Transaction& cTrans_)
{
	return 0; // defaultは何もしない
}

//	FUNCTION public
//	Schema::File::getObjectID --
//		ファイルのオブジェクト ID を格納するフィールドを表すクラスを得る
//
//	NOTES
//		生成前、中のファイルや、排他制御がうまく行われていない場合を除けば、
//		ファイルにはオブジェクト ID を格納するフィールドが
//		必ずひとつ存在するはずである
//
//		ただし、MOD 転置ファイルは除く
//
//	ARGUMENTS
//		Trans::Transaction&			cTrans_
//			操作をするトランザクション記述子
//		bool bForPut_ = false
//			挿入するために得るときtrue
//
//	RETURN
//		0 以外の値
//			得られたオブジェクト ID を格納するフィールドを
//			格納する領域の先頭アドレス
//		0
//			ファイルにはオブジェクト ID を格納するフィールドは存在しない
//
//	EXCEPTIONS

Field*
File::getObjectID(Trans::Transaction& cTrans_, bool bForPut_) const
{
	//【注意】	オブジェクト ID を格納するフィールドは
	//			ファイルの先頭にあるものとして、処理している

	const ModVector<Field*>& fields = getField(cTrans_);

	return (fields.getSize() && fields[0] && fields[0]->isObjectID()
			&& ((bForPut_ && fields[0]->isPutable())
				|| fields[0]->isGetable())) ?
		fields[0] : 0;
}

//	FUNCTION public
//	Schema::File::getPosition --
//		ファイルに存在するフィールドのうち、
//		ある種類のもののファイルの先頭からの位置を得る
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Field::Category::Value	fieldCategory
//			フィールドの種類
//		Trans::Transaction&			cTrans_
//			操作をするトランザクション記述子
//
//	RETURN
//		ファイルに存在する指定された種類のフィールドの
//		ファイルの先頭からの位置を昇順にひとつづつ要素とするベクター
//		指定された種類のフィールドがひとつもないとき、空のベクター
//
//	EXCEPTIONS

ModVector<Field::Position>
File::
getPosition(Field::Category::Value fieldCategory,
			Trans::Transaction& cTrans_) const
{
	const ModVector<Field*>& fields = getField(cTrans_);

	ModVector<Field::Position>	v;
	ModSize	n = fields.getSize();
	if (fieldCategory == Field::Category::Unknown)
		v.reserve(n);

	for (ModSize i = 0; i < n; ++i) {
		; _SYDNEY_ASSERT(fields[i]);
		if (fieldCategory == Field::Category::Unknown ||
			fieldCategory == fields[i]->getCategory())
			v.pushBack(fields[i]->getPosition());
	}

	return v;
}

// FUNCTION public
//	Schema::File::isCompoundIndex -- 2つ以上のキーを持つか
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
File::
isCompoundIndex(Trans::Transaction& cTrans_) const
{
	const ModVector<Field*>& fields = getField(cTrans_);

	int count = 0;
	ModSize	n = fields.getSize();

	for (ModSize i = 0; i < n; ++i) {
		; _SYDNEY_ASSERT(fields[i]);
		if (fields[i]->getCategory() == Field::Category::Key) {
			if (count) return true;
			++count;
		}
	}

	return false;
}

//	FUNCTION public
//	Schema::File::clearHint --
//		ファイルを表すクラスに登録されているヒントを表すクラスを破棄する
//
//	NOTES
//		「ファイル」表は更新されない
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
File::
clearHint()
{
	

	delete m_pHint, m_pHint = 0;
}

//	FUNCTION public
//	Schema::File::clearAreaHint --
//		ファイルを表すクラスに登録されているエリアのヒントを表すクラスを破棄する
//
//	NOTES
//		「ファイル」表は更新されない
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
File::
clearAreaHint()
{
	AutoRWLock l(getRWLock(), Os::RWLock::Mode::Write);

	delete m_pAreaHint, m_pAreaHint = 0;
}

//	FUNCTION public
//	Schema::File::serialize --
//		ファイルを表すクラスのシリアライザー
//
//	NOTES
//
//	ARGUMENTS
//		ModArchive&			archiver
//			シリアル化先(または元)のアーカイバー
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
File::
serialize(ModArchive& archiver)
{
	// まず、スキーマオブジェクト固有の情報をシリアル化する

	Object::serialize(archiver);

	if (archiver.isStore()) {

		// ファイルの種別
		{
		int tmp = _category;
		archiver << tmp;
		}
		// ファイルのファイル ID

		archiver << *m_pFileID;

		// ファイルを使う索引のスキーマオブジェクト ID

		archiver << _indexID;

		// ファイルのヒント

		int hasHint = ((m_pHint) ? 1 : 0) + ((m_pAreaHint) ? 2 : 0);
		archiver << hasHint;
		if (m_pHint) archiver << *m_pHint;
		if (m_pAreaHint) archiver << *m_pAreaHint;

		// 格納するエリアID
		archiver << _areaID;
		archiver << _logAreaID;

		// 作成されているか
		archiver << m_bCreated;

		// fields
		{
			ModSize n = (_fields) ? _fields->getSize() : 0;
			archiver << n;
			if (n) {
				Utility::OutputArchive& out =
					dynamic_cast<Utility::OutputArchive&>(archiver);

				_fields->writeObject(out);
			}
		}

	} else {

		// メンバーをすべて初期化しておく

		clear();

		// ファイルの種別
		{
		int tmp;
		archiver >> tmp;
		_category = static_cast<Category::Value>(tmp);
		}
		// ファイルのファイル ID

		m_pFileID = new LogicalFile::FileID();
		archiver >> *m_pFileID;

		// ファイルを使う索引のスキーマオブジェクト ID

		archiver >> _indexID;

		// ファイルのヒント

		int hasHint;
		archiver >> hasHint;
		if (hasHint % 2) {
			Hint cHint;
			archiver >> cHint;
			m_pHint = new Hint(cHint);
		}
		if (hasHint / 2) {
			Hint cHint;
			archiver >> cHint;
			m_pAreaHint = new Hint(cHint);
		}

		// 格納するエリアID
		archiver >> _areaID;
		archiver >> _logAreaID;

		// 作成されているか
		archiver >> m_bCreated;

		// fields
		{
			ModSize n;
			archiver >> n;
			if (n) {
				resetField();
				Utility::InputArchive& in =
					dynamic_cast<Utility::InputArchive&>(archiver);

				_fields->readObject(in, n);
			}
		}
	}
}

//	FUNCTION public
//	Schema::File::getClassID -- このクラスのクラス ID を得る
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
File::
getClassID() const
{
	return Externalizable::Category::File +
		Common::Externalizable::SchemaClasses;
}

// FUNCTION public
//	Schema::File::makeLogData -- ログデータを作る
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	Common::DataArrayData& cLogData_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
File::
makeLogData(Trans::Transaction& cTrans_,
			Common::DataArrayData& cLogData_) const
{
	cLogData_.pushBack(LogData::createID(getID()));
	const ModVector<Field*>& vecField = getField(cTrans_);
	ModSize n = vecField.getSize();

	ModVector<ObjectID::Value> vecFieldIDs;
	vecFieldIDs.reserve(n);
	for (ModSize i = 0; i < n; ++i) {
		if (vecField[i]->isFunction() == false) {
			vecFieldIDs.pushBack(vecField[i]->getID());
		}
	}
	cLogData_.pushBack(LogData::createIDs(vecFieldIDs));
}

// FUNCTION public
//	Schema::File::getNextFieldID -- get field objectid from logdata
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	const Common::DataArrayData* pLogData_ /* = 0 */
//	
// RETURN
//	ObjectID::Value
//
// EXCEPTIONS

ObjectID::Value
File::
getNextFieldID(Trans::Transaction& cTrans_,
			   const Common::DataArrayData* pLogData_ /* = 0 */) const
{
	if (pLogData_) {
		ModSize iPosition = getField(cTrans_).getSize();
		const ModVector<ID::Value>& vecIDs = LogData::getIDs(pLogData_->getElement(Log::FieldIDs));
		if (iPosition < vecIDs.getSize()) {
			return vecIDs[iPosition];
		}
	}
	return ID::Invalid;
}

//	FUNCTION public
//	Schema::File::getAccessFile --
//		ファイルに対応するAccessFileクラスのインスタンスを得る
//
//	NOTES
//		現在はFullTextFile以外はAccessFileクラスを返す
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//
//	RETURN
//	Schema::AccessFile*
//		ファイルに対応するAccessFileクラスのインスタンス
//
//	EXCEPTIONS

// virtual
AccessFile*
File::
getAccessFile(Trans::Transaction& cTrans_) const
{
	// AccessFileでFileIDを使用するので仮想列を含めるためにloadFieldしておく
	(void)const_cast<File*>(this)->loadField(cTrans_);
	return new AccessFile(cTrans_, *this);
}

//	FUNCTION public
//	Schema::File::setAvailability -- 論理ファイルの利用可能性を設定する
//
//	NOTES
//
//	ARGUMENTS
//		Schema::ObjectID::Value	dbID
//			利用可能性を設定する論理ファイルを使用する
//			データベースのスキーマオブジェクト識別子
//		Schema::ObjectID::Value	fileID
//			利用可能性を設定する論理ファイルのスキーマオブジェクト識別子
//		bool				v
//			true
//				論理ファイルを利用可能にする
//			false
//				論理ファイルを利用不可にする
//
//	RETURN
//		true
//			設定前は論理ファイルは利用可能だった
//		false
//			設定前は論理ファイルは利用不可だった
//
//	EXCEPTIONS

// static
bool
File::setAvailability(Object::ID::Value dbID, Object::ID::Value fileID, bool v)
{
	return Checkpoint::Database::setAvailability(dbID, fileID, v);
}

//	FUNCTION public
//	Schema::File::setAvailability -- 論理ファイルの利用可能性を設定する
//
//	NOTES
//
//	ARGUMENTS
//		Lock::FileName&		lockName
//			利用可能性を設定する論理ファイルのロック名
//		bool				v
//			true
//				論理ファイルを利用可能にする
//			false
//				論理ファイルを利用不可にする
//
//	RETURN
//		true
//			設定前の論理ファイルは利用可能だった
//		false
//			設定前の論理ファイルは利用不可だった
//
//	EXCEPTIONS

// static
bool
File::setAvailability(const Lock::FileName& lockName, bool v)
{
	return Checkpoint::Database::setAvailability(lockName, v);
}

//	FUNCTION public
//	Schema::File::isAvailable -- 論理ファイルが利用可能か調べる
//
//	NOTES
//
//	ARGUMENTS
//		Schema::ObjectID::Value	dbID
//			利用不可か調べる論理ファイルを使用する
//			データベースのスキーマオブジェクト識別子
//		Schema::ObjectID::Value	fileID
//			利用不可か調べる論理ファイルのスキーマオブジェクト識別子
//
//	RETURN
//		true
//			利用可能である
//		false
//			利用不可である
//
//	EXCEPTIONS

// static
bool
File::isAvailable(Object::ID::Value dbID, Object::ID::Value fileID)
{
	return Checkpoint::Database::isAvailable(dbID, fileID);
}

//	FUNCTION public
//	Schema::File::isAvailable -- 論理ファイルの利用可能性を調べる
//
//	NOTES
//
//	ARGUMENTS
//		Lock::FileName&		lockName
//			利用可能性を調べる論理ファイルのロック名
//
//	RETURN
//		true
//			論理ファイルは利用可能である
//		false
//			論理ファイルは利用不可である
//
//	EXCEPTIONS

// static
bool
File::isAvailable(const Lock::FileName& lockName)
{
	return Checkpoint::Database::isAvailable(lockName);
}

//	FUNCTION public
//	Schema::File::isKeyImportable -- importやreflectでファイルに挿入できるデータかを調べる
//
//	NOTES
//
//	ARGUMENTS
//		const Common::Data::Pointer& pKey_
//			挿入できるかを調べるキーデータ
//
//	RETURN
//		true
//			挿入できる
//		false
//			挿入できない
//
//	EXCEPTIONS

bool
File::
isKeyImportable(const Common::Data::Pointer& pKey_) const
{
	return _isKeyImportable(*this, pKey_);
}

//	FUNCTION private
//	Schema::File::setAreaID --
//		使用されるエリアIDを設定する
//
//	NOTES
//
//	ARGUMENTS
//		const Schema::Table& cTable_
//			ファイルが属する表オブジェクト
//		Trans::Transaction& cTrans_
//			トランザクション記述子
//
//	RETURN
//	なし
//
//	EXCEPTIONS

void
File::
setAreaID(const Table& cTable_, Trans::Transaction& cTrans_)
{
	AreaCategory::Value eAreaCategory = getAreaCategory();
	ID::Value iAreaID = cTable_.getAreaID(eAreaCategory, true /* effective */);

	setAreaID(iAreaID, getAreaCategory());
}

//	FUNCTION private
//	Schema::File::setAreaID --
//		使用されるエリアIDを設定する
//
//	NOTES
//
//	ARGUMENTS
//		const Schema::Index& cIndex_
//			ファイルが属する表オブジェクト
//		Trans::Transaction& cTrans_
//			トランザクション記述子
//
//	RETURN
//	なし
//
//	EXCEPTIONS

void
File::
setAreaID(const Index& cIndex_, Trans::Transaction& cTrans_)
{
	// getAreaIDの第二引数のtrueは自身にエリアIDが指定されていないときに
	// 親オブジェクトの指定までさかのぼることを示す

	AreaCategory::Value eAreaCategory = getAreaCategory();
	ID::Value iAreaID = cIndex_.getAreaID(eAreaCategory, true, cTrans_);

	setAreaID(iAreaID, getAreaCategory());
}

//	FUNCTION private
//	Schema::File::checkUndo --
//		Undo情報を検査し反映する
//
//	NOTES
//
//	ARGUMENTS
//		const Schema::Database& cDatabase_
//			ファイルが属するデータベースオブジェクト
//		const Schema::Table& cTable_
//			ファイルが属する表オブジェクト
//		Trans::Transaction& cTrans_
//			トランザクション記述子
//
//	RETURN
//	なし
//
//	EXCEPTIONS

void
File::
checkUndo(const Database& cDatabase_, const Table& cTable_, Trans::Transaction& cTrans_)
{
	using namespace Manager::RecoveryUtility;
	if (Undo::isEntered(cDatabase_.getName(), getTableID(), Undo::Type::AlterTable)) {
		// 親オブジェクトのエリアID割り当てがシステム表の内容と異なる場合があるので
		// ここでもう一度設定しなおす
		setAreaID(cTable_, cTrans_);
	}
	if (Undo::isEntered(cDatabase_.getName(), getTableID(), Undo::Type::RenameTable)) {
		Name cUndoName;
		if (Manager::RecoveryUtility::Name::getUndoFileName(cDatabase_.getName(), getID(), cUndoName)) {
			// Alter後の名前が登録されているときはログデータの名前ではなく
			// Alter後の名前を使用する
			rename(cUndoName);
		}
	}
}

//	FUNCTION private
//	Schema::File::checkUndo --
//		Undo情報を検査し反映する
//
//	NOTES
//
//	ARGUMENTS
//		const Schema::Database& cDatabase_
//			ファイルが属するデータベースオブジェクト
//		const Schema::Index& cIndex_
//			ファイルが属する索引オブジェクト
//		Trans::Transaction& cTrans_
//			トランザクション記述子
//
//	RETURN
//	なし
//
//	EXCEPTIONS

void
File::
checkUndo(const Database& cDatabase_, const Index& cIndex_, Trans::Transaction& cTrans_)
{
	using namespace Manager::RecoveryUtility;
	if (Undo::isEntered(cDatabase_.getName(), getTableID(), Undo::Type::AlterTable)
		|| Undo::isEntered(cDatabase_.getName(), getIndexID(), Undo::Type::AlterIndex)) {
		// 親オブジェクトのエリアID割り当てがシステム表の内容と異なる場合があるので
		// ここでもう一度設定しなおす
		setAreaID(cIndex_, cTrans_);
	}
	if (Undo::isEntered(cDatabase_.getName(), getTableID(), Undo::Type::RenameTable)
		|| Undo::isEntered(cDatabase_.getName(), getIndexID(), Undo::Type::RenameIndex)) {
		Name cUndoName;
		if (Manager::RecoveryUtility::Name::getUndoFileName(cDatabase_.getName(), getIndexID(), cUndoName)) {
			// Alter後の名前が登録されているときはログデータの名前ではなく
			// Alter後の名前を使用する
			rename(cUndoName);
		}
	}
}

//	FUNCTION public
//	Schema::File::setFileID -- ファイル ID を設定する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作をするトランザクション記述子
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
File::setFileID(Trans::Transaction& cTrans_)
{
	// サブクラスでオーバーライドする
	; _SYDNEY_ASSERT(false);
}

//	FUNCTION public
//	Schema::File::getAreaCategory --
//		ファイルを格納するエリアの種別を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		Schema::AreaCategory::Value
//			ファイルを格納するエリアの種別を表す値
//
//	EXCEPTIONS
//

Schema::AreaCategory::Value
Schema::File::
getAreaCategory() const
{
	// サブクラスでオーバーライドする
	; _SYDNEY_ASSERT(false);
	return AreaCategory::Default;
}

// FUNCTION public
//	Schema::File::setFieldTypeToFileID -- FileIDにフィールドの型情報をセットする
//
// NOTES
//
// ARGUMENTS
//	LogicalFile::FileID& cFileID_
//  Schema::Field* pField_
//  ModSize index_
//	Trans::Transaction& cTrans_
//	
// RETURN
//	なし
//
// EXCEPTIONS

void
Schema::File::
setFieldTypeToFileID(LogicalFile::FileID& cFileID_, Field* pField_, ModSize index_, Trans::Transaction& cTrans_)
{
	// フィールドデータ型
	cFileID_.setInteger(_SYDNEY_SCHEMA_FORMAT_KEY(
							  FileCommon::FileOption::FieldType::Key, index_),
						pField_->getType());

	// フィールド最大長
	if (pField_->getLength())
		cFileID_.setInteger(_SYDNEY_SCHEMA_FORMAT_KEY(
								  FileCommon::FileOption::FieldLength::Key, index_),
							pField_->getLength());

	// Scale of Decimal type
	if (pField_->getType() == Common::DataType::Decimal
		|| pField_->getElementType() == Common::DataType::Decimal) {
		cFileID_.setInteger(_SYDNEY_SCHEMA_FORMAT_KEY(
								  FileCommon::FileOption::FieldFraction::Key, index_),
							pField_->getScale());
	}

	// 符号化形式

	if (!Manager::Configuration::isNoEncodingForm()) {
		const Common::StringData::EncodingForm::Value
			encodingForm = pField_->getEncodingForm(cTrans_);
		if (encodingForm != Common::StringData::EncodingForm::Unknown)
			cFileID_.setInteger(
				_SYDNEY_SCHEMA_FORMAT_KEY(
					FileCommon::FileOption::FieldEncodingForm::Key, index_),
				encodingForm);
	}

	// Fixedか
	if (pField_->getType() != Common::DataType::Array
		&& pField_->isFixed())
		cFileID_.setBoolean(_SYDNEY_SCHEMA_FORMAT_KEY(
								  FileCommon::FileOption::FieldFixed::Key, index_),
							true);

	// Collation
	if (Column* column = pField_->getRelatedColumn(cTrans_))
		cFileID_.setInteger(_SYDNEY_SCHEMA_FORMAT_KEY(
								FileCommon::FileOption::FieldCollation::Key,
								index_),
							column->getType().getCollation());

	if (pField_->getType() == Common::DataType::Array) {
		// 配列要素の型と最大長
		cFileID_.setInteger(_SYDNEY_SCHEMA_FORMAT_KEY(
								  FileCommon::FileOption::ElementType::Key, index_),
							pField_->getElementType());
		if (pField_->getElementLength())
			cFileID_.setInteger(_SYDNEY_SCHEMA_FORMAT_KEY(
									  FileCommon::FileOption::ElementLength::Key, index_),
								pField_->getElementLength());

		if (!Manager::Configuration::isNoEncodingForm()) {

			// 配列要素の符号化形式

			const Common::StringData::EncodingForm::Value
				encodingForm = pField_->getElementEncodingForm(cTrans_);
			if (encodingForm != Common::StringData::EncodingForm::Unknown)
				cFileID_.setInteger(
					_SYDNEY_SCHEMA_FORMAT_KEY(
						FileCommon::FileOption::ElementEncodingForm::Key, index_),
					encodingForm);
		}
		// Fixedか
		if (pField_->isFixed())
			cFileID_.setBoolean(_SYDNEY_SCHEMA_FORMAT_KEY(
									  FileCommon::FileOption::ElementFixed::Key, index_),
								true);
	}
}

////////////////////////////////////////////
// データベースファイル化のためのメソッド //
////////////////////////////////////////////

// メタデータベースにおける「ファイル」表の構造は以下のとおり
// create table File_DBXXXX (
//		ID			id,
//		parent		id,
//		name		nvarchar,
//		category	int,
//		index		id,
//		FileID		nvarchar,
//		hint		nvarchar,
//		area		{id, id},
//		areaHint	nvarchar,
//		option		<any>,		 -- サブクラスの付加情報
//		time		timestamp
// )

namespace
{
#define _DEFINE0(_type_) \
	Meta::Definition<File>(Meta::MemberType::_type_)
#define _DEFINE2(_type_, _get_, _set_) \
	Meta::Definition<File>(Meta::MemberType::_type_, &File::_get_, &File::_set_)

	Meta::Definition<File> _vecDefinition[] =
	{
		_DEFINE0(FileOID),		// FileOID
		_DEFINE0(ObjectID),		// ID
		_DEFINE0(ParentID),		// ParentID
		_DEFINE0(Name),			// Name
		_DEFINE0(Integer),		// Category
		_DEFINE2(ID, getIndexID, setIndexID), //IndexID
		_DEFINE0(Binary), //FileID
		_DEFINE0(Binary), //Hint,
		_DEFINE0(IDArray), //AreaIDs
		_DEFINE0(Binary), //AreaHint
		_DEFINE0(Binary), //Option
		_DEFINE0(Timestamp), //Timestamp
		_DEFINE0(Unknown),	// MemberNum
		_DEFINE0(BigInt),	// FileSize
	};
#undef _DEFINE0
#undef _DEFINE2
}

//	FUNCTION public
//	Schema::File::getMetaFieldNumber --
//		スキーマオブジェクトを格納するファイルのフィールド数を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//	フィールドの数
//
//	EXCEPTIONS

int
File::
getMetaFieldNumber() const
{
	return static_cast<int>(Meta::File::MemberNum);
}

//	FUNCTION public
//	Schema::File::getMetaFieldDefinition --
//		スキーマオブジェクトを格納するファイルのフィールド定義を得る
//
//	NOTES
//
//	ARGUMENTS
//		int iMemberID_
//			フィールドのメンバーを識別する番号
//
//	RETURN
//	フィールドの数
//
//	EXCEPTIONS

Meta::MemberType::Value
File::
getMetaMemberType(int iMemberID_) const
{
	return _vecDefinition[iMemberID_].m_eType;
}

//	FUNCTION public
//	Schema::File::packMetaField --
//		スキーマオブジェクトの内容をレコードファイルに格納するため
//		DataArrayDataにする
//
//	NOTES
//
//	ARGUMENTS
//		int iMemberID_
//			データベース表のフィールドに対応する数値
//
//	RETURN
//		0以外...正しく変換された
//		0    ...変換に失敗した
//
//	EXCEPTIONS

Common::Data::Pointer
File::
packMetaField(int iMemberID_) const
{
	Meta::Definition<File>& cDef = _vecDefinition[iMemberID_];

	if (cDef.m_eType < Meta::MemberType::UseObjectMax) {
		// Objectの情報を使う
		return Object::packMetaField(iMemberID_);
	}

	switch (cDef.m_eType) {
	case Meta::MemberType::ID:
		{
			return pack((this->*(cDef.m_funcGet._id))());
		}
	case Meta::MemberType::Integer:
		{
			; _SYDNEY_ASSERT(iMemberID_ == Meta::File::Category);
			return pack(static_cast<int>(getCategory()));
		}
	case Meta::MemberType::IDArray:
		{
			; _SYDNEY_ASSERT(iMemberID_ == Meta::File::AreaIDs);
			ModVector<ID::Value> vecID;
			vecID.reserve(2);
			if (_areaID != ID::Invalid || _logAreaID != ID::Invalid) {
				vecID.pushBack(_areaID);
				if (_logAreaID != ID::Invalid)
					vecID.pushBack(_logAreaID);
			}
			return pack(vecID);
		}
	case Meta::MemberType::Binary:
		{
			return packBinaryMetaField(iMemberID_);
		}
	default:
		// これ以外の型はないはず
		break;
	}
	; _SYDNEY_ASSERT(false);
	return Common::Data::Pointer();
}

//	FUNCTION private
//	Schema::File::unpackMetaField --
//		DataArrayDataをスキーマオブジェクトの内容に反映させる
//
//	NOTES
//
//	ARGUMENTS
//		const Common::Data* pData_
//			内容を反映するData
//		int iMemberID_
//			データベース表のフィールドに対応する数値
//
//	RETURN
//		true...正しく変換された
//		false..変換に失敗した
//
//	EXCEPTIONS

bool
File::
unpackMetaField(const Common::Data* pData_, int iMemberID_)
{
	Meta::Definition<File>& cDef = _vecDefinition[iMemberID_];

	if (cDef.m_eType < Meta::MemberType::UseObjectMax) {
		// Objectの情報を使う
		return Object::unpackMetaField(pData_, iMemberID_);
	}

	switch (cDef.m_eType) {
	case Meta::MemberType::ID:
		{
			ID::Value id;
			if (unpack(pData_, id)) {
				(this->*(cDef.m_funcSet._id))(id);
				return true;
			}
			break;
		}
	case Meta::MemberType::Integer:
		{
			; _SYDNEY_ASSERT(iMemberID_ == Meta::File::Category);
			int value;
			if (unpack(pData_, value)) {
				if (value >= 0 && value < Category::ValueNum) {
					_category = static_cast<Category::Value>(value);
					return true;
				}
			}
			break;
		}
	case Meta::MemberType::IDArray:
		{
			; _SYDNEY_ASSERT(iMemberID_ == Meta::File::AreaIDs);
			ModVector<ID::Value> vecID;
			if (unpack(pData_, vecID)) {
				if (vecID.getSize() > 0)
					_areaID = vecID[0];
				if (vecID.getSize() > 1)
					_logAreaID = vecID[1];
				return true;
			}
			break;
		}
	case Meta::MemberType::Binary:
		{
			return unpackBinaryMetaField(pData_, iMemberID_);
		}
	default:
		break;
	}
	return false;
}

Common::Data::Pointer
File::
packBinaryMetaField(int iMemberID_) const
{
	Utility::BinaryData& cArchiver = getArchiver();
	switch (iMemberID_) {
	case Meta::File::FileID:
		{
			// FileIDのうちパス指定の部分はunpack時に上書きされるので空にしておく
			ModVector<ModUnicodeString> vecPath;
			ModUnicodeString cstrPath;
			ModSize n = 0;
			while (m_pFileID->getString(_SYDNEY_SCHEMA_FORMAT_KEY(
											  FileCommon::FileOption::Area::Key, n),
										cstrPath)) {
				vecPath.pushBack(cstrPath);
				m_pFileID->setString(_SYDNEY_SCHEMA_FORMAT_KEY(
										   FileCommon::FileOption::Area::Key, n),
									 ModUnicodeString());
				++n;
			}

			Common::Data::Pointer result = getArchiver().put(m_pFileID);

			// パス指定の部分を元に戻す
			for (ModSize i = 0; i < n; ++i) {
				m_pFileID->setString(_SYDNEY_SCHEMA_FORMAT_KEY(
										   FileCommon::FileOption::Area::Key, i),
									 vecPath[i]);
			}
			return result;
		}
	case Meta::File::Hint:
		{
			return cArchiver.put(getHint());
		}
	case Meta::File::AreaHint:
		{
			return cArchiver.put(getAreaHint());
		}
	case Meta::File::Option:
		{
			return cArchiver.put(packOption().get());
		}
	default:
		break;
	}
	; _SYDNEY_ASSERT(false);
	return Common::Data::Pointer();
}

bool
File::
unpackBinaryMetaField(const Common::Data* pData_, int iMemberID_)
{
	if (pData_ && pData_->isNull()) {
		// Nullが許されるのはFileID以外
		return (iMemberID_ != Meta::File::FileID);

	} else if (pData_ && pData_->getType() == Common::DataType::Binary) {
		const Common::BinaryData* pBinary =
			_SYDNEY_DYNAMIC_CAST(const Common::BinaryData*, pData_);

		Utility::BinaryData& cArchiver = getArchiver();

		switch (iMemberID_) {
		case Meta::File::FileID:
			{
				ModAutoPointer<Common::Externalizable> pData = cArchiver.get(pBinary);

				if (LogicalFile::FileID* pFileID = dynamic_cast<LogicalFile::FileID*>(pData.get())) {
					m_pFileID = pFileID;
					pData.release();
					return true;
				}
				break;
			}
		case Meta::File::Hint:
			{
				ModAutoPointer<Hint> pData =
					dynamic_cast<Hint*>(cArchiver.get(pBinary));
				if (pData.get())
					m_pHint = pData.release();
				return true;
			}
		case Meta::File::AreaHint:
			{
				ModAutoPointer<Hint> pData =
					dynamic_cast<Hint*>(cArchiver.get(pBinary));
				if (pData.get())
					m_pAreaHint = pData.release();
				return true;
			}
		case Meta::File::Option:
			{
				ModAutoPointer<Common::Data> pData =
					dynamic_cast<Common::Data*>(cArchiver.get(pBinary));
				if (pData.get())
					unpackOption(*pData);
				return true;
			}
		default:
			break;
		}
	}
	return false;
}

//	FUNCTION protected
//	Schema::File::packOption --
//		サブクラス固有の付加情報の内容をレコードファイルに格納するために
//		Dataにする
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//		Data*
//			変換されたデータ。呼び出し側でdeleteをする必要がある
//
//	EXCEPTIONS

Common::Data::Pointer
File::
packOption() const
{
	// サブクラスでオーバーライドする
	return Common::Data::Pointer();
}

//	FUNCTION protected
//	Schema::File::unpackOption --
//		Dataをサブクラス固有の付加情報に反映させる
//
//	NOTES
//
//	ARGUMENTS
//		const Data& cData_
//
//	RETURN
//	なし
//
//	EXCEPTIONS

void
File::
unpackOption(const Common::Data& cData_)
{
	// サブクラスでオーバーライドする
	;
}

//
// Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
