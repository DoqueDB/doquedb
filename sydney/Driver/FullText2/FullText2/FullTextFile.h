// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// FullTextFile.h -- 全文ファイルをあらわすクラス
// 
// Copyright (c) 2010, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_FULLTEXT2_FULLTEXTFILE_H
#define __SYDNEY_FULLTEXT2_FULLTEXTFILE_H

#include "FullText2/Module.h"
#include "FullText2/InvertedFile.h"

#include "FullText2/FileID.h"
#include "FullText2/OpenOption.h"
#include "FullText2/ResultSet.h"
#include "FullText2/Tokenizer.h"

#include "Common/LargeVector.h"
#include "Common/DataArrayData.h"
#include "Common/IntegerArrayData.h"
#include "Common/StringArrayData.h"
#include "Common/WordData.h"

#include "LogicalFile/OpenOption.h"
#include "LogicalFile/TreeNodeInterface.h"

#include "Os/CriticalSection.h"
#include "Os/Path.h"

#include "Trans/Transaction.h"
#include "Trans/TimeStamp.h"

#include "ModLanguageSet.h"
#include "ModPair.h"
#include "ModUnicodeString.h"
#include "ModVector.h"

#include "ModInvertedCoder.h"

#include "ModNLP.h"

_SYDNEY_BEGIN

namespace Utility {
	class ModTerm;
}

_SYDNEY_FULLTEXT2_BEGIN

class IDVectorFile;
class InvertedSection;
class ListManager;
class SearchInformation;
class Query;

//
//	CLASS
//	FullText2::FullTextFile	-- 
//
//	NOTES
//	LogicalInterface ですべて実装してもいいが、
//	InvertedFile を継承したいので、別クラスとした
//
class FullTextFile : public InvertedFile
{
public:
	// コンストラクタ
	FullTextFile(FileID& cFileID);
	// デストラクタ
	virtual ~FullTextFile();

	// 実体である OS ファイルが存在するか調べる
	bool isAccessible(bool force = false) const;
	// マウントされているか調べる
	bool isMounted(const Trans::Transaction& trans) const;

	//
	//  Schema Information
	//

	// 論理ファイルサイズを得る
	ModUInt64 getSize();
	// 論理ファイルに挿入されているタプル数を得る
	// 条件が与えられている場合には、おおよその結果件数を返す
	ModInt64 getCount();

	//
	//  Query Optimization
	//

	// 論理ファイルオープン時のオーバヘッドコストを得る
	double getOverhead();
	// ひとつのタプルを挿入or取得する際のプロセスコストを得る
	double getProcessCost();

	//
	//  Data Manipulation
	//

	// 論理ファイルを作成する
	void create() {}
	// 論理ファイルを破棄する
	void destroy(const Trans::Transaction& cTransaction_);

	// 論理ファイルをマウントする
	void mount(const Trans::Transaction& cTransaction_);
	// 論理ファイルをアンマウントする
	void unmount(const Trans::Transaction& cTransaction_);
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
	bool get(Common::DataArrayData& cTuple_);
	// 取得時のイテレータをリセットする
	void reset();
	// データの挿入を行なう
	void insert(const Common::DataArrayData& cTuple_);
	// データの更新を行なう
	void update(const Common::DataArrayData& cKey_,
				const Common::DataArrayData& cTuple_);
	// データの削除を行なう
	void expunge(const Common::DataArrayData& cKey_);


	//
	//  Utility
	//

	// 同期を取る
	void sync(const Trans::Transaction& trans,
			  bool& incomplete, bool& modified);
	// ファイルを移動する
	void move(const Trans::Transaction& cTransaction_,
			  const Os::Path& cPath_);
	// プロパティを取得する
	void getProperty(Common::DataArrayData& cKey_,
					 Common::DataArrayData& cValue_);

	//
	//	_AutoAttachFileのためのインターフェース
	//

	// ファイルをアタッチする
	void attach(bool bVector_ = false);
	// ファイルをデタッチする
	void detach();
	// ファイルがアタッチされているか否か
	bool isAttached();

