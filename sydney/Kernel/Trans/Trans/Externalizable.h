// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Exernalizable.h -- シリアル化可能なオブジェクト関連のクラス定義、関数宣言
// 
// Copyright (c) 2000, 2001, 2002, 2006, 2007, 2011, 2012, 2014, 2023 Ricoh Company, Ltd.
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

#ifndef	__SYDNEY_TRANS_EXTERNALIZABLE_H
#define	__SYDNEY_TRANS_EXTERNALIZABLE_H

#include "Trans/Module.h"
#include "Common/Externalizable.h"

_SYDNEY_BEGIN
_SYDNEY_TRANS_BEGIN

//	CLASS
//	Trans::CommonExternaizable --
//		Microsoft C/C++ Compiler のバグを回避するためのクラス
//
//	NOTES
//		Trans::Externalizable が Common::Externalizable を直接継承したとき、
//		Trans::Externalizable::operator = を呼び出すと、
//		Microsoft C/C++ Compiler Version 12.00.8804 では、
//		なぜか、Trans::Externalizable::operator = を
//		無限に呼び出すコードが生成される

class CommonExternalizable
	: public	Common::Externalizable
{};

//	CLASS
//	Trans::Externalizable -- シリアル化可能なオブジェクトを表すクラス
//
//	NOTES

class Externalizable
	: public	CommonExternalizable
{
public:
	//	CLASS
	//	Trans::Externalizable::Category --
	//		シリアル化可能なオブジェクトの種別を表すクラス
	//
	//	NOTES

	struct Category
	{
		//	ENUM
		//	Trans::Externalizable::Category::Value --
		//		シリアル化可能なオブジェクトの種別の値を表す列挙型
		//
		//	NOTES

		enum Value
		{
			// 不明
			Unknown =		0,
			// Trans::Log::Data
			LogData,
			// Trans::Log::TransactionBeginData
			TransactionBeginLogData,
			// Trans::Log::InsideTransactionData
			InsideTransactionLogData,
			// Trans::Log::ModificationData
			ModificationLogData,
			// Trans::Log::TransactionCommitData
			TransactionCommitLogData,
			// Trans::Log::TransactionRollbackData
			TransactionRollbackLogData,
			// Trans::Log::StatementCommitData
			StatementCommitLogData,
			// Trans::Log::StatementRollbackData
			StatementRollbackLogData,
			// Trans::Log::TimeStampAssignData
			TimeStampAssignLogData,
			// Trans::TimeStamp
			TimeStamp,

			// Trans::XID (v16.1 以降)
			XID,
			// Trans::Log::TransactionPrepareData (v16.1 以降)
			TransactionPrepareLogData,
			// Trans::Log::BranchHeurDecide (v16.1 以降)
			BranchHeurDecideLogData,
			// Trans::Log::BranchForgetData (v16.1 以降)
			BranchForgetLogData,

			// Trans::Log::StartBatchData (v16.4 以降)
			StartBatchLogData,

			// Trans::Log::XATransactionData (v17.1 以降)
			XATransactionLogData,
			
			// Trans::Log::TransactionBeginForSlaveData (v17.1 以降)
			TransactionBeginForSlaveLogData,

			// 値の数
			Count
		};
	};

	// クラス ID からそれの表すクラスを確保する
	SYD_TRANS_FUNCTION
	static Common::Externalizable* getClassInstance(int classID);
};

_SYDNEY_TRANS_END
_SYDNEY_END

#endif	// __SYDNEY_TRANS_EXTERNALIZABLE_H

//
// Copyright (c) 2000, 2001, 2002, 2006, 2007, 2011, 2012, 2014, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
