// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Hint.h -- 転置ファイル用ヒントパラメータのヘッダーファイル
// 
// Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2008, 2009, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_INVERTED_HINT_H
#define __SYDNEY_INVERTED_HINT_H

#include "Inverted/Module.h"

_SYDNEY_BEGIN
_SYDNEY_INVERTED_BEGIN

namespace Hint
{

namespace TokenizerParameter
{

//
//	CONST
//	Inverted::Hint::TokenizerParameter::Key --
//		トークナイザパラメータの値を取得するためのキー
//
//	NOTES
//	ヒントから
//	トークナイザパラメータの値を取得するためのキー。
//
const char*	const Key = "Tokenizer";

} // end of namespace TokenizerParameter

namespace CoderParameter
{

//
//	CONST
//	Inverted::Hint::CoderParameter::Key --
//		コーダーパラメータの値を取得するためのキー
//
//	NOTES
//	ヒントから
//	コーダーパラメータの値を取得するためのキー。
//
const char*	const Key = "Coder";
} // end of namespace CoderParameter

namespace IdCoderParameter
{

//
//	CONST
//	Inverted::Hint::IdCoderParameter::Key --
//		文書ID符号器の値を取得するためのキー
//
//	NOTES
//	ヒントから
//	文書ID符号器パラメータの値を取得するためのキー。
//
const char*	const Key = "Id";
} // end of namespace IdCoderParameter

namespace FrequencyCoderParameter
{

//
//	CONST
//	Inverted::Hint::FrequencyCoderParameter::Key --
//		頻度符号器パラメーターの値を取得するためのキー
//
//	NOTES
//	ヒントから
//	頻度符号器パラメーターの値を取得するためのキー。
//
const char*	const Key = "Frequency";
} // end of namespace FrequencyCoderParameter

namespace LengthCoderParameter
{

//
//	CONST
//	Inverted::Hint::LengthCoderParameter::Key --
//		圧縮長符号器パラメータの値を取得するためのキー
//
//	NOTES
//	ヒントから
//	圧縮長符号器パラメータの値を取得するためのキー。
//
const char*	const Key = "Length";
} // end of namespace LengthCoderParameter

namespace LocationCoderParameter
{

//
//	CONST
//	Inverted::Hint::LocationCoderParameter::Key --
//		位置情報符号器の値を取得するためのキー
//
//	NOTES
//	ヒントから
//	位置情報符号器パラメータの値を取得するためのキー。
//
const char*	const Key = "Location";
} // end of namespace LocationCoderParameter

namespace WordIdCoderParameter
{

//
//	CONST
//	Inverted::Hint::WordIdCoderParameter::Key --
//		文書ID符号器パラメータ(単語単位検索用)の値を取得するためのキー
//
//	NOTES
//	ヒントから
//	文書ID符号器パラメータ(単語単位検索用)の値を取得するためのキー。
//
const char*	const Key = "WordId";
} // end of namespace WordIdCoderParameter

namespace WordFrequencyCoderParameter
{

//
//	CONST
//	Inverted::Hint::WordFrequencyCoderParameter::Key --
//		頻度符号器パラメーター(単語単位検索用)の値を取得するためのキー
//
//	NOTES
//	ヒントから
//	頻度符号器パラメータ(単語単位検索用)の値を取得するためのキー。
//
const char*	const Key = "WordFrequency";
} // end of namespace WordFrequencyCoderParameter

namespace WordLengthCoderParameter
{

//
//	CONST
//	Inverted::Hint::WordLengthCoderParameter::Key --
//		圧縮長符号器パラメータ(単語単位検索用)の値を取得するためのキー
//
//	NOTES
//	ヒントから
//	圧縮長符号器パラメータ(単語単位検索用)の値を取得するためのキー。
//
const char*	const Key = "WordLength";
} // end of namespace WordLengthCoderParameter

namespace WordLocationCoderParameter
{

//
//	CONST
//	Inverted::Hint::WordLocationCoderParameter::Key --
//		位置情報符号器パラメータ(単語単位検索用)の値を取得するためのキー
//
//	NOTES
//	ヒントから
//	位置情報符号器パラメータ(単語単位検索用)の値を取得するためのキー。
//
const char*	const Key = "WordLocation";
} // end of namespace WordLocationCoderParameter

namespace FileIndexingType
{

//
//	CONST
//	Inverted::Hint::FileIndexingType::Key --
//		索引付けタイプを取得するためのキー
//
//	NOTES
//	ヒントから
//	索引付けタイプを取得するためのキー
//	"Dual"、"Ngram"、"Word" のいずれか
//
const char*	const Key = "Indexing";
} // end of namespace FileIndexingType

namespace Normalized
{

//
//	CONST
//	Inverted::Hint::Normalized::Key --
//		ノーマライズを行うかどうかを取得するためのキー
//
//	NOTES
//	ヒントから
//	ノーマライズを行うかどうかを取得するためのキー
//
const char*	const Key = "Normalized";
} // end of namespace Normalized

namespace Stemming
{
//
//	CONST
//	Inverted::Hint::Stemming::Key --
//		ステミングを行うかどうかを取得するためのキー
//
//	NOTES
//	ヒントから
//	ステミングを行うかどうかを取得するためのキー
//
const char*	const Key = "Stemming";
} // end of namespace Stemming
	
namespace DeleteSpace
{
//
//	CONST
//	Inverted::Hint::DeleteSpace::Key --
//	   	スペース除去を行うかどうかを取得するためのキー
//
//	NOTES
//	ヒントから
//	スペース除去を行うかどうかを取得するためのキー
//
const char*	const Key = "DeleteSpace";
} // end of namespace DeleteSpace

namespace Carriage
{
//
//	CONST
//	Inverted::Hint::Carriage::Key --
//	   	改行を跨った解析を行うかどうかを取得するためのキー
//
//	NOTES
//	ヒントから
//	改行を跨った解析を行うかどうかを取得するためのキー
//
const char*	const Key = "Carriage";
} // end of namespace Carriage
	
namespace Extractor
{
//
//	CONST
//	Inverted::Hint::Extractor::Key --
//	   	抽出パラメータ
//
//	NOTES
//	ヒントから
//	抽出パラメータを取得するためのキー
//
const char*	const Key = "Extractor";
} // end of namespace Extractor
	
namespace Language
{
//
//	CONST
//	Inverted::Hint::Language::Key --
//	   	デフォルト言語
//
//	NOTES
//	ヒントから
//	デフォルト言語を取得するためのキー
//
const char*	const Key = "Language";
} // end of namespace Language
	
namespace Distribute
{
//
//	CONST
//	Inverted::Hint::Distribute::Key --
//	   	ファイル分散数
//
//	NOTES
//	ヒントから
//	ファイル分散数を取得するためのキー
//
const char*	const Key = "Distribute";
} // end of namespace Extractor
	
namespace Nolocation
{

//
//	CONST
//	Inverted::Hint::Nolocation::Key --
//		The key for getting whether the location information is not stored
//
const char*	const Key = "Nolocation";
} // end of namespace Nolocation

namespace NoTF
{

//
//	CONST
//	Inverted::Hint::Nolocation::Key --
//		The key for getting whether the TF information is not stored
//
const char*	const Key = "NoTF";
} // end of namespace NoTF

namespace Clustered
{
//
//	CONST
//	Inverted::Hint::Clustered::Key --
//	   	クラスタ用の特徴語を抽出するかどうか
//
//	NOTES
//	ヒントから
//	クラスタ用の特徴語を抽出するかどうかを取得するためのキー
//
const char*	const Key = "Clustered";
} // end of namespace Clustered

namespace Feature
{
//
//	CONST
//	Inverted::Hint::Feature::Key --
//	   	抽出する特徴語の数
//
//	NOTES
//	ヒントから
//	抽出する特徴語の数を取得するためのキー
//
const char*	const Key = "Feature";
} // end of namespace Feature

} // end of namespace Hint

_SYDNEY_INVERTED_END
_SYDNEY_END

#endif // __SYDNEY_INVERTED_HINT_H

//
//	Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2008, 2009, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
