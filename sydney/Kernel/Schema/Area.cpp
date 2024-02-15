// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Area.cpp -- エリア関連の関数定義
// 
// Copyright (c) 2001, 2002, 2004, 2005, 2006, 2007, 2012, 2013, 2023 Ricoh Company, Ltd.
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
#include "Schema/AreaMap.h"
#include "Schema/AreaContent.h"
#include "Schema/AreaContentMap.h"
#include "Schema/AutoRWLock.h"
#include "Schema/Database.h"
#include "Schema/FakeError.h"
#include "Schema/File.h"
#include "Schema/LogData.h"
#include "Schema/Manager.h"
#include "Schema/Message.h"
#include "Schema/Meta.h"
#include "Schema/ObjectID.h"
#include "Schema/Parameter.h"
#include "Schema/Recovery.h"
#include "Schema/SystemTable_Area.h"
#include "Schema/SystemTable_AreaContent.h"
#include "Schema/Table.h"
#include "Schema/Utility.h"
#ifdef DEBUG
#include "Schema/Debug.h"
#endif

#include "Common/Assert.h"
#include "Common/DataArrayData.h"
#include "Common/InputArchive.h"
#include "Common/Message.h"
#include "Common/NullData.h"
#include "Common/OutputArchive.h"
#include "Common/StringData.h"
#include "Common/StringArrayData.h"
#include "Common/UnicodeString.h"

#include "Exception/AreaAlreadyDefined.h"
#include "Exception/AreaNotFound.h"
#include "Exception/BadArrayElement.h"
#include "Exception/DatabaseNotFound.h"
#include "Exception/FileNotFound.h"
#include "Exception/FileAlreadyExisted.h"
#include "Exception/InvalidPath.h"
#include "Exception/LogItemCorrupted.h"
#include "Exception/MetaDatabaseCorrupted.h"
#include "Exception/OtherObjectDepending.h"
#include "Exception/SchemaObjectNotFound.h"

#include "LogicalFile/FileID.h"
#include "LogicalFile/ObjectID.h"

#include "Os/File.h"
#include "Os/Path.h"

#include "FileCommon/FileOption.h"

#include "Lock/Mode.h"

#include "Statement/AlterAreaAction.h"
#include "Statement/AlterAreaStatement.h"
#include "Statement/AreaDataDefinition.h"
#include "Statement/AreaDefinition.h"
#include "Statement/AreaElementList.h"
#include "Statement/DropAreaStatement.h"
#include "Statement/Identifier.h"
#include "Statement/Literal.h"
#include "Statement/Type.h"

#include "Trans/LogFile.h"
#include "Trans/TimeStamp.h"
#include "Trans/Transaction.h"

#include "ModAutoPointer.h"
#include "ModOsDriver.h"
#include "ModUnicodeOstrStream.h"

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

namespace
{
	namespace _Name
	{
		// エリア名の重複を調べる
		bool _checkExistence(Trans::Transaction& cTrans_, const Database& cDatabase_, const Area* pArea_);
	} // namespace _Name

	namespace _Path
	{
		// パスを絶対パスに変更する
		ModUnicodeString _getFullPathName(const ModUnicodeString& cstrPath_);
		// パス名が他のデータベースやエリアで使用されているか
		bool _isUsedInOthers(Trans::Transaction& cTrans_, const Os::Path& cPath_, const Area* pArea_);
		// Alterで変更されるパス名の存在を調べる
		bool _checkExistence(Trans::Transaction& cTrans_, const Area* pArea_,
							 const ModVector<ModUnicodeString>* pvecPath_,
							 bool bEraseExistence_, bool bNeedExistence_);
	} // namespace _Path
} // namespace

//////////////////////////
// _Name
//////////////////////////

//	FUNCTION local
//	_Name::_checkExistence -- エリア名の重複を調べる
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		const Schema::Database& cDatabase_
//			エリアが属するデータベース
//		const Schema::Area* pArea_
//			作成しようとしているエリア
//
//	RETURN
//		true ... 同じ名前のものが存在している、または作成中である
//		false... 同じ名前のものはない
//
//	EXCEPTIONS
//		Exception::AreaAlreadyDefined
//			同じ名前のものが存在しており、CanceledWhenDuplicatedがfalseである

bool
_Name::_checkExistence(Trans::Transaction& cTrans_,
					   const Database& cDatabase_,
					   const Area* pArea_)
{
	// 作成中のオブジェクトとして登録し、同じ名前のものが登録中でないか調べる
	if (Manager::ObjectName::reserve(pArea_) == false) {

		if (Manager::Configuration::isCanceledWhenDuplicated()) {
			// trueを返し後の処理は何もしない
			SydInfoMessage
				<< "Area definition of the same name in progress("
				<< pArea_->getName()
				<< ") canceled"
				<< ModEndl;
			return true;
		} else {
			// 例外を送出
			SydInfoMessage
				<< "Area definition of the same name in progress("
				<< pArea_->getName()
				<< ")"
				<< ModEndl;
			_SYDNEY_THROW2(Exception::AreaAlreadyDefined, pArea_->getName(), cDatabase_.getName());
		}
	}

	// さらに、同じ名前のエリアがすでにないか調べ、
	// 同時に現在のエリアをマネージャーに読み込んでおく
	// ★注意★
	// doAfterPersistの中でマネージャーに追加されるので
	// ここで読み込んでおかないと追加のときに不完全なAreaを
	// 読み込んでしまう

	bool bFound = false;
	try {
		bFound = (cDatabase_.getArea(pArea_->getName(), cTrans_) != 0);
	} catch (...) {
		Manager::ObjectName::withdraw(pArea_);
		_SYDNEY_RETHROW;
	}
	if (bFound) {

		// 作成中の登録からオブジェクトを外す
		Manager::ObjectName::withdraw(pArea_);

		if (Manager::Configuration::isCanceledWhenDuplicated()) {
			// 0を返し後の処理は何もしない
			SydInfoMessage << "Duplicated area definition("
						   << pArea_->getName()
						   << ") canceled"
						   << ModEndl;
			return true;
		} else {
			// 例外を送出
			SydInfoMessage << "Duplicated area definition("
						   << pArea_->getName()
						   << ")"
						   << ModEndl;
			_SYDNEY_THROW2(Exception::AreaAlreadyDefined, pArea_->getName(), cDatabase_.getName());
		}
	}
	return false;
}

//////////////////////////
// _Path
//////////////////////////

// FUNCTION public
//	$$$::_Path::_getFullPathName -- パスを絶対パスに変更する
//
// NOTES
//
// ARGUMENTS
//	const ModUnicodeString& cstrPath_
//	
// RETURN
//	ModUnicodeString
//
// EXCEPTIONS

ModUnicodeString
_Path::
_getFullPathName(const ModUnicodeString& cstrPath_)
{
	ModUnicodeString cstrResult;
	if (cstrPath_.getLength() > 0) {
		if (ModOsDriver::File::isFullPathName(cstrPath_)) {
			return cstrPath_;
		} else {
			ModOsDriver::File::getFullPathName(Manager::Configuration::getDefaultAreaPath(),
											   cstrPath_,
											   cstrResult);
		}
	}
	return cstrResult;
}

//	FUNCTION local
//	_Path::_isUsedInOthers -- パス名が他のデータベースやエリアで使用されているか
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		const Os::Path& cPath_
//			パス名
//		const Area* pArea_
//			対象のエリア
//
//	RETURN
//		true .. ディレクトリーが他のデータベースやエリアで使用されている
//		false.. ディレクトリーが他のデータベースやエリアで使用されていない
//
//	EXCEPTIONS

bool
_Path::_isUsedInOthers(Trans::Transaction& cTrans_, const Os::Path& cPath_, const Area* pArea_)
{
	const ModVector<Database*>& vecDatabase = Manager::ObjectTree::Database::get(cTrans_);
	ModVector<Database*>::ConstIterator iterator = vecDatabase.begin();
	const ModVector<Database*>::ConstIterator& end = vecDatabase.end();
	for (; iterator != end; ++iterator) {
		// 永続でないデータベースとは比較しない
		if ((*iterator)->getScope() != Object::Scope::Permanent) continue;

		// データベースおよびそれに属するエリアについて調べる
		if ((*iterator)->checkRelatedPath(cTrans_, cPath_, pArea_ /* omit this */))
			return true;
	}
	return false;
}

//	FUNCTION local
//	_Path::_checkExistence -- CreateまたはAlterで変更されるパス名の存在を調べる
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		const Area* pArea_
//			エリアオブジェクト
//		const ModVector<ModUnicodeString>* pvecPath_
//			パス指定配列へのポインター
//		bool bEraseExistence_
//			trueのときすでに存在していたら削除してしまう
//			(redo用)
//		bool bNeedExistence_ = false
//			trueのときパスが存在していなかったらエラー
//			(mount用)
//
//	RETURN
//		true .. 同じパス名のディレクトリーが存在する
//		false.. 同じパス名のディレクトリーは存在しない
//
//	EXCEPTIONS

