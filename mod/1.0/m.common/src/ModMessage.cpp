// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:	
//
// ModMessage.cpp -- メッセージ記録関連のメソッドの定義
// 
// Copyright (c) 1996, 1997, 2010, 2023 Ricoh Company, Ltd.
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


#include "ModConfig.h"

extern "C" {
#include <stdlib.h>
#include <stdio.h>
#if MOD_CONF_FUNC_GETTIMEOFDAY == 1
#include <sys/time.h>
#endif
#include <time.h>
#if MOD_CONF_SYSTEM_LOG == 1
#include <syslog.h>
#endif
#ifdef OS_RHLINUX6_0
#include <sys/syscall.h>
#include <sys/types.h>
#endif
}

#include "ModMessage.h"
#include "ModAutoMutex.h"
#include "ModCommonMutex.h"
#include "ModCommonInitialize.h"
#include "ModThread.h"
#include "ModParameter.h"
#include "ModCharString.h"
#include "ModUnicodeString.h"
#include "ModKanjiCode.h"
#include "ModMultiByteString.h"
#include "ModLanguageSet.h"

namespace {

	// POSIXの場合、ModThisThread::self は pthread_t を返すが、
	// この値は、カーネルが管理しているスレッドIDでないので、
	// ログに出力する時には、gettid を利用する
	
	unsigned int _getThreadIdForLog()
	{
#ifdef OS_RHLINUX6_0
		return ::syscall(SYS_gettid);
#else
		return ModThisThread::self();
#endif
	}
}

//
// CONST
// ModMessageSelection::parameterEnvironment -- メッセージ用パラメータのありかを示す環境変数名
//
// NOTES
// この定数はメッセージ用パラメータのありかを示す環境変数名を示すのに用いる。
// この環境変数の表すパスにあるパラメータにより、メッセージの動作や出力先を
// 制御する。
// ★メッセージ用として環境変数を別に用意する意義がないようなので★
// ★この環境変数の設定は無効とすることにした。98/03/05 ★
//
const char* ModMessageSelection::parameterEnvironment = NULL;

// ModMessageStreamのstatic変数の宣言
char ModMessageStream::commandName[ModCommandNameLength - 1];
char ModMessageStream::hostName[ModHostNameLength - 1];
ModBoolean ModMessageStream::commandNameIsSet = ModFalse;

// statusKey のとり得る値
enum ModMessageStatus {
	statusFlushed = 0,					// 初期状態、flush された状態
	statusFirstOutput,					// 最初のオペレータ処理
	statusContinue						// 続くオペレータ処理
};

//
// FUNCTION
// ModMessageStream::ModMessageStream -- デフォルトコンストラクタ
//
// NOTES
// ModMessageStreamのデフォルトコンストラクタ。
// syslogやeventlogのオープンの処理を行なう。
//
// ARGUMENTS
// ModMessageType type_
//		メッセージの出力先を指定する。
//		ひとつの type にはひとつのインスタンスのみが作られるべきであるが、
//		このクラスのレベルではそれは保証していない。
//		ModMessageSelection でそれを保証する。
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
ModMessageStream::ModMessageStream(MessageType type_) :
	type(type_),
	currentPointer(buffer),
	width(0),
	fill((int)' '),
	numberType(ModMessageStream::typeDec),
	kanjiCode(ModOs::Process::getEncodingType()),
	noHeader(ModFalse),
	wideChar(ModFalse),
	accessCount(0),
	threadId(0),
	currentStatus(statusFlushed)		// 初期状態はflush後と同じ
{
	messageLevel = levelInformation;
	errorNumber = 0;
	fileNameBuf[0] = '\0';
	lineNumber = 0;

#if MOD_CONF_SYSTEM_LOG == 1
	if (type_ == ModMessageStream::toSystemLog) {

		ModAutoMutex<ModOsMutex> m(ModCommonMutex::getMutex());
		m.lock();

		//
		// syslogのための初期化を行なう
		//
		// facility はパラメータから得る

		int myFacility =
			ModMessageSelection::parameter->getInteger("ModMessageFacility");

		int facility;
		switch (myFacility) {
		case 0:
			facility = LOG_LOCAL0;
			break;
		case 1:
			facility = LOG_LOCAL1;
			break;
		case 2:
			facility = LOG_LOCAL2;
			break;
		case 3:
			facility = LOG_LOCAL3;
			break;
		case 4:
			facility = LOG_LOCAL4;
			break;
		case 5:
			facility = LOG_LOCAL5;
			break;
		case 6:
			facility = LOG_LOCAL6;
			break;
		case 7:
			facility = LOG_LOCAL7;
			break;
		default:
			facility = LOG_LOCAL0;
			break;
		}
		::openlog(ModMessageStream::commandName, LOG_PID, facility);
	}
#endif
}

//
// FUNCTION
// ModMessageStream::~ModMessageStream -- デストラクタ
//
// NOTES
// ModMessageStream のデストラクタ。バッファに残った文字列を出力し、
// 終了を通知する。syslog、eventlogの終了処理も行なう。
//
// ARGUMENTS
// なし
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
ModMessageStream::~ModMessageStream()
{
#if MOD_CONF_SYSTEM_LOG == 1
	if (this->type == ModMessageStream::toSystemLog) {
		// SystemLog の終了処理を行なう
		::closelog();
	}
#endif
}

//
// FUNCTION
// ModMessageStream::setCommandName -- コマンド名をセットする
//
// NOTES
// この関数は出力で使用するコマンド名を設定するのに用いる。
// ModMessage、ModErrorMessage、ModDebugMessageを使う前にこの関数で
// コマンド名をセットするべきである。
// セットされていない間は"mod"がコマンド名として用いられる。
//
// ARGUMENTS
// const char* commandName_
//		セットしたいコマンド名
// int category
//		イベントログの分類を指定する。NT専用。
//		この引数を省略するか、以下に挙げる以外の値を指定すると
//		パラメータ"ModMessageCategory"に指定した値が用いられる。
//		パラメータがない場合は server が用いられる。
//		category の値:
//			0: ModMessageCategoryServer
//			1: ModMessageCategoryClient
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//

void
ModMessageStream::setCommandName(const char* commandName_, int category)
{
	const ModKanjiCode::KanjiCodeType
		encodingType = ModOs::Process::getEncodingType();

	const char	sep	= ModOsDriver::File::getPathSeparator();
	const char* cp	= 0;

	if (encodingType == ModKanjiCode::unknown) {
		cp = ModCharTrait::rfind(commandName_, sep);
		// セパレータが含まれていない時には cp は commandName_ を指す
		if (*cp == sep) {
			++cp;
		}
	} else {
		ModUnicodeString str(commandName_, encodingType);
		ModUnicodeChar*	ptr = str.rsearch(sep);
		if (ptr == 0) {
			// セパレータが含まれていない
			cp = commandName_;
		} else {
			// いちばん右のセパレータが先頭から何バイト目にあるか調べて
			// cp に足すとコマンド名の先頭を指すようになる
			str.truncate(ptr);
			cp = commandName_ + ModCharTrait::length(str.getString(encodingType));
			if (*cp == sep) {
				++cp;
			}
		}
	}

	ModAutoMutex<ModOsMutex> m(ModCommonMutex::getMutex());
	m.lock();

	ModCharTrait::copy(ModMessageStream::commandName, cp,
					   ModCommandNameLength - 1);
	ModMessageStream::commandNameIsSet = ModTrue;
}

//
// FUNCTION
// ModMessageStream::operator<< -- ASCII文字の出力
//
// NOTES
// この関数はModMessageStreamのバッファにASCII文字を追加するのに用いる。
// バッファがすでにいっぱいのときは無視される。
//
// ARGUMENTS
// char character
//		追加するASCII文字
//
// RETURN
// バッファに追加し、自身への参照を返す。
//
// EXCEPTIONS
// なし
//

ModMessageStream&
ModMessageStream::operator <<(char v)
{
	if (this->type != toNone) {
		this->check();

		if (this->width <= 1 || this->doFill(1) == ModTrue) {
			switch (this->type) {
			case toStandardOut:
				(void) ::fputc(v, stdout);		break;
			case toStandardError:
				(void) ::fputc(v, stderr);		break;
			case toSystemLog:
				*this->currentPointer = v;
				*++this->currentPointer = 0;	break;
#ifdef MOD_DEBUG
			default:
				; ModAssert(ModFalse);
#endif
			}
		}
		this->width = 0;
	}
	return *this;
}

//
// FUNCTION
// ModMessageStream::operator<< -- ASCII文字の出力(unsigned)
//
// NOTES
// この関数はModMessageStreamのバッファにASCII文字を追加するのに用いる。
// バッファがすでにいっぱいのときは無視される。
//
// ARGUMENTS
// unsigned char character
//		追加するASCII文字
//
// RETURN
// バッファに追加し、自身への参照を返す。
//
// EXCEPTIONS
// なし
//

ModMessageStream&
ModMessageStream::operator <<(unsigned char v)
{
	return *this << (char) v;
}

//
// FUNCTION
// ModMessageStream::operator<< -- 数値の出力(int)
//
// NOTES
// この関数はModMessageStreamのバッファに数値を文字列に変換して
// 追加するのに用いる。
// 直前に ModWc や setWc が呼ばれている場合は ModWideChar とみなして
// ワイド文字として出力される。
// バッファがすでにいっぱいのときは無視される。
//
// ARGUMENTS
// int		v
//		追加する数値
//
// RETURN
// バッファに追加し、自身への参照を返す。
//
// EXCEPTIONS
// なし
//

