// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:	
//
// ModHasher.h -- 各種クラスに対する hash 関数を提供するクラス
// 
// Copyright (c) 1998, 1999, 2000, 2002, 2023 Ricoh Company, Ltd.
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

#ifndef	__ModHasher_H__
#define __ModHasher_H__

#include "ModTypes.h"
#include "ModString.h"
#include "ModUnicodeString.h"
#include "ModUnicodeCharTrait.h"
#include "ModDefaultManager.h"

//
// TEMPLATE CLASS
// ModHasher -- ModSize にキャストできる型のハッシュ関数
//
// TEMPLATE ARGUMENTS
// KeyType ハッシュのキーとなる型。ModSize にキャストできなければならない
//
// NOTES
// このクラスは ModSize にキャストできる型に対してハッシュ関数を
// 提供するものである。実際には入力をそのまま ModSize にキャストして
// 返すにすぎない。
//

template <class KeyType>
class ModHasher
	: public	ModDefaultObject
{
public:
	ModSize operator()(const KeyType& key) const;
};

template <class KeyType>
inline ModSize
ModHasher<KeyType>::operator()(const KeyType& key) const
{
	return (ModSize)key;
}

#ifndef ModBitsPerByte
# define ModBitsPerByte 8
#endif

#ifndef ModSizeBits
# define ModSizeBits (sizeof(ModSize) * ModBitsPerByte)
#endif

//
// CLASS
// ModStringHasher -- 文字列用のハッシュ関数を提供するクラス
//
// NOTES
// このクラスは const char*、ModCharString に対して
// ハッシュ関数を提供するクラスである。
// ModHasher<ModString> としても使用可能。
//

ModTemplateNull class ModHasher<ModCharString>
	: public	ModDefaultObject
{
public:
	ModSize operator()(const char* key) const;
	ModSize operator()(const ModCharString& key) const;
private:
	ModSize calcHashValue(const char* key) const;
};

//
// TYPEDEF
// ModStringHasher -- 文字列用のハッシュ関数の別名
//
// NOTES
// この別名は ModHasher<ModCharString>の別名として、
// ModCharString および char* のためのハッシュ関数を提供する
// クラスを表す。
//
typedef ModHasher<ModCharString> ModStringHasher;

//
// FUNCTION
// ModStringHasher::opeartor() -- ハッシュ値を計算する
//
// NOTES
// この関数は char* で与えられた文字列のハッシュ値を計算するのに用いる。
//
// ARGUMENTS
// const char* key
//		ハッシュ値を計算する文字列
//
// RETURN
// 計算したハッシュ値
//
// EXCEPTIONS
// なし
//
inline ModSize
ModStringHasher::operator()(const char* key) const
{
	return this->calcHashValue(key);
}

//
// FUNCTION
// ModStringHasher::opeartor() -- ハッシュ値を計算する
//
// NOTES
// この関数は ModCharString で与えられた文字列のハッシュ値を計算するのに
// 用いる。
//
// ARGUMENTS
// const ModCharString& key
//		ハッシュ値を計算する文字列
//
// RETURN
// 計算したハッシュ値
//
// EXCEPTIONS
// なし
//
inline ModSize
ModStringHasher::operator()(const ModCharString& key) const
{
	return this->calcHashValue(key.getString());
}

//
// FUNCTION private
// ModStringHasher::opeartor() -- ハッシュ値を計算する(下請け)
//
// NOTES
// この関数は char* で与えられた文字列のハッシュ値を計算するのに用いる。
//
// ARGUMENTS
// const char* key
//		ハッシュ値を計算する文字列
//
// RETURN
// 計算したハッシュ値
//
// EXCEPTIONS
// なし
//
inline ModSize
ModStringHasher::calcHashValue(const char* key) const
{
	const char* cp = key;
	ModSize hashValue = 0;
	ModSize g;
	while (*cp != ModCharTrait::null()) {
		hashValue <<= 4;
		hashValue += (ModSize) *cp++;
		if (g = hashValue & ((ModSize) 0xf << (ModSizeBits - 4))) {
			hashValue ^= g >> (ModSizeBits - 8);
			hashValue ^= g;
		}
	}
	return hashValue;
}

//
// CLASS
// ModWideStringHasher -- 文字列用のハッシュ関数を提供するクラス
//
// NOTES
// このクラスは const ModWideChar*、ModWideString に対して
// ハッシュ関数を提供するクラスである。
// ModHasher<ModWideString> としても使用可能。
//

ModTemplateNull class ModHasher<ModWideString>
	: public	ModDefaultObject
{
public:
	ModSize operator()(const ModWideChar* key) const;
	ModSize operator()(const ModWideString& key) const;
private:
	ModSize calcHashValue(const ModWideChar* key) const;
};

//
// TYPEDEF
// ModWideStringHasher -- ワイド文字列用のハッシュ関数の別名
//
// NOTES
// この別名は ModHasher<ModWideString>の別名として、
// ModWideString および ModWideChar* のためのハッシュ関数を提供する
// クラスを表す。
//
typedef ModHasher<ModWideString> ModWideStringHasher;

