// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// InvertedUnit.h -- 1つの転置ファイルをあらわすクラス
// 
// Copyright (c) 2005, 2008, 2009, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_INVERTED_INVERTEDUNIT_H
#define __SYDNEY_INVERTED_INVERTEDUNIT_H

#include "Inverted/Module.h"
#include "Inverted/InvertedFile.h"
#include "Inverted/LeafPage.h"

#include "Trans/Transaction.h"
#include "Trans/TimeStamp.h"
#include "LogicalFile/OpenOption.h"
#include "Os/Path.h"

#include "ModVector.h"
#include "ModMap.h"

_SYDNEY_BEGIN
_SYDNEY_INVERTED_BEGIN

class FileID;
class BtreeFile;
class LeafFile;
class OverflowFile;

//
//	CLASS
//	Inverted::InvertedUnit
//		-- B木、Leaf、Overflowのファイルを1つづつ保持しているクラス
//
//	NOTES
//	大量データの場合、B木、Leaf、Overflowの組で複数保持するように修正した。
//	それに伴い、InvertedFileで管理していたこれら3つのファイルをこのクラスで
//	管理するように変更した。
//
class InvertedUnit
{
	friend class InvertedFile;

public:
	// コンストラクタ
	InvertedUnit();
	// デストラクタ
	virtual ~InvertedUnit();

	// ファイルを初期化
	void initialize(InvertedFile* pInvertedFile_,
					const FileID* pFileID_,
					int element_,
					bool batch_);

	// サイズを得る
	ModUInt64 getSize() const;
	// 利用サイズを得る
	ModUInt64 getUsedSize(const Trans::Transaction& cTransaction_) const;

	// オープンする
	void open(const Trans::Transaction& cTransaction_,
			  Buffer::Page::FixMode::Value eFixMode_);
	// クローズする
	void close();

	// ファイルを作成する
	void create();
	// ファイルを削除する
	void destroy(const Trans::Transaction& cTransaction_);

	// マウントする
	void mount(const Trans::Transaction& cTransaction_);
	// アンマウントする
	void unmount(const Trans::Transaction& cTransaction_);

	// フラッシュする
	void flush(const Trans::Transaction& cTransaction_);

	// バックアップ開始を宣言する
	void startBackup(const Trans::Transaction& cTransaction_,
					 const bool bRestorable_);
	// バックアップ終了を宣言する
	void endBackup(const Trans::Transaction& cTransaction_);

	// リカバリーする
	void recover(const Trans::Transaction& cTransaction_,
				 const Trans::TimeStamp& cPoint_);
	// リストアする
	void restore(const Trans::Transaction& cTransaction_,
				 const Trans::TimeStamp& cPoint_);

	// ディレクトリを削除する
	void rmdir();

	// 同期処理を行う
	void sync(const Trans::Transaction& cTransaction_,
			  bool& incomplete, bool& modified);

	// ファイルを移動する
	void move(const Trans::Transaction& cTransaction_,
			  const Os::Path& cFilePath_);

	// ファイルの内容をクリアする
	void clear(const Trans::Transaction& cTransaction_,
			   bool bForce_ = false);

	// 整合性検査を開始する
	void startVerification(const Trans::Transaction& cTransaction_,
						   Admin::Verification::Treatment::Value uiTreatment_,
						   Admin::Verification::Progress& cProgress_);
	// 整合性検査を終了する
	void endVerification();
	// B木の整合性検査を実行する
	void verifyBtree();

	// ページをflushする
	void flushAllPages();
	// ページをsaveする
	void saveAllPages();
	// ページを破棄する
	void recoverAllPages();

	// LeafPageを得る
	LeafPage::PagePointer attachLeafPage(PhysicalFile::PageID uiPageID_);
	// OverflowFileを得る
	OverflowFile* getOverflowFile() { return m_pOverflowFile; }
	
	// 削除するIDブロックを登録する
	// (すべての削除が成功した場合に、本当にIDブロックを消す)
	void enterDeleteIdBlock(const ModUnicodeString& cstrKey_,
							ModUInt32 uiFirstDocumentID_);
	// 先頭文書IDを削除したIDブロックのログを登録する
	void enterExpungeFirstDocumentID(const ModUnicodeString& cstrKey_,
									 ModUInt32 uiOldDocumentID_,
									 ModUInt32 uiNewDocumentID_);
	// 先頭文書IDを削除したIDブロックのログを検索する
	ModUInt32 getExpungeFirstDocumentID(const ModUnicodeString& cstrKey_,
										ModUInt32 uiOldDocumentID_);