ModMessageStream&
ModMessageStream::operator <<(int v)
{
	if (this->type == toNone)
		return *this;

	if (this->wideChar) {

		// 数値を ModWideChar へ変換する

		ModWideChar tmp[] =	{ (ModWideChar) v, ModWideCharTrait::null() };

		// フラグは出力したらリセットされる

		this->wideChar = ModFalse;

		// ModWideChar をメッセージストリームへ出力する
		//
		//【注意】	内部で汎用ライブラリーのミューテックスを使って
		//			排他制御を行うので、ここでする必要はない

		return *this << tmp;
	}

	// 数値を文字列へ変換する

	char	tmp[16];
	(void) ::sprintf(tmp, (this->numberType == ModMessageStream::typeDec) ?
					 "%d" : "%x", v);

	// 文字列をメッセージストリームへ出力する
	//
	//【注意】	内部で汎用ライブラリーのミューテックスを使って
	//			排他制御を行うので、ここでする必要はない

	return *this << tmp;
}

//
// FUNCTION
// ModMessageStream::operator<< -- 数値の出力(unsigned int)
//
// NOTES
// この関数はModMessageStreamのバッファに数値を文字列に変換して
// 追加するのに用いる。
// バッファがすでにいっぱいのときは無視される。
//
// ARGUMENTS
// unsigned int	v
//		追加する数値
//
// RETURN
// バッファに追加し、自身への参照を返す。
//
// EXCEPTIONS
// なし
//

ModMessageStream&
ModMessageStream::operator <<(unsigned int v)
{
	if (this->type == toNone)
		return *this;

	if (this->wideChar) {

		// 数値を ModWideChar へ変換する

		ModWideChar tmp[] =	{ (ModWideChar) v, ModWideCharTrait::null() };

		// フラグは出力したらリセットされる

		this->wideChar = ModFalse;

		// ModWideChar をメッセージストリームへ出力する
		//
		//【注意】	内部で汎用ライブラリーのミューテックスを使って
		//			排他制御を行うので、ここでする必要はない

		return *this << tmp;
	}

	// 数値を文字列へ変換する

	char	tmp[16];
	(void) ::sprintf(tmp, (this->numberType == ModMessageStream::typeDec) ?
					 "%u" : "%x", v);

	// 文字列をメッセージストリームへ出力する
	//
	//【注意】	内部で汎用ライブラリーのミューテックスを使って
	//			排他制御を行うので、ここでする必要はない

	return *this << tmp;
}

//
// FUNCTION
// ModMessageStream::operator<< -- 数値の出力(long)
//
// NOTES
// この関数はModMessageStreamのバッファに数値を文字列に変換して
// 追加するのに用いる。
// バッファがすでにいっぱいのときは無視される。
//
// ARGUMENTS
// long value
//		追加する数値
//
// RETURN
// バッファに追加し、自身への参照を返す。
//
// EXCEPTIONS
// なし
//

ModMessageStream&
ModMessageStream::operator <<(long v)
{
	if (this->type == toNone)
		return *this;

	// 数値を文字列へ変換する

	char	tmp[32];
	(void) ::sprintf(tmp, (this->numberType == ModMessageStream::typeDec) ?
					 "%ld" : "%lx", v);

	// 文字列をメッセージストリームへ出力する
	//
	//【注意】	内部で汎用ライブラリーのミューテックスを使って
	//			排他制御を行うので、ここでする必要はない

	return *this << tmp;
}

//
// FUNCTION
// ModMessageStream::operator<< -- 数値の出力(unsgined long)
//
// NOTES
// この関数はModMessageStreamのバッファに数値を文字列に変換して
// 追加するのに用いる。
// バッファがすでにいっぱいのときは無視される。
//
// ARGUMENTS
// unsigned long value
//		追加する数値
//
// RETURN
// バッファに追加し、自身への参照を返す。
//
// EXCEPTIONS
// なし
//

ModMessageStream&
ModMessageStream::operator <<(unsigned long v)
{
	if (this->type == toNone)
		return *this;

	// 数値を文字列へ変換する

	char	tmp[32];
	(void) ::sprintf(tmp, (this->numberType == ModMessageStream::typeDec) ?
					 "%lu" : "%lx", v);

	// 文字列をメッセージストリームへ出力する
	//
	//【注意】	内部で汎用ライブラリーのミューテックスを使って
	//			排他制御を行うので、ここでする必要はない

	return *this << tmp;
}

//
// FUNCTION
// ModMessageStream::operator<< -- 数値の出力(ModInt64)
//
// NOTES
// この関数はModMessageStreamのバッファに数値を文字列に変換して
// 追加するのに用いる。
// バッファがすでにいっぱいのときは無視される。
//
// ARGUMENTS
// ModInt64		v
//		追加する数値
//
// RETURN
// バッファに追加し、自身への参照を返す。
//
// EXCEPTIONS
// なし
//

ModMessageStream&
ModMessageStream::operator <<(ModInt64 v)
{
	if (this->type == toNone)
		return *this;

	// 数値を文字列へ変換する

	char	tmp[32];
	(void) ::sprintf(tmp, (this->numberType == ModMessageStream::typeDec) ?  "%lld" : "%llx" , v);

	// 文字列をメッセージストリームへ出力する
	//
	//【注意】	内部で汎用ライブラリーのミューテックスを使って
	//			排他制御を行うので、ここでする必要はない

	return *this << tmp;
}

//
// FUNCTION
// ModMessageStream::operator<< -- 数値の出力(ModUInt64)
//
// NOTES
// この関数はModMessageStreamのバッファに数値を文字列に変換して
// 追加するのに用いる。
// バッファがすでにいっぱいのときは無視される。
//
// ARGUMENTS
// ModUInt64	v
//		追加する数値
//
// RETURN
// バッファに追加し、自身への参照を返す。
//
// EXCEPTIONS
// なし
//

ModMessageStream&
ModMessageStream::operator <<(ModUInt64 v)
{
	if (this->type == toNone)
		return *this;

	// 数値を文字列へ変換する

	char	tmp[32];
	(void) ::sprintf(tmp, (this->numberType == ModMessageStream::typeDec) ?  "%llu" : "%llx" , v);

	// 文字列をメッセージストリームへ出力する
	//
	//【注意】	内部で汎用ライブラリーのミューテックスを使って
	//			排他制御を行うので、ここでする必要はない

	return *this << tmp;
}

//
// FUNCTION
// ModMessageStream::operator<< -- 数値の出力(float)
//
// NOTES
// この関数はModMessageStreamのバッファに数値を文字列に変換して
// 追加するのに用いる。
// バッファがすでにいっぱいのときは無視される。
//
// ARGUMENTS
// float value
//		追加する数値
//
// RETURN
// バッファに追加し、自身への参照を返す。
//
// EXCEPTIONS
// なし
//

ModMessageStream&
ModMessageStream::operator <<(float v)
{
	if (this->type == toNone)
		return *this;

	// 数値を文字列へ変換する

	char	tmp[32];
	(void) ::sprintf(tmp, "%g", (double) v);

	// 文字列をメッセージストリームへ出力する
	//
	//【注意】	内部で汎用ライブラリーのミューテックスを使って
	//			排他制御を行うので、ここでする必要はない

	return *this << tmp;
}

//
// FUNCTION
// ModMessageStream::operator<< -- 数値の出力(double)
//
// NOTES
// この関数はModMessageStreamのバッファに数値を文字列に変換して
// 追加するのに用いる。
// バッファがすでにいっぱいのときは無視される。
//
// ARGUMENTS
// double value
//		追加する数値
//
// RETURN
// バッファに追加し、自身への参照を返す。
//
// EXCEPTIONS
// なし
//

ModMessageStream&
ModMessageStream::operator <<(double v)
{
	if (this->type == toNone)
		return *this;

	// 数値を文字列へ変換する

	char	tmp[32];
	(void) ::sprintf(tmp, "%g", v);

	// 文字列をメッセージストリームへ出力する
	//
	//【注意】	内部で汎用ライブラリーのミューテックスを使って
	//			排他制御を行うので、ここでする必要はない

	return *this << tmp;
}

//
// FUNCTION
// ModMessageStream::operator<< -- Unicode 文字の出力
//
// NOTES
// この関数はModMessageStreamのバッファに Unicode 文字を追加するのに用いる。
// バッファがすでにいっぱいのときは無視される。
//
// ARGUMENTS
// ModUnicodeChar	v
//		追加する Unicode 文字
//
// RETURN
// バッファに追加し、自身への参照を返す。
//
// EXCEPTIONS
// なし
//

ModMessageStream&
ModMessageStream::operator <<(ModUnicodeChar v)
{
	if (this->type == toNone)
		return *this;

	ModUnicodeChar tmp[] =	{ v, ModUnicodeCharTrait::null() };

	// ModUnicodeChar をメッセージストリームへ出力する
	//
	//【注意】	内部で汎用ライブラリーのミューテックスを使って
	//			排他制御を行うので、ここでする必要はない
	
	return *this << tmp;
}