bool
_Path::_checkExistence(Trans::Transaction& cTrans_,
					   const Area* pArea_,
					   const ModVector<ModUnicodeString>* pvecPath_,
					   bool bEraseExistence_,
					   bool bNeedExistence_)
{
	const ModVector<ModUnicodeString>& vecPath = pvecPath_ ? *pvecPath_ : pArea_->getPath();
	int nPath = vecPath.getSize();

	// 検査対象のパスを入れる
	ModVector<ModUnicodeString> vecCheckedPath;
	vecCheckedPath.reserve(nPath);

	int i = 0;
	for (; i < nPath; ++i) {

		// 変更されていなければalterで対象外のパスなので調べる必要はない
		// ★注意★
		// 同じパスをさす文字列をわざわず指定することもありえるので
		// Os::Path::compareを使う
		if (pvecPath_ && Os::Path::compare(vecPath[i], pArea_->getPath(i))
			== Os::Path::CompareResult::Identical)
			continue;

		if (vecPath[i].getLength() > Manager::ObjectPath::getMaxLength()) {
			// パス名が制限長を超えていたらログに吐いて継続
			SydInfoMessage << "Warning: area path length (" << vecPath[i].getLength()
						   << ") exceeds the limit(" << Manager::ObjectPath::getMaxLength() << "),"
						   << " creating table with long name may be failed." << ModEndl;
		}

		if (i > 0) {
			// 他のパスと同一であれば調べる必要はない
			int n = vecCheckedPath.getSize();
			int j = 0;
			for (; j < n; ++j) {
				if (Os::Path::compare(vecPath[i], vecCheckedPath[j]) == Os::Path::CompareResult::Identical) {
					break;
				}
			}
			if (j < n) {
				// 同一のものを見つけたので次へ
				continue;
			}
		}
		vecCheckedPath.pushBack(vecPath[i]);
	}

	// 移動後のパス配列を使用するパスとして登録すると同時に
	// 他に作成または移動中のエリアで関連のあるパスを使用するものがないか調べる
	if (!Manager::ObjectPath::reserve(pArea_, vecCheckedPath)) {
		// 登録中のパスに関係するものがあった
		SydInfoMessage << "New area path already used in others." << ModEndl;
		return true;
	}

	nPath = vecCheckedPath.getSize();
	i = 0;
	for (; i < nPath; ++i) {

		Os::Path cPath(vecCheckedPath[i]);

		if (Utility::File::isFound(cPath)) {
			// ディレクトリーが存在する
			// データベースの場合と異なりMOUNT中でない限り無条件にエラーとする

			if (bNeedExistence_) {
				// MOUNT中なのでOK

			} else if (bEraseExistence_) {
				// エラーにしないで削除してしまう
				Utility::File::rmAll(cPath);

			} else {
				// 例外送出
				SydInfoMessage << "New area path: " << cPath << " already exists." << ModEndl;
				return true;
			}
		} else {
			// ディレクトリーが存在しない
			// MOUNT中ならエラー
			if (bNeedExistence_) {
				SydInfoMessage << "Area path specified in MOUNT must be created." << ModEndl;
				_SYDNEY_THROW1(Exception::InvalidPath, pArea_->getName());
			}
		}
		if (_isUsedInOthers(cTrans_, cPath, pArea_)) {
			// 他のデータベースやエリアで使用されている
			SydInfoMessage << "New area path: " << cPath << " already used in others." << ModEndl;
			return true;
		}
	}
	if (bNeedExistence_)
		// mount中はファイルの存在でチェックできるので登録からすぐはずす
		Manager::ObjectPath::withdraw(pArea_);

	return false; // 重複はない
}

///////////////////
// Schema::Area	 //
///////////////////

//	FUNCTION public
//		Schema::Area::Area -- コンストラクター
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
//		???
//

Area::
Area()
	: Object(Object::Category::Area),
	  m_pPathArray(0), _contents(0)
{
}

//
//	FUNCTION public
//		Schema::Area::Area -- コンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		const Database& cDatabase_
//			エリアを定義するデータベース
//		const Statement::AreaDefinition& cStatement_
//			エリア定義のステートメント
//
//	RETURN
//		なし
//
//	EXCEPTIONS

Area::
Area(const Database& cDatabase_, const Statement::AreaDefinition& cStatement_)
	: Object(Object::Category::Area, cDatabase_.getScope(),
			 Object::Status::Unknown,
			 ID::Invalid, cDatabase_.getID(), cDatabase_.getID()),
	  m_pPathArray(0), _contents(0)
{
	// エリア定義のSQL文からエリアオブジェクトを作る
	// create area <area名> array [<path>, ...]

	// 渡されたStatement::Objectの中身は正しいはずである

	const Statement::Identifier* identifier = cStatement_.getName();
	_SYDNEY_ASSERT(identifier);
	_SYDNEY_ASSERT(identifier->getIdentifier());
	_SYDNEY_ASSERT(identifier->getIdentifier()->getLength());

	setName(*identifier->getIdentifier());

	const Statement::AreaElementList* pElements = cStatement_.getElements();
	;_SYDNEY_ASSERT(pElements);

	// SQL構文要素からパス名配列を作る

	setPath(*pElements);
}

//
//	FUNCTION public
//		Schema::Area::Area -- コンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		const Database& cDatabase_
//			エリアを定義するデータベース
//		const Object::Name&
//			エリア名
//		const ModVector<ModUnicodeString>& cPathArray_
//			エリア名に対応するパス配列
//
//	RETURN
//		なし
//
//	EXCEPTIONS

Area::
Area(const Database& cDatabase_, const Object::Name& cAreaName_,
	 const ModVector<ModUnicodeString>& cPathArray_)
	: Object(Object::Category::Area, cDatabase_.getScope(),
			 Object::Status::Unknown,
			 ID::Invalid, cDatabase_.getID(), cDatabase_.getID()),
	  m_pPathArray(0), _contents(0)
{
	// エリア名の設定
	setName(cAreaName_);

	// パス名配列の設定
	setPath(cPathArray_);
}

//
//	FUNCTION public
//		Schema::Area::Area -- コンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		const Database& cDatabase_
//			エリアを定義するデータベース
//		const Schema::LogData& cLogData_
//			エリア定義のログデータ
//
//	RETURN
//		なし
//
//	EXCEPTIONS

Area::
Area(const Database& cDatabase_, const LogData& cLogData_)
	: Object(Object::Category::Area, cDatabase_.getScope(),
			 Object::Status::Unknown,
			 ID::Invalid, cDatabase_.getID(), cDatabase_.getID()),
	  m_pPathArray(0), _contents(0)
{
	// ログの内容を反映する
	//	ログの内容:
	//		1．名前
	//		2. ID(使用しない)
	//		3．パス
	// ★注意★
	//	makeLogDataの実装を変えたらここも変える

	setName(cLogData_.getString(Log::Name));
	setPath(cLogData_.getStrings(Log::Path));
}

//
//	FUNCTION public
//		Schema::Area::~Area -- デストラクター
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
//		???
//

Area::
~Area()
{
	destruct();
}

//	FUNCTION public
//		Schema::Area::getNewInstance -- オブジェクトを新たに取得する
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
Area*
Area::
getNewInstance(const Common::DataArrayData& cData_)
{
	ModAutoPointer<Area> pObject = new Area;
	pObject->unpack(cData_);
	return pObject.release();
}

//
//	FUNCTION public
//		Schema::Area::create -- エリアを生成する
//
//	NOTES
//		システム表の永続化は呼び出し側で行う
//
//	ARGUMENTS
//		Database& cDatabase_
//			エリアを定義するデータベース
//		const Statement::AreaDefinition& cStatement_
//			エリア定義のステートメント
//		Schema::LogData& cLogData_
//			ログに出力するデータを格納する変数
//		Trans::Transaction& cTrans_
//			エリア抹消を行うトランザクション記述子
//
//	RETURN
//		Schema::Area*
//			生成したエリアオブジェクト
//
//	EXCEPTIONS
//		Exception::AreaAlreadyDefined
//			存在する名前でcreateしようとした

// static
Area*
Area::
create(Database& cDatabase_, const Statement::AreaDefinition& cStatement_,
	   LogData& cLogData_, Trans::Transaction& cTrans_)
{
	; _SYDNEY_ASSERT(cTrans_.getCategory()
					 != Trans::Transaction::Category::ReadOnly);

	// 定義からエリアオブジェクトを生成する
	ModAutoPointer<Area> pArea = new Area(cDatabase_, cStatement_);
	; _SYDNEY_ASSERT(pArea.get());

	// 作成中のオブジェクトとして登録し、同じ名前のものが登録中でないか調べる
	if (_Name::_checkExistence(cTrans_, cDatabase_, pArea.get())) {
		return 0;
	}

	// IDをふり、状態を変える
	pArea->Object::create(cTrans_);

	// ログデータを作る
	pArea->makeLogData(cLogData_);

	// 生成されたエリアオブジェクトを返す
    return pArea.release();
}

//
//	FUNCTION public
//		Schema::Area::create -- エリアを生成する
//
//	NOTES
//		リカバリー中のredoで呼ばれる
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			エリア定義を行うトランザクション記述子
//		const Schema::Database& cDatabase_
//			エリアを定義するデータベース
//		const Schema::LogData& cLogData_
//			エリア定義のログデータ
//
//	RETURN
//		Schema::Area*
//			生成したエリアオブジェクト
//
//	EXCEPTIONS
//		Exception::AreaAlreadyDefined
//			存在する名前でcreateしようとした

