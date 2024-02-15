// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Externalizable.h -- シリアル化可能クラス
// 
// Copyright (c) 1999, 2000, 2004, 2009, 2012, 2014, 2023 Ricoh Company, Ltd.
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

#ifndef __TRMEISTER_COMMON_EXTERNALIZABLE_H
#define __TRMEISTER_COMMON_EXTERNALIZABLE_H

#include "Common/Module.h"
#include "ModSerial.h"

_TRMEISTER_BEGIN

namespace Common {

//
//	CLASS
//	Common::Externalizable -- シリアル化可能クラスのスーパクラス
//
//	NOTES
//	シリアル化可能なクラスはすべてこのクラスの派生クラスとする
//
class SYD_COMMON_FUNCTION Externalizable : public ModSerializer
{
public:
	//
	//	ENUM
	//
	//	NOTES
	//	シリアル化可能クラスIDの各モジュールの先頭値。
	//	クラスごとのクラスIDは各モジュールで管理する。
	//	増えた場合はここに追加する。
	//
	enum
	{
		None					= 0,
		CommonClasses			= 1,

		BufferClasses			= 1001,
		CheckpointClasses		= 2001,
		CommunicationClasses	= 3001,
		ExecutionClasses		= 4001,
		LockClasses				= 5001,
		LogicalFileClasses		= 6001,
		MetaClasses				= 7001,
		OptClasses				= 8001,
		PhysicalFileClasses		= 9001,
		SchemaClasses			= 10001,
//		SessionClasses			= 11001,
		StatementClasses		= 12001,
		TransClasses			= 13001,
		VersionClasses			= 14001,
		AnalysisClasses			= 15001,
		PlanClasses				= 16001,
		ExecutionV2Classes		= 17001,
		DPlanClasses			= 18001,
		DExecutionClasses		= 19001,
		AdminClasses			= 20001,

		BtreeClasses			= 30001,
		FileCommonClasses		= 31001,
		FullTextClasses			= 32001,
		InvertedClasses			= 33001,
		RecordClasses			= 34001,

		NumberOfValue			= 35001
	};
	
	//コンストラクタ
	Externalizable();
	//デストラクタ
	virtual ~Externalizable();

	//クラスIDを得る
	virtual int getClassID() const = 0;

	//クラスのインスタンスを得る
	static Externalizable* getClassInstance(int iClassID_);

	//インスタンスを確保する関数を登録する
	static void insertFunction(int iClassID_,
							   Externalizable* (*pFunc_)(int));
};

}

_TRMEISTER_END

#endif // __TRMEISTER_COMMON_EXTERNALIZABLE_H

//
//	Copyright (c) 1999, 2000, 2004, 2009, 2012, 2014, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