//
// FUNCTION
// ModMessageStream::operator<< -- ModLanguageCode の出力
//
// NOTES
// この関数はModMessageStreamのバッファに ModLanguageCode を追加するのに用いる。
// バッファがすでにいっぱいのときは無視される。
//
// ARGUMENTS
// ModLanguageCode	v
//		追加する言語コード
//
// RETURN
// バッファに追加し、自身への参照を返す。
//
// EXCEPTIONS
// なし
//

ModMessageStream&
ModMessageStream::operator <<(ModLanguageCode v)
{
	// 文字列を得て、メッセージストリームへ出力する
	//
	//【注意】	内部で汎用ライブラリーのミューテックスを使って
	//			排他制御を行うので、ここでする必要はない
	return (this->type == toNone) ? *this : (*this << ModLanguage::toSymbol(v));
}

//
// FUNCTION
// ModMessageStream::operator<< -- ModLanguageSet の出力
//
// NOTES
// この関数はModMessageStreamのバッファに ModLanguageSet を追加するのに用いる。
// バッファがすでにいっぱいのときは無視される。
//
// ARGUMENTS
// ModLanguageSet&	v
//		追加する言語コード
//
// RETURN
// バッファに追加し、自身への参照を返す。
//
// EXCEPTIONS
// なし
//

ModMessageStream&
ModMessageStream::operator <<(const ModLanguageSet& v)
{
	// 文字列を得て、メッセージストリームへ出力する
	//
	//【注意】	内部で汎用ライブラリーのミューテックスを使って
	//			排他制御を行うので、ここでする必要はない

	//return (this->type == toNone) ? *this : (*this << v.getName());
	// CC: WorkShop Compilers 4.2 16 Jun 1998 C++ 4.2 patch 104631-07
	// でコンパイルエラーになるため、下記のように書き換える

	if (this->type == toNone)
		return *this;
	else
		return (*this << v.getName());
}

//
// FUNCTION
// ModMessageStream::operator<< -- ModCharStringの出力
//
// NOTES
// この関数はModMessageStreamのバッファにModCharStringで表される文字列を
// 追加するのに用いる。
// バッファがすでにいっぱいのときは無視される。
//
// ARGUMENTS
// const ModCharString& string
//		追加する文字列
//
// RETURN
// バッファに追加し、自身への参照を返す。
//
// EXCEPTIONS
// なし
//

ModMessageStream&
ModMessageStream::operator <<(const ModCharString& v)
{
	// 文字列を得て、メッセージストリームへ出力する
	//
	//【注意】	内部で汎用ライブラリーのミューテックスを使って
	//			排他制御を行うので、ここでする必要はない

	return (this->type == toNone) ? *this : (*this << v.getString());
}

//
// FUNCTION
// ModMessageStream::operator<< -- ModWideStringの出力
//
// NOTES
// この関数はModMessageStreamのバッファにModWideStringで表される文字列を
// 追加するのに用いる。
// バッファがすでにいっぱいのときは無視される。
//
// ARGUMENTS
// const ModWideString& string
//		追加する文字列
//
// RETURN
// バッファに追加し、自身への参照を返す。
//
// EXCEPTIONS
// なし
//

ModMessageStream&
ModMessageStream::operator <<(const ModWideString& v)
{
	// 文字列を得て、メッセージストリームへ出力する
	//
	//【注意】	内部で汎用ライブラリーのミューテックスを使って
	//			排他制御を行うので、ここでする必要はない

	return (this->type == toNone) ? *this : (*this << v.getBuffer());
}

//
// FUNCTION
// ModMessageStream::operator<< -- ModUnicodeStringの出力
//
// NOTES
// この関数はModMessageStreamのバッファにModUnicodeStringで表される文字列を
// 追加するのに用いる。
// バッファがすでにいっぱいのときは無視される。
//
// ARGUMENTS
// const ModUnicodeString& string
//		追加する文字列
//
// RETURN
// バッファに追加し、自身への参照を返す。
//
// EXCEPTIONS
// なし
//

ModMessageStream&
ModMessageStream::operator <<(const ModUnicodeString& v)
{
	// 文字列を得て、メッセージストリームへ出力する
	//
	//【注意】	内部で汎用ライブラリーのミューテックスを使って
	//			排他制御を行うので、ここでする必要はない
	
	return (this->type == toNone) ?
		*this : (*this << (const_cast<ModUnicodeString&>(v)).getString(ModOs::Process::getEncodingType()));
}

//
// FUNCTION
// ModMessageStream::operator<< -- char*文字列の出力
//
// NOTES
// この関数はModMessageStreamのバッファにchar*で表される文字列を
// 追加するのに用いる。
// バッファがすでにいっぱいのときは無視される。
// 漢字コードは初期状態ではEUCとして扱われるが、ModEuc、ModShiftJis
// というマニピュレータで変更することができる。
//
// ARGUMENTS
// const char* string
//		追加する文字列
//
// RETURN
// バッファに追加し、自身への参照を返す。
//
// EXCEPTIONS
// なし
//
ModMessageStream&
ModMessageStream::operator<<(const char* string)
{
	if (this->type == toNone) {
		return *this;
	}

	ModAutoMutex<ModOsMutex> m(ModCommonMutex::getMutex());
	m.lock();

	this->check();

	int i;
	const char* cp;
	const ModKanjiCode::KanjiCodeType encodingType = ModOs::Process::getEncodingType();

	if (this->type == toSystemLog) {
		// 文字列長を調べる
		for (i = 0, cp = string;
			 cp != 0 && *cp != 0 && i < ModMessageBufferSize - 1;
			 ++i, ++cp);

		if (this->width <= i || this->doFill(i) == ModTrue) {
			ModSize capacity = ModMessageBufferSize - 1 - this->getSize() - 1;
			if (this->kanjiCode == encodingType
				|| this->kanjiCode == ModKanjiCode::unknown) {
				// 漢字コード変換不要
				for (i = capacity, cp = string; i > 0 && *cp != 0; --i) {
					*this->currentPointer++ = *cp++;
				}
				*this->currentPointer = 0;
			} else {
				// バイト数が capacity の領域にいちばん多くの文字を書ける
				// 文字コードは ASCII なので、 ASCII を想定して文字列を作成
				ModMultiByteString mbstr(string, this->kanjiCode, encodingType);
				for (i = capacity, cp = mbstr.get(); i > 0 && *cp != 0; --i) {
					*this->currentPointer++ = *cp++;
				}
				*this->currentPointer = 0;
			}
		}
		this->width = 0;
	} else {

		// 出力先を求める

		FILE*	out = (this->type == toStandardOut) ? stdout : stderr;

		// 文字列長を調べる
		for (i = 0, cp = string; cp != 0 && *cp != 0; ++i, ++cp);
		if (this->width > i)
			(void) this->doFill(i);

		this->width = 0;

		if (this->kanjiCode == encodingType
			|| this->kanjiCode == ModKanjiCode::unknown)

			// そのまま出力する

			(void) ::fputs(string, out);
		else {
			// 出力先の漢字コードへ変換して出力する
			ModMultiByteString mbstr(string, this->kanjiCode, encodingType);
			(void) ::fputs(mbstr.get(), out);
		}
	}

	return *this;
}

//
// FUNCTION
// ModMessageStream::operator<< -- ModWideChar*文字列の出力
//
// NOTES
// この関数はModMessageStreamのバッファにModWideChar*で表される文字列を
// 追加するのに用いる。
// バッファがすでにいっぱいのときは無視される。
//
// ARGUMENTS
// const ModWideChar* string
//		追加する文字列
//
// RETURN
// バッファに追加し、自身への参照を返す。
//
// EXCEPTIONS
// なし
//
ModMessageStream&
ModMessageStream::operator<<(const ModWideChar* string)
{
	if (this->type == toNone) {
		return *this;
	}

	ModAutoMutex<ModOsMutex> m(ModCommonMutex::getMutex());
	m.lock();

	this->check();

	if (string == 0) {
		return *this << "<null>";
	}

	int i;
	const ModWideChar* wp;
	ModKanjiCode::KanjiCodeType encodingType = ModOs::Process::getEncodingType();

	// もし、文字のエンコーディング方式が unknown になっていても
	// ワイド文字を出力する時は絶対に何らかのマルチバイト文字に
	// 変換しなければいけない。
	if (encodingType == ModKanjiCode::unknown) {
		encodingType = ModKanjiCode::euc;
	}

	if (this->type == toSystemLog) {
		// バイト長を調べる
		for (i = 0, wp = string;
			 *wp != 0 && i < ModMessageBufferSize-1;
			 i += ModWideCharTrait::byteSize(*wp), ++wp);

		if (this->width <= i || this->doFill(i) == ModTrue)
			for (i = ModMessageBufferSize - 1 - this->getSize(), wp = string;
				 i - ModWideCharTrait::byteSize(*wp) >= 0 && *wp != 0;
				 i -= ModWideCharTrait::byteSize(*wp), ++wp) {
				this->currentPointer +=
					ModWideCharTrait::convertToString(this->currentPointer,
													  *wp,
													  encodingType);
			}
		this->width = 0;
	} else {

		// 出力先を求める

		FILE*	out = (this->type == toStandardOut) ? stdout : stderr;

		// バイト長を調べる
		for (i = 0, wp = string; *wp != 0;
			 i += ModWideCharTrait::byteSize(*wp), ++wp);
		// fill は必ず成功する
		if (this->width > i)
			(void) this->doFill(i);

		this->width = 0;

		for (wp = string; *wp != 0; ++wp) {
			*(unsigned int*)&this->buffer[0] = 0;
			ModWideCharTrait::convertToString(this->buffer,
											  *wp,
											  encodingType);
			(void) ::fputs(this->buffer, out);
		}
	}

	return *this;
}