// static
Area*
Area::
create(Trans::Transaction& cTrans_, const Database& cDatabase_,
	   const LogData& cLogData_)
{
	// ログデータからエリアオブジェクトを生成する

	ModAutoPointer<Area> pArea = new Area(cDatabase_, cLogData_);
	; _SYDNEY_ASSERT(pArea.get());

	// 作成中のオブジェクトとして登録し、同じ名前のものが登録中でないか調べる
	if (_Name::_checkExistence(cTrans_, cDatabase_, pArea.get())) {
		return 0;
	}

	// AlterがUNDOされていないか調べる
	// IDはログのものを使用する
	Object::ID::Value id = getObjectID(cLogData_);

	// Alter後のパスが登録されているときはログデータのパスではなく
	// Alter後のパスを使用する
	pArea->checkUndo(cDatabase_, id);

	// IDの整合性を取り、状態を変える
	pArea->Object::create(cTrans_, id);

	// 生成されたエリアオブジェクトを返す
    return pArea.release();
}

//	FUNCTION public
//		Schema::Area::createForMount -- エリアを生成する
//
//	NOTES
//		Mountのエラー処理で呼ばれる
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			エリア定義を行うトランザクション記述子
//		Schema::Database& cDatabase_
//			エリアが属するデータベースオブジェクト
//		Schema::Object::ID::Value iID_
//			エラー処理前のエリアID(Redo処理用)
//		const Schema::Object::Name& cName_
//			エリアの名称
//		const ModVector<ModUnicodeString>& vecPath_
//			エリアのパス
//
//	RETURN
//		Schema::Area*
//			生成したエリアオブジェクト
//
//	EXCEPTIONS
//		Exception::AreaAlreadyDefined
//			存在する名前でcreateしようとした

//static
Area*
Area::
createForMount(Trans::Transaction& cTrans_,
			   Database& cDatabase_,
			   ID::Value iID_,
			   const Name& cName_,
			   const ModVector<ModUnicodeString>& vecPath_)
{
	// エリアオブジェクトを生成する

	ModAutoPointer<Area> pArea = new Area(cDatabase_, cName_, vecPath_);
	; _SYDNEY_ASSERT(pArea.get());

	// MOUNT中に呼ばれる場合はデータベースオブジェクトをセットする必要がある
	pArea->setDatabase(&cDatabase_);

	// IDをふり、状態を変える
	pArea->Object::create(cTrans_, iID_);

	// 生成されたエリアオブジェクトを返す
    return pArea.release();
}

#ifdef OBSOLETE
//
//	FUNCTION public
//		Schema::Area::create -- エリアを実際に生成する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			エリア定義を行うトランザクション記述子
//		const Schema::Database& cDatabase_
//			エリアを定義するデータベース
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Area::
create(Trans::Transaction& cTrans_)
{
    ModSize n = getSize();

	// エラー発生時に自動的にmkdirを取り消すクラス
    ModVector<Utility::File::AutoRmDir> vecAutoRmDir(n);

	// エリアのあらわすパスを実際に作る
	ModSize i = 0;
	for (; i < n; i++) {
		vecAutoRmDir[i].setDir(getPath(i));
	}
	// すべて成功したのでその旨記録する
	// disable()は例外を投げない
	for (i = 0; i < n; i++) {
		vecAutoRmDir[i].disable();
	}
}
#endif

//
//	FUNCTION public
//		Schema::Area::getName -- エリア抹消のSQL文からエリア名を得る
//
//	NOTES
//
//	ARGUMENTS
//		const Statement::DropAreaStatement& cStatement_
//			エリア抹消のステートメント
//
//	RETURN
//		対象のエリア名
//
//	EXCEPTIONS

// static
Object::Name
Area::
getName(const Statement::DropAreaStatement& cStatement_)
{
	const ModUnicodeString* pName = cStatement_.getNameString();
	; _SYDNEY_ASSERT(pName);

	return *pName;
}

//
//	FUNCTION public
//		Schema::Area::getName -- エリア変更のSQL文からエリア名を得る
//
//	NOTES
//
//	ARGUMENTS
//		const Statement::AlterAreaStatement& cStatement_
//			エリア変更のステートメント
//
//	RETURN
//		対象のエリア名
//
//	EXCEPTIONS

// static
Object::Name
Area::
getName(const Statement::AlterAreaStatement& cStatement_)
{
	const Statement::Identifier* pIdentifier = cStatement_.getAreaName();
	; _SYDNEY_ASSERT(pIdentifier);
	; _SYDNEY_ASSERT(pIdentifier->getIdentifier());

	const ModUnicodeString* pName = pIdentifier->getIdentifier();
	; _SYDNEY_ASSERT(pName);

	return *pName;
}

//
//	FUNCTION public
//		Schema::Area::getName -- 
//
//	NOTES
//		getNameをオーバーライドしたので
//		Objectのものを再度定義する
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		エリア名
//
//	EXCEPTIONS

const Object::Name&
Area::
getName() const
{
	return Object::getName();
}

//	FUNCTION public
//		Schema::Area::drop -- エリアの破棄に関する処理をする
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Area& cArea_
//			破棄対象のエリア
//		Scheam::LogData& cLogData_
//			ログに出力するデータを格納する変数
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//
//	RETURN
//		なし
//
//	EXCEPTIONS

//static
void
Area::
drop(Area& cArea_, LogData& cLogData_,
	 Trans::Transaction& cTrans_)
{
	// 破棄マークをつける
	cArea_.drop(cTrans_);
	// ログデータを作る
	cArea_.makeLogData(cLogData_);
}

//
//	FUNCTION public
//		Schema::Area::drop -- エリアを実際に抹消する
//
//	NOTES
//		システム表のXロックは呼び出し側で行う必要がある
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		bool bRecovery_ = false
//			リカバリー処理でのDROPか
//		bool bCheck_ = true
//			trueならエリアと関係づけられているオブジェクトがないか調べる
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		???
//

void
Area::
drop(Trans::Transaction& cTrans_, bool bRecovery_/* = false */, bool bCheck_ /* = true */)
{
	// エリアに関係するスキーマオブジェクトを調べる
	if (bCheck_ && loadContent(cTrans_).isEmpty() == ModFalse) {

		// このエリアを格納場所にするスキーマオブジェクトがあるので例外送出
		SydInfoMessage << "Can't drop area " << getName()
					   << ". Area is not empty." << ModEndl;
#ifdef DEBUG
		const AreaContentMap& contents = loadContent(cTrans_);
		AreaContentMap::ConstIterator iterator = contents.begin();
		const AreaContentMap::ConstIterator& end = contents.end();

		SydInfoMessage << "Remaining content is";
		for (; iterator != end; ++iterator) {
			const AreaContent::Pointer& pContent = AreaContentMap::getValue(iterator);
			SydInfoMessage << " (ObjectID=" << pContent->getObjectID()
						   << ", ObjectCategory=" << pContent->getObjectCategory()
						   << ", AreaCategory=" << pContent->getAreaCategory()
						   << ")";
		}
		SydInfoMessage << ModEndl;
#endif
		_SYDNEY_THROW1(Exception::OtherObjectDepending, getName());
	}

	// 状態を変化させる
	Object::drop(bRecovery_);
}

//	FUNCTION public
//		Schema::Area::dropForMount -- Mount処理の中でエリアの定義を抹消する
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
Area::
dropForMount(Trans::Transaction& cTrans_)
{
	// 抹消マークをつける
	// ★注意★
	// ファイルを実際に削除することはないので
	// リカバリーで実行するDROPと同じ処理をする
	// また、使っているファイルがあるかのチェックも省く
	drop(cTrans_, true /* in recovery */, false /* no check */);
}

//	FUNCTION public
//		Schema::Area::alter -- エリア変更の準備をする
//
//	NOTES
//		システム表の永続化は呼び出し側で行う
//		この関数ではファイルの移動は行われない
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			エリア変更を行うトランザクション記述子
//		Schema::Area& cArea_
//			変更対象のエリアオブジェクト
//		const Statement::AlterAreaStatement& cStatement_
//			エリア変更のSQL文を表すStatementオブジェクト
//	 	ModVector<ModUnicodeString>& vecPrevPath_
//			変更前のパスを入れる配列
//		ModVector<ModUnicodeString>& vecPostPath_
//			変更後のパスを入れる配列
//		Schema::LogData& cLogData_
//			ログに出力するデータを格納する変数
//
//	RETURN
//		true ... 変更の必要がある
//		false... 変更の必要がない
//
//	EXCEPTIONS

// static
bool
Area::
alter(Trans::Transaction& cTrans_,
	  Schema::Area& cArea_,
	  const Statement::AlterAreaStatement& cStatement_,
  	  ModVector<ModUnicodeString>& vecPrevPath_,
	  ModVector<ModUnicodeString>& vecPostPath_,
	  LogData& cLogData_)
{
	; _SYDNEY_ASSERT(cTrans_.getCategory()
					 != Trans::Transaction::Category::ReadOnly);

	const Statement::AlterAreaAction* pAreaAction
		= cStatement_.getAlterAreaAction();
	; _SYDNEY_ASSERT(pAreaAction);

	const Statement::AreaElementList*
		pElements = pAreaAction->getAreaElementList();
	; _SYDNEY_ASSERT(pElements);

	//--- 変更前後のパスを取得する ---

	bool bResult = cArea_.setMovePrepare(pAreaAction->getActionType(), *pElements,
										 vecPrevPath_, vecPostPath_);
	if (bResult) {
		// ログデータを作る
		cArea_.makeLogData(cLogData_);
		// 変更前後のパスを追加する
		cLogData_.addStrings(vecPrevPath_);
		cLogData_.addStrings(vecPostPath_);
		; _SYDNEY_ASSERT(cLogData_.getCount() == Log::Alter::Num);
	}
	return bResult;
}

