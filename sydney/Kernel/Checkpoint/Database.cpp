// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Database.cpp --
// 
// Copyright (c) 2002, 2003, 2005, 2007, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Checkpoint";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyInclude.h"

#include "Checkpoint/Database.h"
#include "Checkpoint/Manager.h"
#include "Checkpoint/TimeStamp.h"

#include "Common/Assert.h"
#include "Os/AutoCriticalSection.h"
#include "Schema/Database.h"
#include "Schema/Manager.h"
#include "Trans/Transaction.h"

_SYDNEY_USING
_SYDNEY_CHECKPOINT_USING

namespace
{

namespace _Database
{
	// 以下の情報を保護するためのラッチ
	Os::CriticalSection		_latch;
	// 利用不可なデータベースを管理するマップ
	Checkpoint::Database::UnavailableMap*	_unavailableMap = 0;
}

}

//	FUNCTION private
//	Checkpoint::Manager::Database::initialize --
//		マネージャーの初期化のうち、データベース関連の初期化を行う
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
Manager::Database::initialize()
{}

//	FUNCTION private
//	Checkpoint::Manager::Database::terminate --
//		マネージャーの後処理のうち、データベース関連の後処理を行う
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
Manager::Database::terminate()
{
	if (_Database::_unavailableMap)

		// 利用不可なデータベースを管理するマップを破棄する

		delete _Database::_unavailableMap, _Database::_unavailableMap = 0;
}

//	FUNCTION public
//	Checkpoint::Database::setAvailability --
//		複数のデータベースの利用可能性を設定する
//
//	NOTES
//
//	ARGUMENTS
//		Checkpoint::Database::UnavailableMap&	map
//			複数の利用不可なデータベースの名前とそれを回復するときの
//			開始時点のタイムスタンプ値を管理するためのマップ
//
//	RETURN
//		なし
//
//	EXCEPTIONS

// static
void
Checkpoint::Database::
setAvailability(const UnavailableMap& map)
{
	if (map.getSize()) {
		Os::AutoCriticalSection	latch(_Database::_latch);

		if (!_Database::_unavailableMap)
			_Database::_unavailableMap =
				new Checkpoint::Database::UnavailableMap(map);
		else
			_Database::_unavailableMap->insert(map.begin(), map.end());
	}
}

//	FUNCTION public
//	Checkpoint::Database::setAvailability -- データベースの利用可能性を設定する
//
//	NOTES
//
//	ARGUMENTS
//		Schema::ObjectID::Value	dbID
//			利用可能性を設定するデータベースのスキーマオブジェクト識別子
//		bool				v
//			true
//				データベースを利用可能にする
//			false
//				データベースを利用不可にする
//		const Trans::TimeStamp&	timestamp
//			利用不能になったタイムスタンプ
//			利用不可なデータベースのリカバリーに失敗した場合のみ設定する
//
//	RETURN
//		true
//			設定前はデータベースは利用可能だった
//		false
//			設定前はデータベースは利用不可だった
//
//	EXCEPTIONS

// static
bool
Checkpoint::Database::setAvailability(Schema::ObjectID::Value dbID,
									  bool v,
									  const Trans::TimeStamp& timestamp)
{
	Os::AutoCriticalSection	latch(_Database::_latch);

	if (v)

		// あるデータベースを利用可能にするとき、
		// 利用不可なデータベースを管理するマップが存在し、
		// 指定されたスキーマオブジェクト識別子が
		// そのマップに登録されていれば、抹消する

		return (_Database::_unavailableMap) ?
			!_Database::_unavailableMap->erase(dbID) : true;

	// あるデータベースを利用不可にする

	if (!_Database::_unavailableMap) {

		// 利用不可なデータベースを
		// 管理するマップが存在しないので、生成する

		_Database::_unavailableMap =
			new Checkpoint::Database::UnavailableMap();
		; _SYDNEY_ASSERT(_Database::_unavailableMap);

	} else if (_Database::_unavailableMap->find(dbID) !=
			   _Database::_unavailableMap->end())

		// 指定されたスキーマオブジェクト識別子が
		// マップに登録されているので、すでに利用不可である

		return false;

	// 指定されたスキーマオブジェクト識別子は
	// 登録されていないので、登録する
	//
	//【注意】	論理ファイルが利用不可になったために
	//			データベースが利用不可になった場合と区別するために、
	//			利用不可な論理ファイルのスキーマオブジェクト識別子として、
	//			不正なスキーマオブジェクト識別子を設定しておく

	UnavailableInfo	info;
	info._files.pushBack(Schema::ObjectID::Invalid);
	if (!timestamp.isIllegal())
		info._t = timestamp;

	(void) _Database::_unavailableMap->insert(dbID, info);

	return true;
}