//
// FUNCTION
// ModMessageStream::operator<< -- ModUnicodeChar*文字列の出力
//
// NOTES
// この関数はModMessageStreamのバッファにModUnicodeChar*で表される文字列を
// 追加するのに用いる。
// バッファがすでにいっぱいのときは無視される。
//
// ARGUMENTS
// const ModUnicodeChar* string
//		追加する文字列
//
// RETURN
// バッファに追加し、自身への参照を返す。
//
// EXCEPTIONS
// なし
//
ModMessageStream&
ModMessageStream::operator<<(const ModUnicodeChar* string)
{
	if (this->type == toNone) {
		return *this;
	}

	ModAutoMutex<ModOsMutex> m(ModCommonMutex::getMutex());
	m.lock();

	this->check();

	if (string == 0) {
		return *this << "<null>";
	}

	// 変換先の文字コードに unknown を渡されている場合、ユーザーは ASCII
	// 文字だけを使うつもりなのだろう。そこで、UCS2 (2byte) を ASCII (1 byte)
	// に変換するため、仮に EUC にコード変換する。
	ModKanjiCode::KanjiCodeType encodingType = ModOs::Process::getEncodingType();
	if (this->kanjiCode == ModKanjiCode::unknown) {
		encodingType = ModKanjiCode::euc;
	}
	ModMultiByteString mbString((const char*)string, ModKanjiCode::ucs2,
								encodingType);
	const char* convertedString = mbString.get();

	int i;
	const char* cp;

	if (this->type == toSystemLog) {
		// 文字列長を調べる
		for (i = 0, cp = convertedString;
			 cp != 0 && *cp != 0 && i < ModMessageBufferSize - 1;
			 ++i, ++cp);

		if (this->width <= i || this->doFill(i) == ModTrue) {
			ModSize capacity = ModMessageBufferSize - 1 - this->getSize() - 1;
			for (i = capacity, cp = convertedString; i > 0 && *cp != 0; --i) {
				*this->currentPointer++ = *cp++;
			}
			*this->currentPointer = 0;
		}
		this->width = 0;
	} else {

		// 出力先を求める

		FILE*	out = (this->type == toStandardOut) ? stdout : stderr;

		// 文字列長を調べる
		for (i = 0, cp = convertedString; cp != 0 && *cp != 0; ++i, ++cp);
		if (this->width > i)
			(void) this->doFill(i);

		this->width = 0;

		(void) ::fputs(convertedString, out);
	}

	return *this;
}

//
// FUNCTION
// ModMessageStream::operator<< -- ModExceptionの出力
//
// NOTES
// この関数はModMessageStreamのバッファにModException.setMessage が返す
// 文字列を追加するのに用いる。
// バッファがすでにいっぱいのときは無視される。
//
// ARGUMENTS
// ModException& exception
//		メッセージに追加する例外
//
// RETURN
// バッファに追加し、自身への参照を返す。
//
// EXCEPTIONS
// なし
//

ModMessageStream&
ModMessageStream::operator <<(ModException& v)
{
	// 文字列を得て、メッセージストリームへ出力する
	//
	//【注意】	内部で汎用ライブラリーのミューテックスを使って
	//			排他制御を行うので、ここでする必要はない

	errorNumber = v.getErrorNumber();
	return (this->type == toNone) ? *this : (*this << v.setMessage());
}

//
// FUNCTION
// ModMessageStream::operator<< -- マニピュレーター
//
// NOTES
// この関数はModMessageStreamに対してマニピュレーターを適用するのに用いる。
//
// ARGUMENTS
// ModMessageStream::Manipulator manipulator
//		適用するマニピュレーター
//
// RETURN
// 自身にマニピュレーターを適用し、自身への参照を返す。
//
// EXCEPTIONS
// なし
//

ModMessageStream&
ModMessageStream::operator <<(Manipulator manipulator)
{
	if (this->type == toNone)
		return *this;

	ModAutoMutex<ModOsMutex> m(ModCommonMutex::getMutex());
	m.lock();
	
	return manipulator(*this);
}

//
// FUNCTION
// ModMessageStream::operator<< -- 引数付マニピュレーター用の operator<<
//
// NOTES
// このオペレータはストリームに対する operator<< の、引数付マニピュレーター用
// を定義する。
//
// ARGUMENTS
// ModMessageStream& stream
//		オペレータの左辺にくるストリーム
// ModManipulator& manipulator
//		引数付マニピュレーターの返す値
//
// RETURN
// マニピュレーターを実行して stream と同じものを返す。
//
// EXCEPTIONS
// なし
//

ModMessageStream&
operator <<(ModMessageStream& stream, ModManipulator& manipulator)
{
	(*manipulator.function)(stream, manipulator.argument);
	return stream;
}

//
// FUNCTION
// ModMessageStream::setWidth -- 続く出力の出力幅の設定
//
// NOTES
// この関数は続くoperator<<による出力の出力幅を指定するのに用いる。
// operator<<がひとつ処理されるとリセットされるので注意。
//
// ARGUMENTS
// int width_
//		出力幅(バイト数で指定)
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
void
ModMessageStream::setWidth(int width_)
{
	(void)ModSetWidthInternal(*this, width_);
}

//
// FUNCTION
// ModMessageStream::setFill -- フィリング文字の設定
//
// NOTES
// この関数はsetWidthで指定した幅を埋めるのに用いる文字を指定するのに用いる。
// これによって指定しない間は空白が使用される。リセットはされない。
//
// ARGUMENTS
// int fill_
//		設定するフィリング文字
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
void
ModMessageStream::setFill(int fill_)
{
	(void)ModSetFillInternal(*this, fill_);
}

//
// FUNCTION
// ModMessageStream::setWc -- ModWideCharが入力されることを宣言
//
// NOTES
// この関数は次の入力がModWideCharであることを宣言するために用いる。
//
// ARGUMENTS
// なし
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
void
ModMessageStream::setWc()
{
	(void)ModWc(*this);
}

//
// FUNCTION
// ModMessageStream::setEuc -- 入力の漢字コードをEUCに設定
//
// NOTES
// この関数は char* が入力に使われるときの漢字コードをEUCに設定するのに用いる。
// 初期状態はEUCに設定されている。
//
// ARGUMENTS
// なし
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
void
ModMessageStream::setEuc()
{
	(void)ModEuc(*this);
}

//
// FUNCTION
// ModMessageStream::setShiftJis -- 入力の漢字コードをShiftJISに設定
//
// NOTES
// この関数は char* が入力に使われるときの漢字コードをShiftJISに設定するのに
// 用いる。
// これで設定した状態は次に ModEndl、ModFlush したときに EUC の戻される。
// 初期状態はEUCに設定されている。
//
// ARGUMENTS
// なし
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
void
ModMessageStream::setShiftJis()
{
	(void)ModShiftJis(*this);
}

//
// FUNCTION
// ModFlush -- ModMessageStream に対する flush マニピュレーター
//
// NOTES
// この関数はModMessageStreamのバッファの内容を出力するために用いる
// マニピュレーターである。
//
// ARGUMENTS
// ModMessageStream& stream
//		flush するストリーム
//
// RETURN
// typeにしたがってバッファの内容を出力し、自身への参照を返す。
// バッファの内容、数値の10/16進の別はリセットされる。
//
// EXCEPTIONS
// なし
//
ModMessageStream&
ModFlush(ModMessageStream& stream)
{
	if (stream.type == ModMessageStream::toNone) {
		return stream;
	}

	if (stream.currentStatus == statusFirstOutput) {
		// バッファをクリアする
		stream.clear();
	} else {
		// バッファの内容を出力し、クリアする
		stream.doFlush();
	}

	// ostream と異なり、flush、endl でリセットされることにする
	stream.numberType = ModMessageStream::typeDec;

	// 漢字コードもリセットする
	stream.kanjiCode = ModOs::Process::getEncodingType();

	// header出力フラグをリセットする
	stream.noHeader = ModFalse;

	// 状態をクリアする
	stream.currentStatus = statusFlushed;

	return stream;
}

//
// FUNCTION
// ModEndl -- ModMessageStream に対する endl マニピュレーター
//
// NOTES
// この関数はModMessageStreamのバッファの内容に改行を付加して出力するために
// 用いるマニピュレーターである。
//
// ARGUMENTS
// ModMessageStream& stream
//		flush するストリーム
//
// RETURN
// バッファの最後に改行を付加してtypeにしたがってバッファの内容を出力し、
// 自身への参照を返す。
// バッファの内容、数値の10/16進の別はリセットされる。
//
// EXCEPTIONS
// なし
//
ModMessageStream&
ModEndl(ModMessageStream& stream)
{
	if (stream.type == ModMessageStream::toNone) {
		return stream;
	}

	if (stream.type == ModMessageStream::toStandardOut ||
		stream.type == ModMessageStream::toStandardError) {
		if (stream.currentStatus == statusContinue) {

			// 出力先を求める

			(void) ::fputc('\n',
						   (stream.type == ModMessageStream::toStandardOut) ?
						   stdout : stderr);
		}
	}
	return ModFlush(stream);
}

