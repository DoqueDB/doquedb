// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ExceptionObject.cpp -- 例外クラスのラッパー
// 
// Copyright (c) 1999, 2000, 2023 Ricoh Company, Ltd.
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
#include "ModUnicodeOstrStream.h"

#include "Common/ExceptionObject.h"
#include "Common/UnicodeString.h"
#include "Common/ClassID.h"
#include "Common/ErrorMessage.h"
#include "Common/ErrorMessageManager.h"
#include "Common/Message.h"

#include "Exception/ErrorMessage.h"

#include "ModUnicodeCharTrait.h"
#include "ModMessage.h"

_TRMEISTER_USING
using namespace Common;

//
//	FUNCTION public
//	Common::ExceptionObject::serialize -- シリアル化を行う
//
//	NOTES
//	シリアル化を行う
//
//	ARGUMENTS
//	ModArchive& cArchiver_
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
ExceptionObject::serialize(ModArchive& cArchiver_)
{
	if (cArchiver_.isStore())
	{
		//書出し
		cArchiver_ << m_cObject;
	}
	else
	{
		//読込み
		cArchiver_ >> m_cObject;
	}
}

//
//	FUNCTION public
//	Common::ExceptionObject::getClassID -- クラスIDを得る
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
ExceptionObject::getClassID() const
{
	return ClassID::ExceptionClass;
}

//
//	FUNCTION public
//	Common::ExceptionObject::getClassID -- 文字列で取り出す
//
//	NOTES
//	文字列で取り出す
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModUnicodeString
//		文字列
//
//	EXCEPTIONS
//	なし
//
ModUnicodeString 
ExceptionObject::toString() const
{
	ModUnicodeOstrStream ostr;
	ostr << "Exception :" << m_cObject;
	return ostr.getString();
}

//
//	Copyright (c) 1999, 2000, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
