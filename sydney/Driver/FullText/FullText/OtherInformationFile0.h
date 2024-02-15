// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// OtherInformationFile0.h -- 
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

#ifndef __SYDNEY_FULLTEXT_OTHERINFORMATIONFILE0_H
#define __SYDNEY_FULLTEXT_OTHERINFORMATIONFILE0_H

#include "FullText/Module.h"
#include "FullText/OtherInformationFile.h"

_SYDNEY_BEGIN
_SYDNEY_FULLTEXT_BEGIN

//	CLASS
//	FullText::OtherInformationFile0
//		-- 何もしないダミーなクラス
//
//	NOTES
//
class OtherInformationFile0 : public OtherInformationFile
{
public:
	// コンストラクタ
	OtherInformationFile0(FileID& cFileID_) {}
	// デストラクタ
	virtual ~OtherInformationFile0() {}
	
	// ファイルサイズを得る
	ModUInt64 getSize(const Trans::Transaction& cTrans_)
	{
		return 0;
	}

	// ファイルに挿入されているタプル数を得る
	ModInt64 getCount() const
	{
		return 0;
	}

	// ファイルを作成する
	void create(const Trans::Transaction& cTransaction_)
	{
	}
	// ファイルを破棄する
	void destroy(const Trans::Transaction& cTransaction_)
	{
	}

	// ファイルをマウントする
	void mount(const Trans::Transaction& cTransaction_)
	{
	}
	// ファイルをアンマウントする
	void unmount(const Trans::Transaction& cTransaction_)
	{
	}

	// ファイルをフラッシュする
	void flush(const Trans::Transaction& cTransaction_)
	{
	}

	// ファイルのバックアップを開始する
	void startBackup(const Trans::Transaction& cTransaction_,
					 const bool bRestorable_)
	{
	}
	// ファイルのバックアップを終了する
	void endBackup(const Trans::Transaction& cTransaction_)
	{
	}

	// ファイルを障害から回復する
	void recover(const Trans::Transaction& cTransaction_,
				 const Trans::TimeStamp& cPoint_)
	{
	}

	// 整合性検査を行う
	void verify(const Trans::Transaction& cTransaction_,
				Admin::Verification::Treatment::Value uiTreatment_,
				Admin::Verification::Progress& cProgress_)
	{
	}
	
	// ある時点に開始された読取専用トランザクションが
	// 参照する版を最新版とする
	void restore(const Trans::Transaction& cTransaction_,
				 const Trans::TimeStamp& cPoint_)
	{
	}

	//論理ファイルをオープンする
	void open(const Trans::Transaction& cTransaction_,
			  const LogicalFile::OpenOption& cOption_)
	{
	}
	//論理ファイルをクローズする
	void close()
	{
	}

	// 同期をとる
	void sync(const Trans::Transaction& cTransaction_,
			  bool& incomplete, bool& modified)
	{
	}

	//ファイルを移動する
	void move(const Trans::Transaction& cTransaction_,
			  const Common::StringArrayData& cArea_)
	{
	}

	// 実体である OS ファイルが存在するか調べる
	bool isAccessible(bool bForce_ = false) const
	{
		return false;
	}
	// マウントされているか調べる
	bool isMounted(const Trans::Transaction& trans) const
	{
		return false;
	}
	// 挿入する
	void insert(ModUInt32 uiRowID_,
				const Common::DataArrayData& cValue_)
	{
	}
	// 更新する
	void update(ModUInt32 uiRowID_,
				const Common::DataArrayData& cValue_,
				const ModVector<int>& vecUpdateFields_)
	{
	}
	// 削除する
	void expunge(ModUInt32 uiRowID_)
	{
	}

	// 取得する
	void get(ModUInt32 uiRowID_,
			 int iField_,
			 Common::Data& cValue_)
	{
	}

	// すべてのページの更新を破棄する
	void recoverAllPages()
	{
	}
	
	// すべてのページの更新を確定する
	void flushAllPages()
	{
	}
	
};

_SYDNEY_FULLTEXT_END
_SYDNEY_END

#endif //__SYDNEY_FULLTEXT_OTHERINFORMATIONFILE0_H

//
//	Copyright (c) 2006, 2008, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