//
// FUNCTION
// ModHex -- ModMessageStream に対する hex マニピュレーター
//
// NOTES
// この関数はModMessageStreamの数値の出力を16進形式に変更するために用いる。
// これで設定したものはModEndl、ModFlushによって10進にリセットされる。
//
// ARGUMENTS
// ModMessageStream& stream
// 対象となるストリーム
//
// RETURN
// 出力形式を16進に変更し、自身への参照を返す。
//
// EXCEPTIONS
// なし
//

ModMessageStream&
ModHex(ModMessageStream& stream)
{
	if (stream.type != ModMessageStream::toNone) {
		ModAutoMutex<ModOsMutex> m(ModCommonMutex::getMutex());
		m.lock();

		stream.numberType = ModMessageStream::typeHex;
	}
	return stream;
}

//
// FUNCTION
// ModDec -- ModMessageStream に対する dec マニピュレーター
//
// NOTES
// この関数はModMessageStreamの数値の出力を16進形式に変更するために用いる。
// これで設定したものはModEndl、ModFlushによって10進にリセットされる。
//
// ARGUMENTS
// ModMessageStream& stream
//		対象となるストリーム
//
// RETURN
// 出力形式を16進に変更し、自身への参照を返す。
//
// EXCEPTIONS
// なし
//

ModMessageStream&
ModDec(ModMessageStream& stream)
{
	if (stream.type != ModMessageStream::toNone) {
		ModAutoMutex<ModOsMutex> m(ModCommonMutex::getMutex());
		m.lock();

		stream.numberType = ModMessageStream::typeDec;
	}
	return stream;
}

//
// FUNCTION
// ModWc -- ModWideCharの入力を宣言するマニピュレーター
//
// NOTES
// この関数はModMessageStreamのint、unsigned intの入力を
// ModWideCharとして文字列で出力することを宣言するために用いる。
// この宣言は一度出力するとリセットされる。
//
// ARGUMENTS
// ModMessageStream& stream
//		対象となるストリーム
//
// RETURN
// ModWideCharが入力することを示すフラグをセットし、自身への参照を返す。
//
// EXCEPTIONS
// なし
//

ModMessageStream&
ModWc(ModMessageStream& stream)
{
	if (stream.type != ModMessageStream::toNone) {
		ModAutoMutex<ModOsMutex> m(ModCommonMutex::getMutex());
		m.lock();

		stream.wideChar = ModTrue;
	}
	return stream;
}

//
// FUNCTION
// ModEuc -- char* の入力漢字コードをEUCに設定するマニピュレーター
//
// NOTES
// この関数はModMessageStreamのchar*の入力の漢字コードをEUCに設定する
// ために用いる。
// 初期状態はEUCに設定されている。
//
// ARGUMENTS
// ModMessageStream& stream
//		対象となるストリーム
//
// RETURN
// 入力の漢字コードを EUC に設定し、自身への参照を返す。
//
// EXCEPTIONS
// なし
//

ModMessageStream&
ModEuc(ModMessageStream& stream)
{
	if (stream.type != ModMessageStream::toNone) {
		ModAutoMutex<ModOsMutex> m(ModCommonMutex::getMutex());
		m.lock();

		stream.kanjiCode = ModKanjiCode::euc;
	}
	return stream;
}

//
// FUNCTION
// ModShiftJis -- char* の入力漢字コードをShiftJISに設定するマニピュレーター
//
// NOTES
// この関数はModMessageStreamのchar*の入力の漢字コードをShiftJISに設定する
// ために用いる。
// これで設定したものは次に ModEndl、ModFlush すると EUC に戻される。
// 初期状態はEUCに設定される。
//
// ARGUMENTS
// ModMessageStream& stream
//		対象となるストリーム
//
// RETURN
// 入力の漢字コードを ShiftJIS に設定し、自身への参照を返す。
//
// EXCEPTIONS
// なし
//

ModMessageStream&
ModShiftJis(ModMessageStream& stream)
{
	if (stream.type != ModMessageStream::toNone) {
		ModAutoMutex<ModOsMutex> m(ModCommonMutex::getMutex());
		m.lock();

		stream.kanjiCode = ModKanjiCode::shiftJis;
	}
	return stream;
}

//
// FUNCTION
// ModLiteralCode -- char* の入力漢字コードを文字列リテラルの漢字コードに設定するマニピュレーター
//
// NOTES
// この関数はModMessageStreamのchar*の入力の漢字コードを
// 文字列リテラルに使われる漢字コードに設定するために用いる。
// これで設定したものは次に ModEndl、ModFlush すると EUC に戻される。
// 初期状態はEUCに設定される。
//
// ARGUMENTS
// ModMessageStream& stream
//		対象となるストリーム
//
// RETURN
// 入力の漢字コードを文字列リテラルに使われる漢字コードに設定し、
// 自身への参照を返す。
//
// EXCEPTIONS
// なし
//

ModMessageStream&
ModLiteralCode(ModMessageStream& stream)
{
	if (stream.type != ModMessageStream::toNone) {
		ModAutoMutex<ModOsMutex> m(ModCommonMutex::getMutex());
		m.lock();

		stream.kanjiCode = ModKanjiCode::literalCode;
	}
	return stream;
}

//
// FUNCTION
// ModSetW -- 続く出力の出力幅を設定するマニピュレーター
//
// NOTES
// この関数は続くoperator<<による出力の出力幅を指定するのに用いる。
// operator<<がひとつ処理されるとリセットされるので注意。
//
// ARGUMENTS
// int width_
//		出力幅(バイト数で指定)
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//

ModManipulator&
ModSetW(int width_)
{
	ModCommonInitialize::checkAndInitialize();

	ModAutoMutex<ModOsMutex> m(ModCommonMutex::getMutex());
	m.lock();

	ModManipulatorSelection::setWidth->argument = width_;

	return *ModManipulatorSelection::setWidth;
}

//
// FUNCTION
// ModSetFill -- フィリング文字を設定するマニピュレーター
//
// NOTES
// この関数はsetWidthで指定した幅を埋めるのに用いる文字を指定するのに用いる。
// これによって指定しない間は空白が使用される。リセットはされない。
//
// ARGUMENTS
// int fill_
//		設定するフィリング文字
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
ModManipulator&
ModSetFill(int fill_)
{
	ModCommonInitialize::checkAndInitialize();

	ModAutoMutex<ModOsMutex> m(ModCommonMutex::getMutex());
	m.lock();

	ModManipulatorSelection::setFill->argument = fill_;

	return *ModManipulatorSelection::setFill;
}

//
// FUNCTION private
// ModMessageStream::check -- 処理前に必要なら flush する
//
// NOTES
// この関数はストリームに対する処理を行なう前に必要なら flush を
// するために用いる。
//
// ARGUMENTS
// なし
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
void
ModMessageStream::check()
{
	ModThreadId	self = ModThisThread::self();
	//
	// すでに許容量いっぱいならflushする
	//
	if (this->getSize() >= (ModSize)(ModMessageBufferSize - 1 - this->width)) {
		this->doFlush();
		if (this->type == toSystemLog) {
			// syslog に出す時はここで一度切る
			this->noHeader = ModFalse;
		}
	}
	//
	// 最初のオペレーターなら mutex ロックを外しておく
	//
	if (this->currentStatus == statusFirstOutput) {

		// ヘッダを出力する
		if (ModMessageSelection::headerFlag == ModTrue &&
			this->noHeader == ModFalse) {
			switch (this->type) {
			case toStandardOut:
				(void) ::fprintf(stdout, "%s+%3u(ms) ",
								 this->headerBuffer, this->milliSecond);
				break;
			case toStandardError:
				(void) ::fprintf(stderr, "%s+%3u(ms) ",
								 this->headerBuffer, this->milliSecond);
				break;
			default:
				break;
			}
			// ModFlushを呼ばれるまで出力しなくてよい
			this->noHeader = ModTrue;
		}
		switch (this->type) {
		case toStandardOut:
			(void) ::fputs(this->buffer, stdout);		break;
		case toStandardError:
			(void) ::fputs(this->buffer, stderr);		break;
		default:
			break;
		}

		// 「続き」状態にする
		this->currentStatus = statusContinue;
	}
}

//
// FUNCTION private
// ModMessageStream::isEmpty -- ストリームが空か調べる
//
// NOTES
// この関数はストリームのバッファが空かを調べるのに用いる
//
// ARGUMENTS
// なし
//
// RETURN
// バッファが空の場合は ModTrueを返し、空でない場合は ModFalseを返す。
//
// EXCEPTIONS
// なし
//

ModBoolean
ModMessageStream::isEmpty(void) const
{
	return (this->currentPointer == this->buffer) ?	ModTrue	: ModFalse;
}

//
// FUNCTION private
// ModMessageStream::clear -- ストリームを空にする
//
// NOTES
// この関数はストリームのバッファを空にするのに用いる。
//
// ARGUMENTS
// なし
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//

void
ModMessageStream::clear(void)
{
	*(this->currentPointer = this->buffer) = 0;
}

