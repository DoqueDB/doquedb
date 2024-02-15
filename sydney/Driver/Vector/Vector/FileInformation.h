// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// FileInformation.h -- レコードファイル管理情報クラスのヘッダファイル
// 
// Copyright (c) 2000, 2001, 2004, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_VECTOR_FILEINFORMATION_H
#define __SYDNEY_VECTOR_FILEINFORMATION_H

#include "ModTypes.h"

#include "Common/DateTimeData.h"
#include "FileCommon/DataManager.h"
#include "FileCommon/OpenMode.h"
#include "PhysicalFile/File.h"
#include "PhysicalFile/Page.h"
#include "PhysicalFile/Types.h"

#include "Vector/PageManager.h"

_SYDNEY_BEGIN

namespace Trans
{
class Transaction;
}

namespace PhysicalFile
{
class File;
}

namespace Vector
{

//
//	CLASS
//	Vector::FileInformation -- レコードファイル管理情報クラス
//
//	NOTES
//	レコードファイル管理情報クラス。
//	指定したページ以降に管理情報を読み書きする。
//
class SYD_VECTOR_FUNCTION_TESTEXPORT FileInformation : public Common::Object
{
public:

	// コンストラクタ(ファイルからデータメンバに値を読み込む)
	FileInformation(
		const PhysicalFile::Page*			pPage_,
		const Buffer::Page::FixMode::Value	eFixMode_,
		bool bFirst_=false);

	// デストラクタ
	~FileInformation();

	// アクセサ
	
	PhysicalFile::Page* getPage();

#ifdef OBSOLETE
	// 最終更新時刻を取得(leak!)
	const Common::DateTimeData	getLastUpdateTime() const;
	// 埋め込んであるバージョン番号を取得
	const ModUInt32	getVersion() const;
#endif
	// 挿入されているオブジェクト数を取得
	const ModUInt32	getObjectCount() const;
	// 先頭オブジェクトの VectorKey を取得
	const ModUInt32	getFirstVectorKey() const;
	// 最終オブジェクトの VectorKey を取得
	const ModUInt32	getLastVectorKey() const;
	// FixModeを取得
	// 現在、PageManager::detachFileInformationのみが使用
	const Buffer::Page::FixMode::Value getFixMode() const;

	// マニピュレータ
	
	// 挿入されているオブジェクト数を設定
	void setObjectCount(ModUInt32 ulObjectCount_);
	// 先頭オブジェクトの VectorKey を設定
	void setFirstVectorKey(ModUInt32 ulVectorKey_);
	// 最終オブジェクトの VectorKey を設定
	void setLastVectorKey(ModUInt32 ulVectorKey_);

	// 管理情報の初期化
	void initialize();

private:
	
	// バージョン番号の埋め込み
	void setVersion();
	// 最終更新時刻を設定
	void setLastUpdateTime();

	//// ここから変数

	// 物理ページへのポインタ
	const PhysicalFile::Page* m_pPage;

	// fixモード
	// (PageManager::detachFileInformation()が使用する)
	const Buffer::Page::FixMode::Value m_eFixMode;
};

//
//	CLASS
//	Vector::AutoFileInformation -- レコードファイル管理情報クラス
//
//	NOTES
//	自動的にページをアタッチ・デタッチし
//	指定したページ以降に管理情報を読み書きする。
//	Autoオブジェクトとでしか生成できない。
//
class AutoFileInformation {
public:
	explicit AutoFileInformation(PageManager& cManager_)
		: m_rPageManager(cManager_)
		, m_pFileInfo(0)
	{
	}

	AutoFileInformation(PageManager& cManager_ ,Buffer::Page::FixMode::Value eFixMode_ )
		: m_rPageManager(cManager_)
		, m_pFileInfo( cManager_.attachFileInformation(eFixMode_) )
	{
	}

	~AutoFileInformation() throw()
	{
		detach();
	}

	FileInformation* get() { return m_pFileInfo; }
	operator FileInformation*()   { return get(); }
	FileInformation* operator->() { return get(); }
	FileInformation& operator*() { return *get(); }

	void attach(Buffer::Page::FixMode::Value eFixMode_ = Buffer::Page::FixMode::ReadOnly)
	{
		_SYDNEY_ASSERT(m_pFileInfo == 0);
		m_pFileInfo = m_rPageManager.attachFileInformation(eFixMode_);
	}

	void detach()
	{
		if (m_pFileInfo) {
			//このメソッドは例外を投げてはいけない
			m_rPageManager.detachFileInformation(m_pFileInfo);
			m_pFileInfo = 0;
		}
	}

private:
	PageManager& m_rPageManager;
	FileInformation* m_pFileInfo;

private://auto のみで作成させる為の処置
	static void* operator new(size_t);
	static void operator delete(void*);
	static void* operator new[](size_t);
	static void operator delete[](void*);
};

} // end of namespace Vector

_SYDNEY_END

#endif // __SYDNEY_VECTOR_FILEINFORMATION_H

//
//	Copyright (c) 2000, 2001, 2004, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
