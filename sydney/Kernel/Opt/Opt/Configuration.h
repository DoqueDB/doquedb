// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Configuration.h -- Optimizerで使うパラメータ
// 
// Copyright (c) 2000, 2008, 2010, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_OPT_CONFIGURATION_H
#define __SYDNEY_OPT_CONFIGURATION_H

#include "Opt/Module.h"

#include "Common/Configuration.h"
#include "Common/Message.h"

_SYDNEY_BEGIN
_SYDNEY_OPT_BEGIN

namespace Configuration
{
	/////////////////////////////////
	// よく使うパラメーター
	/////////////////////////////////

	// Opt::Optimizer::optimize に渡されたパラメータを出力する先
	Common::Configuration::ParameterMessage& getParameterOutput();
	// 使用するオプティマイザーバージョン
	Common::Configuration::ParameterIntegerInRange& getOptimizerVersion();
	// 現バージョンでNotSupportedのときに古いオプティマイザーバージョンを使用する
	Common::Configuration::ParameterBoolean& getUseOlderVersion();
	// Optimizerのトレースメッセージ
	Common::Configuration::ParameterMessage& getTraceOptimization();
	// Executorのトレースメッセージ
	Common::Configuration::ParameterMessage& getTraceExecution();
	// 数値計算のオーバーフローで例外の代わりにNULLにする
	Common::Configuration::ParameterBoolean& getOverflowNull();
	// PREDICATEの計算でUNKNOWNをFALSEとして扱う
	Common::Configuration::ParameterBoolean& getNoUnknown();
	// UNION展開の最大数
	Common::Configuration::ParameterInteger& getUnionMaxNumber();
	// LIKEの計算で正規化パラメーターを指定する
	Common::Configuration::ParameterInteger& getLikeNormalizedString();
	// トレースで出力する文字列の最大長
	Common::Configuration::ParameterInteger& getTraceMaxStringSize();
	// Bulkで読み込むデータの最大サイズ
	Common::Configuration::ParameterIntegerInRange& getBulkMaxSize();
	// Bulkでログに書き込むデータの最大サイズ
	Common::Configuration::ParameterIntegerInRange& getBulkMaxDataLengthInLog();
	// Bulkで外部ファイルに出力するデータの閾値
	Common::Configuration::ParameterIntegerInRange& getBulkExternalThreshold();
	// Bulkで使用するバッファサイズ
	Common::Configuration::ParameterIntegerInRange& getBulkBufferSize();
	// Collectionからの取得をファイルコストで計算する閾値
	Common::Configuration::ParameterIntegerInRange& getCollectionThreshold();
	// Joinの候補最大数
	Common::Configuration::ParameterInteger& getJoinMaxCandidates();
	// Executorが同時に使用するThread数の上限
	Common::Configuration::ParameterIntegerInRange& getThreadMaxNumber();

	// オプティマイザーバージョンの値
	struct OptimizerVersion
	{
		enum Value
		{
			Version1 = 0,
			Version2,
			Current = Version2,
			ValueNum
		};
	};

	// TraceLevelの列挙子
	namespace TraceLevel
	{
		typedef unsigned int Value;
		enum Bit
		{
			Normal		= 0,			// (0000)least trace output
			Process		= 1,			// (0001)output intermediate process trace
			File		= 1 << 1,		// (0002)output file level
			Cost		= 1 << 2,		// (0004)output cost value
			RowInfo		= 1 << 3,		// (0008)output rowinfo for each relation
			Contents	= 1 << 4,		// (0010)output collection contents
			Variable	= 1 << 5,		// (0020)output variable pointer
			Lock		= 1 << 6,		// (0040)trace locking
			Log			= 1 << 7,		// (0080)trace logging
			Thread		= 1 << 8,		// (0100)trace thread manipulation
			Time		= 1 << 9,		// (0200)output execution process time
			ValueNum
		};

		// TraceLevelのビットが立っているかを検査する関数
		bool isOn(Value uLevel_);
	}

} // namespace Configuration

_SYDNEY_OPT_END

// Message
#define _OPT_IS_TRACELEVEL(_level_) \
	Opt::Configuration::TraceLevel::isOn(_level_)
#define _OPT_IS_OUTPUT_OPTIMIZATION(_level_) \
	(Opt::Configuration::getTraceOptimization().isOutput() && _OPT_IS_TRACELEVEL(_level_))
#define _OPT_IS_OUTPUT_OPTIMIZATION_PROCESS(_level_) \
	_OPT_IS_OUTPUT_OPTIMIZATION(Opt::Configuration::TraceLevel::Process | (_level_))
#define _OPT_IS_OUTPUT_EXECUTION(_level_) \
	(Opt::Configuration::getTraceExecution().isOutput() && _OPT_IS_TRACELEVEL(_level_))

#define _OPT_OPTIMIZATION_MESSAGE \
	_SYDNEY_MESSAGE(Opt::Configuration::getTraceOptimization().getParameterName(), Common::MessageStreamBuffer::LEVEL_DEBUG)

#define _OPT_EXECUTION_MESSAGE \
	_SYDNEY_MESSAGE(Opt::Configuration::getTraceExecution().getParameterName(), Common::MessageStreamBuffer::LEVEL_DEBUG)

_SYDNEY_END

#endif //__SYDNEY_OPT_CONFIGURATION_H

//
//	Copyright (c) 2000, 2008, 2010, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
