//
// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:
//
//  ModTermStringFile.cpp -- ModTermStringFile の実装
// 
// Copyright (c) 2002, 2010, 2023 Ricoh Company, Ltd.
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

#include "ModFile.h"
#include "ModOstrStream.h"
#include "ModAutoPointer.h"
#include "LibUna/ModTermStringFile.h"

_UNA_USING

//
// FUNCTION public
// ModTermStringFile::ModTermStringFile -- コンストラクタ
// 
// NOTES
//   コンストラクタ。
//   引数で指定されたファイル内の各文字列をModVectorに順にセットする。
// 
//   ただし以下を前提とする。
//   ・読み込むファイルはutf8である事
//   ・ファイルは空でもよい
// 
// ARGUMENTS
//   const ModUnicodeString& path  ファイルのパス
//   const char recordSep   レコード(文字列)のセパレータ
// 
// RETURN
//   なし
// 
// EXCEPTIONS
//   ModTermErrorFileOpenFail
//     ファイルオープンに失敗
// 
//   ModTermErrorFileReadFail
//     ファイルオープン以外
//      ・読み込み失敗 
// 
ModTermStringFile::ModTermStringFile(
  const ModUnicodeString& path,     // ファイルのパス
  const char recordSep)      // レコード(文字列)のセパレータ
{
  ModOstrStream tmpStream;   // 読込むデータのサイズは分からない
  char uniChar = 0;          // 読み込み用バッファ バイト単位で読込む
                             // 読込むデータはutf-8

  ModFile* file;
  try {
    // ファイルをオープン
    file = new ModFile(path, ModFile::readMode);
  } catch (...) {
    // 例外
	ModErrorMessage << "path:" << path << ModEndl;
	ModThrow(ModModuleStandard, ModCommonErrorBadArgument, ModErrorLevelError);
  }

  // アーカイバー生成
  ModArchive archiver(*file, ModArchive::ModeLoadArchive);

  try {

    // このループは例外で終了する
    // ModOsErrorEndOfFile ならファイルの終わりまで読んだ事になる
    while(1) {

      // 一文字読み込む
      archiver >> uniChar;

      // 改行ならば
      if(uniChar == recordSep) {
#if 0 
        // 読込んだ文字列を Mapに格納(但し空行の場合は格納しない)
        if(tmpStream.isEmpty() != ModTrue) {
          this->pushBack(ModUnicodeString(tmpStream.getString()));
          // クリア
          tmpStream.clear();
        }
#else
        // 読込んだ文字列を Mapに格納(空行の場合も) 2002/10/01
        this->pushBack(ModUnicodeString(tmpStream.getString()));
        // クリア
        tmpStream.clear();
#endif
      // 改行以外ならば
      } else {
        // 読込んだ文字を格納
        tmpStream << uniChar;
      }
    }

  // 後処理 必ずここに来る(読み込みが正常に終了した場合も)。
  } catch (ModException& exception) {
    file->close();
    delete file;
    archiver.closeArchive();

    // ファイルの末尾
    if (exception.getErrorNumber() == ModOsErrorEndOfFile) {
      // これはOK
      if(uniChar != recordSep) {
        // まだ読み込んだ結果を格納していない
        if(tmpStream.isEmpty() != ModTrue) {
          // 結果を格納する
          this->pushBack(ModUnicodeString(tmpStream.getString()));
          // クリア
          tmpStream.clear();
        }
      }
      // 例外をリセット
      ModErrorHandle::reset();

    // これはだめ 例外
    } else {
		ModThrow(ModModuleStandard, ModCommonErrorBadArgument, ModErrorLevelError);
    }
  }
};

//
// FUNCTION public
// ModTermWordFile::ModTermWordFile -- コンストラクタ
// 
// NOTES
//   コンストラクタ。pathで指定されたファイル内の各文字列を
//   キーとし、かつ、値を一律 0 として登録する。
// 
// ARGUMENTS
//   const ModUnicodeString& path  ファイルのパス
//   const char recordSep   レコード(文字列)のセパレータ
// 
// RETURN
//   なし
// 
// EXCEPTIONS
//   なし
// 
ModTermWordFile::ModTermWordFile(
  const ModUnicodeString& path,   // ファイルのパス
  const char recordSep)    // レコード(文字列)のセパレータ
{
  // ファイルからデータの読み込み
  ModTermStringFile file(path, recordSep);

  // マップに登録。値は一律 0 
  for(ModTermStringFile::Iterator p = file.begin(); p != file.end(); p++) {
    this->insert(*p, 0);
  }
}