//
//	FUNCTION public
//		Schema::Area::moveForMount -- エリアの定義を変更する（MOUNT 用）
//
//	NOTES
//		エリアオブジェクトの内容を書き換えるだけで
//		ファイルの移動は伴わない
//		システム表の永続化は呼び出し側で行う
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			エリア変更を行うトランザクション記述子
//		const ModVector<ModUnicodeString>& vecPrevPath_
//			変更前のパス
//		const ModVector<ModUnicodeString>& vecPostPath_
//			変更後のパス
//		bool bUndo_ = false
//			trueなら取り消し処理中なので重ねてのエラー処理はいらない
//		bool bRecovery_ = false
//			trueならリカバリー中の処理である
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		Exception::InvalidPath
//			変更するパス配列の内容が正しくない

void
Area::
moveForMount(Trans::Transaction& cTrans_,
			 const ModVector<ModUnicodeString>& vecPrevPath_,
			 const ModVector<ModUnicodeString>& vecPostPath_,
			 bool bUndo_ /* = false */, bool bRecovery_ /* = false */)
{
	move(cTrans_, vecPrevPath_, vecPostPath_, bUndo_, bRecovery_, true /* mount */);
	// 状態を変更する
	if (bUndo_ && getStatus() != Status::Persistent)
		untouch();
	else
		touch();
}

//
//	FUNCTION public
//		Schema::Area::move -- エリアを変更する
//
//	NOTES
//		システム表の永続化は呼び出し側で行う
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			エリア変更を行うトランザクション記述子
//		const ModVector<ModUnicodeString>& vecPrevPath_
//			変更前のパス
//		const ModVector<ModUnicodeString>& vecPostPath_
//			変更後のパス
//		bool bUndo_ = false
//			trueの場合エラー処理中なので重ねてのエラー処理はしない
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		Exception::InvalidPath
//			変更するパス配列の内容が正しくない

void
Area::
move(Trans::Transaction& cTrans_,
	 const ModVector<ModUnicodeString>& vecPrevPath_,
	 const ModVector<ModUnicodeString>& vecPostPath_,
	 bool bUndo_, bool bRecovery_, bool bMount_)
{
	; _SYDNEY_ASSERT(vecPrevPath_.getSize() == vecPostPath_.getSize());

	enum {
		None,							// 初期値
		Prepared,						// 移動の準備ができた
		Moved,							// ファイルを移動した
		Sweeped,						// 移動の後始末をした
		ValueNum
	} eStatus = None;

	try {
		// マウント時およびREDO時にはファイルの移動を伴う処理はしない
#ifdef OBSOLETE
		if (!bMount_ && !bRecovery_) {
			// 移動先のディレクトリーを作成する
			prepareMove(cTrans_, vecPrevPath_, vecPostPath_, bUndo_);
		}
#endif
		eStatus = Prepared;
		SCHEMA_FAKE_ERROR("Schema::Area", "Move", "Prepared");

		// エリアに関係しているオブジェクトごとにそれを構成するファイルを
		// 新しい格納場所に移動する
		// 同時にエリアが保持するパスも新しい場所にする
		moveFile(cTrans_, vecPrevPath_, vecPostPath_, bUndo_, bRecovery_, bMount_);
		eStatus = Moved;
		SCHEMA_FAKE_ERROR("Schema::Area", "Move", "Moved");

		// マウント時およびREDO時にはファイルの移動を伴う処理はしない
		if (!bMount_ && !bRecovery_) {
			// 移動もとのディレクトリーを破棄する
			sweepMove(cTrans_, vecPrevPath_, vecPostPath_, bUndo_);
			eStatus = Sweeped;
		}
		SCHEMA_FAKE_ERROR("Schema::Area", "Move", "Sweeped");

	} catch (...) {

		_BEGIN_REORGANIZE_RECOVERY;

		switch (eStatus) {
		case Sweeped:
#ifdef OBSOLETE
			{
				if (!bMount_ && !bRecovery_) {
					// 移動前ディレクトリーを再び作成する
					prepareMove(cTrans_, vecPostPath_, vecPrevPath_, true /* undo */);
				}
				// thru.
			}
#endif
		case Moved:
			{
				// 移動前に移動させる
				// エリアが保持するパスの内容も移動前に戻る
				moveFile(cTrans_, vecPostPath_, vecPrevPath_, true /* undo */, bRecovery_, bMount_);
				// thru.
			}
		case Prepared:
			{
				if (!bMount_ && !bRecovery_) {
					// 移動先ディレクトリーを破棄する
					sweepMove(cTrans_, vecPostPath_, vecPrevPath_, true /* undo */);
				}
				// thru.
			}
		case None:
		default:
			break;
		}

		_END_REORGANIZE_RECOVERY;

		_SYDNEY_RETHROW;
	}
}

//
//	FUNCTION public
//		Schema::Area::get -- スキーマオブジェクトIDを指定してエリアを得る
//
//	NOTES
//		マウント中のデータベースに対してこの関数を使用してはいけない
//
//	ARGUMENTS
//		Schema::Object::ID::Value iID_
//			取得するエリアのスキーマオブジェクトID
//		Schema::Database* pDatabase_
//			取得するエリアが属するデータベース
//		Trans::Transaction& cTrans_
//			エリアを取得しようとしているトランザクション記述子
//
//	RETURN
//		取得したエリアのオブジェクト
//
//	EXCEPTIONS
//		???
//

// static
Area*
Area::
get(ID::Value iID_, Database* pDatabase_, Trans::Transaction& cTrans_)
{
	; _SYDNEY_ASSERT(pDatabase_);

	if (iID_ == Object::ID::Invalid)
		return 0;

	return (pDatabase_) ? pDatabase_->getArea(iID_, cTrans_) : 0;
}

//	FUNCTION public
//	Schema::Area::doBeforePersist -- 永続化する前に行う処理
//
//	NOTES
//
//	ARGUMENTS
//		const Schema::AreaPointer& pArea_
//			永続化するエリアのオブジェクト
//		Schema::Object::Status::Value eStatus_
//			永続化する前のエリアの状態
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
Area::
doBeforePersist(const Pointer& pArea_, Status::Value eStatus_,
				bool bNeedToErase_,
				Trans::Transaction& cTrans_)
{
	; _SYDNEY_ASSERT(pArea_.get());

	switch (eStatus_) {
	case Status::Created:
	case Status::Changed:
	case Status::Mounted:
	case Status::DeleteCanceled:
	case Status::DeletedInRecovery:
	{
		; // 何もしない
		break;
	}
	case Status::Deleted:
	{
		// エリアのパスを破棄する
		// ★注意★
		// エラーが起きてもログの内容から再実行できるように
		// ファイルやディレクトリーを実際に「消す」操作は
		// システム表から消す操作を永続化する前に行う
		// また、エリアに割り当てられているファイルはないはずなので
		// 即座に消せる
		// すでにディレクトリーが存在しなくてもエラーにならない

		Area::destroy(cTrans_, pArea_->getPath(), ModUnicodeString());

		break;
	}
	default:
		// 何もしない
		break;
	}
}

//	FUNCTION public
//	Schema::Area::doAfterPersist -- 永続化した後に行う処理
//
//	NOTES
//
//	ARGUMENTS
//		const Schema::AreaPointer& pArea_
//			永続化したエリアのオブジェクト
//		Schema::Object::Status::Value eStatus_
//			永続化する前のエリアの状態
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
Area::
doAfterPersist(const Pointer& pArea_, Status::Value eStatus_,
			   bool bNeedToErase_,
			   Trans::Transaction& cTrans_)
{
	; _SYDNEY_ASSERT(pArea_.get());

	// deleteされる可能性があるのでここでデータベースIDを取得しておく
	ID::Value dbID = pArea_->getDatabaseID();

	switch (eStatus_) {
	case Status::Created:
	case Status::Mounted:
	case Status::DeleteCanceled:
	{
		Database* pDatabase = pArea_->getDatabase(cTrans_);
		; _SYDNEY_ASSERT(pDatabase);
		// エリアはキャッシュに入れない
		(void) pDatabase->addArea(pArea_, cTrans_);
		break;
	}
	case Status::Changed:
	{
		// クラス内でキャッシュしていたらクリアしておく
		pArea_->clearContent();
		break;
	}
	case Status::Deleted:
	case Status::DeletedInRecovery:
	{
		// 変更が削除だったらマネージャーの登録からの削除も行う

		// 状態を「実際に削除された」にする

		pArea_->setStatus(Schema::Object::Status::ReallyDeleted);

		// 下位オブジェクトがあればそれを抹消してからdeleteする
		pArea_->reset();

		Database* pDatabase = pArea_->getDatabase(cTrans_);
		if (pDatabase)
			pDatabase->eraseArea(pArea_->getID());
		break;
	}
	case Status::CreateCanceled:
	default:

		// 何もしない

		return;
	}

	// システム表の状態を変える
	SystemTable::unsetStatus(dbID, Object::Category::Area);
}

