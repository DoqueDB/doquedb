// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ErrorLevel.h -- 
// 
// Copyright (c) 2005, 2023 Ricoh Company, Ltd.
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

#ifndef __TRMEISTER_COMMON_ERRORLEVEL_H
#define __TRMEISTER_COMMON_ERROELEVEL_H

#include "Common/Module.h"
#include "Common/ExecutableObject.h"
#include "Common/Externalizable.h"

_TRMEISTER_BEGIN
_TRMEISTER_COMMON_BEGIN

//
//	CLASS
//	Common::ErrorLevel -- エラーレベルをあらわすクラス
//
//	NOTES
//	
//
class SYD_COMMON_FUNCTION ErrorLevel : public ExecutableObject,
									   public Externalizable
{
public:
	//
	//	ENUM
	//	Type -- エラーレベルの種別をあらわす
	//
	//	NOTES
	//	エラーレベルの種別をあらわす
	//		User		ユーザレベル
	//		System		システムレベル
	//
	enum Type
	{
		User = 1,			// ユーザレベル
		System,				// システムレベル

		Undefined = -1
	};

	//コンストラクタ(1)
	ErrorLevel();
	//コンストラクタ(2)
	ErrorLevel(Type eStatus_);
	//デストラクタ
	~ErrorLevel();

	//レベルを得る
	Type getLevel() const;

	//シリアル化
	void serialize(ModArchive& cArchiver_);

	//クラスIDを得る
	int getClassID() const;

private:
	//レベル
	Type m_eLevel;
};

_TRMEISTER_COMMON_END
_TRMEISTER_END

#endif // __TRMEISTER_COMMON_ERRORLEVEL_H

//
//	Copyright (c) 2005, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//

