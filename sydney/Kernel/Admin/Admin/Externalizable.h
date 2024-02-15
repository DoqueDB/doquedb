// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Exernalizable.h -- シリアル化可能なオブジェクト関連のクラス定義、関数宣言
// 
// Copyright (c) 2014, 2023 Ricoh Company, Ltd.
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

#ifndef	__SYDNEY_ADMIN_EXTERNALIZABLE_H
#define	__SYDNEY_ADMIN_EXTERNALIZABLE_H

#include "Admin/Module.h"
#include "Common/Externalizable.h"

_SYDNEY_BEGIN
_SYDNEY_ADMIN_BEGIN

//
//	CLASS
//	Admin::CommonExternaizable --
//		Microsoft C/C++ Compiler のバグを回避するためのクラス
//
//	NOTES
//		Trans::CommonExternalizable 参照のこと
//
class CommonExternalizable
	: public	Common::Externalizable
{};

//
//	CLASS
//	Admin::Externalizable -- シリアル化可能なオブジェクトを表すクラス
//
//	NOTES
//
class Externalizable
	: public	CommonExternalizable
{
public:
	//
	//	CLASS
	//	Admin::Externalizable::Category --
	//		シリアル化可能なオブジェクトの種別を表すクラス
	//
	struct Category
	{
		//
		//	ENUM
		//	Admin::Externalizable::Category::Value --
		//		シリアル化可能なオブジェクトの種別の値を表す列挙型
		//
		enum Value
		{
			// 不明
			Unknown =		0,
			
			// Admin::Log::ReplicationEndData
			ReplicationEndLogData,

			// 値の数
			Count
		};
	};

	// クラス ID からそれの表すクラスを確保する
	SYD_ADMIN_FUNCTION
	static Common::Externalizable* getClassInstance(int classID);
};

_SYDNEY_ADMIN_END
_SYDNEY_END

#endif	// __SYDNEY_ADMIN_EXTERNALIZABLE_H

//
// Copyright (c) 2014, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
