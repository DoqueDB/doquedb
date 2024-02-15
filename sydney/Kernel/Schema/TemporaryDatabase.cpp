// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// TemporaryDatabase.cpp -- 一時データベース関連の関数定義
// 
// Copyright (c) 2000, 2001, 2002, 2005, 2007, 2023 Ricoh Company, Ltd.
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

#include "Schema/TemporaryDatabase.h"
#include "Schema/Manager.h"
#include "Schema/Message.h"
#include "Schema/NameParts.h"
#include "Schema/PathParts.h"
#include "Schema/SessionID.h"
#include "Schema/SystemTable.h"
#include "Schema/Utility.h"

#include "Common/Assert.h"
#include "Common/ObjectPointer.h"

#include "Os/AutoCriticalSection.h"
#include "Os/CriticalSection.h"
#include "Os/Path.h"

#include "Trans/Transaction.h"
#include "Trans/AutoTransaction.h"

#include "ModCharString.h"
#include "ModHashMap.h"
#include "ModAutoPointer.h"

_SYDNEY_USING
_SYDNEY_SCHEMA_USING

namespace {

// スキーマオブジェクトをオブジェクト ID の昇順でソートするための関数

ModBoolean
ascendingSortObject(Object* l, Object* r)
{
	return (l->getID() < r->getID()) ? ModTrue : ModFalse;
}

// Terminateでそれぞれちゃんと破棄するようにしたので
// AutoPointerにする必要はない
// staticなAutoPointerを使うとプロセス終了時に
// 制御不能な順序で解放処理が行われるので不正なアドレスを参照する可能性がある
//#ifdef PURIFY
//#define SCHEMA_USE_STATIC_AUTOPOINTER
//#endif

#ifdef SCHEMA_USE_STATIC_AUTOPOINTER
#define _AutoPointer(__type__)	ModAutoPointer<__type__ >
#define _IsValidPointer(__pointer__)	(__pointer__.get() != 0)
#define _Free(__pointer__)	{__pointer__ = 0;}
#else
#define _AutoPointer(__type__)	__type__*
#define _IsValidPointer(__pointer__)	(__pointer__ != 0)
#define _Free(__pointer__)	{delete __pointer__; __pointer__ = 0;}
#endif

// スレッド間排他制御するためのクリティカルセクション
// 無名名前空間で実体で定義するとDLLのデタッチでおかしくなるので
// ポインターにする

Os::CriticalSection* _pCriticalSection;

// セッションに対応する一時データベースを保持するハッシュマップ

typedef Common::ObjectPointer<TemporaryDatabase> DatabaseEntry;
typedef ModHashMap<SessionID, DatabaseEntry, ModHasher<SessionID> > TemporaryDatabaseMap;
_AutoPointer(TemporaryDatabaseMap) _databases;

// TemporaryDatabaseMapのコンストラクターに与えるパラメーター
ModSize _temporaryDatabaseMapSize = 41; // 1ユーザーが使う一時表は2つ、推奨同時ユーザー数20
ModBoolean _temporaryDatabaseMapEnableLink = ModFalse; // Iterationしない

namespace _ID
{
	// 一時データベース用のID
	// 普通のデータベースと重なっても問題はないが
	// デバッグなどでわかりにくいと思われるため
	// ID::Invalid - 1 から順に減らしていく

	Object::ID::Value _iValue = Object::ID::Invalid - 1;

	Object::ID::Value
	_assign()
	{
		Os::AutoCriticalSection m(*_pCriticalSection);
		return _iValue--;
	}

} // namespace _ID

namespace _Path
{
	void
	_eraseTemporaryPath()
	{
		Os::Path cstrPath(Manager::Configuration::getDefaultAreaPath());
		cstrPath.addPart(PathParts::SystemTable::Temporary);
		Utility::File::rmAll(cstrPath);
	}
} // namespace _Path

} // namespace

//	FUNCTION public
//	Schema::TemporaryDatabase::TemporaryDatabase --
//		トランザクションから一時データベースを表すクラスのコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			一時データベースを得ようとしているトランザクション記述子
//
//	RETURN
//		なし
//
//	EXCEPTIONS

TemporaryDatabase::TemporaryDatabase(Trans::Transaction& cTrans_)
	: Database(Name(), Scope::SessionTemporary)
{
	// 名前を設定する
	setName(ModCharString().format(NameParts::Database::Temporary, cTrans_.getSessionID()));

	// パスを設定する
	Os::Path cstrPath(Manager::Configuration::getDefaultAreaPath());
	cstrPath.addPart(PathParts::SystemTable::Temporary);
	cstrPath.addPart(getName());

	for (int iCategory = static_cast<int>(Database::Path::Category::Data);
		 iCategory < Database::Path::Category::ValueNum; ++iCategory) {
		setPath(static_cast<Database::Path::Category::Value>(iCategory),
				cstrPath);
	}

	// 新たなIDをふる
	setID(_ID::_assign());

	// 一時表はコンストラクト時に作成してしまう
	Database::create(cTrans_);

	// 一時データベースは永続化の必要がないので
	// この時点でPersistにする
	setStatus(Status::Persistent);
}

