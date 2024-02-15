// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Exernalizable.h -- シリアル化可能なオブジェクト関連のクラス定義、関数宣言
// 
// Copyright (c) 2000, 2001, 2002, 2003, 2007, 2014, 2023 Ricoh Company, Ltd.
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

#ifndef	__SYDNEY_CHECKPOINT_EXTERNALIZABLE_H
#define	__SYDNEY_CHECKPOINT_EXTERNALIZABLE_H

#include "Checkpoint/Module.h"

#include "Common/Externalizable.h"

_SYDNEY_BEGIN
_SYDNEY_CHECKPOINT_BEGIN

//	CLASS
//	Checkpoint::CommonExternaizable --
//		Microsoft C/C++ Compiler のバグを回避するためのクラス
//
//	NOTES
//		Checkpoint::Externalizable が Common::Externalizable を
//		直接継承したとき、Checkpoint::Externalizable::operator = を
//		呼び出すと、Microsoft C/C++ Compiler Version 12.00.8804 では、
//		なぜか、Checkpoint::Externalizable::operator = を
//		無限に呼び出すコードが生成される

class CommonExternalizable
	: public	Common::Externalizable
{};

//	CLASS
//	Checkpoint::Externalizable -- シリアル化可能なオブジェクトを表すクラス
//
//	NOTES

class Externalizable
	: public	CommonExternalizable
{
public:
	//	CLASS
	//	Checkpoint::Externalizable::Category --
	//		シリアル化可能なオブジェクトの種別を表すクラス
	//
	//	NOTES

	struct Category
	{
		//	ENUM
		//	Checkpoint::Externalizable::Category::Value --
		//		シリアル化可能なオブジェクトの種別の値を表す列挙型
		//
		//	NOTES

		enum Value
		{
			// 不明
			Unknown =		0,
			// Checkpoint::Log::CheckpointData
			CheckpointLogData,
			// Checkpoint::Log::CheckpointDatabaseData
			CheckpointDatabaseLogData_0,
			// Checkpoint::Log::CheckpointSystemData
			CheckpointSystemLogData_0,
			// Checkpoint::Log::FileSynchronizeBeginData
			FileSynchronizeBeginLogData,
			// Checkpoint::Log::FileSynchronizeEndData
			FileSynchronizeEndLogData,

			// Checkpoint::Log::CheckpointDatabaseData (v16.1 以降)
			CheckpointDatabaseLogData_1,
			// Checkpoint::Log::CheckpointSystemData (v16.1 以降)
			CheckpointSystemLogData_1,

			// Checkpoint::Log::CheckpointDatabaseData (v17.1 以降)
			CheckpointDatabaseLogData_2,

			// 値の数
			Count
		};
	};

	// クラス ID からそれの表すクラスを確保する
	SYD_CHECKPOINT_FUNCTION
	static Common::Externalizable* getClassInstance(int classID);
};

_SYDNEY_CHECKPOINT_END
_SYDNEY_END

#endif	// __SYDNEY_CHECKPOINT_EXTERNALIZABLE_H

//
// Copyright (c) 2000, 2001, 2002, 2003, 2007, 2014, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
