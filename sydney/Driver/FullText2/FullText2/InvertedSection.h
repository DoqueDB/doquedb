// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// InvertedSection.h -- １つのカラム用の転置ファイルをあらわすクラス
// 
// Copyright (c) 2010, 2011, 2014, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_FULLTEXT2_INVERTEDSECTION_H
#define __SYDNEY_FULLTEXT2_INVERTEDSECTION_H

#include "FullText2/Module.h"
#include "FullText2/InvertedFile.h"

#include "FullText2/FeatureSet.h"
#include "FullText2/FileID.h"
#include "FullText2/FullTextFile.h"
#include "FullText2/Tokenizer.h"

#include "Common/LargeVector.h"
#include "Buffer/Page.h"
#include "Os/CriticalSection.h"
#include "Os/Path.h"
#include "Trans/Transaction.h"
#include "Trans/TimeStamp.h"

#include "ModVector.h"

_SYDNEY_BEGIN
_SYDNEY_FULLTEXT2_BEGIN

class IDVectorFile;
class InvertedBatch;
class InvertedExpungeBatch;
class InvertedExpungeUnit;
class InvertedList;
class InvertedMultiUnit;
class InvertedUnit;
class InvertedUpdateFile;
class SearchInformation;
class OtherInformationFile;

//
//	CLASS
//	FullText2::InvertedSection
//		-- １つのカラムを構成する転置の単位
//
//	NOTES
//	旧全文索引の主な機能はこのクラス内に保持している
//
class InvertedSection : public InvertedFile
{
public:
	// コンストラクタ
	InvertedSection(FullTextFile& cFile_,
					const Os::Path& cPath_,
					bool bBatch_);
	// デストラクタ
	virtual ~InvertedSection();

	// カウントを得る
	ModUInt32 getCount();

	// オープンする
	void open(const Trans::Transaction& cTransaction_,
			  Buffer::Page::FixMode::Value eFixMode_);
	// クローズする
	void close();

	// ファイルを作成する
	void create() {}
	// ファイルを削除する
//	void destroy(const Trans::Transaction& cTransaction_);

	// マウントする
//	void mount(const Trans::Transaction& cTransaction_);
	// アンマウントする
//	void unmount(const Trans::Transaction& cTransaction_);

	// フラッシュする
//	void flush(const Trans::Transaction& cTransaction_);

	// バックアップ開始を宣言する
//	void startBackup(const Trans::Transaction& cTransaction_,
//					 const bool bRestorable_);
	// バックアップ終了を宣言する
//	void endBackup(const Trans::Transaction& cTransaction_);

	// リカバリーする
//	void recover(const Trans::Transaction& cTransaction_,
//				 const Trans::TimeStamp& cPoint_);
	// リストアする
//	void restore(const Trans::Transaction& cTransaction_,
//				 const Trans::TimeStamp& cPoint_);

	// 同期処理を行う
//	void sync(const Trans::Transaction& cTransaction_,
//			  bool& incomplete, bool& modified);

	// ファイルを移動する
	void move(const Trans::Transaction& cTransaction_,
			  const Os::Path& cPath_);

	// 整合性検査を行う
	void verify(const Trans::Transaction& cTransaction_,
				const Admin::Verification::Treatment::Value eTreatment_,
				Admin::Verification::Progress& cProgress_);

	// ページをflushする
//	void flushAllPages();
	// ページを破棄する
//	void recoverAllPages();

	// 文書を挿入する - マージが必要な場合は true を返す
	bool insert(const ModVector<ModUnicodeString>& vecDocument_,
				const ModVector<ModLanguageSet>& vecLanguage_,
				DocumentID uiDocumentID_,
				double dblScoreData_);
	// 文書を削除する
	void expunge(const ModVector<ModUnicodeString>& vecDocument_,
				 const ModVector<ModLanguageSet>& vecLanguage_,
				 DocumentID uiDocumentID_);
	// 文書削除を確定する - マージが必要な場合は true を返す
	bool expungeCommit();
	// 文書削除をロールバックする
	void expungeRollBack(const ModVector<ModUnicodeString>& vecDocument_,
						 const ModVector<ModLanguageSet>& vecLanguage_,
						 DocumentID uiDocumentID_);

	// スコア調整カラムのみを更新する
	void updateScoreData(DocumentID uiDocumentID_,
						 double dblScoreData_);
	// 特徴語リストのみを更新する
	void updateFeatureList(const ModVector<ModUnicodeString>& vecDocument_,
						   const ModVector<ModLanguageSet>& vecLanguage_,
						   DocumentID uiDocumentID_);

	// 与えられたデータが格納されているか確認する
	bool check(const ModVector<ModUnicodeString>& vecDocument_,
			   const ModVector<ModLanguageSet>& vecLanguage_,
			   DocumentID uiDocumentID_);

	// 小転置をマージする - 通常のオープンで行う
	void syncMerge(bool bNoException_ = false);

	// マージのためにオープンする
	void openForMerge(const Trans::Transaction& cTransaction_);
	// マージのためのクローズ
	void closeForMerge(bool success_);
	// １つの転置リストをマージする - 続きがある場合はtrueを返す
	bool mergeList();
	
	// マウントされているか
	bool isMounted(const Trans::Transaction& cTransaction_) const;
	// ファイルが存在するか
	bool isAccessible(bool force_ = false) const;

	// 削除リストを得る
	const Common::LargeVector<DocumentID>& getExpungeList();

