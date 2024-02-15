// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-char
// vi:set ts=4 sw=4:	
//
// ModParameter.cpp -- パラメータ関連のメンバ定義
// 
// Copyright (c) 1997, 1998, 1999, 2000, 2002, 2003, 2004, 2005, 2006, 2023 Ricoh Company, Ltd.
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


extern "C" {
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
}

#include "ModConfig.h"
#include "ModCharTrait.h"
#include "ModParameter.h"
#include "ModParameterSource.h"
#include "ModAutoPointer.h"
#include "ModAutoMutex.h"
#include "ModCommonMutex.h"
#include "ModMultiByteString.h"
#include "ModUnicodeString.h"
#include "ModKanjiCode.h"
#include "ModOsDriver.h"

#define SIGNED64MODIFIER   "%lld"
#define UNSIGNED64MODIFIER "%llu"


//////////////////////////////////////////////////
// ModParameter::Object
//////////////////////////////////////////////////

//	FUNCTION static
//	_SafeCopy -- 長さをチェックするcopy
//
//	NOTES
//		本ファイル内でのみ使用。
//
//	ARGUMENTS
//		char* dst 
//			[out]コピー先の文字列
//		const char* src
//			[in]コピー元の文字列
//      ModSize length
//			[in]コピーできる文字列の長さの上限
//
//	RETURN
//		なし
//
static void _SafeCopy(char* dst, const char* src, ModSize length) {
	if (ModCharTrait::length(src) < length)
		ModCharTrait::copy(dst, src);
}

//	FUNCTION static
//	_SafeAppend -- 長さをチェックするappend
//
//	NOTES
//		本ファイル内でのみ使用。
//
//	ARGUMENTS
//		char* dst 
//			[out]追加先の文字列
//		const char* src
//			[in]追加元の文字列
//      ModSize length
//			[in]追加先の文字列で可能な長さの上限
//
//	RETURN
//		なし
//
static void _SafeAppend(char* dst, const char* src, ModSize length) {
	if (ModCharTrait::length(dst) + ModCharTrait::length(src) < length)
		ModCharTrait::append(dst, src);
}

//	VARIABLE static
//	ModParameter::Object::_allObjects -- ModParameter::Objectのリスト
//
//	NOTES
//		
//static
ModParameter::Object*
ModParameter::Object::_allObjects = 0;

//	FUNCTION public
//	ModParameter::Object::ModParameter::Object --
//		パラメーターを扱うクラスのコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		ModBoolean			doPreLoad
//			ModTrue または指定されないとき(Unix版)
//				パラメーターの値を初期化時にすべて先読みする
//			ModFalse(Unix版)
//				パラメーターの値を初期化時にすべて先読みせずに、
//				取得要求ごとにパラメーターの値を読み出す
//		const char*	environmentName
//			(その2のみ)参照するファイルを記した環境変数の名前
//		const ModParameterSource& source
//			(その3のみ)初期化に用いるModParameterSource
//
//	RETURN
//		なし

// その1 (デフォルトコンストラクタ)
ModParameter::Object::Object(ModBoolean doPreLoad)
	: _lastIndex(ModParameterFileNameListSizeMax-1),
	  _refCount(0)
	, _map(0), _doPreLoad(doPreLoad)
{
	initialize(ModParameterSource());
}

// その2
ModParameter::Object::Object(const char* environmentName, ModBoolean doPreLoad)
	: _lastIndex(ModParameterFileNameListSizeMax-1),
	  _refCount(0)
	, _map(0), _doPreLoad(doPreLoad)
{
	initialize(ModParameterSource(0, environmentName));
}

// その3
ModParameter::Object::Object(const ModParameterSource& source,
						   ModBoolean doPreLoad)
	: _lastIndex(ModParameterFileNameListSizeMax-1),
	  _refCount(0)
 	, _map(0), _doPreLoad(doPreLoad)
{
	initialize(source);
}