//
// FUNCTION private
// ModMessageStream::doFlush -- ストリームの内容を出力し、空にする
//
// NOTES
// この関数はストリームの内容をtypeにしたがって出力し、
// バッファを空にするのに用いる。
//
// ARGUMENTS
// なし
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
void
ModMessageStream::doFlush(void)
{
	{
		ModAutoMutex<ModOsMutex> m(ModCommonMutex::getMutex());
		m.lock();

		if (hostName[0] == 0)
		{
		
			// まだホスト名を取得していないので、取得する

			try {
				ModOsDriver::Socket::getHostname(hostName,
												 ModHostNameLength - 1);
			} catch (...) {

				// 実行しているホスト名を得られないとき、
				// エラーを無視し、ホスト名として "????" を設定しておく

				ModErrorHandle::reset();
				ModCharTrait::copy(hostName, "????");
			}
		}
	}

	const ModKanjiCode::KanjiCodeType encodingType = ModOs::Process::getEncodingType();

	//
	// バッファの内容を出力する
	//
	switch (this->type) {
	case toStandardOut:
		fflush(stdout);		break;
	case toStandardError:
		fflush(stderr);		break;
	case toSystemLog:
#if MOD_CONF_SYSTEM_LOG == 1
		if (this->isEmpty() == ModFalse) {
			//
			// syslog は MSB を落すので JIS に変換する
			// バッファサイズは syslog の作成するヘッダ部分と
			// "+???(ms) "の分だけ減らす
			// (どういうわけか 1024 バイトぴったりは使い切れない)
			//
			ModMultiByteString	mbstr(this->buffer, encodingType,
									  ModKanjiCode::jis);
			const ModSize size =
				ModMessageSyslogSize - this->headerSize - this->logHeaderSize;
			const char* srcPtr = mbstr.get();
			char* dstPtr = this->jisBuffer + this->headerSize;
			for (ModSize i = 0; i < size; ++i, ++srcPtr, ++dstPtr) {
				*dstPtr = *srcPtr;
				if (*srcPtr == 0)
					break;
			}

			// '\n'で切られるのでスペースに変換
			// ESC もスペースに変換しておく
			for (char *cp = this->jisBuffer; *cp != 0; ++cp) {
				if (*cp == '\n' || *cp == '\033') {
					*cp = ' ';
				}
			}
			
			syslog(this->priority, "%s", this->jisBuffer);
		}
#endif
		break;
	default:
		break;
	}
	this->clear();
}

//
// FUNCTION private
// ModMessageStream::getSize -- バッファに入っているバイト数を得る
//
// NOTES
// この関数は現在バッファに入っているバイト数を得るのに用いる。
//
// ARGUMENTS
// なし
//
// RETURN
// バッファに格納されているデータのバイト数を返す。
//
// EXCEPTIONS
// なし
//
ModSize
ModMessageStream::getSize(void) const
{
	return (ModSize)(this->currentPointer - this->buffer);
}

//
// FUNCTION private
// ModMessageStream::initializeMessage -- メッセージの初期化を行なう
//
// NOTES
// この関数はメッセージストリームの初期化を行なう。
//
// ARGUMENTS
// const char* fileName
//		メッセージ出力のコードが書かれているソースファイル名(__FILE__)
// int lineNo
//		メッセージ出力のコードが書かれている行番号(__LINE__)
// ModMessageStream::MessageLevel level
//		メッセージの種別を表す定数
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
void
ModMessageStream::initializeMessage(const char* fileName, int lineNo,
									MessageLevel level)
{
	const ModKanjiCode::KanjiCodeType
		encodingType = ModOs::Process::getEncodingType();

	ModThreadId	self = ModThisThread::self();
	if (this->threadId != self) {
		// スレッドが変わっていたらflushする
		if (this->currentStatus != statusFlushed) {
			ModEndl(*this);
			this->currentStatus = statusFlushed;
		}
		// 新しいスレッドIDを記録する
		this->threadId = self;
	}

	//
	// fileName は通常パス名になっているので basename を得る
	//

	const char	sep = ModOsDriver::File::getPathSeparator();
	const char* cp	= 0;

	if (encodingType == ModKanjiCode::unknown) {
		cp = ModCharTrait::rfind(fileName, sep);
		if (*cp == sep) {
			++cp;
		}
	} else {
		char* tmpPtr = 0;

		// パス名から取り除きたい部分の文字列(UTF8)を作成
		ModMultiByteString mbString(fileName,
									encodingType,
									ModKanjiCode::utf8);
		tmpPtr = ModCharTrait::rfind(mbString.get(), sep);
		if (*tmpPtr == sep) {
			++tmpPtr;
		}
		// 文字列の文字コードを元に戻してから、パス名から取り除きたい
		// 部分のバイト数を求めて cp が basename を指すようにする
		// (cp は何らかのストリームに出力される可能性があるので
		//  元の文字コードに戻さなければいけない)
		if (tmpPtr == mbString.get()) {
			cp = fileName;	// sep はみつからなかった
		} else {
			*tmpPtr = ModCharTrait::null();
			ModMultiByteString disusedString(mbString.get(),
											 ModKanjiCode::utf8,
											 encodingType);
			cp = fileName + ModCharTrait::length(disusedString.get());
		}
	}

	//
	// 種別を表す文字列
	//
	const char* category;

#if MOD_CONF_SYSTEM_LOG == 1

	// syslog のための変数をセットする
	switch (level) {
	case levelDebug:
		category = "[DEBUG]";
		this->priority = LOG_DEBUG;
		break;
	case levelInformation:
		category = "[INFO]";
		this->priority = LOG_INFO;
		break;
	case levelWarning:
		category = "[WARN]";
		this->priority = LOG_WARNING;
		break;
	case levelError:
		category = "[ERR]";
		this->priority = LOG_ERR;
		break;
	default:
		category = "[UNKNOWN]";
		this->priority = LOG_ERR;
		break;
	}
#endif
#if MOD_CONF_SYSTEM_LOG == 2
	// EventLog のための変数をセットする
	switch (level) {
	case levelDebug:
		category = "[DEBUG]";
		this->logType = EVENTLOG_INFORMATION_TYPE;
		this->eventID = ModMessageEventDebug;
		break;
	case levelInformation:
		category = "[INFO]";
		this->logType = EVENTLOG_INFORMATION_TYPE;
		this->eventID = ModMessageEventInformation;
		break;
	case levelWarning:
		category = "[WARN]";
		this->logType = EVENTLOG_WARNING_TYPE;
		this->eventID = ModMessageEventWarning;
		break;
	case levelError:
		category = "[ERR]";
		this->logType = EVENTLOG_ERROR_TYPE;
		this->eventID = ModMessageEventError;
		break;
	default:
		category = "[UNKNOWN]";
		this->logType = EVENTLOG_ERROR_TYPE;
		this->eventID = ModMessageEventError;
		break;
	}
#endif

	//
	// headerをまだ出力していない場合は
	// 出力する
	//
#if MOD_CONF_SYSTEM_LOG == 1
	if (this->type == toSystemLog) {
		// 時刻の部分を作る(msの単位まで計る)
		this->setHeader();

		this->headerSize = ::sprintf(this->jisBuffer, "+%03u(ms) ",
									 this->milliSecond);
	} else
#endif
	if (ModMessageSelection::headerFlag == ModTrue &&
		this->noHeader == ModFalse) {

		// 時刻の部分を作る(msの単位まで計る)
		this->setHeader();

#if MOD_CONF_SYSTEM_LOG == 2
		if (this->type == toSystemLog) {
			this->headerSize = ::sprintf(this->eventlogBuffer, "%s+%03u(ms) ",
										 this->headerBuffer,
										 this->milliSecond);
			
			// ModFlushを呼ばれるまで出力しなくてよい
			this->noHeader = ModTrue;
		}
#endif
	} else {
		this->headerSize = 0;
	}

#if MOD_CONF_SYSTEM_LOG == 2
	if (ModMessageSelection::headerFlag == ModFalse)
	{
		//
		// ヘッダーを出力しないとプロセスIDが出力されないので...
		// プロセスID、スレッドID、file名、行番号、カテゴリを加える
		//
		(void) ::sprintf(this->buffer, "(%u:%u) %s %d: %s ",
						 ModOsDriver::Process::self(),
						 _getThreadIdForLog(), cp, lineNo, category);
	}
	else
	{
#endif
		//
		// スレッドID、file名、行番号、カテゴリを加える
		//
		(void) ::sprintf(this->buffer, "(%u) %s %d: %s ",
						 _getThreadIdForLog(), cp, lineNo, category);
#if MOD_CONF_SYSTEM_LOG == 2
	}
#endif

	::sprintf(fileNameBuf, "%s", cp);
	lineNumber = lineNo;
	messageLevel = level;

	if (this->type == toSystemLog) {
#if MOD_CONF_SYSTEM_LOG == 1
		this->headerSize += ::sprintf(this->jisBuffer + this->headerSize,
									  "%s", this->buffer);
#endif
#if MOD_CONF_SYSTEM_LOG == 2
		this->headerSize += ::sprintf(this->eventlogBuffer + this->headerSize,
									  "%s", this->buffer);
#endif
		this->clear();
	}
}

//
// FUNCTION private
// ModMessageStream::setHeader -- メッセージのヘッダー部を作る
//
// NOTES
// この関数はメッセージをsyslogのように見せるために時刻などの情報を
// 付加するために用いる。
//
// ARGUMENTS
// なし
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//