	// 圧縮器を得る
	ModInvertedCoder* getIdCoder(const ModUnicodeString& cstrKey_)
		{ return m_cFile.getIdCoder(cstrKey_); }
	ModInvertedCoder* getFrequencyCoder(const ModUnicodeString& cstrKey_)
		{ return m_cFile.getFrequencyCoder(cstrKey_); }
	ModInvertedCoder* getLengthCoder(const ModUnicodeString& cstrKey_)
		{ return m_cFile.getLengthCoder(cstrKey_); }
	ModInvertedCoder* getLocationCoder(const ModUnicodeString& cstrKey_)
		{ return m_cFile.getLocationCoder(cstrKey_); }

	// 全文索引ファイルを得る
	FullTextFile& getFile() { return m_cFile; }

	// 検索用のListManagerを得る
	ListManager* getListManager();
	// 検索情報クラスを得る
	SearchInformation* getSearchInformation();

	// 分散しているかどうか
	bool isDistribute() { return m_cFileID.isDistribute(); }
	// 分散している場合、その数
	int getDistributeCount() { return m_cFileID.getDistributeCount(); }

private:
	// マージ中に必要なデータ
	struct MergeData
	{
		MergeData(int iUnitCount_)
			: m_iUnit(0), m_iUnitCount(iUnitCount_),
			  m_bInsDone(false), m_bExpDone(false) {}
		
		// 次に参照する索引単位
		ModUnicodeString m_cKey;
		
		// 今削除しているユニット番号
		int m_iUnit;
		// 全ユニット数
		int m_iUnitCount;

		// 終わったかどうか
		bool m_bInsDone;
		bool m_bExpDone;
	};

	// 転置リスト部分をロールバックする
	void expungeRollBack(SmartLocationListMap& cResult_,
						 DocumentID uiDocumentID_);

	// 必要なファイルをattachする
	void attach(bool bBatch_);
	// デタッチする
	void detach();

	// ファイルを作成する
	void substantiate();

	// マージが必要かどうか
	bool isNeedMerge();
	// 必要ならユニットを変更する
	void changeUnit();

	// トークナイズ結果を転置ファイルユニットに挿入する
	void insertLocationList(InvertedUpdateFile* pInvertedFile_,
							DocumentID uiDocumentID_,
							SmartLocationListMap& cResult_);
	// トークナイズ結果を転置ファイルユニットから削除する
	void expungeLocationList(InvertedUpdateFile* pInvertedFile_,
							 DocumentID uiDocumentID_,
							 SmartLocationListMap& cResult_);

	// １つの転置リストをマージする
	void merge(InvertedList* pInsertList_, InvertedList* pExpungeList_);
	// １つの転置ファイルをマージする
	void mergeFile(InvertedUpdateFile* pInsertFile_,
				   InvertedUpdateFile* pExpungeFile_);
	// 削除用小転置の1つの転置リストをマージする
	bool mergeExpunge(InvertedList* pExpungeList_,
					  InvertedUnit* pUnitFile_,
					  int iUnit_);
	// ユニット単位に削除する
	void mergeExpunge(InvertedUpdateFile* pExpungeFile_,
					  int iUnit_);
	// １つの削除転置リストを１つのユニットにマージする
	bool mergeExpungeList();

	// 挿入用の転置ファイルユニットを得る
	InvertedUpdateFile* getInsertFile(int& iUnitNumber_);
	// 削除用の転置ファイルユニットを得る
	InvertedUpdateFile* getExpungeFile(DocumentID uiDocumentID_);

	// 引数の文書IDの文書が格納されている転置ファイルユニットを得る
	// 戻り値が小転置のことや、バッチ用ファイルのこともある
	InvertedUpdateFile* getInsertedFile(DocumentID uiDocumentID_);

	// エグゼキュータ側の挿入用ファイルユニットを得る
	InvertedUpdateFile*	getInsertFile();
	// エグゼキュータ側の削除用ファイルユニットを得る
	InvertedUpdateFile*	getExpungeFile();
	// マージデーモン側の挿入用ファイルユニットを得る
	InvertedUpdateFile* getMergeInsertFile();
	// マージデーモン側の削除用ファイルユニットを得る
	InvertedUpdateFile* getMergeExpungeFile();

	// 同期マージか否か
	bool isSyncMerge();

	// トランザクション
	const Trans::Transaction* m_pTransaction;
	// オープンモード
	Buffer::Page::FixMode::Value m_eFixMode;
	
	// 全文ファイル
	FullTextFile& m_cFile;

	// 大転置
	InvertedMultiUnit* m_pFull;

	// 挿入用小転置
	InvertedUnit* m_pInsert0;
	InvertedUnit* m_pInsert1;

	// 削除用小転置
	InvertedExpungeUnit* m_pExpunge0;
	InvertedExpungeUnit* m_pExpunge1;

	// その他の情報ファイル
	OtherInformationFile* m_pOtherFile;

	// 削除フラグファイル
	IDVectorFile* m_pExpungeFlag;

	// 削除リスト
	Common::LargeVector<DocumentID>* m_pExpungeIDs;

	// バッチ挿入用転置
	InvertedBatch* m_pBatchInsert;
	// バッチ削除用転置
	InvertedExpungeBatch* m_pBatchExpunge;

	// バッチモードか否か
	bool m_bBatch;

	// マージ中データ
	MergeData* m_pMergeData;

	// 削除用小転置に挿入したときの文書ID(削除のロールバックで利用する)
	DocumentID m_uiExpungeSmallID;
	// 削除に利用した転置
	InvertedUpdateFile* m_pExpungeFile;

	// 以下は削除をロールバックする時に一時的に利用する

	// スコア調整カラムの値
	double m_Undo_dblScoreData;
	// ユニット番号
	int m_Undo_iUnitNumber;
};

_SYDNEY_FULLTEXT2_END
_SYDNEY_END

#endif //__SYDNEY_FULLTEXT2_INVERTEDSECTION_H

//
//	Copyright (c) 2010, 2011, 2014, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
