// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// UNA.cpp -- UNA 関連の関数定義
// 
// Copyright (c) 2004, 2005, 2006, 2009, 2011, 2012, 2023 Ricoh Company, Ltd.
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

namespace
{
const char srcFile[] = __FILE__;
const char moduleName[] = "Utility";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Utility/UNA.h"
#include "Utility/Manager.h"

#include "Common/Assert.h"
#include "Common/Manager.h"
#include "Common/SystemParameter.h"
#include "Common/UnicodeString.h"

#include "Os/AutoCriticalSection.h"
#include "Os/Path.h"

#include "ModAutoPointer.h"
#include "ModHashMap.h"
#include "ModNLP.h"
#ifndef SYD_USE_UNA_V10
#include "ModNormalizer.h"
#include "ModNormRule.h"
#endif
#include "ModOstrStream.h"

#ifdef SYD_OS_WINDOWS
#define USE_UNIFIED_UNA
#endif

_TRMEISTER_USING
_TRMEISTER_UTILITY_USING

namespace
{

namespace _UNA
{
	//	STRUCT
	//	$$$::_Resource -- 正規化用リソースを表す構造体
	//
	//	NOTES

	struct _Resource
	{
		// コンストラクター
		_Resource(Utility::Una::ResourceID::Value id);
		// デストラクター
		~_Resource();

#ifdef SYD_USE_UNA_V10
		UNA::ModNlpResource*	_p;
#else
		const ModNlpResource*	_p;
#endif
	};

#ifndef SYD_USE_UNA_V10
	//	STRUCT
	//	$$$::_Rule -- 正規化用辞書を表す構造体
	//
	//	NOTES
	
	struct _Rule
	{
		// コンストラクター
		_Rule(Utility::Una::ResourceID::Value id);
		// デストラクター
		~_Rule();

		const ModNormRule*		_p;
	};
#endif

	// 以下の情報の排他制御用のラッチ
	Os::CriticalSection	_latch;

