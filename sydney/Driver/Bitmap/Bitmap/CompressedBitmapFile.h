// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// CompressedBitmapFile.h --
// 
// Copyright (c) 2007, 2008, 2011, 2012, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_BITMAP_COMPRESSEDBITMAPFILE_H
#define __SYDNEY_BITMAP_COMPRESSEDBITMAPFILE_H

#include "Bitmap/Module.h"
#include "Bitmap/BitmapFile.h"
#include "Bitmap/FileID.h"
#include "Bitmap/CompressedBitmapIterator.h"

#include "Common/DataArrayData.h"

#include "Trans/Transaction.h"

#include "Os/Path.h"

_SYDNEY_BEGIN
_SYDNEY_BITMAP_BEGIN

class HeaderPage;
class BitmapIterator;

//
//	CLASS
//	Bitmap::CompressedBitmapFile --
//
//	NOTES
//
//
class CompressedBitmapFile : public BitmapFile
{
public:
	// コンストラクタ
	CompressedBitmapFile(const FileID& cFileID_);
	// デストラクタ
	virtual ~CompressedBitmapFile();

	// コピーする
	BitmapFile* copy();

	// ファイルを作成する
	void create();
	// ファイルを破棄する
	void destroy(const Trans::Transaction& cTransaction_);

	// ファイルをマウントする
	void mount(const Trans::Transaction& cTransaction_);
	// ファイルをアンマウントする
	void unmount(const Trans::Transaction& cTransaction_);
	// ファイルをフラッシュする
	void flush(const Trans::Transaction& cTransaction_);

	// ファイルのバックアップを開始する
	void startBackup(const Trans::Transaction& cTransaction_,
					 const bool bRestorable_);
	// ファイルのバックアップを終了する
	void endBackup(const Trans::Transaction& cTransaction_);

	// ファイルを障害から回復する
	void recover(const Trans::Transaction& cTransaction_,
				 const Trans::TimeStamp& cPoint_);
	// ある時点に開始された読取専用トランザクションが
	// 参照する版を最新版とする
	void restore(const Trans::Transaction& cTransaction_,
				 const Trans::TimeStamp& cPoint_);

	// 物理ファイルの整合性検査を開始する
	void startVerification(
		const Trans::Transaction& cTransaction_,
		Admin::Verification::Treatment::Value uiTreatment_,
		Admin::Verification::Progress& cProgress_);
	// 物理ファイルの整合性検査を終了する
	void endVerification();

	//論理ファイルをオープンする
	void open(const Trans::Transaction& cTransaction_,
			  LogicalFile::OpenOption::OpenMode::Value eOpenMode_);
	//論理ファイルをクローズする
	void close();

	// 同期をとる
	void sync(const Trans::Transaction& cTransaction_,
			  bool& incomplete, bool& modified);

	//ファイルを移動する
	void move(const Trans::Transaction& cTransaction_,
			  const Os::Path& cFilePath_);

	// デタッチされている全ページをフラッシュする
	void flushAllPages();
	// デタッチされている全ページを元に戻す
	void recoverAllPages();

	// 整合性チェック
	void verify();

	// ビットマップイテレータを得る
	BitmapIterator* getIterator();
	BitmapIterator* getIterator(Common::Data& cKey_);

	// (GroupBy用) ビットマップイテレータを得る
	BitmapIterator* getIteratorForGroupBy(const Common::Data& cValue_);

	// エリアを attach する
	PhysicalFile::DirectArea
	attachArea(const PhysicalFile::DirectArea::ID& id_);
	// エリアを確保する
	PhysicalFile::DirectArea allocateArea(PhysicalFile::AreaSize size_)
		{ return m_pDirectFile->allocateArea(getTransaction(), size_); }

	// 最大エリアサイズを得る
	PhysicalFile::AreaSize getMaxStorableAreaSize() const
		{ return m_pDirectFile->getMaxStorableAreaSize(); }
	
protected:
	// ビットをONする
	void on(const Common::Data& cKey_, ModUInt32 uiRowID_,
			bool isNull_ = false);
	// ビットをOFFする
	void off(const Common::Data& cKey_, ModUInt32 uiRowID_,
			 bool isNull_ = false);

private:
	// DirectAreaFileをattachする
	void attachDirectAreaFile(const FileID& cFileID_);
	// DirectAreaFileをdetachする
	void detachDirectAreaFile();
	
	// ディレクトリを削除する
	void rmdir();

	// イテレータを得る
	CompressedBitmapIterator* getIteratorImpl(
		const Common::ObjectIDData& cObjectID_,
		bool bInsert_ = false);

	// イテレータ種別を求める
	CompressedBitmapIterator::Type::Value
	getIteratorType(const Common::ObjectIDData& cObjectID_,
					PhysicalFile::DirectArea& cArea_);

	// DirectAreaFile
	PhysicalFile::File* m_pDirectFile;

	// パス
	Os::Path m_cDirectPath;
};

_SYDNEY_BITMAP_END
_SYDNEY_END

#endif //__SYDNEY_BITMAP_COMPRESSEDBITMAPFILE_H

//
//	Copyright (c) 2007, 2008, 2011, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
