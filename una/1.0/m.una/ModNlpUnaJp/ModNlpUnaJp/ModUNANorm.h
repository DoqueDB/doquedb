// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:	
//
// ModUNANorm.h -- UNA ラッパークラス（表記正規化機能付き）の定義
// 
// Copyright (c) 2000-2005, 2023 Ricoh Company, Ltd.
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
#ifndef	__ModUNANorm_H__
#define __ModUNANorm_H__

#include "ModUNA.h"
#include "ModNormDLL.h"
#include "Module.h"
_UNA_BEGIN
_UNA_UNAJP_BEGIN

class ModUnaDLL ModUnaNorm;
class ModUnaDLL ModUnaNormAnalyzer;

class ModEnglishWordStemmer;
class ModNormalizer;

//
// 【注意】
// libUna に以下のクラスを含めると、libNormalizer と libUna が相互参照
// してしまうので、libNormalizer に含めることとした。
//

//
// CLASS
// ModUNANorm -- UNA クラス（表記正規化機能付き）
//
// NOTES
// 表記正規化機能付きの UNA 解析器の生成を支援するクラス。
//
class ModNormDLL ModUnaNorm : public ModUna
{
public:
	static ModUnaNormAnalyzer* createNorm();
	static ModUnaNormAnalyzer* createNorm(const ModBoolean);
	static ModUnaNormAnalyzer* createNorm(ModNormalizer* const,
										  ModEnglishWordStemmer* const);

private:
	ModUnaNorm();					// オブジェクトは作らない
};

//
// CLASS
// ModUnaNormAnalyzer -- UNA 解析器クラス（表記正規化機能付き）
//
// NOTES
// 表記正規化機能付きの UNA の解析機能を提供するラッパークラス。
//
class ModNormDLL ModUnaNormAnalyzer : public ModUnaAnalyzer
{
public:
	ModUnaNormAnalyzer(const ModUnaResource* const resource_,
					   const ModBoolean = ModTrue);
	ModUnaNormAnalyzer(const ModUnaResource* const resource_,
					   ModNormalizer* const,
					   ModEnglishWordStemmer* const);

	void setNormalizer(ModNormalizer* const normalizer_) {
		normalizer = normalizer_; }
	void setStemmer(ModEnglishWordStemmer* const stemmer_) {
		stemmer = stemmer_; }

	virtual ModBoolean getMorph(ModUnicodeString& result,
								const ModBoolean normalize);

	ModBoolean getNormalized(ModUnicodeString& result);
	ModBoolean getNormalized(ModUnicodeString& result,
							 ModNormalizer* const,
							 ModEnglishWordStemmer* const);

	ModBoolean getBlockNormalized(ModUnicodeString&,
								  const ModUnicodeChar separator1,
								  const ModUnicodeChar separator2);
	ModBoolean getBlockNormalized(ModUnicodeString&,
								  const ModUnicodeChar separator1,
								  const ModUnicodeChar separator2,
								  ModNormalizer* const,
								  ModEnglishWordStemmer* const);

private:
	ModBoolean getSimpleNormalized(ModUnicodeString& result);
	ModBoolean getCheckNormalized(ModUnicodeString& result);
	ModBoolean getCheck(ModUnicodeString& result);

	ModBoolean getBlockSimple(ModUnicodeString&,
							  const ModUnicodeChar separator1,
							  const ModUnicodeChar separator2);
	ModBoolean getBlockCheck(ModUnicodeString&,
							 const ModUnicodeChar separator1,
							 const ModUnicodeChar separator2);

	ModNormalizer* /*const*/ normalizer;
	ModEnglishWordStemmer* /*const*/ stemmer;
	ModUInt32 tokenType;
};

//
// FUNCTION
// ModUnaNormAnalyzer::getMorph -- 形態素解析結果の取得
//
// NOTES
// 形態素解析結果を取得する
//
// ARGUMENTS
// ModUnicodeString& data_
//		解析結果
// const ModBoolean normalize_
//		正規化指示
//
// RETURN
// 解析結果が残っていれば ModTrue、そうでなければ ModFalse
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
inline ModBoolean
ModUnaNormAnalyzer::getMorph(ModUnicodeString& data_,
							 const ModBoolean normalize_)
{
	if (normalize_ == ModTrue) {
		return getNormalized(data_);
	}
	if (mode == 0) {
		// 下位構造にばらさない -- ModUnaAnalyzer の関数を呼び出す
		return get(data_);
	}
	// 下位構造にばらす
	return getCheck(data_);
}
_UNA_UNAJP_END
_UNA_END

#endif // __ModUNANorm_H__
//
// Copyright (c) 2000-2005, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
