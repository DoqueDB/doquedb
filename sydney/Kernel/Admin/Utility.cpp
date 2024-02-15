// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Utility.cpp -- ユーティリティ関数定義
// 
// Copyright (c) 2001, 2002, 2004, 2005, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Admin";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Admin/Module.h"
#include "Admin/Utility.h"
#include "Admin/Verification.h"

#include "Common/Assert.h"
#include "Common/DataArrayData.h"
#include "Common/StringData.h"
#include "Communication/Connection.h"
#include "Os/AutoCriticalSection.h"
#include "Os/CriticalSection.h"
#include "Schema/Area.h"
#include "Schema/Database.h"
#include "Trans/Transaction.h"

#include "ModAutoPointer.h"
#include "ModMap.h"

_SYDNEY_USING
_SYDNEY_ADMIN_USING

namespace
{
	//
	// TYPEDEF local
	// 	TransactionList -- トランザクションマップ
	//
	// NOTES
	//	データベース名を Key に、トランザクション ID を保持するクラス
	//	本来はデータベース名だけでいいが、検索を早くする為に Map を使う。
	//
	typedef ModMap<ModUnicodeString, Trans::Transaction::ID , ModLess<ModUnicodeString> >
														BackupList;

	//
	// VALUE static
	// 	TransactionList -- エラー値マップ
	//
	// NOTES
	//	エラー値を格納するマップ
	//
	ModAutoPointer< BackupList >						l_pBackupDBList;

	//
	// VALUE static
	//	l_cCriticalSection -- TransactionList 用のクリティカルセクション
	//
	// NOTES
	//	TransactionList 用のクリティカルセクション
	//
	static Os::CriticalSection							l_cCriticalSection;
}

//
//	FUNCTION public
//	Admin::Utility::makePathList
//		-- Schema::Database が使用しているパスのリストを作成する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			トランザクション記述子
//
//		const Schema::Database& cDatabase_
//			対象データベース
//
//		Utility::PathList& cPathList_
//			パス格納リスト
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま送出
//
void
Utility::
makePathList(Trans::Transaction& cTrans_,
			 const Schema::Database& cDatabase_, Utility::PathList& cPathList_)
{
	// 使用領域を確保
	cPathList_.reserve(Schema::Database::Path::Category::System);

	// Database で使用しているパスリストを追加
	cPathList_.addUnique(
		cDatabase_.getPath(Schema::Database::Path::Category::Data));
	cPathList_.addUnique(
		cDatabase_.getPath(Schema::Database::Path::Category::LogicalLog));
	cPathList_.addUnique(
		cDatabase_.getPath(Schema::Database::Path::Category::System));

	// エリアで使用しているパスを全て追加
	ModVector<Schema::Area*> cAreaList = cDatabase_.getArea(cTrans_);
	ModVector<Schema::Area*>::ConstIterator area = cAreaList.begin();
	ModVector<Schema::Area*>::ConstIterator fin = cAreaList.end();
	for ( ; area != fin; area++ )
	{
		int cntmax = (*area)->getSize();

		// 領域を予約
		cPathList_.reserve(cPathList_.getSize() + cntmax);

		for ( int cnt = 0; cnt < cntmax; cnt++ )
			cPathList_.addUnique((*area)->getPath(cnt));
	}
}

//	FUNCTION public
//	Admin::Utility::setResult -- コネクションに戻り値を設定する
//
//	NOTES
//
//	ARGUMENTS
//		Communication::Connection&	connection
//			戻り値を設定するコネクションを表すクラス
//		Admin::Utility::PathList&	list
//			戻り値であるパス名のリスト
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Utility::setResult(Communication::Connection& connection,
				   const Utility::PathList& list)
{
	const unsigned int n = list.getSize();
	for (unsigned int i = 0; i < n; ++i) {
		Common::ObjectPointer<Common::DataArrayData>
			array(new Common::DataArrayData());
		; _SYDNEY_ASSERT(list[i]);
		array->pushBack(new Common::StringData(*list[i]));

		connection.writeObject(array.get());
	}

	// EOD を返す

	connection.writeObject(0);
}

//	FUNCTION
//	Admin::Utility::setResult -- コネクションに戻り値を設定する
//
//	NOTES
//
//	ARGUMENTS
//		Communication::Connection&	connection
//			戻り値を設定するコネクションを表すクラス
//		Verification::Progress&	result
//			戻り値を得るための整合性検査の経過を表すクラス
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Utility::setResult(Communication::Connection& connection,
				   const Verification::Progress& result)
{
#ifndef ADMIN_RETURN_INTERMEDIATE_RESULT
	// 整合性検査の結果の詳細な説明を表す配列を
	// ひとつひとつタプルとみなして返す

	const Common::DataArrayData& description = result.getDescription();
	const int n = description.getCount();
	for (int i = 0; i < n; ++i)
		connection.writeObject(description.getElement(i).get());
#endif

	// EOD を返す

	connection.writeObject(0);
}

//---------------------------------------------------------------------------
//	Backup Utility

//	FUNCTION public
//	Admin::Utility::Backup::enter
//		-- バックアップをするＤＢ名を登録する
//
//	NOTES
//
//	ARGUMENTS
//	const ModUnicodeString& strDBName_
//		データベース名
//
//	Trans::Transaction&	trans
//		トランザクション記述子
//
//	RETURN
//	bool	true  : 登録に成功した
//			false : 登録に失敗した
//
//	EXCEPTIONS