void
ModMessageStream::setHeader()
{
	{
		ModAutoMutex<ModOsMutex> m(ModCommonMutex::getMutex());
		m.lock();

		if (hostName[0] == 0)
		{
		
			// まだホスト名を取得していないので、取得する
		
			try {
				ModOsDriver::Socket::getHostname(hostName,
												 ModHostNameLength - 1);
			} catch (...) {

				// 実行しているホスト名を得られないとき、
				// エラーを無視し、ホスト名として "????" を設定しておく

				ModErrorHandle::reset();
				ModCharTrait::copy(hostName, "????");
			}
		}
	}

	// 現在時刻を得る

	time_t	sec = 0;
#if	MOD_CONF_FUNC_GETTIMEOFDAY == 1
	struct timeval	tv;
	::gettimeofday(&tv, 0);
	sec = tv.tv_sec;
	this->milliSecond = tv.tv_usec / 1000;
#endif
#if MOD_CONF_FUNC_GETTIMEOFDAY == 2
	struct _timeb	tb;
	::_ftime(&tb);
	sec = tb.time;
	this->milliSecond = tb.millitm;
#endif

	// 現在時刻を表す文字列を得る

	char*	s;
#if defined(MOD_NO_THREAD) || MOD_CONF_LIB_POSIX == 0
	s = ::ctime(&sec);
#else
	const int		bufSize = 32;
	char			buf[bufSize];
#ifdef _POSIX_PTHREAD_SEMANTICS
	s = ::ctime_r(&sec, buf);
#else
#if MOD_CONF_LIB_POSIX == 1
	s = ::ctime_r(&sec, buf, bufSize);
#else
	s = ::ctime_r(&sec, buf);
#endif
#endif
#endif
	// 曜日をとばすために +4 する

	s += 4;

	// ctime の結果は最後に改行が入ってしまうので切る
	// (マルチバイト文字列ではないので、直接文字列操作してもだいじょうぶ)

	char* p = s;
	for (; *p != '\n' && *p; p++) ;
	*p = 0;

	// headerBuffer に格納する
#if MOD_CONF_SYSTEM_LOG == 1
	this->logHeaderSize =
#endif 
		::sprintf(this->headerBuffer, "%.15s %s %s[%u]: ",
				  s, this->hostName, this->commandName,
				  ModOsDriver::Process::self());
}

//
// FUNCTION private
// ModMessageStream::doFill -- フィリングを行なう
//
// NOTES
// この関数はフィリングを行なうのに用いる。
//
// ARGUMENTS
// int width_
//		データとして出力したい部分のバイト数
//
// RETURN
// width_バイトのデータが指定されている出力幅に収まるようにフィリングを行なう。
// フィリングだけでバッファのサイズを超えたらModFalseを返し、
// まだ超えていない場合はModTrueを返す。
//
// EXCEPTIONS
// なし
//
ModBoolean
ModMessageStream::doFill(int width_)
{
	int size;
	ModBoolean result;

	if ((this->type == toSystemLog)
		&& this->getSize() + this->width - width_
		>= (ModSize)(ModMessageBufferSize - 1)) {
		// 加えた結果許容量に達する
		size = ModMessageBufferSize - 1 - this->getSize();
		result = ModFalse;
		for (; size > 0; ++this->currentPointer, --size) {
			*this->currentPointer = (char)this->fill;
		}
		*this->currentPointer = 0;
	} else {
		size = this->width - width_;
		result = ModTrue;
		for (; size > 0; --size) {
			switch (this->type) {
			case toStandardOut:
				(void) ::fputc(this->fill, stdout);			break;
			case toStandardError:
				(void) ::fputc(this->fill, stderr);			break;
			case toSystemLog:
				*this->currentPointer++ = (char)this->fill;	break;
			default:
				break;
			}
		}
	}

	return result;
}

//
// FUNCTION private
// ModSetWidthInternal -- 続く出力の出力幅の設定
//
// NOTES
// この関数は続くoperator<<による出力の出力幅を指定するのに用いる。
// operator<<がひとつ処理されるとリセットされるので注意。
// ModSetWやModMessageStream::setWidth の下請け。
//
// ARGUMENTS
// ModMessageStream& stream
//		対象のストリーム
// int width_
//		出力幅(バイト数で指定)
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
ModMessageStream&
ModSetWidthInternal(ModMessageStream& stream, int width_)
{
	stream.width = width_;
	return stream;
}

//
// FUNCTION private
// ModSetFillInternal -- フィリング文字の設定
//
// NOTES
// この関数はsetWidthで指定した幅を埋めるのに用いる文字を指定するのに用いる。
// これによって指定しない間は空白が使用される。リセットはされない。
// ModSetFillやModMessageStream::setFill の下請け。
//
// ARGUMENTS
// ModMessageStream& stream
//		対象のストリーム
// int fill_
//		フィリング文字
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
ModMessageStream&
ModSetFillInternal(ModMessageStream& stream, int fill_)
{
	stream.fill = fill_;
	return stream;
}

// FUNCTION private
// ModMessageStream::lock -- 出力を排他するためにロックする
//
// NOTES
// この関数はModMessageStreamWrapperから呼ばれる。
//
// ARGUMENTS
// なし
//
// RETURN
// なし
//
// EXCEPTION
// ModOsMutex::lockと同じ
void
ModMessageStream::lock()
{
	ModCommonMutex::lock();
}

// FUNCTION private
// ModMessageStream::unlock -- 出力を排他するためのロックを外す
//
// NOTES
// この関数はModMessageStreamWrapperから呼ばれる。
//
// ARGUMENTS
// なし
//
// RETURN
// なし
//
// EXCEPTION
// ModOsMutex::unlockと同じ

void
ModMessageStream::unlock()
{
//	if (ModCommonMutex::getLockCount() == 1) ModFlush(*this);
	ModCommonMutex::unlock();
}

// ModMessageSelectionのstatic変数の初期化
int ModMessageSelection::initializeCount = 0;
ModMessageStream* ModMessageSelection::coutMessage = 0;
ModMessageStream* ModMessageSelection::cerrMessage = 0;
ModMessageStream* ModMessageSelection::logMessage = 0;
ModMessageStream* ModMessageSelection::slogMessage = 0;
ModMessageStream ModMessageSelection::noneMessage;

ModMessageStream* ModMessageSelection::normalStream = 0;
ModMessageStream* ModMessageSelection::errorStream = 0;
ModMessageStream* ModMessageSelection::debugStream = 0;

ModBoolean ModMessageSelection::headerFlag = ModFalse;

ModParameter* ModMessageSelection::parameter = 0;

ModBoolean ModMessageSelection::initializedFlag = ModFalse;

//
// FUNCTION
// ModMessageSelection::normal -- ModMessageに対応するストリームを得る
//
// NOTES
// この関数はユーザからModMessageとして呼ばれるメッセージに対応する
// 出力先の ModMessageStream を得るために用いる。
// 通常は define マクロを通して呼ばれる。
//
// ARGUMENTS
// const char* fileName
//		メッセージ出力のコードが書かれているソースファイル名(__FILE__)
// int lineNo
//		メッセージ出力のコードが書かれている行番号(__LINE__)
//
// RETURN
// 設定にしたがって出力先に対応するModMessageStreamへの参照を返す。
// パラメータ "ModMessageOutput" または "ModMessageOutputNormal" の値により
// 出力先は以下のようになる。両者が同時に定義されている時は後者が優先。
//
// 		未設定: syslog、eventlog(DEBUG中はstdout)
//			 1: stdout
//			 2: stderr
//			 3: 出力しない
//			 4: syslog、eventlog
//
// EXCEPTIONS
// なし
//
ModMessageStreamWrapper
ModMessageSelection::normal(const char* fileName, int lineNo)
{
	return ModMessageSelection::setupStream(ModMessageSelection::normalStream,
											fileName, lineNo,
											ModMessageStream::levelInformation);
}

//
// FUNCTION
// ModMessageSelection::error -- ModErrorMessageに対応するストリームを得る
//
// NOTES
// この関数はユーザからModErrorMessageとして呼ばれるメッセージに対応する
// 出力先の ModMessageStream を得るために用いる。
// 通常は define マクロを通して呼ばれる。
//
// ARGUMENTS
// const char* fileName
//		メッセージ出力のコードが書かれているソースファイル名(__FILE__)
// int lineNo
//		メッセージ出力のコードが書かれている行番号(__LINE__)
//
// RETURN
// 設定にしたがって出力先に対応するModMessageStreamへの参照を返す。
// パラメータ "ModMessageOutput" または "ModMessageOutputError" の値により
// 出力先は以下のようになる。両者が同時に定義されている時は後者が優先。
//
// 		未設定: syslog、eventlog(DEBUG中はstderr)
//			 1: stdout
//			 2: stderr
//			 3: 出力しない
//			 4: syslog、eventlog
//
// EXCEPTIONS
// なし
//
ModMessageStreamWrapper
ModMessageSelection::error(const char* fileName, int lineNo)
{
	return ModMessageSelection::setupStream(ModMessageSelection::errorStream,
											fileName, lineNo,
											ModMessageStream::levelError);
}

//
// FUNCTION
// ModMessageSelection::debug -- ModDebugMessageに対応するストリームを得る
//
// NOTES
// この関数はユーザからModDebugMessageとして呼ばれるメッセージに対応する
// 出力先の ModMessageStream を得るために用いる。
// 通常は define マクロを通して呼ばれる。
//
// ARGUMENTS
// const char* fileName
//		メッセージ出力のコードが書かれているソースファイル名(__FILE__)
// int lineNo
//		メッセージ出力のコードが書かれている行番号(__LINE__)
//
// RETURN
// 設定にしたがって出力先に対応するModMessageStreamへの参照を返す。
// パラメータ "ModMessageOutput" または "ModMessageOutputDebug" の値により
// 出力先は以下のようになる。両者が同時に定義されている時は後者が優先。
//
// 		未設定: 出力しない(DEBUG中はstderr)
//			 1: stdout
//			 2: stderr
//			 3: 出力しない
//			 4: syslog、eventlog
//
// EXCEPTIONS
// なし
//
ModMessageStreamWrapper
ModMessageSelection::debug(const char* fileName, int lineNo)
{
	return ModMessageSelection::setupStream(ModMessageSelection::debugStream,
											fileName, lineNo,
											ModMessageStream::levelDebug);
}

