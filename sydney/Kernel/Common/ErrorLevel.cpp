// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ErrorLevel.cpp -- エラーレベルをあらわすクラス
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

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Common/ClassID.h"
#include "Common/ErrorLevel.h"

_TRMEISTER_USING
_TRMEISTER_COMMON_USING

//
//	FUNCTION public
//	Common::ErrorLevel::ErrorLevel -- コンストラクタ(1)
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
ErrorLevel::ErrorLevel()
	: m_eLevel(Undefined)
{
}

//
//	FUNCTION public
//	Common::ErrorLevel::ErrorLevel -- コンストラクタ(2)
//
//	NOTES
//
//	ARGUMENTS
//	Common::ErrorLevel::Type eLevel_
//		エラーレベル
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
ErrorLevel::ErrorLevel(Type eLevel_)
	: m_eLevel(eLevel_)
{
}

//
//	FUNCTION public
//	Common::ErrorLevel::~ErrorLevel -- デストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
ErrorLevel::~ErrorLevel()
{
}

//
//	FUNCTION public
//	Common::ErrorLevel::getErrorLevel -- エラーレベルを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Common::ErrorLevel::Type
//	   エラーレベル
//
//	EXCEPTIONS
//	なし
//
ErrorLevel::Type
ErrorLevel::getLevel() const
{
	return m_eLevel;
}

//
//	FUNCTION public
//	Common::ErrorLevel::serialize -- シリアル化のためのメソッド
//
//	NOTES
//	シリアル化を行う
//
//	ARGUMENTS
//	ModSrchive& cArchiver_
//		アーカイバ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
ErrorLevel::serialize(ModArchive& cArchiver_)
{
	if (cArchiver_.isStore())
	{
		//書出し
		int iLevel = m_eLevel;
		cArchiver_ << iLevel;
	}
	else
	{
		//読出し
		int iLevel;
		cArchiver_ >> iLevel;
		m_eLevel = static_cast<Type>(iLevel);
	}
}

//
//	FUNCTION public
//	Common::ErrorLevel::getClassID -- クラスIDを得る
//
//	NOTES
//	クラスIDを得る
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	int
//		クラスID
//
//	EXCEPTIONS
//	なし
//
int
ErrorLevel::getClassID() const
{
	return ClassID::ErrorLevelClass;
}

//
//	Copyright (c) 2005, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