//	FUNCTION private
//	ModParameter::Object::initialize -- パラメータークラスを初期化する
//
//	NOTES
//
//	ARGUMENTS
//		ModParameterSource&	source
//			パラメーターの取得元を表すクラス
//
//	RETURN
//		なし
//
void
ModParameter::Object::initialize(const ModParameterSource& source)
{
	ModCommonInitialize::checkAndInitialize();

	try {
		ModAutoMutex<ModOsMutex> m(ModCommonMutex::getMutex());
		m.lock();

		//_fileNameListには、以下のものが順に設定される。
		//
		// A 「システムパラメータ」のパス+Suffix。
		// これは以下の優先順位で決定される。なお、Suffixについては後述。
		//	1. _systemEnvironmentValue(ModParameterSourceの第5引数)が
		//     空でなければ、その値。
		//	2. _systemEnvironmentValueが空で、_systemEnvironmentName
		//     (ModParameterSourceの第4引数)が空でなく、
		//	   かつ_systemEnvironmentNameの指す環境変数が存在すれば、
		//     その環境変数の値。
		//	3. _systemEnvironmentValueも_systemEnvironmentNameも空で、
		//	   かつ環境変数ModSystemParameterPathが存在すれば、
		//     ModSystemParameterPathの値。
		//	4. 以上のどれにも当てはまらなければ、"system.prm"。
		// 1.-3.の場合、設定値は「:」ごとにパーズされない。
		// また、上記で決定されるパスが存在している必要はない。
		//
		// B "Default"+Suffixが存在していれば、"default.prm"+Suffix。
		//
		// C _environmentValue(ModParameterSourceの第3引数)が空でなければ、
		//   その値を「:」でパーズした各要素+Suffixのうち、実在するものの集合。
		//
		// D _environmentName(ModParameterSourceの第2引数)が空でなく、かつ
		//   _environmentNameの指す環境変数が存在すれば、その環境変数の
		//   値を「:」でパーズした各要素+Suffixのうち、実在するものの集合。
		//
		// E _environmentNameが空で、かつ環境変数ModParameterPathが
		//   存在しなければ、ModParameterPathの値を「:」でパーズした
		//   各要素+Suffixのうち、実在するものの集合。

		// _fileNameListの要素が重複している場合、
		// 最初にあるもの以外の重複要素は捨てられる。
		// (以前のバージョンでは最後にあるもの以外の重複要素を捨てていた)

		// 上記におけるSuffixは、以下の要領で決定する。
		// 1. _environmentValue(ModParameterSourceの第3引数)が空でなく、
		//    かつその値の後ろに"Suffix"を加えた名前の環境変数が存在すれば、
		//    その環境変数の値。
		// 2. _environmentValueが空で、かつ環境変数"ModParameterPathSuffix"が
		//    存在すれば、ModParameterPathSuffixの値。
		// 3. 以上のどれにも当てはまらなければ、空文字列。

		// パスの存非は、parentPath以下の、Suffixのついたパス名によって
		// 判断する。(以前のバージョンではSuffixのないパス名で判断していた)
		// ここでparentPathは、以下の要領で決定する。
		// 1. _parentPath(ModParameterSourceの第1引数)が空でなければ、その値。
		// 2. _parentPathが空ならば、カレントディレクトリ。

		// ファイル名リストをnullでクリアする
		int i;
		for (i = 0; i < ModParameterFileNameListSizeMax; ++i)
			_fileNameList[i] = 0;

		// パス名および環境変数のセット
		//- これではv1.5までとの整合が取れないので修正
		const char* parentPath = source._parentPath 
			? source._parentPath : Object::_parentPath;
		const char* environmentName = source._environmentName
			? source._environmentName : Object::_environmentName;
		const char* systemEnvironmentName = source._systemEnvironmentName
			? source._systemEnvironmentName
			  : Object::_systemEnvironmentName;
		
		// suffixの取得・設定
		char suffixValue[ModParameterEnvironmentValueSizeMax];
		{
			const int bufLength = ModParameterEnvironmentNameSizeMax
				+ sizeof("Suffix");
			char buf[bufLength];
			_SafeCopy(buf, environmentName, bufLength);
			_SafeAppend(buf, "Suffix", bufLength);

			const char* p = ::getenv(buf);
			if (p && *p != ModCharTrait::null())
				_SafeCopy(suffixValue, p, ModParameterEnvironmentValueSizeMax);
			else
				suffixValue[0] = ModCharTrait::null();
		}

		// システムパラメーターファイルのパス名を
		// ファイル名リストの先頭に設定しておく
		const char* p = source._systemEnvironmentValue;
		if (!p) {
			p = ::getenv(systemEnvironmentName);
			if (!p || *p == ModCharTrait::null())
				p = _systemFileName;
		}
		if (ModCharTrait::compare(p, _temporarySystemFileName) != 0)
			setFullPathName(0, parentPath, p, suffixValue);

		int index;
		// 存在するファイルか調べた上でデフォルトパラメーターファイルの
		// パス名を設定する
		setFullPathName(1, parentPath, _defaultFileName, suffixValue);
		// 存在を確認
		if (checkFile(_fileNameList[1])) {
		  index = 2;
		} else {
			delete [] _fileNameList[1], _fileNameList[1] = 0;
		  index = 1;
		}
	
		char fileValue[ModParameterEnvironmentValueSizeMax];
		// source._environmentValueがあればfileValueにセットする
		if (source._environmentValue) {
			_SafeCopy(fileValue, source._environmentValue,
					  ModParameterEnvironmentValueSizeMax);
		} else {
			fileValue[0] = ModCharTrait::null();
		}
		// 続いて、パラメーターファイルの
		// パス名リストを環境変数から取得する
		{   
			const char* p = ::getenv(environmentName);
			if (p && *p != ModCharTrait::null()) {
				if (fileValue && *fileValue != ModCharTrait::null()) {
					_SafeAppend(fileValue, ":",
								ModParameterEnvironmentValueSizeMax);
					_SafeAppend(fileValue, p,
								ModParameterEnvironmentValueSizeMax);
				} else {
					_SafeCopy(fileValue, p,
							  ModParameterEnvironmentValueSizeMax);
				}
			}
		}

		// 取得したパラメーターファイルのパス名リストを解析し、
		// ':'で区切られた各々の値をファイル名リストに設定する
		char*	head;
		ModMultiByteString* utf_p;
		ModAutoPointer<ModMultiByteString> utf;
		char*	tail = 0;
		

		// 環境変数値の日本語コードを求める
		const ModKanjiCode::KanjiCodeType
			encodingType = ModOs::Process::getEncodingType();

		// encodingによる文字列の置き換え
		if (encodingType == ModKanjiCode::utf8 ||
			encodingType == ModKanjiCode::unknown) {
			// 環境変数値の日本語コードが UTF8 や ASCII である場合
			head = fileValue;
		} else {
			utf_p = new ModMultiByteString
				(fileValue, encodingType, ModKanjiCode::utf8, ModTrue);
			head = const_cast<char*>(utf_p->get());
			utf = utf_p;
		}

		do {
			; ModAssert(head);
			tail = ModCharTrait::find(head, ':');
			if (tail) *tail = ModCharTrait::null();

			if (*head != ModCharTrait::null()) {
				setFullPathName(index, parentPath, head, suffixValue);

				char* newItem = _fileNameList[index];
				if (!checkFile(newItem)) {
					// 実在しないファイルならば消す
					delete[] newItem, newItem = 0;
				} else {
					// 実在するならば、設定済のファイル名との重複を調べる
					for (int j = 0; j < index; ++j) {
						if (ModCharTrait::compare(_fileNameList[j], newItem)
							== 0) {
							// 重複があれば消す
							delete[] newItem, newItem = 0;
							break;
						}
					}
					if (newItem) index++;
				}

			}
			head = tail + 1;
		} while (tail != NULL && index < ModParameterFileNameListSizeMax);

		// 最終添字を覚える
		_lastIndex = index-1;
#ifdef DEBUG
		for(i=0; i<=_lastIndex; i++) {
			if (_fileNameList[i] != NULL)
				_SafeModDebugMessage << "[" << i << "] " << _fileNameList[i]
									 << ModEndl;
		}
		if (_lastIndex == 0) {
			// 指定されていたパラメーターファイルで、
			// 存在するものはひとつもなかった
			_SafeModDebugMessage << "No parameter source is available."
								 << ModEndl;
		}		
#endif

#if MOD_CONF_REGISTRY == 0
		// 初期化されていなければ先読みできない
		if (ModMessageSelection::isInitialized() == ModFalse)
			_doPreLoad = ModFalse;
		if (_doPreLoad) preLoad();
#endif

		// 同一性判定のためにsourceの情報をセットしておく
		_sourceInfo.set(source);

	} catch (ModException& exception) {
		terminate();
		ModRethrow(exception);

	}
#ifndef NO_CATCH_ALL
	catch (...) {
		terminate();
		ModUnexpectedThrow(ModModuleStandard);
	}
#endif
}