	//
	//	_AutoDeatchPageのためのインターフェース
	//

	// 版管理を利用しないトランザクションかどうか
	bool isNoVersion();
	// すべてのページの変更を確定する
	void flushAllPages();
	// すべてのページの変更を破棄する
	void recoverAllPages();

	//
	//	MergeDaemonのためのインターフェース
	//

	// オープンする
	void openForMerge(const Trans::Transaction& cTransaction_,
					  int iElement_);
	// クローズする
	void closeForMerge(bool success_);
	// 1つの転置リストをマージする - 続きがある場合はtrueを返す
	bool mergeList();

	//
	//	下位層のためのインターフェース
	//

	// 圧縮器を得る
	ModInvertedCoder* getIdCoder(const ModUnicodeString& cstrKey_);
	ModInvertedCoder* getFrequencyCoder(const ModUnicodeString& cstrKey_);
	ModInvertedCoder* getLengthCoder(const ModUnicodeString& cstrKey_);
	ModInvertedCoder* getLocationCoder(const ModUnicodeString& cstrKey_);

	// 最終文書IDを得る
	ModUInt32 getLastDocumentID();

	// トークナイザーを得る
	Tokenizer::AutoPointer getTokenizer();
	// トークナイザ—を返す(Tokenizer::AutoPointer経由での利用のみ)
	void pushTokenizer(Tokenizer* p_);
	
	// 質問処理器を得る
	Utility::ModTerm* getTerm(const ModUnicodeString& cExtractor_);
	// 質問処理器のリソースを得る
	const Utility::ModTermResource* getTermResource();

	// 索引タイプを得る
	IndexingType::Value getIndexingType()
		{ return m_cFileID.getIndexingType(); }
	// 正規化するかどうか
	bool isNormalized()
		{ return m_cFileID.isNormalized(); }
	// 位置情報を格納するかどうか
	bool isNoLocation()
		{ return m_cFileID.isNolocation(); }
	// TF値を格納するかどうか
	bool isNoTF()
		{ return m_cFileID.isNoTF(); }

	// バッチ用のメモリを消費した数
	void addBatchSize(ModSize uiSize_);

	// ListManagerを得る
	ListManager* getListManager() { return 0; }

	// LockNameを得る
	const Lock::FileName& getLockName() { return m_cFileID.getLockName(); }

	// 処理するキー番号を得る
	int getKeyNumber();
	// キー番号のInvertedSectionを得る
	InvertedSection* getInvertedSection(int key_)
		{ return m_vecpSection[key_]; }
	// 成功したキー番号を設定する
	void pushSuccessKeyNumber(int key_);

	// 転置ファイルで扱えるデータに変換する
	void convertData(int n_, const Common::DataArrayData& cTuple_,
					 ModVector<ModUnicodeString>& vecDocument_,
					 ModVector<ModLanguageSet>& vecLanguage_,
					 double& dblScore_);
	
private:
	// ファイルを作成する
	void substantiate();

	// オープンする(モードごと)
	void openForSearch(OpenOption& cOpenOption_);
	void openForRead(OpenOption& cOpenOption_);
	void openForUpdate(OpenOption& cOpenOption_);
	void openForBatch(OpenOption& cOpenOption_);

	// 検索に必要な情報を保存する
	void saveSearchInfo(OpenOption& cOpenOption_);
	// 検索準備を行う
	void prepareSearch();
	// 検索クラスを用意する
	Query* makeQuery(bool bScore_,
					 bool bTfList_,
					 bool bLocation_,
					 const ModUnicodeChar*& pTea_);

	// リスト管理クラスを得る
	void setListManager();
	// リスト管理クラスを破棄する
	void clearListManager();

	// ベクターファイルをattachする
	void attachVector();
	// 転置ファイルセクションをattachする
	void attachSection(int n_ = 0x7fffffff);

	// 圧縮器を設定する
	void setCoder();
	// 圧縮機を破棄する
	void unsetCoder();
	// UNA解析器にパラメータを設定する
	void makeUnaParameter(UNA::ModNlpAnalyzer::Parameters& p);