//
// FUNCTION
// ModMessageSelection::isInitialized -- メッセージが使用可能か調べる
//
// NOTES
// この関数は ModMessage が使用可能かを示すフラグを得るのに用いる。
// ただし、この関数の返り値が意味を持つのは ModMessageSelection::setStream
// から呼び出される関数で、しかも内部で ModMessage を使用しているもののみ。
//
// ARGUMENTS
// なし
//
// RETURN
// ModMessage が使用可能なら ModTrue を、使用可能でないなら ModFalse を返す。
//
// EXCEPTIONS
// なし
//
ModBoolean
ModMessageSelection::isInitialized()
{
	return ModMessageSelection::initializedFlag;
}

//
// FUNCTION
// ModMessageSelection::initialize -- メッセージの初期化
//
// NOTES
// この関数は ModCommonInitialize から呼ばれる。
//
// ARGUMENTS
// なし
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
void
ModMessageSelection::initialize()
{
	int defaultParameter;

	// パラメータの初期化
	ModMessageSelection::parameter = new ModParameter(ModFalse);

	//
	// ModMessageStream の static 変数の初期化
	//		・コマンド名に"mod"を入れる。
	//		  -> setCommandName が呼ばれるまでの暫定名
	//		・SystemLog以外への出力に用いるホスト名をセットする
	//		・ヘッダ部を出力するかを示すフラグをセットする
	//		・出力先ごとの MessageStream のインスタンスを作る
	//
	if (ModMessageStream::commandNameIsSet == ModFalse) {
		ModCharTrait::copy(ModMessageStream::commandName, "mod");
	}

	// ホスト名を初期化する
	// ホスト名の取得は、必要になった時に行う
	ModMessageStream::hostName[0] = 0;

	// ヘッダ部を出力するかを示すフラグをパラメータからとる
	ModMessageSelection::headerFlag =
		ModMessageSelection::parameter->getBoolean("ModMessageHeader");

	// 出力先を示すパラメータを読み込む
	defaultParameter = ModMessageSelection::parameter->getInteger("ModMessageOutput");

	ModMessageSelection::normalStream =
		ModMessageSelection::setStream(
#ifdef DEBUG
			ModMessageStream::toStandardOut,
#else
			ModMessageStream::toSystemLog,
#endif
			defaultParameter,
			"ModMessageOutputNormal");

	ModMessageSelection::errorStream =
		ModMessageSelection::setStream(
#ifdef DEBUG
			ModMessageStream::toStandardError,
#else
			ModMessageStream::toSystemLog,
#endif
			defaultParameter,
			"ModMessageOutputError");

	ModMessageSelection::debugStream =
		ModMessageSelection::setStream(
#ifdef DEBUG
			ModMessageStream::toStandardError,
#else
			ModMessageStream::toNone,
#endif
			defaultParameter,
			"ModMessageOutputDebug");

	ModManipulatorSelection::initialize();

	ModMessageSelection::initializedFlag = ModTrue;
}

//
// FUNCTION
// ModMessageSelection::terminate -- メッセージの後処理
//
// NOTES
// この関数は ModCommonInitialize から呼ばれる。
//
// ARGUMENTS
// なし
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
void
ModMessageSelection::terminate()
{
	ModManipulatorSelection::terminate();

	if (ModMessageSelection::coutMessage)
		delete ModMessageSelection::coutMessage,
			ModMessageSelection::coutMessage = 0;
	if (ModMessageSelection::cerrMessage)
		delete ModMessageSelection::cerrMessage,
			ModMessageSelection::cerrMessage = 0;
	if (ModMessageSelection::logMessage)
		delete ModMessageSelection::logMessage,
			ModMessageSelection::logMessage = 0;
	if (ModMessageSelection::slogMessage)
		delete ModMessageSelection::slogMessage,
			ModMessageSelection::slogMessage = 0;

	// noneMessage は exited のあとに呼ばれることを考慮して残す

	// 振り分け先をクリアしておく
	ModMessageSelection::normalStream = 0;
	ModMessageSelection::errorStream = 0;
	ModMessageSelection::debugStream = 0;

	if( ModMessageSelection::parameter ){
		delete ModMessageSelection::parameter;
		ModMessageSelection::parameter = 0;
	}

	ModMessageSelection::initializedFlag = ModFalse;
}

//
// FUNCTION private
// ModMessageSelection::setStream -- パラメータからストリームを選択する
//
// NOTES
// この関数はパラメータを読み込み、その結果によってストリームを
// 選択するのに用いる。
//
// ARGUMENTS
// ModMessageStream::MessageType type
//		デフォルトで用いるメッセージストリームを表す type
// int defaultParameter
//		デフォルトで用いるパラメータ値
// const char* parameterName
//		選択に用いるパラメータ名
//
// RETURN
// パラメータの値にしたがって該当するストリームを、必要なら new して
// 返す。
//
// EXCEPTIONS
// なし
//
ModMessageStream*
ModMessageSelection::setStream(ModMessageStream::MessageType defaultType,
							   int defaultParameter, const char* parameterName)
{
	int	output;
	if (ModMessageSelection::parameter->
		getInteger(output, parameterName) == ModFalse)
		output = defaultParameter;

	ModMessageStream::MessageType type;
	switch (output) {
	case 0:
	default:
		type = defaultType;							break;
	case 1:
		type = ModMessageStream::toStandardOut;		break;
	case 2:
		type = ModMessageStream::toStandardError;	break;
	case 3:
		type = ModMessageStream::toNone;			break;
	case 4:
	case 5:
		type = ModMessageStream::toSystemLog;		break;
	}

	ModMessageStream**	p;
	switch (type) {
	case ModMessageStream::toNone:
		return &ModMessageSelection::noneMessage;
	default:
		return 0;

	case ModMessageStream::toStandardOut:
		p = &ModMessageSelection::coutMessage;	break;
	case ModMessageStream::toStandardError:
		p = &ModMessageSelection::cerrMessage;	break;
	case ModMessageStream::toSystemLog:
		p = &ModMessageSelection::logMessage;	break;
	}

	if (*p == 0)
		*p = new ModMessageStream(type);
	return *p;
}

//
// FUNCTION private
// ModMessageSelection::setupStream -- normalなどの下請け
//
// NOTES
// この関数は引数のストリームに対して必要な初期化を施して
// 返すために用いる。
//
// ARGUMENTS
// ModMessageStream*& streamPointer
//		初期化して返すストリームへのポインタ
// const char* fileName
//		メッセージ出力のコードが書かれているソースファイル名(__FILE__)
// int lineNo
//		メッセージ出力のコードが書かれている行番号(__LINE__)
// ModMessageStream::MessageLevel level
//		メッセージの種別を表す定数
//
// RETURN
// 初期化された引数のポインタが指す実体への参照が返される。
//
// EXCEPTIONS
// なし
//
ModMessageStreamWrapper
ModMessageSelection::setupStream(ModMessageStream*& streamPointer,
								 const char* fileName, int lineNo,
								 ModMessageStream::MessageLevel level)
{
	ModCommonInitialize::checkAndInitialize();

	if (streamPointer == 0) {
		// 初期化失敗、またはプログラムがすでに終了している
		streamPointer = &ModMessageSelection::noneMessage;
		return *streamPointer;
	}

	ModMessageStreamWrapper result(*streamPointer);

	// flush されているか、スレッドが変わっていたら初期化し直す
	if (streamPointer->currentStatus == statusFlushed ||
		(streamPointer->threadId != ModThisThread::self())) {
		streamPointer->initializeMessage(fileName, lineNo, level);
		// 初期化したてであることを示す
		streamPointer->currentStatus = statusFirstOutput;
	}

	return result;
}

// ModManipulatorSelectionのstatic変数の初期化
ModManipulator* ModManipulatorSelection::setWidth = 0;
ModManipulator* ModManipulatorSelection::setFill = 0;

//
// FUNCTION
// ModManipulatorSelection::initialize -- 初期化
//
// NOTES
// ModManipulatorSelectionの初期化を行なう
//
// ARGUMENTS
// なし
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
void
ModManipulatorSelection::initialize()
{
	ModManipulatorSelection::setWidth =
		new ModManipulator(ModSetWidthInternal, 0);
	ModManipulatorSelection::setFill =
		new ModManipulator(ModSetFillInternal, 0);
}

//
// FUNCTION
// ModManipulatorSelection::terminate -- 終了処理
//
// NOTES
// ModManipulatorSelectionの終了処理
//
// ARGUMENTS
// なし
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
void
ModManipulatorSelection::terminate()
{
	delete ModManipulatorSelection::setWidth,
		ModManipulatorSelection::setWidth = 0;
	delete ModManipulatorSelection::setFill,
		ModManipulatorSelection::setFill = 0;
}

//
// Copyright (c) 1996, 1997, 2010, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