//	FUNCTION private
//	ModParameter::Object::terminate -- パラメータークラスの後始末を行う
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
void
ModParameter::Object::terminate()
{
	ModAutoMutex<ModOsMutex> m(ModCommonMutex::getMutex());
	m.lock();

	// ファイル名リストをクリアする
	for (int i = 0; i <= _lastIndex; ++i) 
		delete[] _fileNameList[i], _fileNameList[i] = 0;

	if (_doPreLoad) {
		// マップに格納されているパラメーター値のうち、
		// 文字列を記録している領域 pString の破棄を行う
		Map::Iterator iterator(_map->begin());
		const Map::Iterator& end = _map->end();
		for (; iterator != end; ++iterator)
			(*iterator).second.freePString();
		// マップ自体を破棄する
		delete _map, _map = 0;
	}
}

//	FUNCTION private
//	ModParameter::Object::setFullPathName -- パス名を組み立てる
//
//	NOTES
//
//	ARGUMENTS
//		const int index
//			結果を格納する_fileNameList[]の添字
//		const char* parentPath
//			親となるディレクトリ
//		const char* name
//			参照先のファイル名
//		const char* suffixValue
//			付加するサフィックス
//
//	RETURN
//		なし
//
void
ModParameter::Object::setFullPathName(const int index, const char* parentPath,
							  const char* name, const char* suffixValue)
{   
	// 名前が長すぎるので縮めてしまう
	static const int length = ModParameterEnvironmentValueSizeMax;
	static const char ps[2] = {
		ModOsDriver::File::getPathSeparator(), ModCharTrait::null()
	};

	// 領域を割り当てる
	char* fileName = _fileNameList[index] = new char[length];

	if (name[0] != ModOsDriver::File::getPathSeparator()) {
		_SafeCopy(fileName, parentPath, length);
		_SafeAppend(fileName, ps, length);
	} else {
		fileName[0] = ModCharTrait::null();
	}
	_SafeAppend(fileName, name, length);
	if (suffixValue[0] != ModCharTrait::null())
	    _SafeAppend(fileName, suffixValue, length);
}

//	FUNCTION private
//	ModParameter::Object::getValue -- 指定されたキーのパラメーター値を得る
//
//	NOTES
//
//	ARGUMENTS
//		ModParameter::ParameterValue&	pv
//			得られた値を格納するパラメーター値クラス
//		const char*				key
//			null 以外の値
//				値を得たいパラメーターのキー名が
//				格納されている領域の先頭アドレス
//			null または指定されないとき
//				なにもせずに ModFalse を返す
//
//	RETURN
//		ModTrue
//			指定された名前のパラメーターの値が得られた
//		ModFalse
//			指定された名前のパラメーターは見つからなかった
//
ModBoolean
ModParameter::Object::getValue(ModParameter::ParameterValue& pv, const char* key)
{
	;ModAssert(key);

	if (_doPreLoad) return getFromMap(pv, key);
	// ファイルリストの先頭からひとつずつファイルを読み込む
	// (システムパラメータ優先)
	for (int index = 0; index <= _lastIndex; index++)
		if (readFile(pv, _fileNameList[index], key) == ModTrue)
			return ModTrue;
	return ModFalse;
}

//	FUNCTION private
//	ModParameter::Object::setValue -- 指定されたキーのパラメーター値を記録する
//
//	NOTES
//
//	ARGUMENTS
//		ModParameter::ParameterValue&	pv
//			記録する値を表すパラメーター値クラス
//		const char*				key
//			null 以外の値
//				値を記録したいパラメーターのキー名が
//				格納されている領域の先頭アドレス
//			null, または指定されないとき
//				なにもせずに ModFalse を返す
//
//	RETURN
//		ModTrue
//			指定された名前のパラメーター値として記録できた	
//		ModFalse
//			指定された名前のパラメーター値として記録できなかった
//
ModBoolean
ModParameter::Object::setValue(ModParameter::ParameterValue& pv, const char* key)
{
	;ModAssert(key != 0);

	if ((_fileNameList[0] == 0
		 && !_doPreLoad
		)
		|| (_fileNameList[0] != 0
			&& writeFile(pv, _fileNameList[0], key) == ModFalse))
		return ModFalse;
	if (_doPreLoad) setToMap(pv, key);
	return ModTrue;
}

// MACRO
// _ModParameter_GetValueByReturn -- パラメータを取得するためのマクロ
//
// NOTES
//	返り値にパラメータ値を返す関数を定義するためのマクロ。
//
// ARGUMENTS (マクロの)
//	TYPE
//		定義するメソッドの名前から「get」を除いたもの
//	T
//		返り値の型
//	DEFAULT
//		パラメータ値が得られなかったときに返すデフォルトの値
//
// ARGUMENTS (関数定義内)
//	const char* key
//		パラメータのキー値
//
// RETURN (生成する関数の)
//	対応するパラメータが得られた場合、その値を返す。
//	得られなかった場合、DEFAULTで定められた値を返す。
//
// EXCEPTIONS
// なし
//
// その他
// MT-safe
//
#define _ModParameter_GetValueByReturn(TYPE, T, DEFAULT)\
T ModParameter::Object::get##TYPE (const char* key) {\
	T result = DEFAULT;\
	(void) get##TYPE (result, key);\
	return result;\
}
	
// 各型に関するgetXXXの定義
_ModParameter_GetValueByReturn(String, ModCharString, "");
_ModParameter_GetValueByReturn(WideString, ModWideString, "");
_ModParameter_GetValueByReturn(UnicodeString, ModUnicodeString, "");

_ModParameter_GetValueByReturn(Integer, int, 0);
_ModParameter_GetValueByReturn(UnsignedInteger, unsigned int, 0);
_ModParameter_GetValueByReturn(UnsignedLong, unsigned long, 0);
_ModParameter_GetValueByReturn(ModSize, ModSize, 0);
_ModParameter_GetValueByReturn(ModFileSize, ModFileSize, 0);

_ModParameter_GetValueByReturn(Boolean, ModBoolean, ModFalse);

