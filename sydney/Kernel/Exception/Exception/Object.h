// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Object.h -- 例外クラス
// 
// Copyright (c) 2000, 2002, 2005, 2006, 2007, 2023 Ricoh Company, Ltd.
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

#ifndef __TRMEISTER_EXCEPTION_OBJECT_H
#define __TRMEISTER_EXCEPTION_OBJECT_H

#include "Exception/Module.h"
#include "Exception/ErrorNumber.h"

#include "ModDefaultManager.h"
#include "ModMessage.h"
#include "ModOstream.h"
#include "ModSerial.h"

_TRMEISTER_BEGIN
_TRMEISTER_EXCEPTION_BEGIN

//
//	CLASS
//	Object -- 例外をあらわすクラス
//
//	NOTES
//	例外をあらわすクラスの基底クラス。
//	Commonから分離するためにCommon::ExternalizableもCommon::Objectも
//	継承せずにMODのクラスを直接継承する
//
class SYD_EXCEPTION_FUNCTION Object : public ModDefaultObject, public ModSerializer
{
public:
	//コンストラクタ(1)
	Object();
	//コンストラクタ(2)
	Object(ErrorNumber::Type uiErrorNumber_);		//エラー番号
	//コンストラクタ(3)
	Object(ErrorNumber::Type uiErrorNumber_,		//エラー番号
		   const char* pszModuleName_,				//モジュール名
		   const char* pszFileName_,				//ファイル名
		   int iLineNumber_,						//行番号
		   const char* pszStateCode_);				//SQLSTATEコード
	//コンストラクタ(4)
	Object(ErrorNumber::Type uiErrorNumber_,		//エラー番号
		   const ModUnicodeChar* pszModuleName_,	//モジュール名
		   const ModUnicodeChar* pszFileName_,		//ファイル名
		   int iLineNumber,							//行番号
		   const char* pszStateCode_);				//SQLSTATEコード
	//コンストラクタ(5)
	Object(const Object& cObject_,
		   const char* pszStateCode_);				//SQLSTATEコード

	//
	//	FUNCTION public
	//	Exception::Object::getErrorNumber -- エラー番号を得る
	//
	//	NOTES
	//	エラー番号を得る
	//
	//	ARGUMENTS
	//	なし
	//
	//	RETURN
	//	ErrorNumber::Type
	//		エラー番号
	//
	//	OBJECTS
	//	なし
	//
	ErrorNumber::Type getErrorNumber() const
	{
		return m_uiErrorNumber;
	}

	//
	//	FUNCTION public
	//	Exception::Object::getErrorMessageArgument
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
	//	OBJECTS
	//	なし
	//
	ModUnicodeChar* getErrorMessageArgument()
	{
		return m_pszErrorMessageArgument;
	}

	//
	//	FUNCTION public
	//	Exception::Object::getErrorMessageArgument
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
	//	OBJECTS
	//	なし
	//
	const ModUnicodeChar* getErrorMessageArgument() const
	{
		return m_pszErrorMessageArgument;
	}

	//
	//	FUNCTION public
	//	Exception::Object::getModuleName -- モジュール名を得る
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
	//	OBJECTS
	//	なし
	//
	const ModUnicodeChar* getModuleName() const
	{
		return m_pszModuleName;
	}

	//
	//	FUNCTION public
	//	Exception::Object::getFileName -- ファイル名を得る
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
	//	OBJECTS
	//	なし
	//
	const ModUnicodeChar* getFileName() const
	{
		return m_pszFileName;
	}
	
	//
	//	FUNCTION public
	//	Exception::Object::getLineNumber -- 行番号を得る
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
	//	OBJECTS
	//	なし
	//
	int getLineNumber() const
	{
		return m_iLineNumber;
	}
	
	//
	//	FUNCTION public
	//	Exception::Object::getStateCode -- SQLSTATEコードを得る
	//
	//	NOTES
	//
	//	ARGUMENTS
	//	なし
	//
	//	RETURN
	//	const char*
	//		SQLSTATEコード
	//
	//	OBJECTS
	//	なし
	//
	const char* getStateCode() const
	{
		return m_pszStateCode;
	}

