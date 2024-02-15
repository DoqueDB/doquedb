// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// OtherInformationFile2.h -- 全文のその他情報を格納するファイル
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

#ifndef __SYDNEY_FULLTEXT_OTHERINFORMATIONFILE2_H
#define __SYDNEY_FULLTEXT_OTHERINFORMATIONFILE2_H

#include "FullText/Module.h"
#include "FullText/OtherInformationFile.h"
#include "FullText/VectorFile.h"
#include "FullText/VariableFile.h"

#include "Common/DataArrayData.h"

_SYDNEY_BEGIN
_SYDNEY_FULLTEXT_BEGIN

//	CLASS
//	FullText::OtherInformationFile2
//		-- 全文ファイルの転置以外の情報を管理する
//
//	NOTES
//	その他のファイルに格納されるデータは以下の通り。
//	* RowID
//		-> スコア調整値(Double, 固定長)
//		-> セクション情報(ObjectID, 固定長 -> UnsignedIntegerArray, 可変長)
//		-> 特徴語リスト(ObjectID, 固定長 -> Binary, 可変長)
//		-> 正規化前文書長(UnsignedInteger, 固定長)
//
//	バリューの固定長部分は、FullText::VectorFileを使って格納される。
//	バリュー内に可変長部分を持つ場合は、
//	その部分だけFullText::VariableFileを使って格納される。
//
class OtherInformationFile2 : public OtherInformationFile
{
public:
	// コンストラクタ
	OtherInformationFile2(FileID& cFileID_);
	// デストラクタ
	virtual ~OtherInformationFile2();
	
	// ファイルサイズを得る
	ModUInt64 getSize(const Trans::Transaction& cTrans_)
	{
		ModUInt64 size = m_cVectorFile.getSize(cTrans_);
		if (m_pVariableFile)
			size += m_pVariableFile->getSize(cTrans_);
		return size;
	}

	// ファイルに挿入されているタプル数を得る
	ModInt64 getCount() const
	{
		return m_cVectorFile.getCount();
	}

	// ファイルを作成する
	void create(const Trans::Transaction& cTransaction_) {}
	// ファイルを破棄する
	void destroy(const Trans::Transaction& cTransaction_)
	{
		m_cVectorFile.destroy(cTransaction_);
		if (m_pVariableFile)
			m_pVariableFile->destroy(cTransaction_);
	}

	// ファイルをマウントする
	void mount(const Trans::Transaction& cTransaction_);
	// ファイルをアンマウントする
	void unmount(const Trans::Transaction& cTransaction_);

	// ファイルをフラッシュする
	void flush(const Trans::Transaction& cTransaction_)
	{
		m_cVectorFile.flush(cTransaction_);
		if (m_pVariableFile)
			m_pVariableFile->flush(cTransaction_);
	}

	// ファイルのバックアップを開始する
	void startBackup(const Trans::Transaction& cTransaction_,
					 const bool bRestorable_);
	// ファイルのバックアップを終了する
	void endBackup(const Trans::Transaction& cTransaction_)
	{
		m_cVectorFile.endBackup(cTransaction_);
		if (m_pVariableFile)
			m_pVariableFile->endBackup(cTransaction_);
	}

	// ファイルを障害から回復する
	void recover(const Trans::Transaction& cTransaction_,
				 const Trans::TimeStamp& cPoint_)
	{
		m_cVectorFile.recover(cTransaction_, cPoint_);
		if (m_pVariableFile)
			m_pVariableFile->recover(cTransaction_, cPoint_);
	}

	// 整合性検査を行う
	void verify(const Trans::Transaction& cTransaction_,
				Admin::Verification::Treatment::Value uiTreatment_,
				Admin::Verification::Progress& cProgress_);
	
	// ある時点に開始された読取専用トランザクションが
	// 参照する版を最新版とする
	void restore(const Trans::Transaction& cTransaction_,
				 const Trans::TimeStamp& cPoint_)
	{
		m_cVectorFile.restore(cTransaction_, cPoint_);
		if (m_pVariableFile)
			m_pVariableFile->restore(cTransaction_, cPoint_);
	}

	//論理ファイルをオープンする
	void open(const Trans::Transaction& cTransaction_,
			  const LogicalFile::OpenOption& cOption_)
	{
		m_cVectorFile.open(cTransaction_, cOption_);
		if (m_pVariableFile)
			m_pVariableFile->open(cTransaction_, cOption_);
	}
	//論理ファイルをクローズする
	void close()
	{
		m_cVectorFile.close();
		if (m_pVariableFile) m_pVariableFile->close();
	}

	// 同期をとる
	void sync(const Trans::Transaction& cTransaction_,
			  bool& incomplete, bool& modified)
	{
		m_cVectorFile.sync(cTransaction_, incomplete, modified);
		if (m_pVariableFile)
			m_pVariableFile->sync(cTransaction_, incomplete, modified);
	}

	//ファイルを移動する
	void move(const Trans::Transaction& cTransaction_,
			  const Common::StringArrayData& cArea_);

	// 実体である OS ファイルが存在するか調べる
	bool isAccessible(bool bForce_ = false) const
	{
		return m_cVectorFile.isAccessible(bForce_);
	}
	// マウントされているか調べる
	bool isMounted(const Trans::Transaction& trans) const
	{
		return m_cVectorFile.isMounted(trans);
	}

	// すべてのページの更新を破棄する
	void recoverAllPages()
	{
		m_cVectorFile.recoverAllPages();
		if (m_pVariableFile)
			m_pVariableFile->recoverAllPages();
	}
	// すべてのページの更新を確定する
	void flushAllPages()
	{
		m_cVectorFile.flushAllPages();
		if (m_pVariableFile)
			m_pVariableFile->flushAllPages();
	}

	// 挿入する
	void insert(ModUInt32 uiRowID_, const Common::DataArrayData& cValue_);
	// 更新する
	void update(ModUInt32 uiRowID_, const Common::DataArrayData& cValue_,
				const ModVector<int>& vecUpdateFields_);
	// 削除する
	void expunge(ModUInt32 uiRowID_);
	
	// 取得する
	void get(ModUInt32 uiRowID_, int iField_, Common::Data& cValue_);

	// エリアを取得する
	PhysicalFile::DirectArea getArea(ModUInt32 uiRowID_,
									 int iField_);

	// エリアの最大サイズを得る
	ModSize getMaxStorableAreaSize() const
	{
		if (m_pVariableFile)
			return m_pVariableFile->getMaxStorableAreaSize();
		return 0;
	}
	
private:
	// ファイルを作成する
	void substantiate();
	
	// 可変長ファイルをattachする
	void attach();
	// 可変長ファイルをdetachする
	void detach();
	
	// ベクター
	VectorFile m_cVectorFile;
	// 可変長
	VariableFile* m_pVariableFile;

	// データを取得するためのタプル
	Common::DataArrayData m_cTuple;
};

_SYDNEY_FULLTEXT_END
_SYDNEY_END

#endif //__SYDNEY_FULLTEXT_OTHERINFORMATIONFILE2_H

//
//	Copyright (c) 2005, 2008, 2009, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