// MACRO
// _ModParameter_GetValueByArgument -- パラメータを取得するマクロ
//
// NOTES
//	パラメータの値を得るのに用いる関数を定義する。
//
// ARGUMENTS (マクロの)
//	TYPE
//		定義する関数名から「get」を除いた部分
//	T
//		結果の型
// ARGUMENTS (関数定義内)
//	T result
//		[out]パラメータ値を入れる変数への参照あるいはポインタ
//	const char* key
//		[in]パラメータのキー値
//
// RETURN (生成する関数の)
//	対応するパラメータが得られた場合、value に値を入れ、ModTrueを返す。
//	得られなかった場合 value は変えずに ModFalse を返す。
//
// EXCEPTIONS
// なし
//
// その他
// MT-safe
//
#define _ModParameter_GetValueByArgument(TYPE, T)\
ModBoolean ModParameter::Object::get##TYPE(T result, const char* key)\
{\
	ModAutoMutex<ModOsMutex> m(ModCommonMutex::getMutex());\
	m.lock();\
	ModParameter::ParameterValue pv(ModFalse);\
	if (getValue(pv, key) == ModFalse) return ModFalse;\
	pv.getParameterValue(result);\
	return ModTrue;\
}

// 各型に関するgetXXXの定義
_ModParameter_GetValueByArgument(String, char*);
_ModParameter_GetValueByArgument(WideString, ModWideString&);
_ModParameter_GetValueByArgument(UnicodeString, ModUnicodeString&);

_ModParameter_GetValueByArgument(Integer, int&);
_ModParameter_GetValueByArgument(UnsignedInteger, unsigned int&);
_ModParameter_GetValueByArgument(UnsignedLong, unsigned long&);
_ModParameter_GetValueByArgument(ModSize, ModSize&);
_ModParameter_GetValueByArgument(ModFileSize, ModFileSize&);

_ModParameter_GetValueByArgument(Boolean, ModBoolean&);

// 以下は定義がマクロでは片付かないもの

//	FUNCTION public
//	ModParameter::Object::getString -- CharString型のパラメータ値を取得する
//
//	NOTES
//
//	ARGUMENTS
//		ModCharString& result
//		[out]パラメータ値を入れる変数への参照
//	const char* key
//		[in]パラメータのキー値
//
//	RETURN
//		ModBoolean
//			ModTrue: 対応するパラメータが得られた
//			ModFalse: 対応するパラメータが得られなかった
//
ModBoolean ModParameter::Object::getString(ModCharString& result, const char* key)
{
	ModAutoMutex<ModOsMutex> m(ModCommonMutex::getMutex());
	m.lock();
	ParameterValue pv;
	if (getValue(pv, key) == ModFalse) return ModFalse;

	char tmp[ModParameterFileLineSizeMax];
	pv.getParameterValue(tmp);
	result = tmp;
	return ModTrue;
}

// MACRO
// _ModParameter_SetValue -- パラメータ値をセットする関数を定義するマクロ
//
// NOTES
//  このマクロが定義する関数でセットされるパラメータ値は、
//  環境変数 ModSystemParameterPathで与えられるファイル
//  にセットされ、他のパラメータと同様に getXXX で得ることができる。
//
// ARGUMENTS (マクロの)
//	TYPE
//		定義する関数名から「set」を除いた部分
//	T
//		結果の型
// ARGUMENTS (関数定義内)
//	const char* key
//		パラメータのキー値
//	const T value
//		セットするパラメータの値
//
// RETURN (生成する関数の)
// 対応するパラメータが正しくセットできた場合、ModTrue を返す。
// 失敗した場合 ModFalse を返す。
//
// その他
// MT-safe
//
#define _ModParameter_SetValue(TYPE, T)\
ModBoolean \
ModParameter::Object::set##TYPE (const char* key, const T value)\
{\
	ModAutoMutex<ModOsMutex> m(ModCommonMutex::getMutex());\
	m.lock();\
	ModParameter::ParameterValue pv;\
	pv.setParameterValue(value);\
	return setValue(pv, key);\
}

// 各型に関するsetXXXの定義
_ModParameter_SetValue(String, char*);

_ModParameter_SetValue(Integer, int);
_ModParameter_SetValue(UnsignedInteger, unsigned int);
_ModParameter_SetValue(UnsignedLong, unsigned long);

_ModParameter_SetValue(Boolean, ModBoolean);

// 以下は定義がマクロでは片付かないもの


//	FUNCTION public
//	ModParameter::Object::setString -- CharString型のパラメータ値を設定する
//
//	NOTES
//
//	ARGUMENTS
//	const char* key
//		パラメータのキー値
//	ModCharString& value
//		設定するパラメータ値
//
//	RETURN
//		ModBoolean
//			ModTrue: パラメータがセットできた
//			ModFalse: パラメータがセットできなかった
//
ModBoolean
ModParameter::Object::setString(const char* key, const ModCharString& value)
{
	return setString(key, value.getString());
}

//	FUNCTION public
//	ModParameter::Object::setWideString -- WideString型のパラメータ値を設定する(1)
//
//	NOTES
//
//	ARGUMENTS
//	const char* key
//		パラメータのキー値
//	ModWideString& value
//		設定するパラメータ値
//
//	RETURN
//		ModBoolean
//			ModTrue: パラメータがセットできた
//			ModFalse: パラメータがセットできなかった
//
ModBoolean
ModParameter::Object::setWideString(const char* key, const ModWideString& value)
{
	ModWideString wtmp(value);
	ModUnicodeString tmp(wtmp.getString(ModKanjiCode::literalCode),
						 ModKanjiCode::literalCode);
	return setUnicodeString(key, tmp);
}

//	FUNCTION public
//	ModParameter::Object::setWideString -- WideString型のパラメータ値を設定する(2)
//
//	NOTES
//
//	ARGUMENTS
//	const char* key
//		パラメータのキー値
//	ModWideChar* value
//		設定するパラメータ値へのポインタ
//
//	RETURN
//		ModBoolean
//			ModTrue: パラメータがセットできた
//			ModFalse: パラメータがセットできなかった
//
ModBoolean
ModParameter::Object::setWideString(const char* key, const ModWideChar* value)
{
	ModWideString wtmp(value);
	ModUnicodeString tmp(wtmp.getString(ModKanjiCode::literalCode),
						 ModKanjiCode::literalCode);
	return setUnicodeString(key, tmp);
}

//	FUNCTION public
//	ModParameter::Object::setUnicodeString -- UnicodeString型のパラメータ値を設定する(2)
//
//	NOTES
//
//	ARGUMENTS
//	const char* key
//		パラメータのキー値
//	ModWideChar* value
//		設定するパラメータ値へのポインタ
//
//	RETURN
//		ModBoolean
//			ModTrue: パラメータがセットできた
//			ModFalse: パラメータがセットできなかった
//
ModBoolean
ModParameter::Object::setUnicodeString(const char* key, const ModUnicodeString& string)
{
	ModUnicodeString tmp(string);
	return setString(key, tmp.getString(_codingType));
}

