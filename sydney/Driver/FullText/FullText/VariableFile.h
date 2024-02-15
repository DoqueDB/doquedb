// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// VariableFile.h -- 全文のその他情報を格納する可変長ファイル
// 
// Copyright (c) 2005, 2008, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_FULLTEXT_VARIABLEFILE_H
#define __SYDNEY_FULLTEXT_VARIABLEFILE_H

#include "FullText/Module.h"
#include "FullText/File.h"
#include "FullText/FileID.h"

#include "PhysicalFile/File.h"

#include "LogicalFile/OpenOption.h"

#include "Common/DataArrayData.h"

#include "Os/Path.h"

#include "ModSerialIO.h"

_SYDNEY_BEGIN
_SYDNEY_FULLTEXT_BEGIN

//	CLASS
//	FullText::VariableFile
//		-- 全文ファイルの転置以外の情報を管理する
//
//	NOTES
//
class VariableFile : public File
{
public:
	//
	//	CLASS
	//	FullText::VariableFile::Archiver
	//
	//	NOTES
	//	エリアから読み書きするためのクラス
	//
	class Archiver : public ModSerialIO
	{
	public:
		// コンストラクタ
		Archiver(VariableFile& cFile_,
				 const PhysicalFile::DirectArea& cArea_
				 = PhysicalFile::DirectArea());
		// デストラクタ
		~Archiver();

		// 読み出す
		int readSerial(void* buffer_, ModSize byte_, DataType type_);
		// 書き出す
		int writeSerial(const void* buffer_, ModSize byte_, DataType type_);

		// エリアIDを得る
		const PhysicalFile::DirectArea::ID& getAreaID() const
			{ return m_cAreaID; }

	private:
		// VariableFile
		VariableFile& m_cFile;

		// エリア
		PhysicalFile::DirectArea m_cArea;
		// エリアID
		PhysicalFile::DirectArea::ID m_cAreaID;
	};
	
	//コンストラクタ
	VariableFile(FileID& cFileID_);
	//デストラクタ
	virtual ~VariableFile();

	// ファイルサイズを得る
	ModUInt64 getSize(const Trans::Transaction& cTrans_)
	{
		if (isMounted(cTrans_))
			return m_pPhysicalFile->getSize();
		return 0;
	}

	// ファイルに挿入されているタプル数を得る
	ModInt64 getCount() const { return 0; /*dummy*/ }

	// ファイルを作成する
	void create(const Trans::Transaction& cTransaction_);
	// ファイルを破棄する
	void destroy(const Trans::Transaction& cTransaction_);

	// ファイルをマウントする
	void mount(const Trans::Transaction& cTransaction_)
	{
		m_pPhysicalFile->mount(cTransaction_);
	}
	// ファイルをアンマウントする
	void unmount(const Trans::Transaction& cTransaction_)
	{
		// マウントの有無や実体の存在の有無を確認せずに
		// とにかくアンマウントする
		//
		//【注意】	そうしないと下位層で管理している
		//			情報がメンテナンスされない

		m_pPhysicalFile->unmount(cTransaction_);
	}
	
	// ファイルをフラッシュする
	void flush(const Trans::Transaction& cTransaction_)
	{
		if (isMounted(cTransaction_))
			m_pPhysicalFile->flush(cTransaction_);
	}

	// ファイルのバックアップを開始する
	void startBackup(const Trans::Transaction& cTransaction_,
					 const bool bRestorable_)
	{
		if (isMounted(cTransaction_))
			m_pPhysicalFile->startBackup(cTransaction_, bRestorable_);
	}
	// ファイルのバックアップを終了する
	void endBackup(const Trans::Transaction& cTransaction_)
	{
		if (isMounted(cTransaction_))
			m_pPhysicalFile->endBackup(cTransaction_);
	}

	// ファイルを障害から回復する
	void recover(const Trans::Transaction& cTransaction_,
				 const Trans::TimeStamp& cPoint_);

	// 整合性検査を開始する
	void startVerification(const Trans::Transaction& cTransaction_,
						   Admin::Verification::Treatment::Value uiTreatment_,
						   Admin::Verification::Progress& cProgress_);
	// 整合性検査を終了する
	void endVerification();
	
	// 整合性検査を行う
	void verify(const Trans::Transaction& cTransaction_,
				Admin::Verification::Treatment::Value uiTreatment_,
				Admin::Verification::Progress& cProgress_)
	{
		// 自分では特に何も管理している項目はないので、何もしない
	}
	
	
	// ある時点に開始された読取専用トランザクションが
	// 参照する版を最新版とする
	void restore(const Trans::Transaction& cTransaction_,
				 const Trans::TimeStamp& cPoint_)
	{
		if (isMounted(cTransaction_))
			m_pPhysicalFile->restore(cTransaction_, cPoint_);
	}

	//論理ファイルをオープンする
	void open(const Trans::Transaction& cTransaction_,
			  const LogicalFile::OpenOption& cOption_);
	//論理ファイルをクローズする
	void close();

	// 同期をとる
	void sync(const Trans::Transaction& cTransaction_,
			  bool& incomplete, bool& modified)
	{
		if (isMounted(cTransaction_))
			
			//
			//	【注意】
			//	トランケートはしない
			
			m_pPhysicalFile->sync(cTransaction_, incomplete, modified);
	}

	//ファイルを移動する
	void move(const Trans::Transaction& cTransaction_,
			  const Common::StringArrayData& cArea_);

	// 実体である OS ファイルが存在するか調べる
	bool isAccessible(bool bForce_ = false) const
	{
		return m_pPhysicalFile->isAccessible(bForce_);
	}
	// マウントされているか調べる
	bool isMounted(const Trans::Transaction& trans) const
	{
		return m_pPhysicalFile->isMounted(trans);
	}

	// すべてのページの更新を破棄する
	void recoverAllPages();
	// すべてのページの更新を確定する
	void flushAllPages();

	// 挿入する
	PhysicalFile::DirectArea::ID
	insert(const Common::Data& cValue_);
	// 削除する
	void expunge(const PhysicalFile::DirectArea::ID& id_);
	// 取得する
	void get(const PhysicalFile::DirectArea::ID& id_, Common::Data& cValue_);

	// 最大エリアサイズを得る
	PhysicalFile::AreaSize
	getMaxStorableAreaSize() const
	{
		return m_pPhysicalFile->getMaxStorableAreaSize();
	}

	// エリアを attach する
	PhysicalFile::DirectArea attachArea(
		const PhysicalFile::DirectArea::ID& id_);
	// エリアを allocate する
	PhysicalFile::DirectArea allocateArea(ModSize size_);
	
private:
	// 物理ファイルをアタッチする
	void attach();
	// 物理ファイルをデタッチする
	void detach();

	// FileID
	FileID& m_cFileID;

	// 物理ファイル
	PhysicalFile::File* m_pPhysicalFile;

	// トランザクション
	const Trans::Transaction* m_pTransaction;
	// FixMode
	Buffer::Page::FixMode::Value m_eFixMode;
	// 整合性検査か
	bool m_bVerify;
	Admin::Verification::Progress* m_pProgress;

	// パス
	Os::Path m_cPath;
};

_SYDNEY_FULLTEXT_END
_SYDNEY_END

#endif //__SYDNEY_FULLTEXT_VARIABLEFILE_H

//
//	Copyright (c) 2005, 2008, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