	// 圧縮器を得る
	ModInvertedCoder* getIdCoder(const ModUnicodeString& cstrKey_) const
		{ return m_pInvertedFile->getIdCoder(cstrKey_); }
	ModInvertedCoder* getFrequencyCoder(const ModUnicodeString& cstrKey_) const
		{ return m_pInvertedFile->getFrequencyCoder(cstrKey_); }
	ModInvertedCoder* getLengthCoder(const ModUnicodeString& cstrKey_) const
		{ return m_pInvertedFile->getLengthCoder(cstrKey_); }
	ModInvertedCoder* getLocationCoder(const ModUnicodeString& cstrKey_) const
		{ return m_pInvertedFile->getLocationCoder(cstrKey_); }

	// B木関係
	void insertBtree(const ModUnicodeString& cstrKey_,
						PhysicalFile::PageID uiPageID_);
	void expungeBtree(const ModUnicodeString& cstrKey_,
						PhysicalFile::PageID uiPageID_);
	void updateBtree(const ModUnicodeString& cstrKey1_,
						PhysicalFile::PageID uiPageID1_,
						const ModUnicodeString& cstrKey2_,
						PhysicalFile::PageID uiPageID2_);
	bool searchBtree(const ModUnicodeString& cstrKey_,
						PhysicalFile::PageID& uiPageID_);

#ifndef SYD_COVERAGE
	void reportFile(const Trans::Transaction& cTransaction_,
					Buffer::Page::FixMode::Value eFixMode_,
					ModOstream& stream_);
#endif

	// 最終文書IDを得る
	ModUInt32 getLastDocumentID()
		{ return m_pInvertedFile->getLastDocumentID(); }
	// 文書ID -> ROWID変換を行う
	ModUInt32 convertToRowID(ModUInt32 uiDocumentID_)
		{ return m_pInvertedFile->convertToRowID(uiDocumentID_); }
	// ROWID -> 文書ID変換を行う
	ModUInt32 convertToDocumentID(ModUInt32 uiRowID_)
		{ return m_pInvertedFile->convertToDocumentID(uiRowID_); }
	ModUInt32 convertToDocumentID(ModUInt32 uiRowID_, ModInt32& iUnit_)
		{ return m_pInvertedFile->convertToDocumentID(uiRowID_, iUnit_); }

	// 転置リスト数を1つ増やす
	void incrementListCount()
		{ m_pInvertedFile->incrementListCount(m_iElement); }

	// 中断要求がきているかどうか
	bool isCancel() const { return m_pInvertedFile->isCancel(); }

	// ファイルをattachする
	void attach();
	// ファイルをdetachする
	void detach();
	// attachしているか
	bool isAttached() const;

	// マウントされているか
	bool isMounted(const Trans::Transaction& cTransaction_) const;
	// ファイルが存在するか
	bool isAccessible(bool force_ = false) const;

	// このユニットの登録文書数を得る
	ModSize getCount() { return m_pInvertedFile->getUnitCount(m_iElement); }

	// このユニットのユニット番号を得る
	int getUnit() { return m_iElement; }
	
	// Isn't location information stored?
	bool isNolocation() const { return m_bNolocation; }
	
	// Isn't TF information stored?
	bool isNoTF() const { return m_bNoTF; }
	
private:
	//
	//	TYPEDEF
	//	Inverted::InvertedFile::Vector
	//
	typedef ModVector<ModUInt32> Vector;

	//
	//	TYPEDEF
	//	Inverted::InvertedFile::Map
	//
	typedef ModMap<ModUnicodeString, Vector, ModLess<ModUnicodeString> > Map;

	//
	//	TYPEDEF
	//	Inverted::InvertedFile::IDMap
	//
	typedef ModMap<ModPair<ModUnicodeString, ModUInt32>,
		ModUInt32, ModLess<ModPair<ModUnicodeString, ModUInt32> > > IDMap;

	// B木ファイル
	mutable BtreeFile* m_pBtreeFile;
	// リーフファイル
	mutable LeafFile* m_pLeafFile;
	// オーバーフローファイル
	mutable OverflowFile* m_pOverflowFile;
	
	// 削除するIDブロック
	Map m_mapDeleteIdBlock;
	// 先頭文書IDを削除したIDブロック
	IDMap m_mapExpungeFirstDocumentID;

	// 転置ファイル
	mutable InvertedFile* m_pInvertedFile;
	// ファイルID
	const FileID* m_pFileID;
	// 要素番号
	int m_iElement;
	// バッチインサートかどうか
	bool m_bBatch;

	// ファイル分散を利用しているか
	bool m_bDistribute;

	// Isn't location information stored?
	bool m_bNolocation;

	// Isn't TF information stored?
	bool m_bNoTF;

	// パス
	Os::Path m_cPath;
};

_SYDNEY_INVERTED_END
_SYDNEY_END

#endif //__SYDNEY_INVERTED_INVERTEDUNIT_H

//
//	Copyright (c) 2005, 2008, 2009, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