//	FUNCTION public
//	Schema::Area::doAfterLoad -- 読み込んだ後に行う処理
//
//	NOTES
//
//	ARGUMENTS
//		const Schema::AreaPointer& pArea_
//			永続化したエリアのオブジェクト
//		Schema::Database& cDatabase_
//			エリアが属するデータベース
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
Area::
doAfterLoad(const Pointer& pArea_, Database& cDatabase_, Trans::Transaction& cTrans_)
{
	// リカバリー中のUNDO処理において最終的なパスが登録されていたら
	// パス指定を置き換える
	pArea_->checkUndo(cDatabase_, pArea_->getID());

	// システムへ読み出したエリアを表すクラスを追加する
	// また、マネージャーにこのエリアを表すクラスを
	// スキーマオブジェクトとして管理させる
	// エリアはキャッシュに入れない

	cDatabase_.addArea(pArea_, cTrans_);
}

//
//	FUNCTION public
//		Schema::Area::getPath -- エリアに対応するパス名をすべて得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		const ModVector<ModUnicodeString>&
//			パス名の配列
//
//	EXCEPTIONS

const ModVector<ModUnicodeString>&
Area::
getPath() const
{
	AutoRWLock l(getRWLock());
	if (!m_pPathArray) {
		// 書き込みモードにしてもう一度調べる
		l.convert(Os::RWLock::Mode::Write);
		if (!m_pPathArray) {
			m_pPathArray = new ModVector<ModUnicodeString>();
		}
	}
	; _SYDNEY_ASSERT(m_pPathArray);

	return *m_pPathArray;
}

//
//	FUNCTION public
//		Schema::Area::getPath -- 配列上の位置を指定して対応するパス名を得る
//
//	NOTES
//
//	ARGUMENTS
//		int iPosition_
//			取得するパス名の配列上の位置(0-base)
//
//	RETURN
//		const ModUnicodeString&
//			指定された位置にあるパス名
//
//	EXCEPTIONS
//		Exception::BadArrayElement
//			配列の範囲を超えた位置が指定された
//

const ModUnicodeString&
Area::
getPath(int iPosition_) const
{
	AutoRWLock l(getRWLock());

	if (iPosition_ < 0
		|| static_cast<ModSize>(iPosition_) >= getSize()) {
		// 例外送出
		_SYDNEY_THROW0(Exception::BadArrayElement);
	}
	; _SYDNEY_ASSERT(m_pPathArray);

	return m_pPathArray->at(iPosition_);
}

//
//	FUNCTION public
//		Schema::Area::addPath -- 対応するパス名を追加する
//
//	NOTES
//
//	ARGUMENTS
//		const ModUnicodeString& cstrPath_
//			追加するパス名
//
//	RETURN
//		const ModUnicodeString&
//			追加されたパス名
//
//	EXCEPTIONS
//

const ModUnicodeString&
Area::
addPath(const ModUnicodeString& cstrPath_)
{
	AutoRWLock l(getRWLock(), Os::RWLock::Mode::Write);

	if (!m_pPathArray)
		m_pPathArray = new ModVector<ModUnicodeString>();
	; _SYDNEY_ASSERT(m_pPathArray);

	m_pPathArray->pushBack(cstrPath_);

	return m_pPathArray->getBack();
}

//
//	FUNCTION public
//		Schema::Area::resetPath -- パス配列のすべての要素を抹消する
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
//

void
Area::
resetPath()
{
	AutoRWLock l(getRWLock(), Os::RWLock::Mode::Write);

	if (m_pPathArray) {
		// ベクターを空にする
		m_pPathArray->clear();

	} else {
		// パス名を登録するベクターを生成する
		m_pPathArray = new ModVector<ModUnicodeString>();
		; _SYDNEY_ASSERT(m_pPathArray);
	}
}

//
//	FUNCTION public
//		Schema::Area::clearPath --
//
//	NOTES
//
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		???
//
void
Area::
clearPath()
{
	AutoRWLock l(getRWLock(), Os::RWLock::Mode::Write);

	if (m_pPathArray) {
		// ベクターに登録されているパス名があれば、
		// すべて破棄し、ベクターも破棄する
		resetPath();
		delete m_pPathArray, m_pPathArray = 0;
	}
}

//
//	FUNCTION public
//		Schema::Area::getSize -- エリアに対応するパス名配列の要素数を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		ModSize
//
//	EXCEPTIONS
//

ModSize
Area::
getSize() const
{
	AutoRWLock l(getRWLock());
	return m_pPathArray ? m_pPathArray->getSize() : 0;
}

//	FUNCTION public
//		Schema::Area::destroy --
//			エリアに格納されているディレクトリーを破棄する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		const ModUnicodeString& cPathPart_
//			パス名に付加する固有部分
//		bool bForce_ = true
//			trueの場合チェックポイントを待たずに破棄する
//			falseの場合システムパラメーターによる
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Area::
destroy(Trans::Transaction& cTrans_,
		const ModUnicodeString& cPathPart_ /* = ModUnicodeString() */,
		bool bForce_ /* = true */)
{
	const ModVector<ModUnicodeString>& vecPath = getPath();
	ModSize n = vecPath.getSize();
	for (ModSize i = 0; i < n; i++)
		Utility::File::rmAll(getDatabaseID(), Os::Path(vecPath[i]).addPart(cPathPart_), bForce_);
}

//
//	FUNCTION public
//		Schema::Area::destroy --
//			エリアに格納されているあるディレクトリーを破棄する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		const ModVector<ModUnicodeString>& vecPath_
//			エリアのパス指定配列
//		const ModUnicodeString& cPathPart_
//			パス名に付加する固有部分
//		bool bForce_ = true
//			trueの場合チェックポイントを待たずに破棄する
//			falseの場合システムパラメーターによる
//
//	RETURN
//		なし
//
//	EXCEPTIONS

// static
void
Area::
destroy(Trans::Transaction& cTrans_, const ModVector<ModUnicodeString>& vecPath_,
		const ModUnicodeString& cPathPart_, bool bForce_)
{
	ModSize n = vecPath_.getSize();
	for (ModSize i = 0; i < n; i++)
		Utility::File::rmAll(Os::Path(vecPath_[i]).addPart(cPathPart_), bForce_);
}

// FUNCTION public
//	Schema::Area::undoDestroy -- ディレクトリーの破棄を取り消す
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	const ModUnicodeString& cPathPart_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Area::
undoDestroy(Trans::Transaction& cTrans_,
			const ModUnicodeString& cPathPart_)
{
	const ModVector<ModUnicodeString>& vecPath = getPath();
	ModSize n = vecPath.getSize();
	for (ModSize i = 0; i < n; i++)
		Utility::File::undoRmAll(Os::Path(vecPath[i]).addPart(cPathPart_));
}

//	FUNCTION public
//	Schema::Area::serialize --
//		エリアを表すクラスのシリアライザー
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
Area::
serialize(ModArchive& archiver)
{
	// まず、スキーマオブジェクト固有の情報をシリアル化する
	Object::serialize(archiver);

	if (archiver.isStore()) {

		// パス配列の要素数
		ModSize iSize = getSize();

		archiver << iSize;

		if (iSize) {
			for (ModSize i = 0; i < iSize; i++) {
				; _SYDNEY_ASSERT(m_pPathArray->at(i));
				archiver << m_pPathArray->at(i);
			}
		}

		// contents
		ModSize iContents = (_contents)?_contents->getSize():0;
		archiver << iContents;
		if (iContents) {
			Utility::OutputArchive& out =
				dynamic_cast<Utility::OutputArchive&>(archiver);

			_contents->writeObject(out);
		}

	} else {

		// メンバーをすべて初期化しておく
		clear();

		// パス配列の要素数
		ModSize iSize;

		archiver >> iSize;

		if (iSize) {
			prepareArray(iSize);

			for (ModSize i = 0; i < iSize; i++) {
				ModUnicodeString cstrName;
				archiver >> cstrName;
				(void) addPath(cstrName);
			}
		}

		// contents
		ModSize iContents;
		archiver >> iContents;

		if (iContents) {
			Utility::InputArchive& in =
				dynamic_cast<Utility::InputArchive&>(archiver);

			resetContent();

			_contents->readObject(in, iContents);
		}
	}
}

//	FUNCTION public
//	Schema::Area::getClassID -- このクラスのクラス ID を得る
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
Area::
getClassID() const
{
	return Externalizable::Category::Area +
		Common::Externalizable::SchemaClasses;
}

//	FUNCTION public
//	Schema::Area::clear --
//		エリアを表すクラスのメンバーをすべて初期化する
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
Area::
clear()
{
	destruct();
}

//
//	FUNCTION private
//		Schema::Area::destruct -- デストラクター下位関数
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
//		???
//
void
Area::
destruct()
{
	// ★注意★
	// デストラクトのときは保持するオブジェクトを行儀よく片付ける必要はない
	// 必要ならばこのオブジェクトをdeleteする前にresetを呼ぶ
	// ここでは領域を開放するのみ

	delete _contents, _contents = 0;

	clearPath();
}

//
//	FUNCTION public
//		Schema::Area::reset --
//			下位オブジェクトを抹消する
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
//		???
//
void
Area::
reset()
{
	if (_contents)
		resetContent();

	resetPath();
}
void
Area::
reset(Database& cDatabase_)
{
	reset();
}

//	FUNCTION public
//	Schema::Area::getContent --
//		エリアに関係する格納関係のうち、
//		指定したオブジェクトIDを持つオブジェクトと
//		指定したエリア種別を持つ格納関係を表すクラスを得る
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Object::ID::Value	iObjectID_
//			このオブジェクトIDを持つオブジェクトに関係するクラスを得る
//		Schema::AreaCategory eCategory_
//			このエリア種別のクラスを得る
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//
//	RETURN
//		0 以外の値
//			得られた格納関係を格納する領域の先頭アドレス
//		0
//			指定されたオブジェクトと種別の格納関係は存在しない
//
//	EXCEPTIONS