//	FUNCTION public
//	Checkpoint::Database::setAvailability -- 論理ファイルの利用可能性を設定する
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
Checkpoint::Database::setAvailability(
	Schema::ObjectID::Value dbID, Schema::ObjectID::Value fileID, bool v)
{
	Os::AutoCriticalSection	latch(_Database::_latch);

	if (v) {

		// ある論理ファイルを利用可能にする

		if (_Database::_unavailableMap) {

			// 利用不可なデータベースを管理するマップに
			// 指定されたデータベースの
			// スキーマオブジェクト識別子が登録されているか調べる

			UnavailableMap::Iterator ite0(
				_Database::_unavailableMap->find(dbID));
			if (ite0 != _Database::_unavailableMap->end()) {

				// 指定されたスキーマオブジェクト識別子の表す
				// 論理ファイルが登録されていれば、抹消する

				UnavailableInfo& info = (*ite0).second;
				ModVector<Schema::ObjectID::Value>::Iterator
					ite1(info._files.find(fileID));
				if (ite1 != info._files.end()) {
					(void) info._files.erase(ite1);

					if (info._files.isEmpty())

						// 論理ファイルが利用不可になっていたために
						// 利用不可になっていたデータベースなので、
						// その登録を抹消する

						(void) _Database::_unavailableMap->erase(ite0);
				}

				// 現状ではデータベースが利用不可ならば、
				// 論理ファイルもファイル不可である

				return false;
			}
		}
		return true;
	}

	// ある論理ファイルを利用不可にする

	if (!_Database::_unavailableMap) {

		// 利用不可なデータベースを
		// 管理するマップが存在しないので、生成する

		_Database::_unavailableMap =
			new Checkpoint::Database::UnavailableMap();
		; _SYDNEY_ASSERT(_Database::_unavailableMap);
	} else {

		// 指定されたスキーマオブジェクト識別子の表す
		// データベースに関する情報が登録されているか調べる

		UnavailableMap::Iterator ite(_Database::_unavailableMap->find(dbID));
		if (ite != _Database::_unavailableMap->end()) {

			// 指定されたスキーマオブジェクト識別子の表す
			// 論理ファイルが登録されているか調べる

			UnavailableInfo& info = (*ite).second;
			if (info._files.find(fileID) == info._files.end())

				// 登録されていないので、登録する

				info._files.pushBack(fileID);

			// 現状ではデータベースが利用不可であれば、
			// 論理ファイルも利用不可である

			return false;
		}
	}

	// 指定された論理ファイルが登録済のデータベースに関する情報を登録する

	UnavailableInfo	info;
	info._files.pushBack(fileID);

	(void) _Database::_unavailableMap->insert(dbID, info);

	return true;
}

//	FUNCTION public
//	Checkpoint::Database::isAvailable --
//		システムデータベースを含むすべてのデータベースが利用可能か調べる
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		true
//			すべて利用可能である
//		false
//			利用不可なデータベースがある
//
//	EXCEPTIONS

// static
bool
Checkpoint::Database::isAvailable()
{
	Os::AutoCriticalSection	latch(_Database::_latch);

	return !(_Database::_unavailableMap &&
			 _Database::_unavailableMap->getSize());
}