// ===== ここよりParameterValueの関数群 =====

//	FUNCTION public
//	ModParameter::ParameterValue::ParameterValue
//			-- パラメータ値を格納するクラスのコンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	ModBoolean _autoDelete
//		デストラクト時にvalue.pStringを自動的に解放するか否か
//		デフォルトはModTrue(自動的に解放する)
//
//	RETURN
//		なし
//
ModParameter::ParameterValue::ParameterValue(ModBoolean _autoDelete) :
#if MOD_CONF_REGISTRY == 0
  fileNo(-1), 
#endif
  type(typeNone), autoDelete(_autoDelete)
{
    // Messageがavailable == ModStringもavailable == 常にpStringが使用できる
	if (ModMessageSelection::isInitialized()) {
		array = 0;
	} else {
	// Messageが使えない  == ModStringも使えない  == arrayで代替する
		array = new char[ModParameterFileLineSizeMax];
	}
 	value.pString = 0;
}

//	FUNCTION public
//	ModParameter::ParameterValue::~ParameterValue
//			-- パラメータ値を格納するクラスのデストラクタ
//
//	NOTES
//		autoDeleteの値に従って、
//		value.pStringを自動的に解放するか否かが決定される。
//
//	RETURN
//		なし
//
ModParameter::ParameterValue::~ParameterValue()
{
	if (array) { delete [] array, array = 0; }
	if (autoDelete) {
		freePString();
	}
}

//	FUNCTION public
//	ModParameter::ParameterValue::setParameterValue 
//		-- ModUnicodeStirng型の値を設定する
//
//	NOTES
//
//	ARGUMENTS
//		const ModUnicodeString& mus
//			設定するModUnicodeStirng型の値
//
//	RETURN
//		なし
//
void
ModParameter::ParameterValue::setParameterValue(const ModUnicodeString& mus)
{
	ModUnicodeString tmp = mus;
	setParameterValue(tmp.getString(ModParameter::_codingType));
}

//	FUNCTION public
//	ModParameter::ParameterValue::setParameterValue 
//		-- 文字列型の値を設定する
//
//	NOTES
//
//	ARGUMENTS
//		const char* str
//			UTF-8で表記された文字列へのポインタ
//
//	RETURN
//		なし
//
void
ModParameter::ParameterValue::setParameterValue(const char* str)
{
	if (array != 0) {
		_SafeCopy(array, str, ModParameterFileLineSizeMax);
	} else {
		if (value.pString)
			*(value.pString) = str;
		else
			value.pString = new ModString(str);
	}
	type = typeString;
}

//	FUNCTION public
//	ModParameter::ParameterValue::setParameterValue 
//		-- 符号つき整数型の値を設定する
//
//	NOTES
//
//	ARGUMENTS
//		const int n
//			設定する符号つき整数
//
//	RETURN
//		なし
//
void
ModParameter::ParameterValue::setParameterValue(const int n)
{
	value.numeric = (NumericType)n;
	type = typeNumeric;
}

//	FUNCTION public
//	ModParameter::ParameterValue::setParameterValue 
//		-- 符号なし整数型の値を設定する
//
//	NOTES
//
//	ARGUMENTS
//		const unsigned int u
//			設定する符号なし整数
//
//	RETURN
//		なし
//
void
ModParameter::ParameterValue::setParameterValue(const unsigned int u)
{
	value.uNumeric = (UnsignedNumericType)u;
	type = typeUnsignedNumeric;
}

//	FUNCTION public
//	ModParameter::ParameterValue::setParameterValue 
//		-- 符号なし整数型の値を設定する
//
//	NOTES
//
//	ARGUMENTS
//		const unsigned long ul
//			設定する符号なし整数
//
//	RETURN
//		なし
//
void
ModParameter::ParameterValue::setParameterValue(const unsigned long ul)
{
	value.uNumeric = (UnsignedNumericType)ul;
	type = typeUnsignedNumeric;
}

//	FUNCTION public
//	ModParameter::ParameterValue::setParameterValue 
//		-- 符号つき64ビット整数型の値を設定する
//
//	NOTES
//
//	ARGUMENTS
//		const NumericType u
//			設定する符号つき64ビット整数
//
//	RETURN
//		なし
//
void
ModParameter::ParameterValue::setParameterValue(const NumericType n)
{
	value.numeric = n;
	type = typeNumeric;
}

//	FUNCTION public
//	ModParameter::ParameterValue::setParameterValue 
//		-- 符号なし64ビット整数型の値を設定する
//
//	NOTES
//
//	ARGUMENTS
//		const UnsignedNumericType un
//			設定する符号なし64ビット整数
//
//	RETURN
//		なし
//
void
ModParameter::ParameterValue::setParameterValue(const UnsignedNumericType un)
{
	value.uNumeric = un;
	type = typeUnsignedNumeric;
}

//	FUNCTION public
//	ModParameter::ParameterValue::setParameterValue 
//		-- 論理型の値を設定する
//
//	NOTES
//
//	ARGUMENTS
//		const ModBoolean b
//			設定する論理値
//
//	RETURN
//		なし
//
void
ModParameter::ParameterValue::setParameterValue(const ModBoolean b)
{
	value.boolean = b;
	type = typeBoolean;
}


//	FUNCTION public
//	ModParameter::ParameterValue::getParameterValue 
//		-- 文字列型の値を取得する
//
//	NOTES
//		getParameterValueでは、内部表現がいかなる型で保持されて
//		あっても、適切な変換を施して返り値とする努力をする。
//
//	ARGUMENTS
//		char* result
//			[out]取得した文字列の格納場所
//
//	RETURN
//		なし
//
void
ModParameter::ParameterValue::getParameterValue(char* result) const
{
	switch (type) {
	case typeString:
		_SafeCopy(result, toTmpString(), ModParameterFileLineSizeMax);
		break;
	case typeNumeric:
		(void) ::sprintf(result, SIGNED64MODIFIER, value.numeric);
		break;
	case typeUnsignedNumeric:
		(void) ::sprintf(result, UNSIGNED64MODIFIER, value.uNumeric);
		break;
	case typeBoolean:
		(void) ::strcpy(result, (value.boolean) ? "TRUE" : "FALSE");
		break;
	case typeNone:
	default:
		break;
	}
}

