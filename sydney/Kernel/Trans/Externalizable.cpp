// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Externalizable.cpp -- シリアル化可能なオブジェクト関連の関数定義
// 
// Copyright (c) 2000, 2001, 2006, 2007, 2011, 2012, 2014, 2023 Ricoh Company, Ltd.
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

#include "Trans/Externalizable.h"
#include "Trans/LogData.h"
#include "Trans/Manager.h"
#include "Trans/TimeStamp.h"
#include "Trans/XID.h"

#include "Common/Assert.h"

_SYDNEY_USING
_SYDNEY_TRANS_USING

namespace
{
}

//	FUNCTION private
//	Trans::Manager::Externalizable::initialize --
//		マネージャーのうち、シリアル化可能なオブジェクト関連を初期化する
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
Manager::Externalizable::initialize()
{
	// トランザクション関連のシリアル化可能なオブジェクトを
	// 確保するための関数を共通ライブラリに登録する

	Common::Externalizable::insertFunction(
		Common::Externalizable::TransClasses,
			Trans::Externalizable::getClassInstance);
}

//	FUNTION private
//	Trans::Manager::Externalizable::terminate --
//		マネージャーのうち、シリアル可能なオブジェクト関連を終了処理する
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

// static
void
Manager::Externalizable::terminate()
{}

//	FUNCTION public
//	Trans::Externalizable::getClassInstance --
//		指定された種別のシリアル化可能なオブジェクトを確保する
//
//	NOTES
//
//	ARGUMENTS
//		int					classID
//			確保するオブジェクトの種別を表す値
//
//	RETURN
//		確保したシリアル化可能なオブジェクトを格納する領域の先頭アドレス
//
//	EXCEPTIONS

// static
Common::Externalizable*
Externalizable::
getClassInstance(int classID)
{
	; _SYDNEY_ASSERT(classID >= Common::Externalizable::TransClasses);

	Common::Externalizable*	object = 0;

	Externalizable::Category::Value	category =
		static_cast<Externalizable::Category::Value>(
			classID - Common::Externalizable::TransClasses);
		
	switch (category) {
	case Category::LogData:
		object = new Log::Data();						break;
	case Category::TransactionBeginLogData:
		object = new Log::TransactionBeginData();		break;
	case Category::InsideTransactionLogData:
		object = new Log::InsideTransactionData();		break;
	case Category::ModificationLogData:
		object = new Log::ModificationData();			break;
	case Category::TransactionCommitLogData:
		object = new Log::TransactionCommitData();		break;
	case Category::TransactionRollbackLogData:
		object = new Log::TransactionRollbackData();	break;
	case Category::StatementCommitLogData:
		object = new Log::StatementCommitData();		break;
	case Category::StatementRollbackLogData:
		object = new Log::StatementRollbackData();		break;
	case Category::TimeStampAssignLogData:
		object = new Log::TimeStampAssignData();		break;
	case Category::TimeStamp:
		object = new TimeStamp();						break;
	case Category::XID:
		object = new XID();								break;
	case Category::TransactionPrepareLogData:
		object = new Log::TransactionPrepareData();		break;
	case Category::BranchHeurDecideLogData:
		object = new Log::BranchHeurDecideData();		break;
	case Category::BranchForgetLogData:
		object = new Log::BranchForgetData();			break;
	case Category::StartBatchLogData:
		object = new Log::StartBatchData();				break;
	case Category::XATransactionLogData:
		object = new Log::XATransactionData();			break;
	case Category::TransactionBeginForSlaveLogData:
		object = new Log::TransactionBeginForSlaveData(); break;
	default:
		; _SYDNEY_ASSERT(false);
	}

	return object;
}

//
// Copyright (c) 2000, 2001, 2006, 2007, 2011, 2012, 2014, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
