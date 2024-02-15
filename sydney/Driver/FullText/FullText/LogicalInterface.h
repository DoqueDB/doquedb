// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// LogicalInterface.h -- 論理ファイル
// 
// Copyright (c) 2003, 2004, 2005, 2006, 2008, 2009, 2011, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_FULLTEXT_LOGICALINTERFACE_H
#define __SYDNEY_FULLTEXT_LOGICALINTERFACE_H

#include "FullText/Module.h"
#include "FullText/FileID.h"
#include "FullText/OtherInformationFile.h"
#include "FullText/IndexFile.h"

#include "Inverted/SearchCapsule.h"
#include "Inverted/GetLocationCapsule.h"
#include "Inverted/SortParameter.h"
#include "Inverted/FileIDNumber.h"
#include "Inverted/FieldType.h"
#include "Inverted/OptionDataFile.h"
#include "Inverted/ModInvertedTypes.h"

#include "LogicalFile/File.h"

#include "PhysicalFile/DirectArea.h"

#include "Common/UnsignedIntegerArrayData.h"

#include "ModUnicodeString.h"
#include "ModLanguageSet.h"
#include "ModVector.h"

_SYDNEY_BEGIN
_SYDNEY_FULLTEXT_BEGIN

//
//	CLASS
//	FullText::LogicalInterface -- 論理ファイル
//
//	NOTES
//	全文索引に格納されるデータは以下のとおり。
//	* 転置索引
//		FullText::IndexFile, Inverted::InvertedFile を参照
//	* その他の情報
//		FullText::OtherInformationFile を参照
//
//	HOW TO USE OF KWIC
//	getSearchParameter()
//	 OpenOptionのContains->Option->KwicSize->Optionに、KWIC長を指定。
//	getProjectionParameter()
//	 取得フィールドに_SYDNEY::Inverted::FieldType::RoughKwicPositionを指定。
//	getProperty()
//	 プロパティを取得。(荒いKWIC長を含む)
//	get()
//	 荒いKWICの開始位置を取得。
//
class SYD_FULLTEXT_FUNCTION LogicalInterface
	: public LogicalFile::File,
	  public Inverted::OptionDataFile
{
public:
	//
	//  STRUCT
	//  FullText::LogicalInterface::PutValue
	//
	struct PutValue
	{
		enum Type
		{
			DOCUMENT = 0,
			LANGUAGE,
			SCORE
		};
	};
	
	// コンストラクタ
	LogicalInterface(const LogicalFile::FileID& cFileID_);
	// デストラクタ
	virtual ~LogicalInterface();

	// 実体である OS ファイルが存在するか調べる
	bool isAccessible(bool force = false) const;
	// マウントされているか調べる
	bool isMounted(const Trans::Transaction& trans) const;

	//
	//  Schema Information
	//

	// ファイルIDを得る
	const LogicalFile::FileID& getFileID() const;
	// 論理ファイルサイズを得る
	ModUInt64 getSize(const Trans::Transaction& cTrans_);
	// 論理ファイルに挿入されているタプル数を得る
	ModInt64 getCount() const;

	//
	//  Query Optimization
	//

	// 論理ファイルオープン時のオーバヘッドコストを得る
	double getOverhead() const;
	// ひとつのタプルを挿入or取得する際のプロセスコストを得る
	double getProcessCost() const;
	// オープンパラメータを得る
	bool getSearchParameter(const LogicalFile::TreeNodeInterface* pCondition_,
							LogicalFile::OpenOption& cOpenOption_) const;
	// プロジェクションオープンパラメータを得る
	bool getProjectionParameter(const LogicalFile::TreeNodeInterface* pNode_,
								LogicalFile::OpenOption& cOpenOption_) const;

	// 更新オープンパラメータを得る
	bool getUpdateParameter(const Common::IntegerArrayData& cUpdateFields_,
							LogicalFile::OpenOption& cOpenOption_) const;
	// ソート順パラメータを設定する
	bool getSortParameter(const LogicalFile::TreeNodeInterface* pNode_,
						  LogicalFile::OpenOption& cOpenOption_) const;
	// 取得数と取得位置を設定する
	bool getLimitParameter(const Common::IntegerArrayData& cSpec_,
						   LogicalFile::OpenOption& cOpenOption_) const;

	//
	//  Data Manipulation
	//

	// 論理ファイルを作成する
	const LogicalFile::FileID& create(const Trans::Transaction& cTransaction_);
	// 論理ファイルを破棄する
	void destroy(const Trans::Transaction& cTransaction_);

	// 論理ファイルをマウントする
	const LogicalFile::FileID& mount(const Trans::Transaction& cTransaction_);
	// 論理ファイルをアンマウントする
	const LogicalFile::FileID& unmount(const Trans::Transaction& cTransaction_);
	// 論理ファイルをフラッシュする
	void flush(const Trans::Transaction& cTransaction_);

	// 論理ファイルのバックアップを開始する
	void startBackup(const Trans::Transaction& cTransaction_,
					 const bool bRestorable_);
	// 論理ファイルのバックアップを終了する
	void endBackup(const Trans::Transaction& cTransaction_);

	// 論理ファイルを障害から回復する
	void recover(const Trans::Transaction& cTransaction_,
				 const Trans::TimeStamp& cPoint_);
	// ある時点に開始された読取専用トランザクションが
	// 参照する版を最新版とする
	void restore(const Trans::Transaction& cTransaction_,
				 const Trans::TimeStamp& cPoint_);

	// 整合性検査を行う
	void verify(const Trans::Transaction& cTransaction_,
				const Admin::Verification::Treatment::Value eTreatment_,
				Admin::Verification::Progress& cProgress_);

	// 論理ファイルをオープンする
	void open(const Trans::Transaction& cTransaction_,
			  const LogicalFile::OpenOption& cOption_);
	// 論理ファイルをクローズする
	void close();
	// データの取得を行なう
	bool get(Common::DataArrayData* pTuple_);
	// データの挿入を行なう
	void insert(Common::DataArrayData* pTuple_);
	// データの更新を行なう
	void update(const Common::DataArrayData* pKey_,
				Common::DataArrayData* pTuple_);
	// データの削除を行なう
	void expunge(const Common::DataArrayData* pKey_);
	// 検索条件を設定する
	void fetch(const Common::DataArrayData* pOption_);
	// 巻き戻しの位置を記録する
	void mark();
	// 巻き戻す
	void rewind();
	// 論理ファイルへのカーソルをリセットする
	void reset();
	// 自分自身との比較
	bool equals(const Common::Object* pOther_) const;
	// 同期を取る
	void sync(const Trans::Transaction& trans,
			  bool& incomplete, bool& modified);

	// プロパティを得る
	void getProperty(Common::DataArrayData* pKey_,
					 Common::DataArrayData* pValue_);

	//
	//  Utility
	//

	// ファイルを移動する
	void move(const Trans::Transaction& cTransaction_,
			  const Common::StringArrayData& cArea_);

	// ラッチが不要なオペレーションを返す
	Operation::Value getNoLatchOperation();

	// Capabilities of file driver
	Capability::Value getCapability();

	// 文字列表現を得る
	ModUnicodeString toString() const;

	//
	//  以下、独自のメソッド
	//

	// ファイルをattachする
	void attachFile();
	// ファイルをdetachする
	void detachFile();
	// ファイルがattachされているか
	bool isAttached() const;
	// ファイルがオープンされているか
	bool isOpened() const;

	//
	//  以下は Inverted::OptionDataFile 用の実装
	//

	// スコア調整用の値があるかどうか
	bool isModifierValue();
	// スコア調整用の値を得る
	bool getModifierValue(ModUInt32 uiRowID_, double& value_);

	// 特徴語データがあるかどうか
	bool isFeatureValue();
	// 特徴語データを得る
	bool getFeatureValue(ModUInt32 uiRowiD_,
						 Inverted::FeatureSetPointer& pFeatureSet_);
	
	// すべてのページをdetachする
	void detachAllPages();

private:

	// 取得フィールドを設定する
	void setGetField(const LogicalFile::OpenOption& cOpenOption_);
	// 更新フィールドを設定する
	void setPutField(const LogicalFile::OpenOption& cOpenOption_);
	bool getField(FieldMask::FieldType type);

	// OtherInformationファイルのフィールドを取得
	void getOtherInformationField();

	///////////////////////////////////////////////////////////////////////
	// Tupleを取得するメンバー関数へのポインタ
	bool (LogicalInterface::*m_pGetTuple)(Common::DataArrayData& cTuple_);

	///////////////////////////////////////////////////////////////////////
	// 次のメンバー関数で転置ライブラリの検索結果をtupleで取得する
	//
	// BitSetで結果を得る
	bool getByBitSet(Common::DataArrayData& cTuple_);
	// 検索の結果を得る
	bool getSearchResultTuple(Common::DataArrayData& cTuple_);
	// 単語取得の結果を得る
	bool getWordTuple(Common::DataArrayData& cTuple_);
	// 文書長を得る
	bool getLength(Common::DataArrayData& cTuple_);
	// SearchByBitSetで結果を得る
	bool getSearchByBitSet(Common::DataArrayData& cTuple_);
	////////////////////////////////////////////////////////////////////////

	// 転置に挿入できる形にコンバートする
	void convert(const Common::DataArrayData* pTuple_,
				 ModUInt32& uiTupleID_,
				 ModUnicodeString& cstrDocument_,
				 ModVector<ModLanguageSet>& vecLanguage_,
				 ModVector<ModSize>& vecSectionOffset_);
	// 全文データをコンバートする
	void convertDocumentData(const Common::Data* pData_,
							 ModUnicodeString& cstrDocument_,
							 ModVector<ModSize>& vecSectionOffset_);
	// 言語データをコンバートする
	void convertLanguageData(const Common::Data* pData_,
							 ModVector<ModLanguageSet>& vecLanguage_,
							 bool checkArray_ = true);
	// ROWIDをコンバートする
	void convertRowidData(const Common::Data* pData_,
						  ModUInt32& uiRowID_);

	// 拡張文書を登録する
	void insertExpandDocument(const Common::DataArrayData* pTuple_);

	// その他情報を挿入できる形に整える
	void makeInsertOtherInformationTuple(
		const ModVector<ModSize>& vecSectionOffset_,
		const Common::DataArrayData* pTuple_,
		const ModInvertedFeatureList& vecFeature_,
		const ModUnicodeString& cstrDocument_,
		Common::DataArrayData& cTuple);
	// その他情報を更新できる形に整える
	void makeUpdateOtherInformationTuple(
		const ModVector<ModSize>& vecSectionOffset_,
		const Common::DataArrayData* pTuple_,
		const ModInvertedFeatureList& vecFeature_,
		const ModUnicodeString& cstrDocument_,
		Common::DataArrayData& cTuple_,
		ModVector<int>& vecUpdateField);

	// セクション情報データを作成する
	Common::Data::Pointer makeSectionFieldData(
		const ModVector<ModSize>& vecSectionOffset_);

	// 特長語情報データを作成する
	Common::Data::Pointer makeFeatureFieldData(
		const ModInvertedFeatureList& vecFeature_);

	// ヒットしたセクションを求める
	bool hitSection(const Common::UnsignedIntegerArrayData& cSectionOffset_,
					const Inverted::GetLocationCapsule::ResultSet& cLocation_,
					Common::Data::Pointer pHitSection_);

	// 特徴語情報データを取得(タプル格納用)
	void getFeatureValue(ModUInt32 uiRowID_,
						 Common::Data::Pointer pData_);
	
	// KWICのサイズを取得
	ModSize getKwicSize(
		const LogicalFile::OpenOption& cOpenOption_) const;

	// 荒いKWICのためのマージを取得
	ModSize getKwicMarginScaleFactor() const;
	
	// 索引語位置やKWICサイズを調整する係数を取得
	double getAdjustFactor(ModUInt32 uiTupleID_,
						   ModSize& uiUnnormalizedCharLength_);
	
	// KWICの開始位置を計算
	ModSize getKwicPosition(
		const Inverted::GetLocationCapsule::ResultSet& vecResult_,
		ModSize uiTermCount_,
		ModSize uiKwicSize_) const;

	// 調整済みのKWICのサイズを取得
	ModSize getAdjustKwicSize(double dAdjustFactor_) const;
	
	// 荒いKWICの開始位置を調整
	ModSize adjustRoughKwicPosition(ModSize uiPosition_,
									double dAdjustFactor_,
									ModSize uiUnnormalizedCharLength_) const;

	// 検索結果絞り込み用のビットセットを取得
	const Common::BitSet* getBitSetForNarrowing(
		const LogicalFile::OpenOption& cOpenOption_) const;

	// 検索結果の取得開始位置を取得
	ModSize getGetPos();
	
	// クラスタIDを取得
	bool getClusterID(ModSize uiPos_);

	// fieldmask
	ModUInt32 m_cFieldMask;
	
	// ファイルID
	FileID m_cFileID;

	// トランザクション
	const Trans::Transaction* m_pTransaction;
	// オープンモード
	LogicalFile::OpenOption::OpenMode::Value m_eOpenMode;

	// 転置索引ファイル
	IndexFile* m_pIndexFile;
	// その他情報ファイル
	OtherInformationFile* m_pOtherFile;

	// 検索器
	Inverted::SearchCapsule* m_pSearchCapsule;
	Inverted::GetLocationCapsule* m_pGetLocationCapsule;

	// 検索結果
	Inverted::SearchCapsule::WordSet m_cWordSet;
	Inverted::SearchCapsule::ResultSet m_pSearchResult;
	ModVector<ModInt32> m_cCluster;		// ModVector<ModInt32>
	// 検索結果：wordlistを指定した場合、wordlistに指定された単語数が戻る
	ModSize m_sizeWordlist;
	const Inverted::SearchCapsule::WordSet* m_pWordSet;

	// 現在のカーソル
	ModSize  m_iGetPos;
	Inverted::SearchCapsule::WordSet::ConstIterator m_wordIter;

	// 取得フィールド
	ModVector<FieldMask::FieldType> m_vecGetField;

	// 更新フィールド
	ModVector<PutValue::Type> m_vecPutField;

	// ソートパラメータ
	Inverted::SortParameter::Value m_eSortParameter;

	// 現在の取得数
	int m_iGetCount;
	// 結果取得上限
	int m_iLimit;
	// 結果取得開始位置
	int m_iOffset;

	// 検索結果型
	ModUInt32 m_cResultType;
	// 絞り込むビットセット
	const Common::BitSet* m_pSearchByBitSet;
	// それのイテレータ
	Common::BitSet::ConstIterator m_bitsetIter;

	// スコア調整フィールドの取得カラム
	int m_iGetScoreField;
	// セクション情報の取得カラム
	int m_iGetSectionField;
	// 特徴語情報の取得カラム
	int m_iGetFeatureField;
	
	// KWIC長
	ModSize m_uiKwicSize;
	// 正規化前文字列長フィールドの取得カラム
	int m_iGetUnnormalizedCharLengthField;
};

_SYDNEY_FULLTEXT_END
_SYDNEY_END

#endif //__SYDNEY_FULLTEXT_LOGICALINTERFACE_H

//
//	Copyright (c) 2003, 2004, 2005, 2006, 2008, 2009, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
