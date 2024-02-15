// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Hold.h -- スキーマモジュールで使うロックに関する定義
// 
// Copyright (c) 2001, 2008, 2023 Ricoh Company, Ltd.
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

#ifndef	__SYDNEY_SCHEMA_HOLD_H
#define	__SYDNEY_SCHEMA_HOLD_H

#include "Schema/Module.h"

#include "Lock/Duration.h"
#include "Lock/Mode.h"
#include "Lock/Name.h"
#include "Lock/Timeout.h"

_SYDNEY_BEGIN

namespace Trans
{
	class Transaction;
}

_SYDNEY_SCHEMA_BEGIN

namespace Hold
{
	namespace Operation {
		//	ENUM public
		//	Schema::Hold::Operation::Value -- ロック後に行う操作を表す型
		//
		//	NOTES
		//		ロックをかけるときにモードを決定するのに用いる
		enum Value {
			Drop			= 0,				// オブジェクトの破棄
			MoveDatabase,						// データベースの移動
			ReadForWrite,						// 後で書くために読む
			ReadForImport,						// 再構成のために読む

			// 以下の Operation は Trans::Transaction のものを使う
			// テーブルの実装上の都合で
			// 以下の定義は必ずスキーマ独自のものより後である必要がある

			ReadOnly,							// 読み取りのみ
			ReadWrite,							// 読み書き
			Unknown,							// 不明
			ValueNum
		};
	}

	namespace Target {
		//	ENUM public
		//	Schema::Hold::Target::Value -- ロックの対象を表す型
		//
		//	NOTES
		enum Value {
			MetaDatabase,						// メタデータベース
			MetaTable,							// データベースのシステム表
			MetaTuple,							// システム表のタプル
			Database,							// データベース
			Table,								// 表
			Tuple,								// タプル
			LogicalLog,							// 論理ログファイル
			ValueNum
		};
	}

	SYD_SCHEMA_FUNCTION
	bool			getAdequateLock(Trans::Transaction& cTrans_,
									Lock::Name::Category::Value eLocked_,
									Lock::Name::Category::Value eManipulate_,
									Operation::Value eOperation_,
									Lock::Mode::Value& eReturnMode_,
									Lock::Duration::Value& eReturnDuration_);

	SYD_SCHEMA_FUNCTION
	bool			hold(Trans::Transaction& cTrans_,
						 const Lock::Name& cLockName_,
						 Hold::Target::Value eTarget_,
						 Lock::Name::Category::Value eManipulate_,
						 Hold::Operation::Value eOperation_,
						 Lock::Timeout::Value iTimeout_
							 = Lock::Timeout::Unlimited);
												// 適切なロックモードで
												// 指定された対象をロックする
	SYD_SCHEMA_FUNCTION
	bool			convert(Trans::Transaction& cTrans_,
							const Lock::Name& cLockName_,
							Hold::Target::Value iTargets_,
							Lock::Name::Category::Value eManipulateFrom_,
							Hold::Operation::Value eOperationFrom_,
							Lock::Name::Category::Value eManipulateTo_,
							Hold::Operation::Value eOperationTo_,
							Lock::Timeout::Value iTimeout_
								= Lock::Timeout::Unlimited);
												// 指定された対象のロックを
												// 適切なロックモードに変換する
	SYD_SCHEMA_FUNCTION
	void			release(Trans::Transaction& cTrans_,
							const Lock::Name& cLockName_,
							Hold::Target::Value eTarget_,
							Lock::Name::Category::Value eManipulate_,
							Hold::Operation::Value eOperation_);
												// 必要ならアンロックする
}

_SYDNEY_SCHEMA_END
_SYDNEY_END

#endif	// __SYDNEY_SCHEMA_HOLD_H

//
// Copyright (c) 2001, 2008, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
