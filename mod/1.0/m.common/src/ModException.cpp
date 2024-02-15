// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:	
//
//	ModException.cpp --- 例外関連の定義
// 
// Copyright (c) 1997, 2009, 2023 Ricoh Company, Ltd.
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


#include "ModError.h"
#include "ModException.h"
#include "ModExceptionMessage.h"
#include "ModOsDriver.h"
#include "ModCharTrait.h"

namespace
{
	//
	//	FUNCTION local
	//	_GetBaseName -- ファイル名からディレクトリ部分を除く
	//
	//	NOTES
	//	ファイル名からディレクトリ部分を除く。
	//
	//	ARGUMENTS
	//	const char* pszSrcName_
	//		取除く対象の文字列
	//
	//	RETURN
	//	const char*
	//		ディレクトリ等を除いた部分の先頭のポインタ
	//
	//	EXCEPTIONS
	//	なし
	//
	const char*
	_GetBaseName(const char* pszSrcName_)
	{
		const char* p = pszSrcName_;
		if (p)
		{
			const char* cp = 0;
			for (; *p != '\0'; p++)
			{
				switch (*p)
				{
				case '/':
				case '\\':
					cp = p;
					break;
				default:
					;
				}
			}
			if (cp)
			{
				p = cp+1;
			}
			else
			{
				p = pszSrcName_;
			}
		}
		return p;
	}
}

//
// FUNCTION public
// ModException::setError -- 例外クラスの内容設定
//
// NOTES
//	例外クラスの内容として、モジュール番号、エラー番号、エラーレベルを
//	設定する。最後の引数は、スレッドのエラー状態をModTrueに設定するかどうかを示す。
//	デフォルトではエラー状態に設定する。これによって、この
//	例外オブジェクトをもつスレッドがエラー処理中で
//	あることを示す。エラー処理中にメモリが獲得できない場合は非常用メモリが
//	獲得される。
//	メモリ獲得モジュール内部の特殊な場合に限り、エラーが起きてもエラーレベルが
//	設定されない場合がある。最後の引数はこのような場合に使用する。
//
// ARGUMENTS
//	ModModule module_
//		モジュール番号	
//	ModErrorNumber errorNumber_
//		エラー番号
//	ModErrorLevel errorLevel_
//		エラーレベル
//	int osError_
//		OSが返したエラー番号
//	ModBoolean settingFlag
//		エラー状態を設定するかどうか
//
// RETURN
//	なし
//
// EXCEPTIONS
//	なし
//
void
ModException::setError(ModModule module_, ModErrorNumber errorNumber_, 
					   ModErrorLevel errorLevel_, int osError_,
					   ModBoolean settingFlag)
{
	this->module = module_;
	this->errorNumber = errorNumber_;
	this->errorLevel = errorLevel_;
	this->osError = osError_;
	this->path[0] = '\0';
	if (settingFlag == ModTrue) {
		// この例外に対してではなく、スレッド保有の例外オブジェクトに対して
		// 設定する。リセットもそのオブジェクトに対して実行すること。
		// this->status = ModTrue;
		ModErrorHandle::setError();
	}
}

//
// FUNCTION public
// ModException::setMessage -- 出力する例外メッセージを内部に設定する
//
// NOTES
//	ログ出力用の例外メッセージを内部の領域に設定する。
//	例外オブジェクトに設定されているエラー発生の
//	モジュール番号、エラー番号、エラーレベルと、行番号、ファイル名が
//	内部に保持するメッセージ用の領域に設定し、メッセージ領域へのポインタ
//	を返す。この領域は静的な領域なので解放してはいけない。
//	
// ARGUMENTS
//	なし
//
// RETURN
//	メッセージを格納した静的領域へのポインタ
//
// EXCEPTIONS
//	なし
//

char*
ModException::setMessage()
{
	if (this->errorNumber == ModCommonErrorAssert) {

		// アサーションに失敗したときだけ特別なメッセージにする

		const char	msg[] = "Assertion FAILED, Please check your source !!";
		ModOsDriver::Memory::copy(this->message, msg, sizeof(msg));
	} else

		// 例外の内容を表すメッセージを得る

		ModExceptionMessage::setMessage(*this, this->message);

	return this->message;
}

// FUNCTION public
//	ModException::setThrowInfo -- スローされた場所の情報をセットする
//
// NOTES
//
// ARGUMENTS
//	int line
//	const char* file
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
ModException::
setThrowInfo(int line, const char* file)
{
	this->lineNumber = line;
	ModCharTrait::copy(this->path, _GetBaseName(file));
}

//
// FUNCTION 
// ModException::operator new -- メモリを確保する
//
// NOTES
//	グローバルな::newでメモリを確保し、返す。もしメモリが確保できなければ、
//	エラー番号ModErrorSystemMemoryExhaustを設定しModExceptionを送出する。
//
// ARGUMENTS
//	size_t size
//		要求サイズ
// RETURN
//	確保できたメモリの先頭アドレス
//
// EXCEPTIONS
//	ModOsErrorSystemMemoryExhaust
//		システムのメモリが確保できない(::newに失敗)
//
void*
ModException::operator new(size_t size)
{
	return ModOsDriver::newSetHandler((ModSize)size);
}

// FUNCTION private
//	ModException::copyPath -- 
//
// NOTES
//
// ARGUMENTS
//	const ModException& original_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
ModException::
copyPath(const ModException& original_)
{
	ModOsDriver::Memory::copy(this->path, original_.path, ModExceptionFileLength);
}

//
// Copyright (c) 1997, 2009, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