AreaContent*
Area::
getContent(ID::Value iObjectID_, AreaCategory::Value eCategory_,
		   Trans::Transaction& cTrans_) const
{
	const AreaContentMap& cMap = const_cast<Area*>(this)->loadContent(cTrans_);

	AutoRWLock l(getRWLock());
	return cMap.find(BoolFunction2<AreaContent, ID::Value, AreaCategory::Value>
					 (AreaContentMap::findByCategory, iObjectID_, eCategory_));
}

//	FUNCTION public
//	Schema::Area::addContent --
//		表を表すクラスの格納関係として、
//		指定された格納関係を表すクラスを追加する
//
//	NOTES
//		「格納関係」表は更新されない
//
//	ARGUMENTS
//		const Schema::AreaContent::Pointer&		content
//			追加する格納関係を表すクラス
//			これは、呼び出し中には更新されないが、
//			その後、更新される可能性がある
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//
//	RETURN
//		追加した格納関係を表すクラス
//
//	EXCEPTIONS

const AreaContent::Pointer&
Area::addContent(const AreaContent::Pointer& content, Trans::Transaction& cTrans_)
{
	// 「格納関係」表のうち、このエリアに関する部分を
	// 読み出していなければ、まず読み出してから、
	// 与えられた格納関係を追加する

	(void) loadContent(cTrans_);

	AutoRWLock l(getRWLock(), Os::RWLock::Mode::Write);
	(void) _contents->insert(content);

	return content;
}

//	FUNCTION public
//	Schema::Area::eraseContent --
//		表を表すクラスからある格納関係を表すクラスの登録を抹消する
//
//	NOTES
//		「格納関係」表は更新されない
//
//	ARGUMENTS
//		Schema::Object::ID::Value	contentID
//			登録を抹消する格納関係のオブジェクト ID
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Area::
eraseContent(ID::Value contentID)
{
	AutoRWLock l(getRWLock(), Os::RWLock::Mode::Write);

	if (_contents) {
		(void) _contents->erase(contentID);
	}
}

//	FUNCTION public
//	Schema::Area::resetContent --
//		表には格納関係を表すクラスが登録されていないことにする
//
//	NOTES
//		「格納関係」表は更新されない
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Area::
resetContent()
{
	AutoRWLock l(getRWLock(), Os::RWLock::Mode::Write);

	if (_contents) {
		// ハッシュマップを空にすれば登録されているオブジェクトは
		// ObjectPointerの機構により破棄される

		_contents->reset();

	} else {
		// 格納関係を表すクラスを登録するハッシュマップを生成する

		_contents = new AreaContentMap;
		; _SYDNEY_ASSERT(_contents);
	}
}

//	FUNCTION public
//	Schema::Area::clearContent --
//		表を表すクラスに登録されている格納関係を表すクラスと、
//		その管理用のベクターを破棄する
//
//	NOTES
//		「格納関係」表は更新されない
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Area::
clearContent()
{
	AutoRWLock l(getRWLock(), Os::RWLock::Mode::Write);

	if (_contents) {

		// ハッシュマップに登録されている格納関係を表すクラスがあれば、
		// すべて破棄し、ハッシュマップも破棄する

		resetContent();
		delete _contents, _contents = 0;
	}
}

//	FUNCTION public
//	Schema::Area::loadContent --
//		表に存在するすべての格納関係を表すクラスを読み出す
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		bool bRecovery_ = false
//			trueの場合Mount後のリカバリーのための特別な動作を行う
//
//	RETURN
//		表に存在する格納関係をひとつづつ要素とするハッシュマップ
//
//	EXCEPTIONS

const AreaContentMap&
Area::loadContent(Trans::Transaction& cTrans_, bool bRecovery_ /* = false */)
{
	AutoRWLock l(getRWLock());
	if (!_contents) {
		// 「格納関係」表のうち、このエリアに関する部分を
		// 一度も読み出していないので、まず、読み出す
		// ただし作成後永続化していないなら読み出す必要はないので
		// ベクターを初期化するのみ

		l.convert(Os::RWLock::Mode::Write);
		// 書き込みロックをしてから再度調べる
		if (!_contents) {
			if ((getScope() != Scope::Permanent
				 || getStatus() == Status::Created)
				&& !bRecovery_)
				resetContent();
			else {
				Database* pDatabase = getDatabase(cTrans_);
				SystemTable::AreaContent(*pDatabase).load(cTrans_, *this, bRecovery_);
				// loadしたらキャッシュサイズを調べる
				if (Manager::ObjectTree::Database::checkCacheSize()) {
					// 超えていたらキャッシュのクリアを試みる
					Manager::ObjectTree::Database::clearCache();
				}
			}
			; _SYDNEY_ASSERT(_contents);
		}
	}
	return *_contents;
}

//
//	FUNCTION private
//		Schema::Area::prepareArray -- パス名配列の要素数を確保する
//
//	NOTES
//
//	ARGUMENTS
//		ModSize iSize_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//

void
Area::
prepareArray(ModSize iSize_)
{
	// 排他制御は呼び出し側が必要に応じて行う

	resetPath();
	; _SYDNEY_ASSERT(m_pPathArray);

	m_pPathArray->reserve(iSize_);
}

// 異なるエリアで同じパスを使っていても問題が生じないように
// 与えられたパス名にエリア名を追加することをしていたが、
// データベースローカルに定義することにしたために
// エリア名だけでは分離不可能になった
// 結局、createのmkdirのときにディレクトリーが存在していたら
// エラーにするという対応で一意性を保証するので
// エリア名を付加する必要はない

//
//	FUNCTION private
//		Schema::Area::setPath -- パス名指定のSQL構文からパス配列を作る
//
//	NOTES
//
//	ARGUMENTS
//		const Statement::AreaElementList& cStatement_
//			パス名を指定するSQL構文を表すクラス
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//

void
Area::
setPath(const Statement::AreaElementList& cStatement_)
{
	// 呼び出し側で排他すること
	int n = cStatement_.getCount();
	prepareArray(n);

	for (int i = 0; i < n; i++) {
		Statement::AreaDataDefinition* pElement =
			_SYDNEY_DYNAMIC_CAST(Statement::AreaDataDefinition*,
								 cStatement_.getAt(i));
		; _SYDNEY_ASSERT(pElement);
		; _SYDNEY_ASSERT(pElement->getAreaData());

		Common::Data::Pointer pValue = pElement->getAreaData()->createData();
		Common::StringData* pPath =
			_SYDNEY_DYNAMIC_CAST(Common::StringData*, pValue.get());
		; _SYDNEY_ASSERT(pPath);

		addPath(_Path::_getFullPathName(pPath->getValue()));
	}
}

//
//	FUNCTION private
//		Schema::Area::setPath -- パス配列を設定する
//
//	NOTES
//
//	ARGUMENTS
//		const ModVector<ModUnicodeString>&
//			パス名を指定するパス配列
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//

void
Area::
setPath(const ModVector<ModUnicodeString>& cPathArray_)
{
	// 呼び出し側で排他すること
	ModSize n = cPathArray_.getSize();
	if (!m_pPathArray) {
		resetPath();
	}

	// パスの設定
	// Redo/Undoの仕様変更によりsetPathに渡されるパスは
	// 変更がなくても値が入っているので普通にコピーすればよい
	*m_pPathArray = cPathArray_;
}

