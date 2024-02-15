// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// InvertedFile.h --
// 
// Copyright (c) 2002, 2003, 2004, 2005, 2008, 2009, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_INVERTED_INVERTEDFILE_H
#define __SYDNEY_INVERTED_INVERTEDFILE_H

#include "Inverted/Module.h"
#include "Inverted/FileID.h"
#include "Common/Object.h"
#include "PhysicalFile/Page.h"
#include "LogicalFile/FileID.h"
#include "LogicalFile/OpenOption.h"
#include "LogicalFile/TreeNodeInterface.h"

#include "ModUnicodeString.h"
#include "ModInvertedFile.h"
#include "ModLanguageSet.h"
#include "ModPair.h"
#include "ModVector.h"
#include "ModOstream.h"

class ModInvertedCoder;
class ModTerm;

_SYDNEY_BEGIN
_SYDNEY_INVERTED_BEGIN

class DocumentIDVectorFile;
class RowIDVectorFile;
class RowIDVectorFile2;

class InvertedList;
class ListManager;
class BatchBaseList;
class BatchListMap;
class LocationListMap;
class InvertedUnit;
class File;

//
//	CLASS
//	Inverted::InvertedFile -- 転置ファイルをあらわすクラス
//
//	NOTES
//	転置索引に格納されるデータは以下の通り。
//	* 索引語 -> DocumentID
//	* DocumentID -> RowID, 正規化後データ長
//		Inverted::DocumentIDVectorFile
//	* RowID -> DocumentID, (ユニット番号)
//		Inverted::RowIDVectorFile, (Inverted::RowIDVectorFile2)
//	検索に最低限必要なデータ以外は持たない。
//
class InvertedFile : public Common::Object,
					 public ModInvertedFile
{
public:
	// 初期化
	SYD_INVERTED_FUNCTION
	static void initialize();
	// 終了処理
	SYD_INVERTED_FUNCTION
	static void terminate();
	
	// コンストラクタ
	SYD_INVERTED_FUNCTION
	InvertedFile();

	// デストラクタ
	SYD_INVERTED_FUNCTION
	virtual ~InvertedFile();

	//
	//	スキーマ情報
	//

	// ファイルIDを得る
	SYD_INVERTED_FUNCTION
	const LogicalFile::FileID& getFileID() const;

	SYD_INVERTED_FUNCTION
	void  setFileID(const LogicalFile::FileID& cFileID_)
		{ m_cFileID = cFileID_; }

	// ファイルサイズを得る
	SYD_INVERTED_FUNCTION
	ModUInt64 getSize(const Trans::Transaction& cTransaction_);

	// 使用ファイルサイズを得る
	SYD_INVERTED_FUNCTION
	ModUInt64 getUsedSize(const Trans::Transaction& cTransaction_);

	// ファイルに挿入されているタプル数を得る
	SYD_INVERTED_FUNCTION
	ModInt64 getCount() const;

	//
	//	オプティマイズ情報
	//

	// ファイルオープン時のオーバヘッドコストを得る
	SYD_INVERTED_FUNCTION
	double getOverhead() const;

	// ひとつのタプルを挿入or取得する際のプロセスコストを得る
	SYD_INVERTED_FUNCTION
	double getProcessCost() const;

	// 検索オープンパラメータを得る
	SYD_INVERTED_FUNCTION
	static bool
	getSearchParameter(const LogicalFile::TreeNodeInterface* pCondition_,
					   bool isLanguage_,	// 言語フィールドがあるか
					   bool isScoreField_,	// スコア調整フィールドがあるか
					   const LogicalFile::FileID& cFileID_,
					   LogicalFile::OpenOption& cOpenOption_);

	//
	//	ファイル操作
	//

	// ファイルを作成する
	SYD_INVERTED_FUNCTION
	const LogicalFile::FileID& create(const Trans::Transaction& cTransaction_,
									  bool bBigInverted_);

	// ファイルを削除する
	SYD_INVERTED_FUNCTION
	void destroy(const Trans::Transaction& cTransaction_);

	// ファイルをマウントする
	SYD_INVERTED_FUNCTION
	const LogicalFile::FileID& mount(const Trans::Transaction& cTransaction_);

	// ファイルをアンマウントする
	SYD_INVERTED_FUNCTION
	const LogicalFile::FileID& unmount(const Trans::Transaction& cTransaction_);

	// ファイルをフラッシュする
	SYD_INVERTED_FUNCTION
	void flush(const Trans::Transaction& cTransaction_);

	// ファイルのバックアップを開始する
	SYD_INVERTED_FUNCTION
	void startBackup(const Trans::Transaction& cTransaction_,
					 const bool bRestorable_);

	// ファイルのバックアップを終了する
	SYD_INVERTED_FUNCTION
	void endBackup(const Trans::Transaction& cTransaction_);

	// ファイルを障害から回復する
	SYD_INVERTED_FUNCTION
	void recover(const Trans::Transaction& cTransaction_,
				 const Trans::TimeStamp& cPoint_);

	// ある時点に開始された読取専用トランザクションが
	// 参照する版を最新版とする
	SYD_INVERTED_FUNCTION
	void restore(const Trans::Transaction& cTransaction_,
				 const Trans::TimeStamp& cPoint_);

	// 整合性検査を行う
	SYD_INVERTED_FUNCTION
	virtual void verify(const Trans::Transaction& cTransaction_,
				Admin::Verification::Treatment::Value uiTreatment_,
				Admin::Verification::Progress& cProgress_);

	//論理ファイルをオープンする 
	SYD_INVERTED_FUNCTION
	void open(const Trans::Transaction& cTransaction_,
			  const LogicalFile::OpenOption& cOpenOption_);

	//論理ファイルをクローズする
	SYD_INVERTED_FUNCTION
	void close();

	// 同期を取る
	SYD_INVERTED_FUNCTION
	void
	sync(const Trans::Transaction& cTransaction_,
		 bool& incomplete, bool& modified);

	//ファイルを移動する
	SYD_INVERTED_FUNCTION
	void move(const Trans::Transaction& cTransaction_,
			  const Os::Path& cFilePath_);

	// 実体である OS ファイルが存在するか調べる
	SYD_INVERTED_FUNCTION
	bool isAccessible(bool bForce_ = false) const;
	// マウントされているか調べる
	SYD_INVERTED_FUNCTION
	bool isMounted(const Trans::Transaction& trans) const;

	// ファイルをクリアする
	SYD_INVERTED_FUNCTION
	void clear(const Trans::Transaction& cTransaction_, bool bForce_ = false);

	// 文書を挿入する
	SYD_INVERTED_FUNCTION
	void insert(const ModUnicodeString& cstrDocument_,
				const ModVector<ModLanguageSet>& vecLanguage_,
				ModUInt32 uiTupleID_,
				ModVector<ModSize>& vecSectionOffset_,
				ModInvertedFeatureList& vecFeature_);

	// 文書を削除する
	SYD_INVERTED_FUNCTION
	void expunge(const ModUnicodeString& cstrDocument_,
				 const ModVector<ModLanguageSet>& vecLaguage_,
				 ModUInt32 uiTupleID_,
				 const ModVector<ModSize>& vecSectionOffset_);

	// 1つの転置リストをマージする(大転置用)
	SYD_INVERTED_FUNCTION
	void merge(InvertedList* pInsertList_, InvertedList* pExpungeList_);

	// バッチインサートを開始する
	SYD_INVERTED_FUNCTION
	void beginBatchInsert(ModSize uiMaxSize_ = 0);

	// バッチインサートを終了する
	SYD_INVERTED_FUNCTION
	void endBatchInsert();

	// アタッチしたすべてのページを確定し、デタッチする
	SYD_INVERTED_FUNCTION
	void detachAllPages();

	// 転置リストを１つ得る(小転置用)
	SYD_INVERTED_FUNCTION
	InvertedList* lowerBound(const ModUnicodeString& cstrKey_);
	// 次の転置リストを１つ得る(小転置用)
	SYD_INVERTED_FUNCTION
	InvertedList* next();

	// ベクターファイルの内容をマージする(大転置用)
	SYD_INVERTED_FUNCTION
	void mergeVectorFile(InvertedFile* pInsertFile_,
						 InvertedFile* pExpungeFile_);

	// 指定したROWIDのデータが含まれているか
	SYD_INVERTED_FUNCTION
	bool contains(ModUInt32 uiRowID_);

	// UNAのパラメータを取得
	SYD_INVERTED_FUNCTION
	void getUnaParameter(Common::DataArrayData& cUnaParameterKey_,
						 Common::DataArrayData& cUnaParameterValue_);

	//
	//	以下は外部非公開メソッド
	//

	// 圧縮器を得る
	SYD_INVERTED_FUNCTION
	ModInvertedCoder* getIdCoder(const ModUnicodeString& cstrKey_) const;
	SYD_INVERTED_FUNCTION
	ModInvertedCoder* getFrequencyCoder(const ModUnicodeString& cstrKey_) const;
	SYD_INVERTED_FUNCTION
	ModInvertedCoder* getLengthCoder(const ModUnicodeString& cstrKey_) const;
	SYD_INVERTED_FUNCTION
	ModInvertedCoder* getLocationCoder(const ModUnicodeString& cstrKey_) const;

	// B木関係(デバッグ用)
	SYD_INVERTED_FUNCTION
	void insertBtree_debug(const ModUnicodeString& cstrKey_,
						   PhysicalFile::PageID uiPageID_);
	SYD_INVERTED_FUNCTION
	void expungeBtree_debug(const ModUnicodeString& cstrKey_,
							PhysicalFile::PageID uiPageID_);
	SYD_INVERTED_FUNCTION
	void updateBtree_debug(const ModUnicodeString& cstrKey1_,
						   PhysicalFile::PageID uiPageID1_,
						   const ModUnicodeString& cstrKey2_,
						   PhysicalFile::PageID uiPageID2_);
	SYD_INVERTED_FUNCTION
	bool searchBtree_debug(const ModUnicodeString& cstrKey_,
						   PhysicalFile::PageID& uiPageID_);

#ifndef SYD_COVERAGE
	SYD_INVERTED_FUNCTION
	void reportFile(ModOstream& stream_);
#endif

	// トークナイザーを作成する
	SYD_INVERTED_FUNCTION
	ModInvertedTokenizer* createTokenizer();
	// トークナイザーを削除する
	SYD_INVERTED_FUNCTION
	static void deleteTokenizer(ModInvertedTokenizer* tokenizer_);
	// トークナイザーを設定する
	SYD_INVERTED_FUNCTION
	void setTokenizer(ModInvertedTokenizer* tokenizer_);

	// 最終文書IDを得る
	ModUInt32 getLastDocumentID();
	// 文書ID -> ROWID変換を行う
	ModUInt32 convertToRowID(ModUInt32 uiDocumentID_);
	// ROWID -> 文書ID変換を行う
	ModUInt32 convertToDocumentID(ModUInt32 uiRowID_);
	// ROWID -> 文書IDとユニット番号変換を行う
	ModUInt32 convertToDocumentID(ModUInt32 uiRowID_, ModInt32& iUnit_);

	// DocumentIDVectorFileを得る
	DocumentIDVectorFile* getDocumentIDVectorFile()
		{ return m_pDocumentIDVectorFile; }
	// RowIDVectorFileを得る
	RowIDVectorFile* getRowIDVectorFile() { return m_pRowIDVectorFile; }
	RowIDVectorFile2* getRowIDVectorFile2() { return m_pRowIDVectorFile2; }

	// 転置リスト数を1つ増やす
	void incrementListCount(int element_);

	// ファイルにアタッチする
	void attachFile(bool bUpdate_ = true);
	// ファイルをデタッチする
	void detachFile();

	// 整合性検査のための存在チェック
	SYD_INVERTED_FUNCTION
	bool check(const ModUnicodeString& cstrDocument_,
			   ModUInt32 uiTupleID_,
			   const ModVector<ModLanguageSet>& vecLanguage_,
			   ModVector<ModSize>& vecSectionByteOffset_);

	// ファイルに格納されている総文書長を得る
	SYD_INVERTED_FUNCTION
	ModUInt64 getTotalDocumentLength();

	// デフォルトの言語指定を得る
	SYD_INVERTED_FUNCTION
	const ModLanguageSet& getDefaultLanguageSet();

	// ファイルがアタッチされているか
	SYD_INVERTED_FUNCTION
	bool isAttached() const;
	// バッチインサート中かどうか
	SYD_INVERTED_FUNCTION
	bool isBatchInsert() const;
	// 中断要求がきているかどうか
	SYD_INVERTED_FUNCTION
	bool isCancel() const;

	// ModTermを得る
	ModTerm* getLibTerm(ModSize resourceID_,
						ModSize unaResourceID_,
						ModSize collectionSize_,
						ModSize averageLength_);

	// 索引タイプを得る
	ModInvertedFileIndexingType getIndexingType() const
		{ return m_cFileID.getIndexingType(); }
	// 異表記正規化ありか
	bool isNormalized() const { return m_cFileID.isNormalized(); }
	// ステミングありか
	bool isStemming() const { return m_cFileID.isStemming(); }
	// Isn't location information stored?
	bool isNolocation() const { return m_cFileID.isNolocation(); }
	// Isn't TF information stored?
	bool isNoTF() const { return m_cFileID.isNoTF(); }
	// スペース処理モードを得る
	ModInvertedUnaSpaceMode getSpaceMode() const
		{ return m_cFileID.getSpaceMode(); }
	// 抽出パラメータを得る
	const ModUnicodeString& getExtractor() const
		{ return m_cFileID.getExtractor(); }

	// ロック名を得る
	SYD_INVERTED_FUNCTION
	const Lock::FileName& getLockName() const
		{ return m_cFileID.getLockName(); }

	// ユニットの登録文書数を得る
	ModSize getUnitCount(int unit_);

	//
	//	以下はModInvertedFileで必要なメソッド
	//	なぜかすべてのメソッドにconstが付いている
	//

	// 新しいMod転置リストを取得
	SYD_INVERTED_FUNCTION
	ModInvertedList* getInvertedList() const;
	// 既存のMod転置リストをリセット
	SYD_INVERTED_FUNCTION
	ModBoolean getInvertedList(
		const ModUnicodeString& cstrKey_,
		ModInvertedList& cModInvertedList_,
		const ModInvertedListAccessMode	eAccessMode_) const;
	// 最終文書IDを得る
	SYD_INVERTED_FUNCTION
	ModUInt32 getLastDocumentID() const
		{ return const_cast<InvertedFile*>(this)->getLastDocumentID(); }
	// 文書情報ファイルを得る
	SYD_INVERTED_FUNCTION
	ModInvertedDocumentLengthFile* getDocumentLengthFile() const;
	// 文書頻度を返す
	SYD_INVERTED_FUNCTION
	ModSize getDocumentFrequency() const;
	// トークナイザを返す
	SYD_INVERTED_FUNCTION
	ModInvertedTokenizer* getTokenizer() const;
	// 最大文書IDを得る
	SYD_INVERTED_FUNCTION
	ModInvertedDocumentID getMaxDocumentID() const;
	// 最小文書IDを得る
	SYD_INVERTED_FUNCTION
	ModInvertedDocumentID getMinDocumentID() const;

private:
	// 圧縮器を設定する
	void setCoder();
	// 圧縮器を削除する
	void unsetCoder();

	// ファイルを本当に作成する
	void substantiate();

	// openの下請け
	void open(const Trans::Transaction& cTransaction_,
			  Buffer::Page::FixMode::Value eFixMode_);

	// アタッチしたすべてのページを確定し、デタッチする
	void flushAllPages();
	// アタッチしたすべてのページを破棄し、デタッチする
	void recoverAllPages();
	// 変更したすべてのページを確定する
	void saveAllPages();

	// 通常の文書挿入の下請け
	void insertFile(LocationListMap& cLocationListMap_,
					ModUInt32 uiDocumentLength_, ModUInt32 uiTupleID_);
	// バッチインサート時の文書挿入の下請け
	void insertBatch(LocationListMap& cLocationListMap_,
					 ModUInt32 uiDocumentLength_, ModUInt32 uiTupleID_);
	// バッチの内容を反映する
	void mergeBatch();

	// 整合性検査を開始する
	void startVerification(const Trans::Transaction& cTransaction_,
						   Admin::Verification::Treatment::Value uiTreatment_,
						   Admin::Verification::Progress& cProgress_);
	// 整合性検査を終了する
	void endVerification();
	// ベクターファイルの整合性検査を行う
	void verifyVectorFile(Admin::Verification::Treatment::Value uiTreatment_,
						  Admin::Verification::Progress& cProgress_,
						  const Os::Path& cRootPath_);

	// ROWIDベクターファイルを得る
	File* getRowIDVector();
	const File* getRowIDVector() const;

	// 検索に必要なユニットのみオープンする
	void openUnit() const;

	// 言語指定をチェックし、指定されていない場合はデフォルトのものを設定する
	ModVector<ModLanguageSet> checkLanguageSet(
		ModSize iSize_,
		const ModVector<ModLanguageSet>& vecLanguageSet_);

	// BatchListを作成する
	BatchBaseList* makeBatchList(InvertedUnit* pInvertedUnit_,
								 const ModUnicodeString& cstrKey_);
	
	// 文書ID->ROWIDのベクタファイル
	mutable DocumentIDVectorFile* m_pDocumentIDVectorFile;
	// ROWID->文書IDのベクタファイル
	mutable RowIDVectorFile* m_pRowIDVectorFile;
	mutable RowIDVectorFile2* m_pRowIDVectorFile2;

	// 転置ファイル単位の配列
	mutable InvertedUnit* m_pInvertedUnit;
	// 転置ファイル単位の配列数
	mutable int m_iMaxUnitCount;
	// 検索に必要なユニットのみオープンしたかどうか
	mutable bool m_bOpenUnit;

	// 圧縮器
	mutable ModInvertedCoder* m_pIdCoder;
	mutable ModInvertedCoder* m_pFrequencyCoder;
	mutable ModInvertedCoder* m_pLengthCoder;
	mutable ModInvertedCoder* m_pLocationCoder;
	mutable ModInvertedCoder* m_pWordIdCoder;
	mutable ModInvertedCoder* m_pWordFrequencyCoder;
	mutable ModInvertedCoder* m_pWordLengthCoder;
	mutable ModInvertedCoder* m_pWordLocationCoder;

	// 転置リストマネージャー
	ListManager* m_pListManager;
	// バッチリスト
	BatchListMap* m_pBatchListMap;
	// バッチリストの最大サイズ
	ModSize m_uiMaximumBatchSize;

	// 転置ファイルパラメータ
	FileID m_cFileID;

	// 検索語数
	int m_iTermCount;

	// トランザクション
	const Trans::Transaction* m_pTransaction;
	// FIXモード
	Buffer::Page::FixMode::Value m_eFixMode;
};

_SYDNEY_INVERTED_END
_SYDNEY_END

#endif //__SYDNEY_INVERTED_INVERTEDFILE_H

//
//	Copyright (c) 2002, 2003, 2004, 2005, 2008, 2009, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