//	FUNCTION private
//	Schema::TemporaryDatabase::destruct --
//		データベースを表すクラスのデストラクター下位関数
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
TemporaryDatabase::
destruct()
{
	// 一時データベースはオブジェクトが破棄されるときに
	// 実体も破棄する

	if (getStatus() == Object::Status::Persistent) {
		// トランザクション記述子は何でもよいはずである
		Trans::AutoTransaction pTransaction(Trans::Transaction::attach());

		Database::drop(*pTransaction);

		// 状態を「実際に削除された」にする
		setStatus(Schema::Object::Status::ReallyDeleted);

		// データベースを構成するファイルとディレクトリーを削除する
		// ・一時データベースはすぐに消す
		Database::destroy(*pTransaction, true);

		// オブジェクトが保持する下位オブジェクトを抹消する
		reset();

		// システム表の永続化情報を保持する構造を破棄する
		SystemTable::eraseStatus(getID());
	}
}

//	FUNCTION public
//	Schema::TemporaryDatabase::initialize --
//		一時データベースモジュールの初期化をする
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

// static
void
TemporaryDatabase::
initialize()
{
	// クリティカルセクションを作る
	_pCriticalSection = new Os::CriticalSection;

	// 途中で落ちたときなどに一時データベースが残っている場合があるので
	// 一時データベースのディレクトリを消す
	_Path::_eraseTemporaryPath();
}

//	FUNCTION public
//	Schema::TemporaryDatabase::terminate --
//		一時データベースモジュールの後処理をする
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

// static
void
TemporaryDatabase::
terminate()
{
	// データベースのマップをクリアする
	_Free(_databases);

	// 一時データベースのディレクトリはすぐに消す
	_Path::_eraseTemporaryPath();

	// クリティカルセクションを消す
	delete _pCriticalSection, _pCriticalSection = 0;
}

//	FUNCTION public
//	Schema::TemporaryDatabase::create --
//		あるセッションに対応する一時データベースを生成する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			一時データベースを得ようとしているトランザクション記述子
//
//	RETURN
//		得られたデータベースを格納する領域の先頭アドレス
//
//	EXCEPTIONS

// static
Database*
TemporaryDatabase::create(Trans::Transaction& cTrans_)
{
	Os::AutoCriticalSection m(*_pCriticalSection);

	if (!_IsValidPointer(_databases))
		_databases = new TemporaryDatabaseMap(_temporaryDatabaseMapSize, _temporaryDatabaseMapEnableLink);
	else if (Database* pResult = TemporaryDatabase::get(cTrans_))
		return pResult;

	// ★注意★
	// 一時データベースはオブジェクトを作成した時点でcreateされてしまう

	ModAutoPointer<TemporaryDatabase> pTmp = new TemporaryDatabase(cTrans_);
	(void) _databases->insert(cTrans_.getSessionID(), DatabaseEntry(pTmp.release()), ModTrue);

	return pTmp.get();
}

//	FUNCTION public
//	Schema::TemporaryDatabase::get --
//		あるセッションに対応するデータベースを得る
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			一時データベースを得ようとしているトランザクション記述子
//
//	RETURN
//		得られたデータベースを格納する領域の先頭アドレス
//
//	EXCEPTIONS

// static
Database*
TemporaryDatabase::
get(Trans::Transaction& cTrans_)
{
	Os::AutoCriticalSection m(*_pCriticalSection);

	if (_IsValidPointer(_databases)) {
		// ハッシュマップに登録されているか調べる
		TemporaryDatabaseMap::Iterator iterator = _databases->find(cTrans_.getSessionID());
		if (iterator != _databases->end())
			return TemporaryDatabaseMap::getValue(iterator).get();
	}
	return 0;
}

//	FUNCTION public
//	Schema::TemporaryDatabase::drop --
//		あるセッションに対応するデータベースを抹消する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			一時データベースを抹消しようとしているトランザクション記述子
//
//	RETURN
//		なし
//
//	EXCEPTIONS

// static
void
TemporaryDatabase::
drop(SessionID iSessionID_)
{
	Os::AutoCriticalSection m(*_pCriticalSection);

	// ハッシュマップから抹消する
	// 一時データベースはクラスを破棄すると
	// データベースもdropされる

	if (_IsValidPointer(_databases))
		_databases->erase(iSessionID_);
}

//
// Copyright (c) 2000, 2001, 2002, 2005, 2007, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
