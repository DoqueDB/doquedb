// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// FileID.h -- LogicalFile::FileIDのラッパークラス
// 
// Copyright (c) 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_FULLTEXT2_FILEID_H
#define __SYDNEY_FULLTEXT2_FILEID_H

#include "FullText2/Module.h"
#include "FullText2/Types.h"

#include "LogicalFile/FileID.h"
#include "LogicalFile/OpenOption.h"
#include "LogicalFile/TreeNodeInterface.h"
#include "Common/IntegerArrayData.h"
#include "Lock/Name.h"
#include "Buffer/Page.h"
#include "Trans/Transaction.h"
#include "FileCommon/HintArray.h"

#include "ModUnicodeString.h"
#include "ModLanguageSet.h"

_SYDNEY_BEGIN
_SYDNEY_FULLTEXT2_BEGIN

//
//	TYPEDEF
//	FullText2::LogicalFileID --
//
//	NOTES
//	FullText2::FileIDが直接LogicalFile::FileIDを継承できないので、
//	このtypedefを間に挟む。VC6のバグ。
//
typedef LogicalFile::FileID LogicalFileID;

//
//	CLASS
//	FullText2::FileID -- 転置ファイルドライバーのFileID
//
//	NOTES
//	FullTextとの分担については、FullText::FileIDを参照
//	基本的にはhint句の中を処理
//
class FileID : public LogicalFileID
{
public:
	struct DataType
	{
		enum Value
		{
			// [NOTE] キーを追加する時は末尾に追加する。
			
			String,
			StringArray,
			Integer,
			IntegerArray,
			UnsignedInteger,
			UnsignedIntegerArray,
			Language,
			LanguageArray,
			Double,
			DoubleArray,
			Word,
			WordArray,
		};
	};

	// コンストラクタ(1)
	FileID();
	// コンストラクタ(2)
	FileID(const LogicalFile::FileID& cLogicalFileID_);
	// デストラクタ
	virtual ~FileID();

	// バージョンを確認する
	static bool checkVersion(const LogicalFile::FileID& cLogicalFileID_);
	// バージョンを得る
	int getVersion() const;

	// ファイルIDの内容を作成する
	void create();

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

	// キー数を得る
	int getKeyCount() const;
	// キーが配列か否か
	bool isKeyArray(int key_) const;
	
	// 異表記正規化ありか
	bool isNormalized() const;
	// ステミングありか
	bool isStemming() const;
	// スペースを除去するかどうか
	bool isDeleteSpace() const;
	// 改行を跨って解析を行うか
	bool isCarriage() const;
	// 位置情報を格納しないかどうか
	bool isNolocation() const;
	// TF値を格納しないかどうか
	bool isNoTF() const;
	// 削除フラグを利用するかどうか
	bool isExpungeFlag() const;
	// バキュームを利用するかどうか
	bool isVacuum() const;
	
	// パス名を得る
	const Os::Path& getPath() const;
	void setPath(const Os::Path& cPath_);

	// インデックスタイプ
	IndexingType::Value getIndexingType() const;
	// トークナイズパラメータ
	const ModUnicodeString& getTokenizeParameter() const;

	// UNAのリソース番号を得る
	ModUInt32 getResourceID() const;
	
	// 圧縮器
	ModUnicodeString getIdCoder() const;
	ModUnicodeString getFrequencyCoder() const;
	ModUnicodeString getLengthCoder() const;
	ModUnicodeString getLocationCoder() const;
	ModUnicodeString getWordIdCoder() const;
	ModUnicodeString getWordFrequencyCoder() const;
	ModUnicodeString getWordLengthCoder() const;
	ModUnicodeString getWordLocationCoder() const;

	// 抽出パラメータ
	const ModUnicodeString& getExtractor() const;
	// デフォルト言語を得る
	ModUnicodeString getDefaultLanguage() const;

	// 最大単語長を得る
	int getMaxWordLength() const;

	// ファイル分散を利用するかどうか
	bool isDistribute() const;
	// ファイル分散数を得る
	int getDistributeCount() const;