	// 拡張文書を設定する
	void pushExpandDocument(const Common::DataArrayData& cTuple_);

	// 取得するフィールドを設定する
	void setProjectionParameter(OpenOption& cOpenOption_);
	// ソートするフィールドを設定する
	void setSortParameter(OpenOption& cOpenOption_);
	// 更新するフィールドを設定する
	void setUpdateParameter(OpenOption& cOpenOption_);

	// 検索情報クラスを得る
	SearchInformation& getSearchInformation();

	// スコアが必要かどうか
	bool isScore() const { return m_bScore; }
	// TF値リストを取得するかどうか
	bool isTfList() const { return m_bTfList; }
	// 位置情報を利用するかどうか
	bool isLocation() const { return m_bLocation; }

	// 新しいDocIDを得る
	DocumentID getNewDocumentID();
	// ベクターファイルを作成する
	void createVector();
	// ベクターファイルに挿入する
	void insertVector(DocumentID uiDocID_, ModUInt32 uiRowID_);
	// ベクターファイルから削除する
	void expungeVector(DocumentID uiDocID_, ModUInt32 uiRowID_);

	// タプルデータからROWIDのデータを取り出す
	ModUInt32 getRowID(const Common::DataArrayData& cTuple_);

	// 文書データを変換する
	void convertString(const Common::Data* pData_,
					   ModVector<ModUnicodeString>& vecDocument_);
	// 言語データを変換する
	void convertLanguage(const Common::Data* pData_,
						 ModVector<ModLanguageSet>& vecLanguage_);
	// スコア調整値を変換する
	void convertScore(const Common::Data* pData_,
					  double& dblScore_);
	// ROWIDを変換する
	void convertRowID(const Common::Data* pData_,
					  ModUInt32& uiRowID_);

	// バッチインサート時に、必要ならマージする
	void checkAndMerge();

	// 検索結果を取得する
	bool getForSearch(Common::DataArrayData& cTuple_);
	// 検索しないで得られるフィールドの値を取得する
	bool getForRead(Common::DataArrayData& cTuple_);
	// 単語を取得する
	bool getForWord(Common::DataArrayData& cTuple_);
	// 整合性検査のためのequal検索の結果を取得する
	bool getForEqual(Common::DataArrayData& cTuple_);

	// フィールド指定をパースする
	void parseField(ModVector<int>& vecField_,
					const ModUnicodeChar*& p_);
	// スケールをパースする
	void parseScale(const ModVector<int>& vecField_,
					ModVector<Query::ScaleData>& vecScale_,
					const ModUnicodeChar*& p_);
	// 平均文書長をセットする
	void setAverageLength(const ModVector<int>& vecField_,
						  SearchInformation& cSearchInfo_,
						  const ModUnicodeChar*& p_);
	// 総文書数をセットする
	void setDocumentFrequency(const ModVector<int>& vecField_,
							  SearchInformation& cSearchInfo_,
							  const ModUnicodeChar*& p_);

	// トークナイザ—を解放する
	void clearTokenizer();
	// マルチスレッド関係の変数をクリアする
	void clearMP();

	// 排他制御用のクリティカルセクション
	Os::CriticalSection m_cLatch;

	// トランザクション
	const Trans::Transaction* m_pTransaction;
	// オープンモード
	LogicalFile::OpenOption::OpenMode::Value m_eOpenMode;
	// FIXモード
	Buffer::Page::FixMode::Value m_eFixMode;
	// バッチモードかどうか
	bool m_bBatch;
	// マージ対象の要素
	int m_iMergeElement;
	
	// ROWID->文書IDのベクターファイル
	mutable IDVectorFile* m_pRowIDVectorFile;
	// 文書ID->ROWIDのベクターファイル
	mutable IDVectorFile* m_pDocIDVectorFile;

	// 転置ファイルセクション
	mutable ModVector<InvertedSection*> m_vecpSection;
	// リスト管理クラス
	mutable ModVector<ListManager*> m_vecpListManager;