	// 生成済のリソースを管理するためのハッシュマップ
	typedef ModHashMap<Utility::Una::ResourceID::Value, _Resource*, ModHasher<Utility::Una::ResourceID::Value> > _ResourceMap;
	_ResourceMap		_resourceMap;
#ifndef SYD_USE_UNA_V10
	// 生成済のリソースを管理するためのハッシュマップ
	typedef ModHashMap<Utility::Una::ResourceID::Value, _Rule*, ModHasher<Utility::Una::ResourceID::Value> > _RuleMap;
	_RuleMap			_ruleMap;
#endif
}

namespace _StringData
{
	// 文字列の LIKE 時に正規化のために使用するリソースの識別子
	Utility::Una::ResourceID::Value	_resourceID = Utility::Una::ResourceID::Unknown;
}

//	FUNCTION public
//	$$$::_UNA::_Resource::_Resource --
//		正規化用リソースを表す構造体のコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		Utility::Utility::Una::ResourceID::Value		id
//			リソースの識別子
//
//	RETURN
//		なし
//
//	EXCEPTIONS

_UNA::_Resource::_Resource(Utility::Una::ResourceID::Value id)
	: _p(0)
{
	// [NOTE] もともとはCommonモジュールの一部だったので、
	//  まずUtilityをチェックし、未設定ならCommonもチェックする。
	ModOstrStream stream;
	stream << "Utility_UnaResource_" << id;
	Os::Path path;
	if (Common::SystemParameter::getValue(
			_TRMEISTER_U_STRING(stream.getString()), path) == false)
	{
		ModOstrStream stream;
		stream << "Common_UnaResource_" << id;
		Common::SystemParameter::getValue(
			_TRMEISTER_U_STRING(stream.getString()), path);
	}
#ifdef SYD_USE_UNA_V10
	// リソースファイルのロード中にエラーが発生する場合があるので、
	// auto_ptr で処理する
	
	ModAutoPointer<UNA::ModNlpResource> p = new UNA::ModNlpResource;
	// リソースファイルをロードする
	p->load(path, ModFalse);
	
	_p = p.release();
#else
#ifdef USE_UNIFIED_UNA
	//
	// 今は暫定的にUNAのリソース番号が5の場合のみTeragarm版のUNAを使用する
	// 本来ならリソースファイルで自動的に分けたほうがいいかな。
	//
	if (id == 5)
		_p = new ModNlpResource(path, ModLanguageSet(), 0, ModNlpTeraTag);
	else
		_p = new ModNlpResource(path, ModLanguageSet(), 0, ModNlpUnaOnly);
#else
	_p = new ModNlpResource(path, ModLanguageSet(), 0, ModNlpUnaOnly);
#endif
#endif
	; _TRMEISTER_ASSERT(_p);
}

//	FUNCTION public
//	$$$::_UNA::_Resource::~_Resource --
//		正規化用リソースを表す構造体のデストラクター
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS

_UNA::_Resource::~_Resource()
{
#ifdef SYD_USE_UNA_V10
	_p->unload();
#endif
	delete _p, _p = 0;
}

#ifndef SYD_USE_UNA_V10
//	FUNCTION public
//	$$$::_UNA::_Rule::_Rule -- 正規化用辞書を表すクラスのコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		Utility::Utility::Una::ResourceID::Value		id
//			リソースの識別子
//
//	RETURN
//		なし
//
//	EXCEPTIONS

_UNA::_Rule::_Rule(Utility::Una::ResourceID::Value id)
	: _p(0)
{
	// [NOTE] もともとはCommonモジュールの一部だったので、
	//  まずUtilityをチェックし、未設定ならCommonもチェックする。
	ModOstrStream stream;
	stream << "Utility_UnaResource_" << id;
	Os::Path path;
	if (Common::SystemParameter::getValue(
			_TRMEISTER_U_STRING(stream.getString()), path) == false)
	{
		ModOstrStream stream;
		stream << "Common_UnaResource_" << id;
		Common::SystemParameter::getValue(
			_TRMEISTER_U_STRING(stream.getString()), path);
	}

//	【注意】	現状、末尾が必ずセパレータである必要があるので、ここで加えない
//
//	path += ModOsDriver::File::getPathSeparator();

	path += _TRMEISTER_U_STRING("norm");

//	【注意】	現状、末尾が必ずセパレータである必要があるので、ここで加える

	path += ModOsDriver::File::getPathSeparator();

	_p = new ModNormRule(path);
	; _TRMEISTER_ASSERT(_p);
}

//	FUNCTION public
//	$$$::_UNA::_Rule::~_Rule -- 正規化用辞書を表す構造体のデストラクター
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS

_UNA::_Rule::~_Rule()
{
	delete _p, _p = 0;
}
#endif
}

//	FUNCTION public
//	Utility::Manager::Una::initialize -- UNA を使うための初期化を行う
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

void
Manager::Una::initialize()
{
	// [NOTE] もともとはCommonモジュールの一部だったので、
	//  まずUtilityをチェックし、未設定ならCommonもチェックする。
	int tmp = 0;
	if (Common::SystemParameter::getValue(
			_TRMEISTER_U_STRING("Utility_LikeUnaResource"), tmp) == false)
	{
		Common::SystemParameter::getValue(
			_TRMEISTER_U_STRING("Common_LikeUnaResource"), tmp);
	}
	_StringData::_resourceID = (tmp > 0) ? tmp : 1;
}

//	FUNCTION public
//	Utility::Manager::Una::terminate -- UNA を使うための後処理を行う
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Manager::Una::terminate()
{
	 _StringData::_resourceID = Utility::Una::ResourceID::Unknown;

	if (_UNA::_resourceMap.getSize()) {
		_UNA::_ResourceMap::Iterator		ite(_UNA::_resourceMap.begin());
		const _UNA::_ResourceMap::Iterator&	end = _UNA::_resourceMap.end();

		do {
			delete (*ite).second, (*ite).second = 0;
		} while (++ite != end) ;

		_UNA::_resourceMap.clear();
	}

#ifndef SYD_USE_UNA_V10
	if (_UNA::_ruleMap.getSize()) {
		_UNA::_RuleMap::Iterator			ite(_UNA::_ruleMap.begin());
		const _UNA::_RuleMap::Iterator&		end = _UNA::_ruleMap.end();

		do {
			delete (*ite).second, (*ite).second = 0;
		} while (++ite != end) ;

		_UNA::_ruleMap.clear();
	}
#endif
}

//	FUNCTION
//	Utility::Una::Manager::getModNlpAnalyzer --
//		ある識別子の表すリソースを使用する ModNlpAnalyzer を生成する
//
//	NOTES
#ifdef SYD_USE_UNA_V10
//		得られた ModNlpAnalyzer を開放する際には、
//		ModNlpAnalyzer::releaseResource を実行し、リソースファイルの利用を
//		開放する必要がある。さらに、ModNlpAnalyzer のインスタンス自体も
//		読み出し側で開放する必要がある
#else
//		得られた ModNlpAnalyzer を格納する領域は呼び出し側で解放する必要がある
#endif
//
//	ARGUMENTS
//		Utility::Utility::Una::ResourceID::Value		id
//			使用する正規化用リソースの識別子
#ifndef SYD_USE_UNA_V10
//		unsigned int		maxWordLength
//			0 以外の値が指定されたとき
//				切り出し単語の最大文字数
//			0 または指定されないとき
//				切り出し単語の長さは無制限である
#endif
//
//	RETURN
//		得られた ModNlpAnalyzer を格納する領域の先頭アドレス
//
//	EXCEPTIONS

#ifdef SYD_USE_UNA_V10
UNA::ModNlpAnalyzer*
Una::Manager::getModNlpAnalyzer(ResourceID::Value id)
#else
ModNlpAnalyzer*
Una::Manager::getModNlpAnalyzer(ResourceID::Value id,
								unsigned int maxWordLength)
#endif
{
	Os::AutoCriticalSection	latch(_UNA::_latch);

	// すでに生成済のリソースがあるか調べる

	const _UNA::_ResourceMap::Iterator& ite = _UNA::_resourceMap.find(id);
	_UNA::_Resource* resource =
		(ite != _UNA::_resourceMap.end()) ? (*ite).second : 0;

	if (!resource) {

		// ないので、新たにリソースを生成し、
		// また参照するときのために登録しておく

		; _TRMEISTER_ASSERT(ite == _UNA::_resourceMap.end());

		ModAutoPointer<_UNA::_Resource> destructor(new _UNA::_Resource(id));
		resource = destructor.get();
		; _TRMEISTER_ASSERT(resource);

		_UNA::_resourceMap.insert(id, resource, ModTrue);

		(void) destructor.release();
	}

#ifdef SYD_USE_UNA_V10
	UNA::ModNlpAnalyzer* a = new UNA::ModNlpAnalyzer;
	a->setResource(resource->_p);
	return a;
#else
	return new ModNlpAnalyzer(resource->_p, maxWordLength);
#endif
}

#ifndef SYD_USE_UNA_V10
//	FUNCTION
//	Utility::Una::Manager::getModNormalizer --
//		ある識別子の表すリソースを使用する ModNormalizer を生成する
//
//	NOTES
//		得られた ModNormalizer を格納する領域は呼び出し側で解放する必要がある
//
//	ARGUMENTS
//		Utility::Utility::Una::ResourceID::Value		id
//			使用する正規化用リソースの識別子
//
//	RETURN
//		得られた ModNormalizer を格納する領域の先頭アドレス
//
//	EXCEPTIONS

ModNormalizer*
Una::Manager::getModNormalizer(ResourceID::Value id)
{
	Os::AutoCriticalSection	latch(_UNA::_latch);

	// すでに生成済のルールがあるか調べる

	const _UNA::_RuleMap::Iterator& ite = _UNA::_ruleMap.find(id);
	_UNA::_Rule* rule = (ite != _UNA::_ruleMap.end()) ? (*ite).second : 0;

	if (!rule) {

		// ないので、新たにルールを生成し、
		// また参照するときのために登録しておく

		; _TRMEISTER_ASSERT(ite == _UNA::_ruleMap.end());

		ModAutoPointer<_UNA::_Rule> destructor(new _UNA::_Rule(id));
		rule = destructor.get();
		; _TRMEISTER_ASSERT(rule);

		_UNA::_ruleMap.insert(id, rule, ModTrue);

		(void) destructor.release();
	}

	return new ModNormalizer(rule->_p);
}
#endif

//	FUNCTION
//	Utility::Una::Manager::getResourceForLikeOperator --
//		文字列への like 時に正規化で使用するリソースを得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたリソースの識別子
//
//	EXCEPTIONS
//		なし

Una::ResourceID::Value
Una::Manager::getResourceForLikeOperator()
{
	return _StringData::_resourceID;
}

//	FUNCTION
//	Utility::Una::Manager::getVersion --
//		UNAのバージョンを得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		バージョン
//
//	EXCEPTIONS
//		なし

ModUnicodeString
Una::Manager::getVersion()
{
	// UNAのアナライザーを得る
	ModAutoPointer<UNA::ModNlpAnalyzer> anal
		= Una::Manager::getModNlpAnalyzer(_StringData::_resourceID);

	return anal->getVersion();
}

//
// Copyright (c) 2004, 2005, 2006, 2009, 2011, 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
