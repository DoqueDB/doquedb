// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// OtherInformationFile.h -- 転置ファイル以外の情報を格納するファイル
// 
// Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_FULLTEXT2_OTHERINFORMATIONFILE_H
#define __SYDNEY_FULLTEXT2_OTHERINFORMATIONFILE_H

#include "FullText2/Module.h"
#include "FullText2/MultiFile.h"
#include "FullText2/FileID.h"
#include "FullText2/MultiVectorFile.h"
#include "FullText2/VariableFile.h"
#include "FullText2/FeatureSet.h"

#include "Common/DataArrayData.h"

_SYDNEY_BEGIN
_SYDNEY_FULLTEXT2_BEGIN

//	CLASS
//	FullText2::OtherInformationFile
//		-- 転置以外の情報を管理する
//
//	NOTES
//	その他のファイルに格納されるデータは以下の通り
//	キー：文書ID
//	バリュー：
//		正規化後文書長	(UnsignedInteger, 固定長)	※必須バリュー
//		正規化前文書長	(UnsignedInteger, 固定長)
//		ユニット番号	(UnsignedInteger, 固定長)
//		セクション情報	(UnsignedIntegerArray, 可変長)
//		スコア調整値	(Double, 固定長)
//		特徴語リスト	(Binary, 可変長)
//
//	正規化後文書長以外のバリューはオプションである
//	固定長部分は、FullText2::VectorFileを使って格納される
//	可変長部分は、FullText2::VariableFileを使って格納される
//
class OtherInformationFile : public MultiFile
{
	//
	//	ENUM
	//	FullText::OtherInformationFile::ValueType -- データタイプ
	//
	struct ValueType
	{
		enum Value
		{
			DocumentLength = 0,		// 正規化後文書長
			OriginalLength,			// 正規化前文書長
			UnitNumber,				// 挿入したユニット番号
			SectionInfo,			// セクション情報
			ScoreData,				// スコア調整値
			FeatureList,			// 特徴語リスト

			ValueNum
		};
	};

public:
	// コンストラクタ
	OtherInformationFile(FileID& cFileID_,
						 const Os::Path& cPath_,
						 bool bBatch_);
	// デストラクタ
	virtual ~OtherInformationFile();
	
	// ファイルに挿入されているタプル数を得る
	ModUInt32 getCount();
	
	// ファイルを作成する
	void create();

	// 整合性検査を行う
	void verify(const Trans::Transaction& cTransaction_,
				const Admin::Verification::Treatment::Value eTreatment_,
				Admin::Verification::Progress& cProgress_);

	// 移動する
	void move(const Trans::Transaction& cTransaction_,
			  const Os::Path& cPath_);

	// 実体である OS ファイルが存在するか調べる
	bool isAccessible(bool bForce_ = false) const
	{
		return m_pVectorFile->isAccessible(bForce_);
	}
	// マウントされているか調べる
	bool isMounted(const Trans::Transaction& trans) const
	{
		return m_pVectorFile->isMounted(trans);
	}

	// オープンする
	void open(const Trans::Transaction& cTransaction_,
			  Buffer::Page::FixMode::Value eFixMode_);
	// クローズする
	void close();

	// ページの更新を破棄する
	void recoverAllPages();
	// ページの更新を反映する
	void flushAllPages();

	// 挿入する
	//
	//【注意】
	// vecSize_ の合計値は getDocumentLength で取得する
	// vecOriginalSize_ は合計値しか取得できない(getOriginalLength)
	//
	void insert(ModUInt32 uiDocumentID_,			// キー
				const ModVector<ModSize>& vecSize_,
				const ModVector<ModSize>& vecOriginalSize_,
				ModInt32 iUnitNumber_,
				double dblScoreData_,
				const FeatureList& cFeatureList_);
	// 削除する
	void expunge(ModUInt32 uiDocumentID_);
	// 更新する
	void updateScoreData(ModUInt32 uiDocumentID_,
						 double dblScoreData_);
	void updateFeatureList(ModUInt32 uiDocumentID_,
						   const FeatureList& cFeatureList_);

	// オリジナルの文書長があるか
	bool isOriginalLength() const
		{ return m_pFieldPosition[ValueType::OriginalLength] != -1; }
	// スコア調整カラムがあるか
	bool isScoreData() const
		{ return m_pFieldPosition[ValueType::ScoreData] != -1; }
	// セクションサイズ情報があるか
	bool isSectionSize() const
		{ return m_pFieldPosition[ValueType::SectionInfo] != -1; }
	// 特徴語セットがあるか
	bool isFeatureSet() const
		{ return m_pFieldPosition[ValueType::FeatureList] != -1; }
	
