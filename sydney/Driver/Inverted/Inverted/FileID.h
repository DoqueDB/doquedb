// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// FileID.h -- LogicalFile::FileIDのラッパークラス
// 
// Copyright (c) 2002, 2003, 2004, 2005, 2008, 2009, 2011, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_INVERTED_FILEID_H
#define __SYDNEY_INVERTED_FILEID_H

#include "Inverted/Module.h"
#include "LogicalFile/FileID.h"
#include "Lock/Name.h"
#include "Buffer/Page.h"
#include "Trans/Transaction.h"
#include "FileCommon/HintArray.h"

#include "ModInvertedTypes.h"
#include "ModCharString.h"
#include "ModUnicodeString.h"
#include "ModLanguageSet.h"

_SYDNEY_BEGIN
_SYDNEY_INVERTED_BEGIN

//
//	TYPEDEF
//	Inverted::LogicalFileID --
//
//	NOTES
//	Inverted::FileIDが直接LogicalFile::FileIDを継承できないので、
//	このtypedefを間に挟む。VC6のバグ。
//
typedef LogicalFile::FileID LogicalFileID;

//
//	CLASS
//	Inverted::FileID -- 転置ファイルドライバーのFileID
//
//	NOTES
//	FullTextとの分担については、FullText::FileIDを参照
//	基本的にはhint句の中を処理
//
class FileID : public LogicalFileID
{
public:
	struct KeyID
	{
		enum Value
		{
			// [NOTE] キーを追加する時は末尾に追加する。
			//  キーはこの順番で永続化されるため。
			
			// リーフページのページサイズ(Integer)
			LeafPageSize = LogicalFile::FileID::DriverNumber::Inverted,
			// オーバーフローページのページサイズ(Integer)
			OverflowPageSize,

			// インデックスタイプ(Integer)
			IndexingType,
			// トークナイズパラメータ(String)
			TokenizeParameter,
			// 異表記正規化
			Normalized,

			// 文書ID圧縮器(String)
			IdCoder,
			// 頻度圧縮器(String)
			FrequencyCoder,
			// ビット長圧縮器(String)
			LengthCoder,
			// 位置情報圧縮器(String)
			LocationCoder,
			// 文書ID圧縮器 - Dual時セパレータ用(String)
			WordIdCoder,
			// 頻度圧縮器 - Dual時セパレータ用(String)
			WordFrequencyCoder,
			// ビット長圧縮器 - Dual時セパレータ用(String)
			WordLengthCoder,
			// 位置情報圧縮器 - Dual時セパレータ用(String)
			WordLocationCoder,

			// ステミング
			Stemming,
			// スペース処理モード
			SpaceMode,

			// 抽出パラメータ(for libTerm)
			Extractor,
			// デフォルト言語指定
			Language,

			// B木ファイルのページサイズ(Integer)
			BtreePageSize,

			// ファイル分散数(Integer)
			Distribute,

			// clusteredキーワード
			Clustered,
			// 特徴数を指定するキーワード
			Feature,

			// 改行を跨って正規化を行うかどうか
			Carriage,

			// Isn't location information stored?
			Nolocation,
			// Isn't TF information stored?
			NoTF,

			// size of rough kwic
			RoughKwicSize
		};
	};
	FileID(){}
	// コンストラクタ
	FileID(const LogicalFile::FileID& cLogicalFileID_);
	// デストラクタ
	virtual ~FileID();

	// ファイルIDの内容を作成する
	void create(bool bBigInverted = false);

	// LeafPageのページサイズ
	int getLeafPageSize() const;
	// OverflowPageのページサイズ
	int getOverflowPageSize() const;
	// BtreePageのページサイズ
	int getBtreePageSize() const;
	// その他のページサイズ
	int getPageSize() const;

	// LockNameを得る
	const Lock::FileName& getLockName() const;

	// 読み取り専用か
	bool isReadOnly() const;
	// 一時か
	bool isTemporary() const;
	// マウントされているか
	bool isMounted() const;
	void setMounted(bool bFlag_);
	// 異表記正規化ありか
	bool isNormalized() const;
	SYD_INVERTED_FUNCTION
	static bool isNormalized(const ModUnicodeString& cstrHint_);
	// ステミングありか
	bool isStemming() const;
	// Isn't location information stored?
	bool isNolocation() const;
	static bool isNolocation(const LogicalFile::FileID& cFileID_);
	// Isn't TF information stored?
	bool isNoTF() const;
	// スペース処理モードを得る
	ModInvertedUnaSpaceMode getSpaceMode() const;
	// 改行を跨って解析を行うか
	bool isCarriage() const;
	
	// パス名を得る
	const Os::Path& getPath() const;
	void setPath(const Os::Path& cPath_);

	// インデックスタイプ
	ModInvertedFileIndexingType getIndexingType() const;
	SYD_INVERTED_FUNCTION
	static ModInvertedFileIndexingType getIndexingType(
		const LogicalFile::FileID& cFileID_);
	SYD_INVERTED_FUNCTION
	static ModInvertedFileIndexingType getIndexingType(
		const ModUnicodeString& cstrHint_);
	// トークナイズパラメータ
	const ModCharString& getTokenizeParameter() const;