//
// FUNCTION
// ModWideStringHasher::opeartor() -- ハッシュ値を計算する
//
// NOTES
// この関数は ModWideChar* で与えられた文字列のハッシュ値を計算するのに用いる。
//
// ARGUMENTS
// const ModWideChar* key
//		ハッシュ値を計算する文字列
//
// RETURN
// 計算したハッシュ値
//
// EXCEPTIONS
// なし
//
inline ModSize
ModWideStringHasher::operator()(const ModWideChar* key) const
{
	return this->calcHashValue(key);
}

//
// FUNCTION
// ModWideStringHasher::opeartor() -- ハッシュ値を計算する
//
// NOTES
// この関数は ModWideString で与えられた文字列のハッシュ値を計算するのに
// 用いる。
//
// ARGUMENTS
// const ModWideString& key
//		ハッシュ値を計算する文字列
//
// RETURN
// 計算したハッシュ値
//
// EXCEPTIONS
// なし
//
inline ModSize
ModWideStringHasher::operator()(const ModWideString& key) const
{
	return this->calcHashValue(key.getBuffer());
}

//
// FUNCTION private
// ModWideStringHasher::opeartor() -- ハッシュ値を計算する(下請け)
//
// NOTES
// この関数は ModWideChar* で与えられた文字列のハッシュ値を計算するのに用いる。
//
// ARGUMENTS
// const ModWideChar* key
//		ハッシュ値を計算する文字列
//
// RETURN
// 計算したハッシュ値
//
// EXCEPTIONS
// なし
//
inline ModSize
ModWideStringHasher::calcHashValue(const ModWideChar* key) const
{
	const ModWideChar* wp = key;
	ModSize hashValue = 0;
	ModSize g;
	while (*wp != ModWideCharTrait::null()) {
		hashValue <<= 4;
		hashValue += (ModSize) *wp++;
		if (g = hashValue & ((ModSize) 0xf << (ModSizeBits - 4))) {
			hashValue ^= g >> (ModSizeBits - 8);
			hashValue ^= g;
		}
	}
	return hashValue;
}

//
// CLASS
// ModUnicodeStringHasher -- 文字列用のハッシュ関数を提供するクラス
//
// NOTES
// このクラスは const ModUnicodeChar*、ModUnicodeString に対して
// ハッシュ関数を提供するクラスである。
// ModHasher<ModUnicodeString> としても使用可能。
//

ModTemplateNull class ModHasher<ModUnicodeString>
	: public	ModDefaultObject
{
public:
	ModSize operator()(const ModUnicodeChar* key) const;
	ModSize operator()(const ModUnicodeString& key) const;
private:
	ModSize calcHashValue(const ModUnicodeChar* key) const;
};

//
// TYPEDEF
// ModUnicodeStringHasher -- ワイド文字列用のハッシュ関数の別名
//
// NOTES
// この別名は ModHasher<ModUnicodeString>の別名として、
// ModUnicodeString および ModUnicodeChar* のためのハッシュ関数を提供する
// クラスを表す。
//
typedef ModHasher<ModUnicodeString> ModUnicodeStringHasher;

//
// FUNCTION
// ModUnicodeStringHasher::opeartor() -- ハッシュ値を計算する
//
// NOTES
// この関数は ModUnicodeChar* で与えられた文字列のハッシュ値を計算するのに用いる。
//
// ARGUMENTS
// const ModUnicodeChar* key
//		ハッシュ値を計算する文字列
//
// RETURN
// 計算したハッシュ値
//
// EXCEPTIONS
// なし
//
inline ModSize
ModUnicodeStringHasher::operator()(const ModUnicodeChar* key) const
{
	return this->calcHashValue(key);
}

//
// FUNCTION
// ModUnicodeStringHasher::opeartor() -- ハッシュ値を計算する
//
// NOTES
// この関数は ModUnicodeString で与えられた文字列のハッシュ値を計算するのに
// 用いる。
//
// ARGUMENTS
// const ModUnicodeString& key
//		ハッシュ値を計算する文字列
//
// RETURN
// 計算したハッシュ値
//
// EXCEPTIONS
// なし
//
inline ModSize
ModUnicodeStringHasher::operator()(const ModUnicodeString& key) const
{
	return this->calcHashValue(key);
}

//
// FUNCTION private
// ModUnicodeStringHasher::opeartor() -- ハッシュ値を計算する(下請け)
//
// NOTES
// この関数は ModUnicodeChar* で与えられた文字列のハッシュ値を計算するのに用いる。
//
// ARGUMENTS
// const ModUnicodeChar* key
//		ハッシュ値を計算する文字列
//
// RETURN
// 計算したハッシュ値
//
// EXCEPTIONS
// なし
//
inline ModSize
ModUnicodeStringHasher::calcHashValue(const ModUnicodeChar* key) const
{
	const ModUnicodeChar* wp = key;
	ModSize hashValue = 0;
	ModSize g;
	while (*wp != ModUnicodeCharTrait::null()) {
		hashValue <<= 4;
		hashValue += (ModSize) *wp++;
		if (g = hashValue & ((ModSize) 0xf << (ModSizeBits - 4))) {
			hashValue ^= g >> (ModSizeBits - 8);
			hashValue ^= g;
		}
	}
	return hashValue;
}

#endif	// __ModHasher_H__

//
// Copyright (c) 1998, 1999, 2000, 2002, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
