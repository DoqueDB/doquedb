// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// OtherInformationFile1.h -- 全文のその他情報を格納するファイル
// 
// Copyright (c) 2006, 2008, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_FULLTEXT_OTHERINFORMATIONFILE1_H
#define __SYDNEY_FULLTEXT_OTHERINFORMATIONFILE1_H

#include "FullText/Module.h"
#include "FullText/OtherInformationFile.h"
#include "FullText/SectionFile.h"

_SYDNEY_BEGIN
_SYDNEY_FULLTEXT_BEGIN

//
//	CLASS
//	FullText::OtherInformationFile1
//		-- 全文ファイルの転置以外の情報を管理する
//
//	NOTES
//
class OtherInformationFile1 : public OtherInformationFile
{
public:
	// コンストラクタ
	OtherInformationFile1(FileID& cFileID_)
		: m_cSectionFile(cFileID_) {}
	// デストラクタ
	virtual ~OtherInformationFile1() {}
	
	// ファイルサイズを得る
	ModUInt64 getSize(const Trans::Transaction& cTrans_)
	{
		return m_cSectionFile.getSize(cTrans_);
	}

	// ファイルに挿入されているタプル数を得る
	ModInt64 getCount() const
	{
		return m_cSectionFile.getCount();
	}

	// ファイルを作成する
	void create(const Trans::Transaction& cTransaction_)
	{
		m_cSectionFile.create(cTransaction_);
	}
	// ファイルを破棄する
	void destroy(const Trans::Transaction& cTransaction_)
	{
		m_cSectionFile.destroy(cTransaction_);
	}

	// ファイルをマウントする
	void mount(const Trans::Transaction& cTransaction_)
	{
		m_cSectionFile.mount(cTransaction_);
	}
	// ファイルをアンマウントする
	void unmount(const Trans::Transaction& cTransaction_)
	{
		m_cSectionFile.unmount(cTransaction_);
	}

	// ファイルをフラッシュする
	void flush(const Trans::Transaction& cTransaction_)
	{
		m_cSectionFile.flush(cTransaction_);
	}

	// ファイルのバックアップを開始する
	void startBackup(const Trans::Transaction& cTransaction_,
					 const bool bRestorable_)
	{
		m_cSectionFile.startBackup(cTransaction_, bRestorable_);
	}
	// ファイルのバックアップを終了する
	void endBackup(const Trans::Transaction& cTransaction_)
	{
		m_cSectionFile.endBackup(cTransaction_);
	}

	// ファイルを障害から回復する
	void recover(const Trans::Transaction& cTransaction_,
				 const Trans::TimeStamp& cPoint_)
	{
		m_cSectionFile.recover(cTransaction_, cPoint_);
	}

	// 整合性検査を行う
	void verify(const Trans::Transaction& cTransaction_,
				Admin::Verification::Treatment::Value uiTreatment_,
				Admin::Verification::Progress& cProgress_)
	{
		m_cSectionFile.verify(cTransaction_,
							   uiTreatment_,
							   cProgress_);
	}
	
	// ある時点に開始された読取専用トランザクションが
	// 参照する版を最新版とする
	void restore(const Trans::Transaction& cTransaction_,
				 const Trans::TimeStamp& cPoint_)
	{
		m_cSectionFile.restore(cTransaction_, cPoint_);
	}

	//論理ファイルをオープンする
	void open(const Trans::Transaction& cTransaction_,
			  const LogicalFile::OpenOption& cOption_)
	{
		m_cSectionFile.open(cTransaction_, cOption_);
	}
	//論理ファイルをクローズする
	void close()
	{
		m_cSectionFile.close();
	}

	// 同期をとる
	void sync(const Trans::Transaction& cTransaction_,
			  bool& incomplete, bool& modified)
	{
		m_cSectionFile.sync(cTransaction_, incomplete, modified);
	}

	//ファイルを移動する
	void move(const Trans::Transaction& cTransaction_,
			  const Common::StringArrayData& cArea_)
	{
		m_cSectionFile.move(cTransaction_, cArea_);
	}

	// 実体である OS ファイルが存在するか調べる
	bool isAccessible(bool bForce_ = false) const
	{
		return m_cSectionFile.isAccessible(bForce_);
	}
	// マウントされているか調べる
	bool isMounted(const Trans::Transaction& trans) const
	{
		return  m_cSectionFile.isMounted(trans);
	}

	// 挿入する
	void insert(ModUInt32 uiRowID_,
				const Common::DataArrayData& cValue_)
	{
		m_cSectionFile.insert(uiRowID_, *cValue_.getElement(0));
	}
	// 更新する
	void update(ModUInt32 uiRowID_,
				const Common::DataArrayData& cValue_,
				const ModVector<int>& vecUpdateFields_)
	{
		m_cSectionFile.update(uiRowID_, *cValue_.getElement(0));
	}
	// 削除する
	void expunge(ModUInt32 uiRowID_)
	{
		m_cSectionFile.expunge(uiRowID_);
	}

	// 取得する
	void get(ModUInt32 uiRowID_,
			 int iField_,
			 Common::Data& cValue_)
	{
		bool result = m_cSectionFile.get(uiRowID_, cValue_);
		if (result == false)
		{
			cValue_.setNull();
		}
	}

	// すべてのページの更新を破棄する
	void recoverAllPages()
	{
		m_cSectionFile.recoverAllPages();
	}
	// すべてのページの更新を確定する
	void flushAllPages()
	{
		m_cSectionFile.flushAllPages();
	}

private:
	// セクションファイル
	SectionFile m_cSectionFile;
};

_SYDNEY_FULLTEXT_END
_SYDNEY_END

#endif //__SYDNEY_FULLTEXT_OTHERINFORMATIONFILE1_H

//
//	Copyright (c) 2006, 2008, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