//
// FUNCTION public
// ModTermParameterFile::ModTermParameterFile -- コンストラクタ
// 
// NOTES
//   コンストラクタ。pathで指定されたファイル内の各文字列を
//   キーとし、かつ、値を一律 0 として登録する。
// 
// ARGUMENTS
//   const ModUnicodeString& path  ファイルのパス
//   const char fieldSep    フィールドのセパレータ
//   const char recordSep   レコードのセパレータ
// 
// RETURN
//   なし
// 
// EXCEPTIONS
//   なし
// 
ModTermParameterFile::ModTermParameterFile(
  const ModUnicodeString& path,   // ファイルのパス
  const char fieldSep,     // フィールドのセパレータ
  const char recordSep)    // レコードのセパレータ
{
  // ファイルからデータの読み込み
  ModTermStringFile file(path, recordSep);

  // マップに登録
  for(ModTermStringFile::Iterator p = file.begin(); p != file.end(); p++) {
    // レコード
    ModUnicodeString record(*p);
    // コメント行のスキップ
    if(record[0] == '%') {
      continue;
    }
    // キーの取り出し
    ModUnicodeString key;
    ModSize i;
    for(i = 0; i < record.getLength(); i++) {
      if(record[i] == fieldSep) {
        break;
      }
      key += record[i];
    }
    // 値の取り出し
    ModUnicodeString rest;
    for(i++; i < record.getLength(); i++) {
      rest += record[i];
    }
    // 正しく取り出せなかった場合はスキップ
    if(key.getLength() == 0 || rest.getLength() == 0) {
      continue;
    }
    double value = ModUnicodeCharTrait::toFloat((const ModUnicodeChar*)rest);
    // マップにキーと値を登録
    this->insert(key, value);
  }
}

//
// FUNCTION public
// ModTermParameterFile::getValue -- パラメタ値の取得
// 
// NOTES
//   パラメタ値を取得し引数にセットする。
// 
// ARGUMENTS
//   const ModUnicodeString& key   パラメタ
//   double& value                 値
// 
// RETURN
//   パラメタ値が取得できたときはModTrue, それ以外はModFalse
// 
// EXCEPTIONS
//   なし
// 
ModBoolean
ModTermParameterFile::getValue(
  const ModUnicodeString& key,  // パラメタ
  double& value) const          // 値
{
  ModTermParameterFile::ConstIterator p = this->find(key);
  if(p != this->end()) {
    value = (*p).second;
    return ModTrue;
  }
  return ModFalse;
}

//
// FUNCTION public
// ModTermTypeFile::ModTermTypeFile -- コンストラクタ
// 
// NOTES
//   コンストラクタ。pathで指定されたファイル内の各文字列を検索語タイプ(整数値)
//   に変換してキーとし、かつ、値を一律 0 として登録する。
// 
// ARGUMENTS
//   const ModUnicodeString& path  ファイルのパス
//   const char recordSep   レコード(文字列)のセパレータ
// 
// RETURN
//   なし
// 
// EXCEPTIONS
//   なし
// 
ModTermTypeFile::ModTermTypeFile(
  const ModUnicodeString& path,  // ファイルのパス
  const char recordSep)   // レコード(文字列)のセパレータ
{
  // ファイルからデータの読み込み
  ModTermStringFile file(path, recordSep);

  // マップに登録。値は一律 0 
  for(ModTermStringFile::Iterator p = file.begin(); p != file.end(); p++) {
    int type = ModUnicodeCharTrait::toInt((const ModUnicodeChar*)*p, 10);
    this->insert(type, 0);
  }
}

//
// FUNCTION public
// ModTermTypeTable::ModTermTypeTable -- コンストラクタ
// 
// NOTES
//   コンストラクタ。pathで指定されたファイル内の各文字列を検索語タイプ(整数値)
//   に変換して値とし、ファイル中の行位置(0起算)をキーとして登録する。
//
//   形態素品詞番号から検索語タイプへの変換に用いる。
// 
// ARGUMENTS
//   const ModUnicodeString& path  ファイルのパス
//   const char recordSep   レコード(文字列)のセパレータ
// 
// RETURN
//   なし
// 
// EXCEPTIONS
//   なし
// 
ModTermTypeTable::ModTermTypeTable(
  const ModUnicodeString& path,  // ファイルのパス
  const char recordSep)   // レコード(文字列)のセパレータ
{
  // ファイルからデータの読み込み
  ModTermStringFile file(path, recordSep);

  // テーブルに登録
  this->reserve(file.getSize());
  for(ModTermStringFile::Iterator p = file.begin(); p != file.end(); p++) {
    ModTermType type = ModUnicodeCharTrait::toInt((const ModUnicodeChar*)*p, 10);
    this->pushBack(type);
  }
}

//
// Copyright (c) 2002, 2010, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
