// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ExceptionObject.h -- 例外クラス
// 
// Copyright (c) 1999, 2001, 2007, 2023 Ricoh Company, Ltd.
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

#ifndef __TRMEISTER_COMMON_EXCEPTIONOBJECT_H
#define __TRMEISTER_COMMON_EXCEPTIONOBJECT_H

#include "Common/Module.h"
#include "Common/ExecutableObject.h"
#include "Common/Externalizable.h"

#include "Exception/Object.h"

_TRMEISTER_BEGIN
_TRMEISTER_COMMON_BEGIN

//
//	CLASS
//	ExceptionObject -- 例外をあらわすクラスのラッパー
//
//	NOTES
//	例外をあらわすクラスのラッパークラス。
//	TRMeisterから投げられる例外をポートに投げるときはこのクラスに入れる。
//	コネクションごしにクライアントに投げられるのでExternalizableの派生で、
//	エグゼキュータも扱うのでExecutableObjectの派生である。
//
class SYD_COMMON_FUNCTION ExceptionObject : public Externalizable,
							   public ExecutableObject
{
public:
	//コンストラクタ(1)
	ExceptionObject()
	{ }
	//コンストラクタ(2)
	ExceptionObject(const Exception::Object& cObject_)
		: m_cObject(cObject_)
	{ }

	//
	//	FUNCTION public
	//	Common::ExceptionObject::getErrorNumber -- エラー番号を得る
	//
	//	NOTES
	//	エラー番号を得る
	//
	//	ARGUMENTS
	//	なし
	//
	//	RETURN
	//	unsigned int
	//		エラー番号
	//
	//	EXCEPTIONOBJECTS
	//	なし
	//
	unsigned int getErrorNumber() const
	{
		return m_cObject.getErrorNumber();
	}

	//
	//	FUNCTION public
	//	Common::ExceptionObject::getErrorMessageArgument
	//								-- エラーメッセージ引数を得る
	//
	//	NOTES
	//	エラーメッセージ引数を得る。
	//
	//	ARGUMENTS
	//	なし
	//
	//	RETURN
	//	const ModUnicodeChar*
	//		エラーメッセージ引数
	//
	//	EXCEPTIONOBJECTS
	//	なし
	//
	ModUnicodeChar* getErrorMessageArgument()
	{
		return m_cObject.getErrorMessageArgument();
	}

	//
	//	FUNCTION public
	//	Common::ExceptionObject::getErrorMessageArgument
	//								-- エラーメッセージ引数を得る
	//
	//	NOTES
	//	エラーメッセージ引数を得る。
	//
	//	ARGUMENTS
	//	なし
	//
	//	RETURN
	//	const ModUnicodeChar*
	//		エラーメッセージ引数
	//
	//	EXCEPTIONOBJECTS
	//	なし
	//
	const ModUnicodeChar* getErrorMessageArgument() const
	{
		return m_cObject.getErrorMessageArgument();
	}

	//
	//	FUNCTION public
	//	Common::ExceptionObject::getModuleName -- モジュール名を得る
	//
	//	NOTES
	//	モジュール名を得る
	//
	//	ARGUMENTS
	//	なし
	//
	//	RETURN
	//	const ModUnicodeChar*
	//		モジュール名
	//
	//	EXCEPTIONOBJECTS
	//	なし
	//
	const ModUnicodeChar* getModuleName() const
	{
		return m_cObject.getModuleName();
	}

	//
	//	FUNCTION public
	//	Common::ExceptionObject::getFileName -- ファイル名を得る
	//
	//	NOTES
	//	ファイル名を得る
	//
	//	ARGUMENTS
	//	なし
	//
	//	RETURN
	//	const ModUnicodeChar*
	//		ファイル名
	//
	//	EXCEPTIONOBJECTS
	//	なし
	//
	const ModUnicodeChar* getFileName() const
	{
		return m_cObject.getFileName();
	}
	
	//
	//	FUNCTION public
	//	Common::ExceptionObject::getLineNumber -- 行番号を得る
	//
	//	NOTES
	//	行番号を得る
	//
	//	ARGUMENTS
	//	なし
	//
	//	RETURN
	//	int
	//		行番号
	//
	//	EXCEPTIONOBJECTS
	//	なし
	//
	int getLineNumber() const
	{
		return m_cObject.getLineNumber();
	}

	//
	//	FUNCTION public
	//	Common::ExceptionObject::throwClassInstance
	//		-- 例外を throw する
	//
	//	NOTES
	//	このクラスが持つ Excpetion::Object のエラー番号に対応する
	//	例外を throw する
	//
	//	ARGUMENTS
	//	なし
	//
	//	RETURN
	//	なし
	//
	//	EXCEPTIONOBJECTS
	//	なし
	//
	void throwClassInstance() const
	{
		Exception::throwClassInstance(m_cObject);
	}

	//シリアル化
	void serialize(ModArchive& cArchiver_);
	//クラスIDを得る
	int getClassID() const;

	//文字列で取り出す
	ModUnicodeString toString() const;

private:
	// 例外クラスのオブジェクト
	Exception::Object	m_cObject;
};

_TRMEISTER_COMMON_END
_TRMEISTER_END

#endif // __TRMEISTER_COMMON_EXCEPTIONOBJECT_H

//
//	Copyright (c) 1999, 2001, 2007, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//

