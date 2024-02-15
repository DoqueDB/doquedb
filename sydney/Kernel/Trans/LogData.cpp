// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// LogData.cpp -- 論理ログデータ関連の関数定義
// 
// Copyright (c) 2000, 2001, 2002, 2003, 2004, 2007, 2012, 2014, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Trans";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Trans/LogData.h"

#include "ModArchive.h"
#include "ModUnicodeOstrStream.h"

_SYDNEY_USING
_SYDNEY_TRANS_USING
_SYDNEY_TRANS_LOG_USING

namespace
{
}

//	FUNCTION public
//	Trans::Log::Data::serialize -- クラスをシリアル化する
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
Data::serialize(ModArchive& archiver)
{
	// 自分自身をシリアル化する

	if (archiver.isStore()) {
		{
		int tmp = _category;
		archiver << tmp;
		}
		archiver << _timeStamp;
	} else {
		{
		int tmp;
		archiver >> tmp;
		_category = static_cast<Category::Value>(tmp);
		}
		archiver >> _timeStamp;
	}
}

#ifndef SYD_COVERAGE
//	FUNCTION public
//	Trans::Log::Data::toString -- オブジェクトを表す文字列を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたユニコード文字列
//
//	EXCEPTIONS

ModUnicodeString
Data::toString() const
{
	static char* category[] =
	{
		"Unknown",
		"TimeStampAssign",
		"TransactionBegin",
		"TransactionCommit",
		"TransactionRollback",
		"StatementCommit",
		"StatementRollback",
		"CheckpointDatabase",
		"CheckpointSystem",
		"TupleModify",
		"SchemaModify",
		"FileSynchronizeBegin",
		"FileSynchronizeEnd",
		"TransactionPrepare",
		"BranchHeurDecide",
		"BranchForget",
		"DriverModify",
		"StartBatch",
		"XATransaction",
		"ReplicationEnd"
	};

	ModUnicodeOstrStream	buf;
	buf << moduleName << "::Log::Data {"
		<< getTimeStamp() << "," << category[getCategory()] << "}";
	return buf.getString();
}
#endif

//	FUNCTION public
//	Trans::Log::InsideTransactionData::serialize -- クラスをシリアル化する
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
InsideTransactionData::serialize(ModArchive& archiver)
{
	// まず、親クラスをシリアル化する

	Data::serialize(archiver);

	// 自分自身をシリアル化する

	if (archiver.isStore())
		archiver << _beginTransactionLSN << _endStatementLSN << _backwardLSN;
	else
		archiver >> _beginTransactionLSN >> _endStatementLSN >> _backwardLSN;
}

//	FUNCTION public
//	Trans::Log::ModificationData::serialize -- クラスをシリアル化する
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
ModificationData::serialize(ModArchive& archiver)
{
	// まず、親クラスをシリアル化する

	InsideTransactionData::serialize(archiver);

	// 自分自身をシリアル化する

	archiver(_undoable);
}

//	FUNCTION public
//	Trans::Log::TransactionPrepareData::serialize -- クラスをシリアル化する
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
TransactionPrepareData::serialize(ModArchive& archiver)
{
	// まず、親クラスをシリアル化する

	InsideTransactionData::serialize(archiver);

	// 自分自身をシリアル化する

	archiver(_xid);
}

//	FUNCTION public
//	Trans::Log::BranchHeurDecideData::serialize -- クラスをシリアル化する
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
BranchHeurDecideData::serialize(ModArchive& archiver)
{
	// まず、親クラスをシリアル化する

	Data::serialize(archiver);

	// 自分自身をシリアル化する

	if (archiver.isStore()) {
		archiver << _xid;
		{
		int tmp = _decision;
		archiver << tmp;
		}
	} else {
		archiver >> _xid;
		{
		int tmp;
		archiver >> tmp;
		_decision = static_cast<Branch::HeurDecision::Value>(tmp);
		}
	}
}

//	FUNCTION public
//	Trans::Log::BranchForgetData::serialize -- クラスをシリアル化する
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
BranchForgetData::serialize(ModArchive& archiver)
{
	// まず、親クラスをシリアル化する

	Data::serialize(archiver);

	// 自分自身をシリアル化する

	archiver(_xid);
}

//	FUNCTION public
//	Trans::Log::XATransactionData::serialize -- クラスをシリアル化する
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
XATransactionData::serialize(ModArchive& archiver)
{
	// まず、親クラスをシリアル化する

	InsideTransactionData::serialize(archiver);

	// 自分自身をシリアル化する

	archiver(_xid);
}

//
//	FUNCTION public
//	Trans::Log::TransactionBeginForSlaveData::serialize
//		-- クラスをシリアル化する
//
//	NOTES
//
//	ARGUMENTS
//		ModArchive&			archiver_
//			シリアル化先(または元)のアーカイバー
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//
void
TransactionBeginForSlaveData::serialize(ModArchive& archiver_)
{
	// まず、親クラスをシリアル化する
	TransactionBeginData::serialize(archiver_);

	// 自分自身をシリアル化する
	archiver_(m_uiMasterLSN);
}

//
// Copyright (c) 2000, 2001, 2002, 2003, 2004, 2007, 2012, 2014, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