//	FUNCTION public
//	ModParameter::ParameterValue::getParameterValue 
//		-- ModUnicodeString型の値を取得する
//
//	NOTES
//		getParameterValueでは、内部表現がいかなる型で保持されて
//		あっても、適切な変換を施して返り値とする努力をする。
//
//	ARGUMENTS
//		ModUnicodeString& result
//			[out]取得した文字列の格納場所
//
//	RETURN
//		なし
//
void
ModParameter::ParameterValue::getParameterValue(ModUnicodeString& result) const
{
	char buf[ModParameterFileLineSizeMax];
	getParameterValue(buf);
	result = ModUnicodeString(buf, ModParameter::_codingType);
}

//	FUNCTION public
//	ModParameter::ParameterValue::getParameterValue 
//		-- ModWideString型の値を取得する
//
//	NOTES
//		getParameterValueでは、内部表現がいかなる型で保持されて
//		あっても、適切な変換を施して返り値とする努力をする。
//
//	ARGUMENTS
//		ModWideString& result
//			[out]取得した文字列の格納場所
//
//	RETURN
//		なし
//
void
ModParameter::ParameterValue::getParameterValue(ModWideString& result) const
{
	switch (type) {
	case typeString:
	{
		ModUnicodeString tmp(toTmpString(), ModParameter::_codingType);
		if (ModOs::Process::getEncodingType() == ModKanjiCode::unknown) {
			// unknown の場合、文字列はASCIIのはずだが、
			// これをワイド文字に変換するために仮にliteralCodeだと仮定する
			result = tmp.getString(ModKanjiCode::literalCode);
		} else {
			result.clear();
			result.append(tmp.getString(ModOs::Process::getEncodingType()),
						  0, ModOs::Process::getEncodingType());
		}
	}
		break;
	case typeNumeric:
	{
		if (value.numeric > (NumericType)ModInt32Max ||
			value.numeric < (NumericType)ModInt32Min) {
			// 32bitの範囲で表せない
			char tmp[23];
			::sprintf(tmp, SIGNED64MODIFIER, value.numeric);
			result = tmp;
		} else {
			ModCharString tmp;
			tmp.format("%d", (int) value.numeric);
			result = tmp.getString();
		}
	}
		break;
	case typeUnsignedNumeric:
	{
		if (value.uNumeric > (UnsignedNumericType) ModInt32Max) {
			// 32bitの範囲で表せない
			char tmp[23];
			::sprintf(tmp, UNSIGNED64MODIFIER, value.uNumeric);
			result = tmp;
		} else {
			ModCharString tmp;
			tmp.format("%u", (unsigned int) value.uNumeric);
			result = tmp.getString();
		}
	}
		break;
	case typeBoolean:
		result = (value.boolean) ? "TRUE" : "FALSE";
		break;
	case typeNone:
	default:
		break;
	}
}

//	MACRO
//	_ParameterValue_getParameterNumeric
//		-- ParameterValueを数値型に変換する関数を定義するマクロ
//
//	NOTES
//
//	ARGUMENTS (マクロ)
//		T
//			定義する関数の返り値の型
//		ConvType
//			文字列を数値に変換するときに使用する関数名の一部
//
//	ARGUMENTS (関数定義内)
//		T& result
//			[out]取得した数値の格納場所
//
//	RETURN
//		なし
//
#define _ParameterValue_getParameterNumeric(T, ConvType)\
void \
ModParameter::ParameterValue::getParameterValue(T& result) const\
{\
	switch (type) {\
	case typeString:\
	    result = value.pString->to##ConvType();\
		break;\
	case typeNumeric:\
		result = (T)value.numeric;\
		break;\
	case typeUnsignedNumeric:\
		result = (T)value.uNumeric;\
		break;\
	case typeBoolean:\
		result = (T)(value.boolean ? 1 : 0);\
		break;\
	case typeNone:\
	default:\
		break;\
	}\
}

// 上述のマクロを用いた関数の定義
_ParameterValue_getParameterNumeric(int, Int);
_ParameterValue_getParameterNumeric(unsigned int, ModSize);
_ParameterValue_getParameterNumeric(unsigned long, ModSize);
_ParameterValue_getParameterNumeric(ModFileSize, ModFileSize);
 
//	FUNCTION public
//	ModParameter::ParameterValue::getParameterValue 
//		-- ModBoolean型の値を取得する
//
//	NOTES
//		getParameterValueでは、内部表現がいかなる型で保持されて
//		あっても、適切な変換を施して返り値とする努力をする。
//
//	ARGUMENTS
//		ModBoolean& result
//			[out]取得した論理値の格納場所
//
//	RETURN
//		なし
//
void
ModParameter::ParameterValue::getParameterValue(ModBoolean& result) const
{
	result = ModFalse;
	switch (type) {
	case typeString:
		; ModAssert(value.pString != 0);
		{
			char topCharacter[4]; // utf8 では1文字が最大で4バイト
			if (ModOs::Process::getEncodingType() == ModKanjiCode::unknown) {
				topCharacter[0] = (*value.pString)[0];
			} else {
				ModKanjiCode::jjTransfer(topCharacter,
										 sizeof(topCharacter),
										 ModKanjiCode::utf8,
										 value.pString->getString(),
										 ModOs::Process::getEncodingType());
			}
			switch (topCharacter[0]) {
			case 'T':
			case 't':
				result = ModTrue;
			}
		}
		break;
	case typeNumeric:
		if (value.numeric) result = ModTrue;
		break;
	case typeUnsignedNumeric:
		if (value.uNumeric) result = ModTrue;
		break;
	case typeBoolean:
		result = value.boolean;	
		break;
	case typeNone:
	default:
		break;
	}
}

//	FUNCTION public
//	ModParameter::ParameterValue::getParameterValue 
//		-- 文字列表現へのポインタを取得する
//
//	NOTES
//		新規にオブジェクトを生成せずに文字列表現へのポインタを渡す。
//		ポインタの参照先は*pStringによって自動的に管理される。
//
//	ARGUMENTS
//
//	RETURN
//		const char*: 文字列表現へのポインタ
//
const char* ModParameter::ParameterValue::toTmpString() const
{
	; ModAssert(type == typeString);
	return array ? array : value.pString->getString();
}

