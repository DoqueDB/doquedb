// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// FreeAreaManager.h -- 領域の確保/解放を行なうクラスのヘッダーファイル
// 
// Copyright (c) 2000, 2007, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_RECORD_FREEAREAMANAGER_H
#define __SYDNEY_RECORD_FREEAREAMANAGER_H

#include "Record/Module.h"
#include "Record/Tools.h"
#include "Common/Common.h"
#include "Common/Object.h"
#include "PhysicalFile/Types.h"
#include "PhysicalFile/Page.h"

#include "ModVector.h"
#include "ModMap.h"

_SYDNEY_BEGIN

namespace PhysicalFile
{
class File;
class Page;
}

namespace Trans
{
class Transaction;
}

_SYDNEY_RECORD_BEGIN

class UseInfo;
//
//	CLASS
//	Record::FreeAreaManager -- 空き領域の操作、空き領域の情報保持を行なうクラス
//
//	NOTES
//	「空き領域に関する情報」と「物理ファイル記述子」を所有しているので、
//	ファイルを読み書きする時に必要な情報を全て持っているはずである。
//
//	領域のアロケートと解放を行なうメソッドも持っている。
//	
class FreeAreaManager : public Common::Object
{
	friend class DirectObject;
	friend class ArrayDataObject;
	friend class VariableDataObject;
	friend class IndexObject;
	friend class FieldIterator;

public:

	//
	// ENUM
	//
	struct Operation {
		enum Value {
			Read,				// 読み込み
			Write,				// 書き込み
			Expunge,			// 読み込み&エリアの解放
			Batch,				// バッチモードの書き込み
			ValueNum
		};
	};

	// コンストラクタ
	FreeAreaManager(const Trans::Transaction&	Transaction_,
					PhysicalFile::File&			File_,
					const PhysicalFile::PageID	StartPageID_,
					const bool					DoCachePage_ = false);

	// デストラクタ
	~FreeAreaManager();

	//
	// アクセッサ
	//

	// 使用していた領域を解放する
	void freeUsedArea(const PhysicalFile::AreaID	AreaID_,
					  PhysicalFile::Page*			Page_,
					  Tools::ObjectID&				iFreeID_) const;

	// 使用中のページ情報を設定する（FreeAreaのVerify用）
	void setUseInfo(Tools::ObjectID iObjectID_,
	                UseInfo& cUseInfo_,
	                Admin::Verification::Progress& cProgress_);

	// ページをアタッチ/デタッチする
	PhysicalFile::Page* attachPage(PhysicalFile::PageID iPageID_,
								   Operation::Value eOperation_) const;
	PhysicalFile::Page* verifyPage(PhysicalFile::PageID iPageID_,
								   Operation::Value eOperation_,
								   Admin::Verification::Progress& cProgress_) const;
	void detachPage(PhysicalFile::Page*& pPage_,
					Operation::Value eOperation_,
					bool bDirty_,
					bool bForce_ = false) const;

	// 確保可能な領域のサイズ(最大値)を返す
	Os::Memory::Size getPageDataSize() const;

	// SearchFreePageで探すことができるサイズの閾値を得る
	Os::Memory::Size getSearchPageThreshold() const;

#ifdef OBSOLETE
	// トランザクション記述子を返す
	const Trans::Transaction& getTransaction() const;
#endif //OBSOLETE

	// 領域の取得/解放の対象になっている物理ファイル記述子を返す
	PhysicalFile::File& getFile() const;

	//
	// マニピュレータ
	//

	// 要求された大きさの領域を探す(またはアロケートする)
	// 要求されたサイズしか許さない版
	void getFreeArea(const Os::Memory::Size	Size_,
					 PhysicalFile::PageID&	PageID_,		// out
					 PhysicalFile::AreaID&	AreaID_,		// out
					 Operation::Value eOperation_);
	// 要求されたサイズ以下でも許す版
	void getFreeArea(const Os::Memory::Size	Size_,
					 PhysicalFile::PageID&	PageID_,		// out
					 PhysicalFile::AreaID&	AreaID_,		// out
					 Tools::ObjectID&		iFreeID_,		// in/out
					 Os::Memory::Size&		iAllocated_,	// out
					 Operation::Value eOperation_);

	void clearCachePage(bool bForce_ = false);
//	void setDoCache(bool bDoCache_);

	// 正常終了時にfreePageすべきページを解放する
	void freePageAll();

	// 異常終了時にfreePageすべきページの記録を破棄する
	void discardFreePage();

private:

	// 任意の物理ページ記述子から空き領域を探して返す
	bool getFreeAreaFromCachedPage(
		const bool				Compaction_,
		const Os::Memory::Size	Size_,
		PhysicalFile::PageID&	FoundPageID_,	// out
		PhysicalFile::AreaID&	FoundAreaID_);	// out
	bool getFreeAreaFromPage(
		PhysicalFile::Page*		pPage_,
		const bool				Compaction_,
		const Os::Memory::Size	Size_,
		PhysicalFile::PageID&	FoundPageID_,	// out
		PhysicalFile::AreaID&	FoundAreaID_);	// out

	// コピーコンストラクタ、代入オペレータの使用を禁止する
	FreeAreaManager(const FreeAreaManager&);
	FreeAreaManager& operator=(const FreeAreaManager&);

	//
	// データメンバ
	//

	// トランザクション記述子
	const Trans::Transaction&	m_Transaction;

	// アロケートを行なう物理ファイルの記述子
	// (本クラスでデタッチすることはない)
	PhysicalFile::File&			m_File;

	// 先頭データページのページID
	const PhysicalFile::PageID	m_StartPageID;

	// キャッシュしている物理ページ記述子(ReadOnly時)
	mutable PhysicalFile::Page*	m_CachedPage;

	// キャッシュしている物理ページ記述子(Write時)
	typedef ModMap<PhysicalFile::PageID, PhysicalFile::Page*, ModLess<PhysicalFile::PageID> > AttachedPageMap;
	mutable AttachedPageMap m_mapAttachedPage;

	// 成功時にfreePageするページID
	mutable ModVector<PhysicalFile::PageID> m_vecFreePageID;

	// キャッシュしているページのattach時の操作内容
	mutable Operation::Value m_eCacheOperation;

	// 
	Os::Memory::Size			m_PageDataSize;
	// searchFreePageで探すことができないサイズの閾値
	Os::Memory::Size			m_iSearchPageThreshold;
};

_SYDNEY_RECORD_END
_SYDNEY_END

#endif // __SYDNEY_RECORD_FREEAREAMANAGER_H

//
//	Copyright (c) 2000, 2007, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
