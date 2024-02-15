// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:
//
//  ModTermStringFile.h 
//   - ModTermStringFile の宣言
//   - ModTermWordFile   の宣言
// 
// Copyright (c) 2000, 2005, 2010, 2012, 2013, 2023 Ricoh Company, Ltd.
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
#ifndef __SYDNEY_UTILITY_MODTERMSTRINGFILE_H__
#define __SYDNEY_UTILITY_MODTERMSTRINGFILE_H__

#include "Utility/Module.h"
#include "Utility/ModTermElement.h"  // ModTermType

#include "ModVector.h"
#include "ModHashMap.h"

_TRMEISTER_BEGIN
_TRMEISTER_UTILITY_BEGIN

//
// CLASS
// ModTermStringFile -- 文字列ファイル
//
// NOTES
//	パスで指定されたファイル中の utf8 文字列をベクトルに格納する。
//
//	文字列はセパレータ(デフォルトは改行)で区切られてファイル中に
//  格納されている。
//
class ModTermStringFile : public ModVector<ModUnicodeString>
{
public:

  // コンストラクタ
  ModTermStringFile(
    const ModUnicodeString& path,            // ファイルパス
    const char separator = '\n');     // 文字列のセパレータ

protected:

private:
};

//
// CLASS
// ModTermWordFile -- 語形辞書ファイル
//
// NOTES
//	語形辞書ファイル。
//
class ModTermWordFile
  : public ModHashMap<ModUnicodeString, double, ModUnicodeStringHasher>
{
public:
  // コンストラクタ
  ModTermWordFile(
    const ModUnicodeString& path,            // ファイルパス
    const char separator = '\n');     // 語形のセパレータ

  // 検索関数
  ModBoolean isFound(const ModUnicodeString& _string) const
    { return this->find(_string) != this->end() ? ModTrue : ModFalse; }

protected:

private:

};

//
// CLASS
//   ModTermParameterFile -- パラメタファイル
//
// NOTES
//	 パラメタファイル。
//   パラメタとその値のペア(":"で区切る)を１行とする。
//   行頭"%"はコメント。
//
class ModTermParameterFile
  : public ModHashMap<ModUnicodeString, double, ModUnicodeStringHasher>
{
public:
  // コンストラクタ
  ModTermParameterFile(
    const ModUnicodeString& path,        // ファイルパス
    const char fieldSep  = ':',   // フィールド(キーと値)のセパレータ
    const char recordSep = '\n'); // レコードのセパレータ

  // 検索関数
  ModBoolean getValue( 
    const ModUnicodeString& key,  // パラメタ
    double& value) const;         // 値

protected:

private:

};

typedef ModHasher<int> ModTermTypeHasher;

//
// CLASS
// ModTermTypeFile -- タイプ辞書ファイル
//
// NOTES
//	タイプ辞書ファイル。
//
class ModTermTypeFile : public ModHashMap<int, double, ModTermTypeHasher>
{
public:
  // コンストラクタ
  ModTermTypeFile(
    const ModUnicodeString& path,            // ファイルパス
    const char separator = '\n');     // 検索語タイプのセパレータ

  // 検索関数
  ModBoolean isFound(const int _type) const
    { return this->find(_type) != this->end() ? ModTrue : ModFalse; }

protected:

private:

};

//
// CLASS
// ModTermTypeTable -- タイプ辞書テーブル
//
// NOTES
//  タイプ辞書テーブル
//
class ModTermTypeTable : public ModVector<ModTermType>
{
public:
  // コンストラクタ
  ModTermTypeTable(
    const ModUnicodeString& path,            // ファイルパス
    const char separator = '\n');     // 検索語タイプのセパレータ

protected:

private:

};

_TRMEISTER_UTILITY_END
_TRMEISTER_END

#endif  // __SYDNEY_UTILITY_MODTERMSTRINGFILE_H__

//
// Copyright (c) 2000, 2005, 2010, 2012, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