//
// FUNCTION private
// ModParameter::Object::parseLine
//  -- パラメータファイルの一行を解析する
//
// NOTES
// パラメータファイルの一行を解析して、ParameterValue にセットする。
//
// ARGUMENTS
// ParameterValue& pv
//
// char* line
//		解析対象のバッファ
//      解析に成功した場合、キー値のみが残る
//      (実際にはキー値の直後に'\0'が置かれる)。
// const char* keyName
//		パラメータのキー値
//
// RETURN
// 解析に成功した場合は ModTrue を返し、失敗した場合は ModFalse を返す。
//
// EXCEPTIONS
// なし
//
ModBoolean
ModParameter::Object::parseLine(ParameterValue& pv, char* line, const char* keyName)
{
	ModSize length = ModCharTrait::length(line);
	int count = 0;

	// コメントを取り除く
	ModSize	i;
	for (i = 0; i < length; i++) {
		if ((i == 0 || (i > 0 && line[i - 1] != '\\')) && line[i] == '"') {
			// '"'の数を数える
			count++;
		}
		if (count != 1 && line[i] == '#') {
			line[i] = ModCharTrait::null();
			length = i;
			break;
		}
	}
	// 最後尾の連続した空白を取り除く
	if (length > 0) {
		i--;
	}
	for (; i > 0; i--) {
		if (ModCharTrait::isSpace(line[i]) == ModFalse) {
			break;
		}
	}
	line[i + 1] = ModCharTrait::null();

	char* cp;
	char* dest;

	// 先頭の空白を飛ばす
	for (cp = line;
		 ModCharTrait::isSpace(*cp) == ModTrue && *cp != ModCharTrait::null();
		 ++cp);

	// 空白でない部分を取り出す。ついでに空白の分前にずらす
	for (dest = line;
		 ModCharTrait::isSpace(*cp) == ModFalse && *cp != ModCharTrait::null();
		 ++dest, ++cp) {
		if (dest != cp) {
			*dest = *cp;
		}
	}

	if (dest == line) return ModFalse; // 空行

	// キー値の終りを null terminate し、cp をひとつ進めておく
	*dest = ModCharTrait::null();
	++cp;

	// (この時点でlineの中身はキー値のみとなる)

	// キー値の指定があるときは該当しなければ終り
	if (keyName && ModCharTrait::compare(keyName, line)) return ModFalse;

	// 値の前の空白を飛ばす
	for (;
		 ModCharTrait::isSpace(*cp) == ModTrue && *cp != ModCharTrait::null();
		 ++cp);
	// 値の先頭を記憶
	dest = cp;

	// パラメータ値の型を決める
	ParameterType tmpType;
	if (ModCharTrait::isDigit(*cp) || *cp == '+') {
		// 符号なし数値
		tmpType = typeUnsignedNumeric;

	} else if (*cp == '-') {
		// 符号つき数値
		tmpType = typeNumeric;

	} else if (*cp == '"') {
		tmpType = typeString;

	} else if (*cp == 'T' || *cp == 'F' || *cp == 't' || *cp == 'f') {
		tmpType = typeBoolean;

	} else {
		// 判定不能
		_SafeModMessage << "Illegal Parameter Value: \""
						<< line << " " << dest << "\"" << ModEndl;
		return ModFalse;
	}

	switch (tmpType) {
	case typeNumeric:					// 数値
	case typeUnsignedNumeric:
	{
		NumericType sign;
		NumericType tmpValue = 0; // 64bitsを確保

		if (*cp == '+') {
			sign = 1;
			cp++;
		} else if (*cp == '-') {
			sign = -1;
			cp++;
		} else {
			sign = 1;
		}

		// 数値に変換できる部分が続く範囲だけ走査する
		if (*cp == '0' && (*(cp + 1) == 'x' || *(cp + 1) == 'X')) {
			// 16進表現
			cp += 2;
			for (; ModCharTrait::isXDigit(*cp) == ModTrue; cp++) {
				tmpValue *= 16;
				tmpValue +=	(NumericType)(*cp -
										  ((ModCharTrait::isDigit(*cp)) ? 
										   '0' : (*cp <= 'F') ? 'A' : 'a'));
			}
		} else {
			// 10進表現
			for (; ModCharTrait::isDigit(*cp) == ModTrue; cp++) {
				tmpValue *= 10;
				tmpValue += (NumericType)(*cp-'0');
			}
		}
		if (sign == 1) {
			pv.setParameterValue((UnsignedNumericType)tmpValue);
		} else {
			pv.setParameterValue((NumericType)(tmpValue * sign));
		}
	}
	break;

	case typeBoolean:					// 真偽値
	{
		ModBoolean tmpBoolean;
		if (*cp == 'T' || *cp == 't') {
			tmpBoolean = ModTrue;
		} else if (*cp == 'F' || *cp == 'f') {
			tmpBoolean = ModFalse;
#ifdef DEBUG
		} else {
			;ModAssert(ModFalse);
#endif
		}
		pv.setParameterValue(tmpBoolean);
	}
	break;

	case typeString:					// 文字列
	{
		;ModAssert(*cp == '"');
		cp++;

		char dp_array[ModParameterFileLineSizeMax];
		char* dp = dp_array;

		for (; *cp != '"' && *cp != ModCharTrait::null(); cp++) {
			if (*cp == '\\') {			// エスケープ文字列
				cp++;
				switch(*cp) {
				case 'b':
					*dp++ = '\b'; break;
				case 'f':
					*dp++ = '\f'; break;
				case 'n':
					*dp++ = '\n'; break;
				case 'r':
					*dp++ = '\r'; break;
				case 't':
					*dp++ = '\t'; break;
				case 'v':
					*dp++ = '\v'; break;
				default:
					*dp++ = *cp;  break;
				}
				if (*cp == ModCharTrait::null()) {
					break;
				}
			} else {					// 通常の文字
				*dp++ = *cp;
			}
		}
		// null-terminate しておく
		*dp = ModCharTrait::null();

		if (*cp != '"') {
			// 最後は '"' でなければならない
			_SafeModMessage << "Terminating '\"' is required: \""
							<< line << " " << dest << "\"" << ModEndl;
			return ModFalse;
		}

		pv.setParameterValue(dp_array);
	}
	break;

#ifdef DEBUG
	default:
		;ModAssert(ModFalse);
		break;
#endif
	}

	return ModTrue;
}

