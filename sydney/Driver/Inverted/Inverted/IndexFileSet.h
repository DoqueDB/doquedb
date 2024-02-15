// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// IndexFile.h --
// 
// Copyright (c) 2007, 2008, 2009, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_INVERTED_INDEXFILESET_H
#define __SYDNEY_INVERTED_INDEXFILESET_H

#include "Inverted/Module.h"
#include "Os/Path.h"

_SYDNEY_BEGIN
_SYDNEY_INVERTED_BEGIN


class IndexFileSet: public ModVector<IndexFile>
{
public:
	typedef ModVector<IndexFile>::Iterator Iterator;
	// コンストラクタ
	SYD_INVERTED_FUNCTION
	IndexFileSet(IntermediateFileID& cFileID_);
	SYD_INVERTED_FUNCTION
	IndexFileSet(LogicalFile::FileID& cFileID_,
				const Os::Path& cPath_);
	// デストラクタ
	SYD_INVERTED_FUNCTION
	virtual ~IndexFileSet();

	// 大転置ファイルを得る
	SYD_INVERTED_FUNCTION	
	Iterator getFullInvert();

	// マージする(同期マージ)
	SYD_INVERTED_FUNCTION	
	void syncMerge();

	// 1つの転置リストをマージする
	SYD_INVERTED_FUNCTION	
	bool mergeListInternal(IndexFile& cInsertFile_,
							IndexFile& cExpungeFile_);

	SYD_INVERTED_FUNCTION	
	ModInvertedFileIndexingType getIndexingType()
	{
		// getSize()が0であることはありえないから、検査しない
		LogicalFile::FileID & cFileID = const_cast<LogicalFile::FileID & >(*(begin()->getFileID()));
		return FileID::getIndexingType(cFileID);
	}
	// ファイルサイズを得る
	SYD_INVERTED_FUNCTION	
	ModUInt64 getSize(const Trans::Transaction& cTrans_);
	// ファイルに挿入されているタプル数を得る(全件)
	SYD_INVERTED_FUNCTION	
	virtual ModInt64 getCount();

	// 総文書長を得る
	SYD_INVERTED_FUNCTION	
	ModUInt64 getTotalDocumentLength();

	// ファイルを作成する
	SYD_INVERTED_FUNCTION	
	virtual void create(const Trans::Transaction& cTransaction_,
						IntermediateFileID& cFileID_);
	// ファイルを破棄する
	SYD_INVERTED_FUNCTION	
	virtual void destroy(const Trans::Transaction& cTransaction_);

	// ファイルをマウントする
	SYD_INVERTED_FUNCTION	
	virtual void mount(const Trans::Transaction& cTransaction_);
	// ファイルをアンマウントする
	SYD_INVERTED_FUNCTION	
	virtual void unmount(const Trans::Transaction& cTransaction_);

	// ファイルをフラッシュする
	SYD_INVERTED_FUNCTION	
	virtual void flush(const Trans::Transaction& cTransaction_);

	// ファイルのバックアップを開始する
	SYD_INVERTED_FUNCTION	
	virtual void startBackup(const Trans::Transaction& cTransaction_,
					 const bool bRestorable_);
	// ファイルのバックアップを終了する
	SYD_INVERTED_FUNCTION	
	virtual void endBackup(const Trans::Transaction& cTransaction_);

	// ファイルを障害から回復する
	SYD_INVERTED_FUNCTION	
	virtual void recover(const Trans::Transaction& cTransaction_,
				 const Trans::TimeStamp& cPoint_);


	// ある時点に開始された読取専用トランザクションが
	// 参照する版を最新版とする
	SYD_INVERTED_FUNCTION	
	virtual void restore(const Trans::Transaction& cTransaction_,
				 const Trans::TimeStamp& cPoint_);

	//論理ファイルをオープンする
	SYD_INVERTED_FUNCTION	
	void open(const Trans::Transaction& cTransaction_,
			const LogicalFile::OpenOption& cOption_,
			bool bBatch=false);
	//論理ファイルをクローズする
	SYD_INVERTED_FUNCTION	
	void close(bool bBatch=false);

	// 同期をとる
	SYD_INVERTED_FUNCTION	
	virtual void sync(const Trans::Transaction& cTransaction_,
				bool& incomplete, bool& modified);

	//ファイルを移動する
	SYD_INVERTED_FUNCTION	
	virtual void move(const Trans::Transaction& cTransaction_,
			const Common::StringArrayData& cArea_);

	// すべてのページの更新を破棄する
	SYD_INVERTED_FUNCTION	
	void recoverAllPages();
	// すべてのページの更新を確定する
	SYD_INVERTED_FUNCTION	
	void flushAllPages();

	void detachAllPages(){ flushAllPages();}
	
	SYD_INVERTED_FUNCTION	
	Iterator find(ModUInt32 signature);
	
	SYD_INVERTED_FUNCTION
	Iterator findEntity(ModUInt32 signature);

	SYD_INVERTED_FUNCTION
	int hash(ModUInt32 sign);

	SYD_INVERTED_FUNCTION	
	void IndexFileSet::setTokenizer(ModInvertedTokenizer *tokenizer_);

	// 文書長を取得
	SYD_INVERTED_FUNCTION	
	ModSize getDocumentLength(ModUInt32 uiRowID_);
	
	bool contains(ModUInt32 uiRowID_);
	bool check(const ModUnicodeString& cstrDocument_,
					ModUInt32 uiTupleID_,
					const ModVector<ModLanguageSet>& vecLanguage_,
					ModVector<ModSize>& vecSectionByteOffset_);
	
private:
	// パス
	Os::Path m_cPath;
};
_SYDNEY_INVERTED_END
_SYDNEY_END

#endif //__SYDNEY_INVERTED_INDEXFILESET_H

//
//  Copyright (c) 2007, 2008, 2009, 2023 Ricoh Company, Ltd.
//  All rights reserved.
//