	// クラスタリングモードか否か
	bool isClustering() const;
	// 登録する特徴語数を得る
	int getFeatureSize() const;

	// 遅延更新かどうか
	bool isDelayed() const;
	// 遅延更新が同期か非同期か
	bool isSyncMerge() const;
	// セクション検索かどうか
	bool isSectionized() const;
	// 言語情報があるかどうか
	bool isLanguage() const;
	// スコア調整値があるかどうか
	bool isScoreField() const;
	// 粗いKWIC取得用のデータがあるかどうか
	bool isRoughKwic() const;

	// プロジェクションパラメータを設定する
	bool getProjectionParameter(const LogicalFile::TreeNodeInterface* pNode_,
								LogicalFile::OpenOption& cOpenOption_) const;
	// 更新パラメータを設定する
	bool getUpdateParameter(const Common::IntegerArrayData& cUpdateFields_,
							LogicalFile::OpenOption& cOpenOption_) const;
	// ソートパラメータを設定する
	bool getSortParameter(const LogicalFile::TreeNodeInterface* pNode_,
						  LogicalFile::OpenOption& cOpenOption_) const;

private:
	// 遅延更新を設定する
	void setDelayed(const ModUnicodeString& cstrHint_,
					const FileCommon::HintArray& cHintArray_);
	// セクション検索を設定する
	void setSectionized(const ModUnicodeString& cstrHint_,
						const FileCommon::HintArray& cHintArray_);
	// 粗いKWIC取得用のデータを格納するかを設定する
	void setRoughKwic(const ModUnicodeString& cstrHint_,
					  const FileCommon::HintArray& cHintArray_);
	// 転置部分を設定する
	void setInvertedParameter(const ModUnicodeString& cstrHint_,
							  const FileCommon::HintArray& cHintArray_);
	// 索引タイプを設定する
	void setIndexingType(const ModUnicodeString& cstrHint_,
						 const FileCommon::HintArray& cHintArray_);
	// トークナイズパラメータを設定する
	void setTokenizeParameter(const ModUnicodeString& cstrHint_,
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
					   const FileCommon::HintArray& cHintArray_);
	// No location information
	void setNolocation(const ModUnicodeString& cstrHint_,
					   const FileCommon::HintArray& cHintArray_);
	// No TF
	void setNoTF(const ModUnicodeString& cstrHint_,
				 const FileCommon::HintArray& cHintArray_);
	// 検索結果のクラスタリング
	void setClustered(const ModUnicodeString& cstrHint_, 
					  const FileCommon::HintArray& cHintArray_);
	// 最大単語長
	void setMaxWordLength(const ModUnicodeString& cstrHint_, 
						  const FileCommon::HintArray& cHintArray_);
	// 削除フラグを設定する
	void setExpungeFlag(const ModUnicodeString& cstrHint_,
						const FileCommon::HintArray& cHintArray_);
	
	// ヒントを解釈し、格納されている文字列を得る
	static bool readHint(const ModUnicodeString& cstrHint_,
						 const FileCommon::HintArray& cHintArray_,
						 const ModUnicodeString& cstrKey_,
						 ModUnicodeString& cstrValue_);

	// ヒントの整合性をチェックする
	bool verifyHint() const;

	// フィールドの型をチェックする
	bool check();
	// フィールドの型をチェックする
	bool checkFieldType(int n_, DataType::Value eType_) const;

	// 以下はFileIDの中にあるが、スピードを考え同じ値をメンバーとして持つもの

	// Fileのロック名
	mutable Lock::FileName m_cLockName;
	// パス名
	mutable Os::Path m_cPath;

	// 抽出パラメータ
	mutable ModUnicodeString m_cExtractor;
	// トークナイズパラメータ
	mutable ModUnicodeString m_cTokenizeParameter;
};

_SYDNEY_FULLTEXT2_END
_SYDNEY_END

#endif //__SYDNEY_FULLTEXT2_FILEID_H

//
//	Copyright (c) 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