	// UNAのリソース番号を得る
	static ModUInt32 getResourceID(const LogicalFile::FileID& cFileID_);
	
	// 圧縮器
	const ModCharString& getIdCoder() const;
	const ModCharString& getFrequencyCoder() const;
	const ModCharString& getLengthCoder() const;
	const ModCharString& getLocationCoder() const;
	const ModCharString& getWordIdCoder() const;
	const ModCharString& getWordFrequencyCoder() const;
	const ModCharString& getWordLengthCoder() const;
	const ModCharString& getWordLocationCoder() const;

	// 抽出パラメータ
	const ModUnicodeString& getExtractor() const;

	// デフォルト言語を得る
	const ModLanguageSet& getDefaultLanguageSet() const;
	const ModUnicodeString& getDefaultLanguageSetName() const;
	static ModUnicodeString getDefaultLanguageSetName(
		const LogicalFile::FileID& cFileID_);

	// ファイル分散を利用するかどうか
	bool isDistribution() const;
	// ファイル分散数を得る
	int getDistribute() const;
#ifdef SYD_CLUSTERING
	// クラスタリングモードか否か
	bool isClustering() const;
	static bool isClustering(const LogicalFile::FileID& cFileID_);
	SYD_INVERTED_FUNCTION
	static bool isClustered(const ModUnicodeString& cstrHint_);
	// 登録する特徴語数を得る
	int getFeatureSize() const;
#endif

private:
	// トークナイズパラメータを設定する
	void setTokenizeParameter(const ModUnicodeString& cstrHint_,
							  const FileCommon::HintArray& cHintArray_);
	// 索引タイプを設定する
	void setIndexingType(const ModUnicodeString& cstrHint_,
						 const FileCommon::HintArray& cHintArray_);
	// Get an indeixng type
	static ModInvertedFileIndexingType getIndexingType(
		const ModUnicodeString& cstrHint_,
		const FileCommon::HintArray& cHintArray_);
	// 異表記正規化
	void setNormalized(const ModUnicodeString& cstrHint_,
					   const FileCommon::HintArray& cHintArray_);
	// 圧縮器
	void setCoderParameter(const ModUnicodeString& cstrHint_,
						   const FileCommon::HintArray& cHintArray_);
	// 抽出パラメータ
	void setExtractor(const ModUnicodeString& cstrHint_,
					  const FileCommon::HintArray& cHintArray_);
	// デフォルト言語
	void setLanguage(const ModUnicodeString& cstrHint_,
					 const FileCommon::HintArray& cHintArray_);
	// ファイル分散
	void setDistribute(const ModUnicodeString& cstrHint_,
					   const FileCommon::HintArray& cHintArray_,
					   bool bBigInverted_);
	// No location information
	void setNolocation(const ModUnicodeString& cstrHint_,
					   const FileCommon::HintArray& cHintArray_);
	// No TF
	void setNoTF(const ModUnicodeString& cstrHint_,
				 const FileCommon::HintArray& cHintArray_);

#ifdef SYD_CLUSTERING
	// 検索結果のクラスタリング
	void setClustered(const ModUnicodeString& cstrHint_, 
					  const FileCommon::HintArray& cHintArray_);
#endif
	// ヒントを解釈し、格納されている文字列を得る
	static bool readHint(const ModUnicodeString& cstrHint_,
						 const FileCommon::HintArray& cHintArray_,
						 const char* const pKey_, ModUnicodeString& cstrValue_);

	// ヒントの整合性をチェックする
	bool verifyHint() const;

	// 以下はFileIDの中にあるが、スピードを考え同じ値をメンバーとして持つもの

	// Fileのロック名
	mutable Lock::FileName m_cLockName;
	// パス名
	mutable Os::Path m_cPath;
	// デフォルト言語指定
	mutable bool m_bLoadLanguage;
	mutable ModLanguageSet m_cLanguageSet;
	mutable ModUnicodeString m_cLanguageSetName;

	// 抽出パラメータ
	mutable ModUnicodeString m_cExtractor;

	// トークナイズパラメータ
	mutable ModCharString m_cTokenizeParameter;
	// 各種圧縮器
	mutable ModCharString m_cIdCoder;
	mutable ModCharString m_cFrequencyCoder;
	mutable ModCharString m_cLengthCoder;
	mutable ModCharString m_cLocationCoder;
	mutable ModCharString m_cWordIdCoder;
	mutable ModCharString m_cWordFrequencyCoder;
	mutable ModCharString m_cWordLengthCoder;
	mutable ModCharString m_cWordLocationCoder;
};

_SYDNEY_INVERTED_END
_SYDNEY_END

#endif //__SYDNEY_INVERTED_FILEID_H

//
//	Copyright (c) 2002, 2003, 2004, 2005, 2008, 2009, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