bool
Utility::Backup::enter(
	const ModUnicodeString& dbName, const Trans::Transaction& trans)
{
	Os::AutoCriticalSection cAuto(l_cCriticalSection);

	if (!l_pBackupDBList.get())

		// バックアップ対象のデータベースと、
		// バックアップしているトランザクションを記憶するためのマップを確保する

		l_pBackupDBList = new BackupList;

	if (l_pBackupDBList->find(dbName) != l_pBackupDBList->end())

		// バックアップ対象のデータベースはすでにバックアップが開始されている

		return false;

	// バックアップをするデータベースと、
	// バックアップを開始するトランザクションを記憶する

	l_pBackupDBList->insert(dbName, trans.getID());
	return true;
}

//	FUNCTION public
//	Admin::Utility::Backup::isEntered
//		-- バックアップ中のＤＢ名がないか問い合わせる
//
//	NOTES
//
//	ARGUMENTS
//	const ModUnicodeString& strDBName_
//		データベース名
//
//	RETURN
//	bool	true  : バックアップ中のデータベースが有る
//			false : バックアップ中のデータベースは無い
//
//	EXCEPTIONS

#ifdef OBSOLETE
bool
Utility::Backup::isEntered(const ModUnicodeString& dbName)
{
	Os::AutoCriticalSection cAuto(l_cCriticalSection);

	return l_pBackupDBList.get() &&
		l_pBackupDBList->find(dbName) != l_pBackupDBList->end();
}
#endif

bool
Utility::Backup::isEntered(
	const ModUnicodeString& dbName, const Trans::Transaction& trans)
{
	Os::AutoCriticalSection cAuto(l_cCriticalSection);

	if (l_pBackupDBList.get()) {
		const BackupList::Iterator& ite = l_pBackupDBList->find(dbName);
		return ite != l_pBackupDBList->end() && (*ite).second == trans.getID();
	}

	return false;
}

//	FUNCTION public
//	Admin::Utility::Backup::leave
//		-- バックアップをするＤＢ名を解除する
//
//	NOTES
//
//	ARGUMENTS
//	const ModUnicodeString& strDBName_
//		データベース名
//
//	RETURN
//	なし
//
//	EXCEPTIONS

void
Utility::Backup::leave(
	const ModUnicodeString& dbName, const Trans::Transaction& trans)
{
	Os::AutoCriticalSection cAuto(l_cCriticalSection);

	if (l_pBackupDBList.get()) {

		// マップが存在するということは、なにかが登録されているはずである

		; _SYDNEY_ASSERT(l_pBackupDBList->getSize());

		const BackupList::Iterator& ite = l_pBackupDBList->find(dbName);
		if (ite != l_pBackupDBList->end() && (*ite).second == trans.getID()) {

			// バックアップを開始していることを忘れる

			l_pBackupDBList->erase(ite);

			if (!l_pBackupDBList->getSize())

				// マップが空になれば、マップを破棄する

				l_pBackupDBList = 0;
		}
	}
}

//	FUNCTION public
//	Admin::Utility::Backup::getDatabaseName
//		-- 指定されたトランザクションがバックアップ中のデータベース名を取得する
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transactin& cTrans_
//		対象となるトランザクション記述子
//	ModVector< ModUincodeString >& cDBName_
//		データベース名が格納されている配列
//
//	RETURN
//	なし
//
//	EXCEPTIONS

void
Utility::Backup::getEnteredDatabaseName(
	const Trans::Transaction& trans, ModVector<ModUnicodeString>& dbNames)
{
	; _SYDNEY_ASSERT(dbNames.isEmpty());

	Os::AutoCriticalSection cAuto(l_cCriticalSection);

	if (l_pBackupDBList.get()) {

		// 指定されたトランザクションのトランザクション識別子を得る

		const Trans::ID& id = trans.getID();

		BackupList::Iterator 		ite(l_pBackupDBList->begin());
		const BackupList::Iterator& end = l_pBackupDBList->end();

		for (; ite != end; ++ite)
			if ((*ite).second == id)
				dbNames.pushBack((*ite).first);
	}
}

//	FUNCTION public
//	Admin::Utility::PathList::addUnique -- 
//		重複したものが登録されていなければ、登録する
//
//	NOTES
//
//	ARGUMENTS
//		Os::Path&	v
//			登録するパス名
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Utility::PathList::addUnique(const Os::Path& v)
{
	const unsigned int n = getSize();
	for (unsigned int i = 0; i < n; ++i) {
		; _SYDNEY_ASSERT((*this)[i]);
		switch ((*this)[i]->compare(v)) {
		case Os::Path::CompareResult::Identical:
		case Os::Path::CompareResult::Parent:

			// 与えられたパス名と同じものがすでに登録されている
			// または、与えられたものの親がすでに登録されている

			return;

		case Os::Path::CompareResult::Child:

			// 与えられたものの子が登録されていれば、置き換える

			delete (*this)[i];
			(*this)[i] = new Os::Path(v);
			return;
		}
	}

	// 登録する

	reserve(n + 1);
	pushBack(new Os::Path(v));
}

//
//	Copyright (c) 2001, 2002, 2004, 2005, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