	// 正規化後文書長を得る
	bool getDocumentLength(ModUInt32 uiDocumentID_,
						   ModUInt32& uiLength_);
	// 正規化前文書長を得る
	bool getOriginalLength(ModUInt32 uiDocumentID_,
						   ModUInt32& uiLength_);
	// 挿入したユニット番号を得る
	bool getUnitNumber(ModUInt32 uiDocumentID_,
					   ModInt32& iUnitNumber_);
	// スコア調整値を得る
	bool getScoreData(ModUInt32 uiDocumentID_,
					  double& dblScore_);
	// セクションサイズを得る
	bool getSectionSize(ModUInt32 uiDocumentID_,
						ModVector<ModSize>& vecSectionSize_);
	// 特徴語リストを得る
	bool getFeatureSet(ModUInt32 uiDocumentID_,
					   FeatureSetPointer& pFeatureSet_);

	// 最終文書IDを得る
	ModUInt32 getLastDocumentID();
	// 総文書長を得る
	ModUInt64 getTotalDocumentLength();
	
	// 入れ替える(マージ中になる)
	void flip();
	// マージ中かどうか
	bool isProceeding();
	// マージ中断に設定する
	void mergeCancel();
	// マージが中断したかどうか(確認後中断ではなくマージ中になる)
	bool isCanceled();
	// マージ終了に設定する
	void mergeDone();
	// エグゼキュータ側の小転置の要素番号を得る
	int getIndex();
	// エグゼキュータ側の小転置の最少文書IDを得る
	DocumentID getFirstDocumentID();
	// マージデーモン側の小転置の最少文書IDを得る
	DocumentID getMergeFirstDocumentID();

	// 大転置の最大文書IDを得る
	DocumentID getFullMaxID();
	// 小転置の最大文書IDを得る
	DocumentID getIns0MaxID();
	DocumentID getIns1MaxID();

	// 挿入に利用するユニット番号を得る
	ModInt32 getInsertUnit();
	// 挿入に利用するユニット番号を設定する
	void setInsertUnit(ModInt32 iUnitNumber_);
	// 1ユニットあたりの最大ファイルサイズを得る
	ModUInt64 getMaxFileSize();
	// 1ユニットあたりの最大ファイルサイズを更新する
	ModUInt64 updateMaxFileSize();

	// インスタンスをコピーする(OpenMPのため)
	OtherInformationFile* copy();
	
protected:
	// ロック名を得る
	const Lock::FileName& getLockName() const
		{ return m_cFileID.getLockName(); }

private:
	//
	//	STRUCT
	//	FullText2::OtherInformationFile::Header -- ヘッダー
	//
	struct Header
	{
		ModUInt64	m_ulTotalDocumentLength;	// 総文書長
		
		ModInt32	m_iIndex;			// エグゼキュータ側の小転置
		ModInt32	m_iProceeding;		// マージ中かどうか
										// 0: マージ中ではない
										// 1: マージ中、2: マージ中断
		
		ModUInt32	m_uiFullMaxID;		// 大転置の最大文書ID
		ModUInt32	m_uiIns0MinID;		// 小転置０の最小文書ID
		ModUInt32	m_uiIns0MaxID;		// 小転置０の最大文書ID
		ModUInt32	m_uiIns1MinID;		// 小転置１の最小文書ID
		ModUInt32	m_uiIns1MaxID;		// 小転置１の最大文書ID
		
		ModInt32	m_iInsertUnit;		// 挿入転置ユニット
		
		ModUInt64	m_ulMaxFileSize;	// 1ユニットあたりの最大ファイルサイズ

		ModUInt32	m_pUnitCount[1];	// 各ユニットごとの登録件数
	};

	// ファイルをattachする
	void attach(bool bBatch_);
	// ファイルをdetachする
	void detach();

	// ヘッダーを読む
	void readHeader();
	// ヘッダーを初期化する
	void initializeHeader();

	// ユニット番号を更新する
	void updateUnitNumber(ModUInt32 uiFromID_, ModUInt32 uiEndID_,
						  ModInt32 iUnitNumber_);

	// 整合性検査を開始する
	void startVerification(const Trans::Transaction& cTransaction_,
						   Admin::Verification::Treatment::Value eTreatment_,
						   Admin::Verification::Progress& cProgress_);
	// 整合性検査を終了する
	void endVerification();
	
	// ファイルID
	FileID& m_cFileID;
	// 親のパス
	Os::Path m_cParentPath;

	// ベクター
	MultiVectorFile* m_pVectorFile;
	// 可変長
	VariableFile* m_pVariableFile;

	// ヘッダー
	Header* m_pHeader;

	// ベクターファイルの各フィールドの型
	ModVector<Common::DataType::Type> m_vecVectorElements;
	ModVector<ValueType::Value> m_vecVariableElements;
	// フィールド位置
	int m_pFieldPosition[ValueType::ValueNum];

	// 遅延更新かどうか
	bool m_bDelay;

	// トランザクション
	const Trans::Transaction* m_pTransaction;
	// FIXモード
	Buffer::Page::FixMode::Value m_eFixMode;
};

_SYDNEY_FULLTEXT2_END
_SYDNEY_END

#endif //__SYDNEY_FULLTEXT2_OTHERINFORMATIONFILE_H

//
//	Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