//
//	FUNCTION public
//		Schema::Area::setMovePrepare -- move の為の予備準備
//
//	NOTES
//		変更前パスリストと変更後パスリストを設定する
//
//
//	ARGUMENTS
//		const Statement::AreaElementList& cStatement_
//			エリア変更を行うトランザクション記述子
//
//	RETURN
//		bool
//			true  : 正常終了
//			false : 以上終了
//
//	EXCEPTIONS
bool
Area::
setMovePrepare(const int iType_,
			   const Statement::AreaElementList& cStatement_,
			   ModVector<ModUnicodeString>& vecPrevPath_,
			   ModVector<ModUnicodeString>& vecPostPath_)
{
	bool bRet = false;
	int n = getSize();

	; _SYDNEY_ASSERT(vecPrevPath_.isEmpty());
	; _SYDNEY_ASSERT(vecPostPath_.isEmpty());

	// REDO/UNDOの仕様変更により変更のないパスについてもログデータから
	// パスがわかる必要があるようになった
	vecPrevPath_ = getPath();
	vecPostPath_ = getPath();

	switch (static_cast<Statement::AlterAreaAction::ActionType>(iType_)) {
	case Statement::AlterAreaAction::SingleModify:
	case Statement::AlterAreaAction::FullAryModify:
	{
		if (cStatement_.getCount() != n)
		{
			// 変更前と後でパス指定の個数が一致していないので例外送出
			SydInfoMessage << "Can't alter area. The number of path elements don't match ("
						   << cStatement_.getCount() << " != " << n << ")."
						   << ModEndl;
			_SYDNEY_THROW1(Exception::InvalidPath, getName());
		}

		// 構文要素にしたがって新しいパス名をセットする
		// ★注意★
		// オブジェクトのパス指定を変更するわけではないので
		// setPathは使わない

		for (int i = 0; i < n; i++) {
			Statement::AreaDataDefinition* pElement =
				_SYDNEY_DYNAMIC_CAST(Statement::AreaDataDefinition*,
									 cStatement_.getAt(i));
			; _SYDNEY_ASSERT(pElement);
			; _SYDNEY_ASSERT(pElement->getAreaData());

			Common::Data::Pointer pValue = pElement->getAreaData()->createData();
			Common::StringData* pPath =
				_SYDNEY_DYNAMIC_CAST(Common::StringData*, pValue.get());
			; _SYDNEY_ASSERT(pPath);
			vecPostPath_[i] = _Path::_getFullPathName(pPath->getValue());

			if (!bRet
				&& (Os::Path::compare(vecPrevPath_[i], vecPostPath_[i])
					!= Os::Path::CompareResult::Identical))
				bRet = true;
		}
		break;
	}
	case Statement::AlterAreaAction::ElemAryModify:
	{
		if (cStatement_.getCount() > n) {
			// 要素指定が変更前のパス指定の個数を超えているので例外送出
			SydInfoMessage << "Can't alter area. Specified element number (" << cStatement_.getCount()
						   << ") out of range (" << n << ")."
						   << ModEndl;
			_SYDNEY_THROW1(Exception::InvalidPath, getName());
		}

		// 構文要素にしたがってパス名を置き換える
		// ★注意★
		// オブジェクトのパス指定を変更するわけではないので
		// replacePathは使わない

		for (int i = 0; i < n; i++) {
			Statement::Object* pElement = cStatement_.getAt(i);
			if (!pElement
				|| pElement->getType() != Statement::ObjectType::AreaDataDefinition) {
				// エリア名指定の要素ではないので次へ
				continue;
			}
			Statement::AreaDataDefinition* pDataDefinition =
				_SYDNEY_DYNAMIC_CAST(Statement::AreaDataDefinition*, pElement);
			; _SYDNEY_ASSERT(pDataDefinition);
			if (!pDataDefinition->getAreaData())
				// 置き換え対象ではない要素なので次へ
				continue;

			Common::Data::Pointer pValue = pDataDefinition->getAreaData()->createData();
			Common::StringData* pPath =
				_SYDNEY_DYNAMIC_CAST(Common::StringData*, pValue.get());
			; _SYDNEY_ASSERT(pPath);

			// パス名を置き換える
			vecPostPath_[i] = _Path::_getFullPathName(pPath->getValue());

			if (!bRet
				&& (Os::Path::compare(vecPrevPath_[i], vecPostPath_[i])
					!= Os::Path::CompareResult::Identical))
				bRet = true;
			// 置き換えるパスはただひとつなのでこれで終了
			break;
		}
		break;
	}
	default:
		; _SYDNEY_ASSERT(false);
		break;
	}

	return bRet;
}

//	FUNCTION private
//		Schema::Area::moveFile --
//			エリアに関係するスキーマオブジェクトを構成するファイルを
//			新しい格納場所に移動する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		const ModVector<ModUnicodeString>& vecPrevPath_
//		const ModVector<ModUnicodeString>& vecPostPath_
//			移動前後のパス
//		bool bUndo_ = false
//			trueの場合エラー処理中なので重ねてのエラー処理はしない
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		Exception::MetaDatabaseCorrupted
//			エリア格納関係の内容が正しくない

void
Area::
moveFile(Trans::Transaction& cTrans_,
		 const ModVector<ModUnicodeString>& vecPrevPath_,
		 const ModVector<ModUnicodeString>& vecPostPath_,
		 bool bUndo_, bool bRecovery_, bool bMount_)
{
	// このエリアと関係するすべてのエリア格納関係について
	// ファイルの移動を行う
	const AreaContentMap& contents = loadContent(cTrans_);

	AutoRWLock l(getRWLock());

	AreaContentMap::ConstIterator iterator = contents.begin();
	const AreaContentMap::ConstIterator& end = contents.end();

	try {
		for (; iterator != end; ++iterator) {
			const AreaContent::Pointer& pContent = AreaContentMap::getValue(iterator);

			if (pContent.get()) {
				pContent->moveFile(cTrans_, vecPrevPath_, vecPostPath_, bUndo_, bRecovery_, bMount_);
			}
		}
		SCHEMA_FAKE_ERROR("Schema::Area", "MoveFile", "Moved");

		l.convert(Os::RWLock::Mode::Write);
		// パス設定を新しいものにする
		setPath(vecPostPath_);

	} catch (...) {

		_BEGIN_REORGANIZE_RECOVERY;

		l.convert(Os::RWLock::Mode::Write);
		// パス設定を新しいものにする
		setPath(vecPostPath_);
		// 移動してしまった格納関係について元に戻す
		AreaContentMap::ConstIterator err = contents.begin();
		for (; err != iterator; ++err) {
			const AreaContent::Pointer& pContent = AreaContentMap::getValue(err);
			if (pContent.get()) {
				pContent->moveFile(cTrans_, vecPostPath_, vecPrevPath_, true /* undo */, bRecovery_, bMount_);
			}
		}
		// パス設定を元に戻す
		setPath(vecPrevPath_);

		_END_REORGANIZE_RECOVERY;

		_SYDNEY_RETHROW;
	}
}

#ifdef OBSOLETE
//	FUNCTION private
//		Schema::Area::prepareMove --
//			エリアの移動の準備
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		const ModVector<ModUnicodeString>& vecPrevPath_
//		const ModVector<ModUnicodeString>& vecPostPath_
//			移動前後のパス
//		bool bUndo_ = false
//			trueの場合エラー処理中なので重ねてのエラー処理はしない
//
//	RETURN
//		なし
//
//	EXCEPTIONS

// static
void
Area::
prepareMove(Trans::Transaction& cTrans_,
			const ModVector<ModUnicodeString>& vecPrevPath_,
			const ModVector<ModUnicodeString>& vecPostPath_,
			bool bUndo_)
{
	ModSize n = vecPostPath_.getSize();

	// エラー発生時に自動的にmkdirを取り消すためのクラス
	ModVector<Utility::File::AutoRmDir> vecAutoRmDir(n); 

	for (ModSize i = 0; i < n; ++i) {
		// 変更があるなら変更後のディレクトリーを作成する
		// ★注意★
		// 変更がない場合でも同一のパスをさす文字列をわざわざ指定するかもしれないので
		// Os::Path::compareを使う
		if (Os::Path::compare(vecPostPath_[i], vecPrevPath_[i])
			!= Os::Path::CompareResult::Identical) {
			vecAutoRmDir[i].setDir(vecPostPath_[i]);
			// エラー処理中は重ねてのエラー処理はしないのでここで成功にする
			if (bUndo_) vecAutoRmDir[i].disable();
		}
	}
	if (!bUndo_) {
		// すべて成功したのでエラー処理を解除する
		for (ModSize i = 0; i < n; ++i) {
			vecAutoRmDir[i].disable();
		}
	}
}
#endif

//	FUNCTION private
//		Schema::Area::sweepMove --
//			エリアの移動の後始末
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		const ModVector<ModUnicodeString>& vecPrevPath_
//		const ModVector<ModUnicodeString>& vecPostPath_
//			移動前後のパス
//		bool bUndo_ = false
//			trueの場合エラー処理中なので重ねてのエラー処理はしない
//
//	RETURN
//		なし
//
//	EXCEPTIONS

// static
void
Area::
sweepMove(Trans::Transaction& cTrans_,
		  const ModVector<ModUnicodeString>& vecPrevPath_,
		  const ModVector<ModUnicodeString>& vecPostPath_,
		  bool bUndo_)
{
	ModSize n = vecPrevPath_.getSize();
	; _SYDNEY_ASSERT(vecPostPath_.getSize() == n);

	ModSize i = 0;
#ifdef OBSOLETE
	try {
#endif
		// 移動後にないディレクトリーを破棄する
		for (; i < n; ++i) {
			ModSize j = 0;
			for (; j < n; ++j) {
				if (Os::Path::compare(vecPrevPath_[i], vecPostPath_[j])
					== Os::Path::CompareResult::Identical)
					break;
			}
			if (j == n) {
				// 同じものがなかったので破棄する
				Utility::File::rmAll(Os::Path(vecPrevPath_[i]));
			}
		}
		SCHEMA_FAKE_ERROR("Schema::Area", "SweepMove", "Moved");

#ifdef OBSOLETE
	} catch (...) {

		if (!bUndo_) {

			// 破棄した可能性のあるパスを再度作成する
			for (ModSize iErr = 0; iErr < i; ++iErr) {
				Utility::File::mkdir(Os::Path(vecPrevPath_[iErr]));
			}
		}

		_SYDNEY_RETHROW;
	}
#endif
}

//	FUNCTION public
//	Schema::Area::checkPath --
//		パスが他のデータベースやエリアに使われていないか調べる
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		const Area* pArea_
//			データベースオブジェクト
//		const ModVector<ModUnicodeString>* vecPath_ = 0
//			パス指定配列へのポインター
//			0ならエリアのパスが使用される
//		bool bEraseExistence_ = false
//			trueのときすでに存在していたら削除してしまう
//			(redo用)
//		bool bNeedExistence_ = false
//			trueのときパスが存在していなかったらエラー
//			(mount用)
//
//	RETURN
//		true .. 同じパス名のディレクトリーが存在する
//		false.. 同じパス名のディレクトリーは存在しない
//
//	EXCEPTIONS