	//シリアル化
	void serialize(ModArchive& cArchiver_);

	//
	//	FUNCTION public
	//	Exception::Object::isUserLevel -- Userレベルの例外かどうか
	//
	virtual bool isUserLevel() const
	{
		return false;
	}

protected:
	//エラーメッセージを設定する
	void setErrorMessageArgument(const ModUnicodeChar* pszErrorMessageArgument_);

#ifdef OBSOLETE /* 以下のメソッドは廃止、例外発生時のログは投げる人が書く */

	//エラーメッセージをログに書く
	void printLogError();
	void printLogInfo();

#endif

private:
	//エラー番号
	unsigned int m_uiErrorNumber;
	
	//エラーメッセージ引数
	ModUnicodeChar m_pszErrorMessageArgument[1024];

	//モジュール名
	ModUnicodeChar m_pszModuleName[128];
	//ファイル名
	ModUnicodeChar m_pszFileName[128];
	//行番号
	int m_iLineNumber;
	// SQLSTATEコード
	char m_pszStateCode[6];
};

//	FUNCTION
//	Exception::throwClassInstance -- 例外クラスを得る
//
//	NOTES
//
//	ARGUMENTS
//	const Exception::Object& cObject_
//		基底クラス
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし

SYD_EXCEPTION_FUNCTION
void throwClassInstance(const Object& cObject_);

//	FUNCTION
//	Exception::rethrowLog -- 例外再送のログを出力する
//
//	NOTES
//	マクロ _TRMEISTER_RETHROW から呼び出される。
//	例外を再送したことをINFOログに出力する。
//
//	ARGUMENTS
//	const char* pszModuleName_
//		モジュール名
//	const char* pszFileName_
//		ファイル名
//	int iLineNumber_
//		行番号
//
//	RETURN
//	なし
//
//	EXCEPTIONS

SYD_EXCEPTION_FUNCTION
void rethrowLog(const char* pszModuleName_,
				const char* pszFileName_,
				int iLineNumber_);

_TRMEISTER_EXCEPTION_END
_TRMEISTER_END

//	MACRO
//	_TRMEISTER_THROW0 -- 引数のない例外クラスをthrowする
//
//	NOTES

#define _TRMEISTER_THROW0(_e_)	throw _e_(moduleName, srcFile, __LINE__)
#define _SYDNEY_THROW0 _TRMEISTER_THROW0

//	MACRO
//	_TRMEISTER_THROW1 -- 引数が1つの例外クラスをthrowする
//
//	NOTES

#define _TRMEISTER_THROW1(_e_, _arg0_)	throw _e_(moduleName, srcFile, __LINE__, _arg0_)
#define _SYDNEY_THROW1 _TRMEISTER_THROW1

//	MACRO
//	_TRMEISTER_THROW2 -- 引数が2つの例外クラスをthrowする
//
//	NOTES

#define _TRMEISTER_THROW2(_e_, _arg0_, _arg1_)	throw _e_(moduleName, srcFile, __LINE__, _arg0_, _arg1_)
#define _SYDNEY_THROW2 _TRMEISTER_THROW2

//	MACRO
//	_TRMEISTER_THROW3 -- 引数が3つの例外クラスをthrowする
//
//	NOTES

#define _TRMEISTER_THROW3(_e_, _arg0_, _arg1_, _arg2_)	throw _e_(moduleName, srcFile, __LINE__, _arg0_, _arg1_, _arg2_)
#define _SYDNEY_THROW3 _TRMEISTER_THROW3

//	MACRO
//	_TRMEISTER_RETHROW -- 例外を再送する
//
//	NOTES

#define _TRMEISTER_RETHROW	\
				_TRMEISTER::Exception::rethrowLog(moduleName, srcFile, __LINE__), throw
#define _SYDNEY_RETHROW _TRMEISTER_RETHROW

#endif // __TRMEISTER_EXCEPTION_OBJECT_H

//
//	Copyright (c) 2000, 2002, 2005, 2006, 2007, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//

