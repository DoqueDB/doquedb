// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// SubFile2.h -- ベクターファイル
// 
// Copyright (c) 2012, 2013, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_KDTREE_SUBFILE2_H
#define __SYDNEY_KDTREE_SUBFILE2_H

#include "KdTree/Module.h"
#include "KdTree/File.h"

#include "KdTree/FileID.h"

#include "Version/File.h"
#include "Version/Page.h"

#include "Os/Path.h"

_SYDNEY_BEGIN
_SYDNEY_KDTREE_BEGIN

//	CLASS
//	KdTree::SubFile2 --
//
//	NOTES
//
class SubFile2 : public File
{
public:
	//コンストラクタ
	SubFile2(FileID& cFileID_, const Os::Path& cPath_);
	//デストラクタ
	virtual ~SubFile2();

	// ファイルサイズを得る
	ModUInt64 getSize()
	{
		return m_pVersionFile->getSize();
	}

	// ファイルを作成する
	void create();
	// ファイルを破棄する
	void destroy(const Trans::Transaction& cTransaction_);

	// ファイルをマウントする
	void mount(const Trans::Transaction& cTransaction_)
	{
		m_pVersionFile->mount(cTransaction_);
	}
	// ファイルをアンマウントする
	void unmount(const Trans::Transaction& cTransaction_)
	{
		// マウントの有無や実体の存在の有無を確認せずに
		// とにかくアンマウントする
		//
		//【注意】	そうしないと下位層で管理している
		//			情報がメンテナンスされない

		m_pVersionFile->unmount(cTransaction_);
		m_bMounted = false;
	}
	
	// ファイルをフラッシュする
	void flush(const Trans::Transaction& cTransaction_)
	{
		if (isMounted(cTransaction_))
		{
			m_pVersionFile->flush(cTransaction_);
		}
	}

	// ファイルのバックアップを開始する
	void startBackup(const Trans::Transaction& cTransaction_,
					 const bool bRestorable_)
	{
		if (isMounted(cTransaction_))
		{
			m_pVersionFile->startBackup(cTransaction_, bRestorable_);
		}
	}
	// ファイルのバックアップを終了する
	void endBackup(const Trans::Transaction& cTransaction_)
	{
		if (isMounted(cTransaction_))
		{
			m_pVersionFile->endBackup(cTransaction_);
		}
	}

	// ファイルを障害から回復する
	void recover(const Trans::Transaction& cTransaction_,
				 const Trans::TimeStamp& cPoint_);

	// ある時点に開始された読取専用トランザクションが
	// 参照する版を最新版とする
	void restore(const Trans::Transaction& cTransaction_,
				 const Trans::TimeStamp& cPoint_)
	{
		if (isMounted(cTransaction_))
		{
			m_pVersionFile->restore(cTransaction_, cPoint_);
		}
	}

	// 同期をとる
	void sync(const Trans::Transaction& cTransaction_,
			  bool& incomplete, bool& modified)
	{
		if (isMounted(cTransaction_))
		{
			//
			//	【注意】
			//	トランケートはしない
			
			m_pVersionFile->sync(cTransaction_, incomplete, modified);
		}
	}

	//ファイルを移動する
	void move(const Trans::Transaction& cTransaction_,
			  const Os::Path& cPath_);

	// 実体である OS ファイルが存在するか調べる
	bool isAccessible(bool bForce_ = false) const
	{
		return m_pVersionFile->isAccessible(bForce_);
	}
	// マウントされているか調べる
	bool isMounted(const Trans::Transaction& trans) const
	{
		if (m_bMounted == false)
			
			// falseだったらバージョンに聞く
			// trueだったらdetachするまでfalseになることはない
			
			m_bMounted = m_pVersionFile->isMounted(trans);
		
		return m_bMounted;
	}

protected:
	// 物理ファイルをアタッチする
	void attach();
	// 物理ファイルをデタッチする
	void detach();

	// 整合性検査を開始する
	void startVerification(const Trans::Transaction& cTransaction_,
						   Admin::Verification::Treatment::Value eTreatment_,
						   Admin::Verification::Progress& cProgress_);
	// 整合性検査を終了する
	void endVerification();

	// バージョンファイル
	Version::File* m_pVersionFile;
	
	// マウントされているかどうか
	mutable bool m_bMounted;
};

_SYDNEY_KDTREE_END
_SYDNEY_END

#endif //__SYDNEY_KDTREE_SUBFILE2_H

//
//	Copyright (c) 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