//static
ModParameter::Object*
ModParameter::Object::attach(const ModParameterSource& source,
						   ModBoolean doPreLoad)
{
	ModParameter::Object* pParam = 0;

	ModAutoMutex<ModOsMutex> m(ModCommonMutex::getMutex());
	m.lock();

	ModParameter::Object* pElement = _allObjects;
	while (pElement) {
		if (pElement->isEquivalent(source, doPreLoad) == ModTrue) {
			pParam = pElement;
			break;
		}
		pElement = pElement->_next;
	}
	if (!pParam) {
		pParam = new Object(source, doPreLoad);
		pParam->_next = _allObjects;
		_allObjects = pParam;
	}
	// 参照回数を増やす
	++pParam->_refCount;

	return pParam;
}

//static
void
ModParameter::Object::detach(ModParameter::Object*& pParameter)
{
	ModAutoMutex<ModOsMutex> m(ModCommonMutex::getMutex());
	m.lock();

	if (!--pParameter->_refCount) {
		ModParameter::Object* pElement = _allObjects;
		ModParameter::Object* pPrev = 0;
		while (pElement) {
			if (pElement == pParameter) break;
			pPrev = pElement;
			pElement = pElement->_next;
		}
		if (pElement) {
			if (pPrev)
				pPrev->_next = pElement->_next;
			else
				_allObjects = pElement->_next;
			pElement->_next = 0;
		}
		delete pParameter, pParameter = 0;
	}
}

// ModParameter::Objectの同一性を判定する
ModBoolean
ModParameter::Object::isEquivalent(const ModParameterSource& source,
								 ModBoolean doPreLoad) const
{
	return (
#if MOD_CONF_REGISTRY == 0
		_doPreLoad == doPreLoad &&
#endif
		_sourceInfo == SourceInfo(source))
		? ModTrue : ModFalse;
}

ModParameter::Object::SourceInfo::SourceInfo()
	: ModPair<char*, ModSize>((char*)0, (ModSize)0)
{
}

ModParameter::Object::SourceInfo::SourceInfo(const ModParameterSource& source)
	: ModPair<char*, ModSize>((char*)0, (ModSize)0)
{
	set(source);
}

ModParameter::Object::SourceInfo::~SourceInfo()
{
	delete [] first, first = 0;
	second = 0;
}

void
ModParameter::Object::SourceInfo::set(const ModParameterSource& source)
{
	const int NumOfMember = 5;
	static ModSize n[NumOfMember] = {0, 0, 0, 0, 0};
	static const char* ModParameterSource::* p[NumOfMember] =
	{
		&ModParameterSource::_parentPath,
		&ModParameterSource::_environmentName,
		&ModParameterSource::_environmentValue,
		&ModParameterSource::_systemEnvironmentName,
		&ModParameterSource::_systemEnvironmentValue,
	};

	if (first) {
		delete first, first = 0;
	}
	second = 0;

	int i = 0;
	for (; i < NumOfMember; ++i) {
		if (source.*(p[i])) {
			second += (n[i] = (ModSize)::strlen(source.*(p[i])));
		}
	}
	if (second) {
		// 一つでもnullでないメンバーがあったらコピーする
		second += NumOfMember + 1; // 区切り記号 + null-terminate
		char* value = first = new char[second];
		for (i = 0; i < NumOfMember; ++i) {
			if (n[i]) {
				ModCharTrait::copy(value, source.*(p[i]));
				value += n[i];
			}
			*value = '@'; // 区切り
			++value;
		}
		*value = '\0';
	}
}

ModBoolean
ModParameter::Object::SourceInfo::operator==(const SourceInfo& info) const
{
	return (info.first == first
			|| (info.first && first
				&& info.second == second
				&& ModOsDriver::Memory::compare(info.first,
												first,
												info.second) == 0))
		? ModTrue : ModFalse;
}


//////////////////////////////////////////////////
// ModParameter
//////////////////////////////////////////////////

//	CONSTANT static
//	ModParameter::_codingType -- 文字列の内部表現に用いる文字コード
//
//	NOTES
#if MOD_CONF_UNICODE_PARAMETER == 0
//		パラメーターソースの文字コードとしてリテラルコードが用いられる
#else
//		UTF-8が用いられる
#endif
//		
//static
ModKanjiCode::KanjiCodeType
ModParameter::_codingType = 
#if MOD_CONF_UNICODE_PARAMETER == 0
	ModKanjiCode::literalCode;
#else
	ModKanjiCode::utf8;
#endif

//	VARIABLE static
//	ModParameter::_defaultParameter -- デフォルトのModParameter
//
//	NOTES
//		
//static
ModParameter*
ModParameter::_defaultParameter = 0;

//	FUNCTION public
//	ModParameter::ModParameter --
//		パラメーターを扱うクラスのコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		ModBoolean			doPreLoad
//			ModTrue または指定されないとき(Unix版)
//				パラメーターの値を初期化時にすべて先読みする
//			ModFalse(Unix版)
//				パラメーターの値を初期化時にすべて先読みせずに、
//				取得要求ごとにパラメーターの値を読み出す
//		const char*	environmentName
//			(その2のみ)参照するファイルを記した環境変数の名前
//		const ModParameterSource& source
//			(その3のみ)初期化に用いるModParameterSource
//
//	RETURN
//		なし

// その1 (デフォルトコンストラクタ)
ModParameter::ModParameter(ModBoolean doPreLoad)
	: _parameter(0)
{
	_parameter = Object::attach(ModParameterSource(), doPreLoad);
}

// その2
ModParameter::ModParameter(const char* environmentName, ModBoolean doPreLoad)
	: _parameter(0)
{
	_parameter = Object::attach(
		ModParameterSource(0, environmentName), doPreLoad);
}

// その3
ModParameter::ModParameter(const ModParameterSource& source,
						   ModBoolean doPreLoad)
	: _parameter(0)
{
	_parameter = Object::attach(source, doPreLoad);
}

//	FUNCTION public
//	ModParameter::~ModParameter -- デストラクター
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
ModParameter::~ModParameter()
{
	if (_parameter) {
		Object::detach(_parameter);
	}
}

//	FUNCTION public
//	ModParameter::initialize --
//		パラメーターに関する初期化を行う
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//static
void
ModParameter::initialize()
{
	// default引数のパラメーターを作っておく
	_defaultParameter = new ModParameter;
}

//	FUNCTION public
//	ModParameter::terminate --
//		パラメーターに関する後処理を行う
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//static
void
ModParameter::terminate()
{
	// デフォルトのパラメーターをクリアする
	delete _defaultParameter, _defaultParameter = 0;
}

//
// Copyright (c) 1997, 1998, 1999, 2000, 2002, 2003, 2004, 2005, 2006, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