//	FUNCTION public
//	Checkpoint::Database::isAvailable -- データベースが利用可能か調べる
//
//	NOTES
//
//	ARGUMENTS
//		Schema::ObjectID::Value	dbID
//			利用可能か調べるデータベースのスキーマオブジェクト識別子
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
Checkpoint::Database::isAvailable(Schema::ObjectID::Value dbID)
{
	Os::AutoCriticalSection	latch(_Database::_latch);

	return !(_Database::_unavailableMap &&
			 _Database::_unavailableMap->find(dbID) !=
			 _Database::_unavailableMap->end());
}

//	FUNCTION public
//	Checkpoint::Database::isAvailable -- 論理ファイルが利用不可か調べる
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
Checkpoint::Database::isAvailable(
	Schema::ObjectID::Value dbID, Schema::ObjectID::Value fileID)
{
	// 現状ではデータベースが利用不可であれば、
	// 論理ファイルも利用不可である
	//
	// 逆に、論理ファイルが利用不可であれば、
	// その論理ファイルを使用するデータベースも利用不可である

	return isAvailable(dbID);
}

//	FUNCTION private
//	Checkpoint::Database::getUnavailable -- 利用不可なデータベースの一覧を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		利用不可なデータベースのスキーマオブジェクト識別子と
//		それを回復するときの開始時点のタイムスタンプ値を管理するためのマップ
//
//	EXCEPTIONS

// static
const Checkpoint::Database::UnavailableMap&
Checkpoint::Database::getUnavailable()
{
	Os::AutoCriticalSection	latch(_Database::_latch);

	if (!_Database::_unavailableMap) {

		// 利用不可なデータベースの名前を
		// 管理するマップが存在しないので、生成する

		_Database::_unavailableMap =
			new Checkpoint::Database::UnavailableMap();
		; _SYDNEY_ASSERT(_Database::_unavailableMap);
	}

	return *_Database::_unavailableMap;
}

//	FUNCTION private
//	Checkpoint::Database::setStartRecoveryTime --
//		利用不可なデータベースを回復するときの
//		開始時点のタイムスタンプを求め、設定する
//
//	NOTES
//		チェックポイント処理中に、そのチェックポイント処理の
//		終了時のタイムスタンプを求める前に呼び出される
//
//	ARGUMENTS
//		Trans::Transaction&	trans
//			チェックポイント処理で使用する
//			トランザクションのトランザクション記述子
//
//	RETURN
//		なし
//
//	EXCEPTIONS

// static
void
Checkpoint::Database::setStartRecoveryTime(Trans::Transaction& trans)
{
	; _SYDNEY_ASSERT(trans.isInProgress());

	Os::AutoCriticalSection	latch(_Database::_latch);

	if (_Database::_unavailableMap) {

		// システム表が利用可能であれば、利用不可なデータベースごとに、
		// それぞれを回復するときの開始時点のタイムスタンプを求める

		UnavailableMap::Iterator	ite(_Database::_unavailableMap->begin());
		const UnavailableMap::Iterator&
			end = _Database::_unavailableMap->end();
		UnavailableMap::Iterator	next;

		for (; ite != end; ite = next) {

			// 反復子の指す要素を削除すると、
			// 次の要素が得られなくなるので、
			// ここで次の要素を指す反復子を求めておく

			++(next = ite);

			UnavailableInfo& info = (*ite).second;
			if (Trans::TimeStamp::isIllegal(info._t))

				// 利用不可なデータベースを回復するときの
				// 開始時点のタイムスタンプが設定されていないので、
				// 2 つ前のチェックポイント処理時のタイムスタンプを設定する
				//
				//【注意】	Schema::Database::get は
				//			利用不可なデータベースに関する情報を取得できない

				if (const Schema::Database* database =
					Schema::Manager::ObjectTree::Database::get(
						(*ite).first, trans))

					// 利用不可になったデータベースは
					// データベース表に登録されている

					info._t = TimeStamp::getSecondMostRecent(
						database->getLogFile()->getLockName());
				else
					// データベースの DROP や UNMOUNT 時に
					// データベースが利用不可になってしまったため、
					// そのデータベースはデータベース表に登録されていないので、
					// 利用不可であるという登録も抹消する

					_Database::_unavailableMap->erase(ite);
		}
	}
}

//
// Copyright (c) 2002, 2003, 2005, 2007, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