bool
Area::
checkPath(Trans::Transaction& cTrans_,
		  const ModVector<ModUnicodeString>* vecPath_,
		  bool bEraseExistence_,
		  bool bNeedExistence_)
{
	return _Path::_checkExistence(cTrans_, this, vecPath_,
								  bEraseExistence_, bNeedExistence_);
}

////////////////////////////////////////////
// データベースファイル化のためのメソッド //
////////////////////////////////////////////

//	FUNCTION public
//	Schema::Area::makeLogData --
//		ログデータを作る
//
//	NOTES
//		引数のログデータには種別が設定されている必要がある
//
//	ARGUMENTS
//		LogData& cLogData_
//			値を設定するログデータ
//
//	RETURN
//	なし
//
//	EXCEPTIONS

void
Area::
makeLogData(LogData& cLogData_) const
{
	// 全ログに共通のデータ
	//	1. 名前
	//	2. ID
	cLogData_.addString(getName());
	cLogData_.addID(getID());

	switch (cLogData_.getSubCategory()) {
	case LogData::Category::CreateArea:
	{
		//	エリア作成
		//		3．パス
		cLogData_.addStrings(getPath());
		; _SYDNEY_ASSERT(cLogData_.getCount() == Log::Create::Num);
		break;
	}
	case LogData::Category::DropArea:
	{
		//	 エリアの破棄
		//		3．パス
		cLogData_.addStrings(getPath());
		; _SYDNEY_ASSERT(cLogData_.getCount() == Log::Drop::Num);
		break;
	}
	case LogData::Category::AlterArea:
	{
		//	 エリアの定義変更
		//		(変更前後のパスはArea::alterでセットされる)

		break;
	}
	default:
		; _SYDNEY_ASSERT(false);
		break;
	}
}

//
//	FUNCTION public
//		Schema::Area::getPathArray -- ID配列からログに出すためのパス配列データを得る
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		const Schema::Database& cDatabase_
//			エリアが属するデータベース
//		const ModVector<ID::Value>& vecID_
//			パス配列を得たいIDの配列
//
//	RETURN
//		パス配列を格納したCommon::Data::Pointer
//
//	EXCEPTIONS
//

//static
Common::Data::Pointer
Area::
getPathArray(Trans::Transaction& cTrans_,
			 const Database& cDatabase_,
			 const ModVector<ID::Value>& vecID_)
{
	ModAutoPointer<Common::DataArrayData> pPaths = new Common::DataArrayData;
	ModSize n = vecID_.getSize();
	if (n) {
		pPaths->reserve(n);
		for (ModSize i = 0; i < n; ++i) {
			Area* pArea = cDatabase_.getArea(vecID_[i], cTrans_);
			if (pArea) {
				pPaths->pushBack(LogData::createStrings(pArea->getPath()));

			} else {
				pPaths->pushBack(LogData::createStrings(ModVector<ModUnicodeString>()));
			}
		}
	}
	return pPaths.release();
}

//
//	FUNCTION public
//		Schema::Area::getPathArray -- ログから指定した位置のパスデータを得る
//
//	NOTES
//
//	ARGUMENTS
//		const Common::DataArrayData& cData_
//			ログデータ
//		int iIndex_
//			パス指定を得たいエリアの配列中の位置
//		ModVector<ModUnicodeString>& vecPath_
//			結果を格納する配列
//
//	RETURN
//		true .. 取得できた
//		false.. iIndex_がログデータのサイズを超えていた
//
//	EXCEPTIONS
//

//static
bool
Area::
getPathArray(const Common::DataArrayData& cData_,
			 int iIndex_,
			 ModVector<ModUnicodeString>& vecPath_)
{
	; _SYDNEY_ASSERT(vecPath_.isEmpty());
	int n = cData_.getCount();
	if (n > iIndex_) {
		vecPath_ = LogData::getStrings(cData_.getElement(iIndex_));
		return true;
	}
	return false;
}

//
//	FUNCTION public
//		Schema::Area::getObjectID -- ログデータより Area ID を取得する
//
//	NOTES
//
//	ARGUMENTS
//		const LogData& log_
//			ログテータ
//		ObjectID::Value& id_
//			ID 値
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//

// static 

Object::ID::Value
Area::
getObjectID(const LogData& log_)
{
	return log_.getID(Log::ID);
}

//
//	FUNCTION public
//		Schema::Area::getName -- ログデータからエリア名を得る
//
//	NOTES
//		エリアの変更または抹消のログデータから取得する
//
//	ARGUMENTS
//		const Schema::LogData& cLogData_
//			エリア変更または抹消のログデータ
//
//	RETURN
//		対象のエリア名
//
//	EXCEPTIONS
//		Exception::LogItemCorrupted
//			ログデータからエリア名が得られなかった

// static
Object::Name
Area::
getName(const LogData& cLogData_)
{
	// ログデータの内容を取得
	// ★注意★
	//	makeLogDataの実装を変えたらここも変える
	return cLogData_.getString(Log::Name);
}

//
//	FUNCTION public
//		Schema::Area::getPath -- パスの配列を得る
//
//	NOTES
//
//	ARGUMENTS
//		const LogData& cLogData_
//			ログデータ
//		int iIndex_
//			ログファイル中のパス配列が格納されている位置
//		ModVector<ModUnicodeString>& vecPath_
//			パス格納配列
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//

// static
void
Area::
getPath(const LogData& cLogData_, int iIndex_, ModVector<ModUnicodeString>& vecPath_)
{
	// ログデータの内容を取得
	// ★注意★
	//	makeLogDataの実装を変えたらここも変える

	vecPath_ = cLogData_.getStrings(iIndex_);
}

//	FUNCTION private
//	Schema::Area::checkUndo --
//		Undo情報を検査し反映する
//
//	NOTES
//
//	ARGUMENTS
//		const Schema::Database& cDatabase_
//			エリアが属するデータベースオブジェクト
//		Schema::Object::ID::Value iID_
//			検査に使用するID
//
//	RETURN
//	なし
//
//	EXCEPTIONS

void
Area::
checkUndo(const Database& cDatabase_, ID::Value iID_)
{
	using namespace Manager::RecoveryUtility;
	if (Undo::isEntered(cDatabase_.getName(), iID_, Undo::Type::AlterArea)) {
		ModVector<ModUnicodeString> vecPath;
		if (Path::getUndoAreaPath(cDatabase_.getName(), iID_, vecPath)) {
			// Alter後のパスが登録されているときはログデータのパスではなく
			// Alter後のパスを使用する
			setPath(vecPath);
		}
	}
}

////////////////////////////////////////////////////////////
// メタデータベースのための定義
////////////////////////////////////////////////////////////

// メタデータベースにおける「エリア」表の構造は以下のとおり
// create table Area (
//		ID		id,
//		name	nvarchar,
//		path	<path array>, -- StringData or DataArrayData
//		time	timestamp
// )

namespace
{
#define _DEFINE0(_type_) \
	Meta::Definition<Area>(Meta::MemberType::_type_)
#define _DEFINE2(_type_, _get_, _set_) \
	Meta::Definition<Area>(Meta::MemberType::_type_, &Area::_get_, &Area::_set_)

	Meta::Definition<Area> _vecDefinition[] =
	{
		_DEFINE0(FileOID),		// FileOID
		_DEFINE0(ObjectID),		// ID
		_DEFINE0(Name),			// Name
		_DEFINE2(StringArray, getPath, setPath), // Path
		_DEFINE0(Timestamp),	// Timestamp
	};
#undef _DEFINE0
#undef _DEFINE2
}

//	FUNCTION public
//	Schema::Area::getMetaFieldNumber --
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
Area::
getMetaFieldNumber() const
{
	return static_cast<int>(Meta::Area::MemberNum);
}

//	FUNCTION public
//	Schema::Area::getMetaFieldDefinition --
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
Area::
getMetaMemberType(int iMemberID_) const
{
	return _vecDefinition[iMemberID_].m_eType;
}

//	FUNCTION public
//	Schema::Area::packMetaField --
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
Area::
packMetaField(int iMemberID_) const
{
	Meta::Definition<Area>& cDef = _vecDefinition[iMemberID_];

	if (cDef.m_eType < Meta::MemberType::UseObjectMax) {
		// Objectの情報を使う
		return Object::packMetaField(iMemberID_);
	}

	switch (cDef.m_eType) {
	case Meta::MemberType::StringArray:
		{
			return pack((this->*(cDef.m_funcGet._strings))());
		}
	default:
		// これ以外の型はないはず
		break;
	}
	; _SYDNEY_ASSERT(false);
	return Common::Data::Pointer();
}

//	FUNCTION private
//	Schema::Area::unpackMetaField --
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
Area::
unpackMetaField(const Common::Data* pData_, int iMemberID_)
{
	Meta::Definition<Area>& cDef = _vecDefinition[iMemberID_];

	if (cDef.m_eType < Meta::MemberType::UseObjectMax) {
		// Objectの情報を使う
		return Object::unpackMetaField(pData_, iMemberID_);
	}

	switch (cDef.m_eType) {
	case Meta::MemberType::StringArray:
		{
			ModVector<ModUnicodeString> vecPath;
			if (unpack(pData_, vecPath)) {
				(this->*(cDef.m_funcSet._strings))(vecPath);
				return true;
			}
			break;
		}
	default:
		// これ以外の型はないはず
		break;
	}
	return false;
}

//
// Copyright (c) 2001, 2002, 2004, 2005, 2006, 2007, 2012, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