	// 圧縮器
	ModInvertedCoder* m_pIdCoder;
	ModInvertedCoder* m_pFrequencyCoder;
	ModInvertedCoder* m_pLengthCoder;
	ModInvertedCoder* m_pLocationCoder;
	ModInvertedCoder* m_pWordIdCoder;
	ModInvertedCoder* m_pWordFrequencyCoder;
	ModInvertedCoder* m_pWordLengthCoder;
	ModInvertedCoder* m_pWordLocationCoder;

	// トークナイザー
	ModVector<Tokenizer*> m_vecpTokenizer;

	// バッチ用のメモリ領域の消費サイズ
	ModInt64 m_lBatchSize;

	// 検索条件
	ModUnicodeString m_cstrCondition;
	// 検索タイプ
	OpenOption::Type::Value m_eSearchType;

	// 検索クラス
	Query* m_pQuery;
	// 検索結果クラス
	ResultSet* m_pResultSet;
	// 検索情報クラス(検索用)
	SearchInformation* m_pSearchInfo;
	
	// 整合性検査のための完全一致検索時に利用する
	ModUInt32 m_uiRowID;
	// 整合性検査のための完全一致検索かどうか
	bool m_bEqual;

	// スコアを取得する必要があるかどうか
	bool m_bScore;
	// TF値リストを取得するかどうか
	bool m_bTfList;
	// 位置情報を利用するかどうか
	bool m_bLocation;
	// 単語取得かどうか
	bool m_bGetWord;
	// クラスタリングを行うかどうか
	bool m_bCluster;
	// ビットセットによる取得かどうか
	bool m_bGetByBitSet;

	// 最初の検索かどうか
	bool m_bFirst;
	// 検索を実行したかどうか
	bool m_bSearch;
	// 検索の準備を実行したかどうか
	bool m_bPrepare;
	
	// セレクトリスト(リスト順が保存されている)
	ModVector<ResultSet::FieldData> m_vecGetField;

	// ソート条件
	OpenOption::SortParameter::Value m_eSortParameter;
	
	// LIMIT, OFFSET
	ModSize m_uiLimit;
	ModSize m_uiOffset;	// 0-base

	// 取得したカウント
	ModSize m_uiGetCount;

	// スコア合成関数
	AdjustMethod::Value m_eAdjustMethod;

	// クラスターの閾値
	float m_fClusteredLimit;
	// クラスターの重み
	ModVector<ModPair<int, float> > m_vecClusterScale;

	// スコア合成方法
	Query::CombineMethod::Value m_eCombineMethod;
	
	// KWICサイズ
	ModVector<ModPair<int, ModSize> > m_vecKwicSize;
	ModSize m_uiKwicMarginScaleFactor;

	// 更新対象のフィールド
	ModVector<int> m_vecUpdateField;
	// スコア調整フィールドのみの更新か
	bool m_bOnlyScoreField;

	// 関連語
	Common::LargeVector<Common::WordData> m_vecWord;
	// イテレータ
	Common::LargeVector<Common::WordData>::Iterator m_wordIte;

	// 検索語の数
	int m_iTermCount;

	// バッチインサート時の登録数
	int m_iBatchCount;

	// ビットセットによる絞り込み時の文書IDの配列
	Common::LargeVector<DocumentID>* m_pNarrowing;
	// ビットセットで絞った集合でのランキング検索時の文書IDのビットセット
	Common::BitSet* m_pRanking;
	// ビットセットで絞った集合でのランキング検索時の件数
	ModSize m_uiRankingDocumentCount;

	// 適合性フィードバックのためのデータ
	ModVector<ModUnicodeString> m_vecExpandDocument;
	ModVector<ModLanguageSet> m_vecExpandLanguage;

	// 位置情報を取得するフィールド
	ModVector<int> m_vecLocationField;

	// 次に処理するキー番号
	int m_iNextKeyNumber;
	// 挿入または削除に成功したキー
	ModVector<int> m_vecSuccessKeyNumber;
};

_SYDNEY_FULLTEXT2_END
_SYDNEY_END

#endif //__SYDNEY_FULLTEXT2_INVERTEDSECTION_H

//
//	Copyright (c) 2010, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
