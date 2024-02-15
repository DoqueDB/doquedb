// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Configuration.h -- チェックポイント処理マネージャーの設定関連の関数宣言
// 
// Copyright (c) 2000, 2001, 2002, 2005, 2009, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_CHECKPOINT_CONFIGURATION_H
#define	__SYDNEY_CHECKPOINT_CONFIGURATION_H

#include "Checkpoint/Module.h"

_SYDNEY_BEGIN
_SYDNEY_CHECKPOINT_BEGIN

//	NAMESPACE
//	Checkpoint::Configuration --
//		チェックポイント処理マネージャーの設定に関する名前空間
//
//	NOTES

namespace Configuration
{
#ifdef OBSOLETE
	// 設定をすべてシステムパラメーターから読み出す
	void					get();
#endif
	// 設定のリセットを行う
	void					reset();

	//	NAMESPACE
	//	Checkpoint::Configuration::Period --
	//		チェックポイント処理と処理の間の時間の設定に関する名前空間
	//
	//	NOTES

	namespace Period
	{
		//	CONST
		//	Checkpoint::Configuration::Period::Name --
		//		パラメーター名を表す定数
		//
		//	NOTES

		const char* const	Name = "Period";

		//	CONST
		//	Checkpoint::Configuration::Period::Default --
		//		デフォルト値を表す定数
		//
		//	NOTES

		const unsigned int	Default = 30 * 60 * 1000;

		// パラメーター値を取得する
		unsigned int		get();
		// 記録しているパラメーター値を忘れる
		void				reset();
	}

	//	NAMESPACE
	//	Checkpoint::Configuration::TruncateLogicalLog --
	//		チェックポイント処理の終了時に可能であれば
	//		論理ログファイルをトランケートするかの設定に関する名前空間
	//
	//	NOTES

	namespace TruncateLogicalLog
	{
		//	CONST
		//	Checkpoint::Configuration::TruncateLogicalLog::Name --
		//		パラメーター名を表す定数
		//
		//	NOTES

		const char* const	Name = "TruncateLogicalLog";

		//	CONST
		//	Checkpoint::Configuration::TruncateLogicalLog::Default --
		//		デフォルト値を表す定数
		//
		//	NOTES

		const bool			Default = true;

		// パラメーター値を取得する
		bool				get();
		// 記録しているパラメーター値を忘れる
		void				reset();
	}

	//	NAMESPACE
	//	Checkpoint::Configuration::EnableFileSynchronizer --
	//		バージョンファイル同期スレッドを起動するかの設定に関する名前空間
	//
	//	NOTES

	namespace EnableFileSynchronizer
	{
		//	CONST
		//	Checkpoint::Configuration::EnableFileSynchronizer::Name --
		//		パラメーター名を表す定数
		//
		//	NOTES

		const char* const	Name = "EnableFileSynchronizer";

		//	ENUM
		//	Checkpoint::Configuration::EnableFileSynchronizer::Value --
		//		動作タイプをあらわす数値

		enum Value
		{
			SIZE,		// サイズ優先
						// 同期処理をチェックポイントのたびに実行する
			SPEED,		// スピード優先
						// 更新されているデータベースの同期処理はスキップする
			OFF			// 同期処理をまったく実行しない
		};

		//	CONST
		//	Checkpoint::Configuration::EnableFileSynchronizer::Default --
		//		デフォルト値を表す定数
		//
		//	NOTES

		const Value			Default = SPEED;

		// パラメーター値を取得する
		Value				get();
		// 記録しているパラメーター値を忘れる
		void				reset();
	}

	//	NAMESPACE
	//	Checkpoint::Configuration::TimeStampTableSize --
	//		データベースごとにチェックポイント処理時のタイムスタンプを管理する
	//		ハッシュ表のサイズの設定に関する名前空間
	//
	//	NOTES

	namespace TimeStampTableSize
	{
		//	CONST
		//	Checkpoint::Configuration::TimeStampTableSize::Name --
		//		パラメーター名を表す定数
		//
		//	NOTES

		const char* const	Name = "TimeStampTableSize";

		//	CONST
		//	Checkpoint::Configuration::TimeStampTableSize::Default --
		//		デフォルト値を表す定数
		//
		//	NOTES

		const unsigned int	Default = 7;

		// パラメーター値を取得する
		unsigned int		get();
		// 記録しているパラメーター値を忘れる
		void				reset();
	}

	//	NAMESPACE
	//	Checkpoint::Configuration::LoadSynchronizeCandidate --
	//		同期処理の候補を最初のチェックポイント時にロードするかどうかの
	//		設定に関する名前空間
	//
	//	NOTES

	namespace LoadSynchronizeCandidate
	{
		//	CONST
		//	Checkpoint::Configuration::LoadSynchronizeCandidate::Name --
		//		パラメーター名を表す定数
		//
		//	NOTES

		const char* const	Name = "LoadSynchronizeCandidate";

		//	CONST
		//	Checkpoint::Configuration::LoadSynchronizeCandidate::Default --
		//		デフォルト値を表す定数
		//
		//	NOTES

		const bool	Default = true;

		// パラメーター値を取得する
		bool				get();
		// 記録しているパラメーター値を忘れる
		void				reset();
	}
}

_SYDNEY_CHECKPOINT_END
_SYDNEY_END

#endif	// __SYDNEY_CHECKPOINT_CONFIGURATION_H

//
// Copyright (c) 2000, 2001, 2002, 2005, 2009, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
