// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// LinkedObject.h -- リンクオブジェクトクラスのヘッダーファイル
// 
// Copyright (c) 2001, 2004, 2005, 2006, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_RECORD_LINKEDOBJECT_H
#define __SYDNEY_RECORD_LINKEDOBJECT_H

#include "Common/Common.h"
#include "Common/Object.h"

#include "Record/Tools.h"
#include "Record/Module.h"
#include "Record/FreeAreaManager.h"

#include "ModVector.h"

_SYDNEY_BEGIN

namespace PhysicalFile
{
	class Page;
}

_SYDNEY_RECORD_BEGIN

class UseInfo;

//	CLASS
//	Record::LinkedObject -- リンクオブジェクトのクラス
//
//	NOTES
//	リンクオブジェクトにアクセスするためのクラス。
//
//	先頭にオブジェクト種と次のオブジェクトのオブジェクトIDを保持する

class LinkedObject : public Common::Object
{
public:
	typedef FreeAreaManager::Operation Operation;

	// コンストラクタ(指定できるのは Read/Write/Batch のいずれか)
	LinkedObject(Operation::Value	eOperation_,
				 FreeAreaManager&	cFreeAreaManager_);

	// デストラクタ
	~LinkedObject();

	// リンクオブジェクトの作成を行なう
	Tools::ObjectID create(Os::Memory::Size	ulRequiredSize_,
						   Tools::ObjectID& iFreeID_);

	// リンクオブジェクトのアクセスを開始/終了する
	void attachObject(Tools::ObjectID iTopObjectID_);
	void verifyObject(Tools::ObjectID iObjectID_ ,Admin::Verification::Progress& cProgress_);
	void detachObject();
	bool isAttached() const;

#ifdef OBSOLETE
	// 現在位置を先頭にする
	void reset();
#endif //OBSOLETE

	// 現在の位置からオブジェクトを読み込む/書き込む
	void read(void* pPointer_, Os::Memory::Size iSize_);
	void write(const void* pConstPointer_, Os::Memory::Size iSize_);
	bool write(const Tools::DataType& cType_,
			   const Tools::DataType& cElementType_,
			   const Common::DataArrayData::Pointer& pData_,
			   Os::Memory::Size iSize_);
	// 指している位置をずらす
	void skip(Os::Memory::Size iSize_);

	// リンクオブジェクトの破棄を行なう
	void deleteAll(Tools::ObjectID cObjectID_, Tools::ObjectID& iFreeID_);

	// リンクオブジェクトが提供しているサイズの合計を得る
	Os::Memory::Size getTotalSizeVerify(Admin::Verification::Progress& cProgress_);

	// 使用中のページ情報を設定する
	void setUseInfo(Tools::ObjectID iObjectID_,
					UseInfo& cUseInfo_,
					Admin::Verification::Progress& cProgress_);

	// 現在見ているオブジェクトにすべて格納されていないか
	bool isSplit(Os::Memory::Size iSize_) const
	{
		return m_iRestSize < iSize_;
	}

	// 現在のバッファを得る
	const char* read(Os::Memory::Size iSize_)
	{
		const char* p = m_pConstPointer;
		m_pConstPointer += iSize_;
		m_iRestSize -= iSize_;
		return p;
	}

private:
	// コピーコンストラクタ、operator= の使用を禁止する
	LinkedObject(const LinkedObject&);
	LinkedObject& operator=(const LinkedObject&);

	// リンクオブジェクトのアクセスを開始/終了する
	void attachLinkedObject(Tools::ObjectID iObjectID_);
	void verifyLinkedObject(Tools::ObjectID iObjectID_ ,Admin::Verification::Progress& cProgress_);
	void detachLinkedObject();

	void setConstPointer(PhysicalPosition& pos);

	// ページをアタッチ/デタッチする
	PhysicalFile::Page* attachPage(PhysicalFile::PageID iPageID_,
								   Operation::Value	eOperation_);
	PhysicalFile::Page* verifyPage(PhysicalFile::PageID iPageID_,
								   Operation::Value eOperation_,
								   Admin::Verification::Progress& cProgress_);
	void detachPage(PhysicalFile::Page*& pPage_,
					Operation::Value eOperation_);

	// 次のオブジェクトIDがあるかを得る
	bool hasNext() const;

	//
	// データメンバ
	//

	// 現在のモード
	Operation::Value		m_eOperation;

	// 読み書きの対象となるファイルの情報(ファイル記述子、その他)
	FreeAreaManager&		m_rFreeAreaManager;

	// リンクオブジェクトの先頭オブジェクトID
	Tools::ObjectID			m_iTopObjectID;

	// 現在指しているオブジェクトの次のリンクオブジェクトのオブジェクトID
	Tools::ObjectID			m_iNextObjectID;

	// 現在指しているページとデータポインター
	PhysicalFile::Page* m_pPage;
	union {
		char*		m_pPointer;
		const char*	m_pConstPointer;
	};
	// 現在指しているページのヘッダーサイズ
	Os::Memory::Size m_iHeaderSize;
	// 現在指しているエリアの残りサイズ
	Os::Memory::Size m_iRestSize;
	// 現在指しているエリアが書き換えられたか
	bool m_bDirty;
};

_SYDNEY_RECORD_END
_SYDNEY_END

#endif // __SYDNEY_RECORD_LINKEDOBJECT_H

//
//	Copyright (c) 2001, 2004, 2005, 2006, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
