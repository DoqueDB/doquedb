// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// PageManager.h -- 物理ページの操作を行うクラスのヘッダファイル
// 
// Copyright (c) 2000, 2001, 2002, 2003, 2004, 2006, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_VECTOR_PAGEMANAGER_H
#define __SYDNEY_VECTOR_PAGEMANAGER_H

#include "Vector/Module.h"
#include "Vector/Object.h"

#include "Common/Assert.h"
#include "Buffer/Page.h"
#include "PhysicalFile/Page.h"
#include "PhysicalFile/Types.h"

_SYDNEY_BEGIN

namespace Common
{
class DataArrayData;
}

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
class FileInformation;
class FileParameter;
class OpenParameter;

//
//	CLASS
//	Vector::PageManager -- 物理ページの管理を行うクラス
//	
//	NOTE
//	物理ページという概念を他クラスから隠す。
//	

class SYD_VECTOR_FUNCTION_TESTEXPORT PageManager : public Common::Object
{
public:
	class AutoPageObject {
	public:
		explicit AutoPageObject(PageManager& cManager_)
			: m_rPageManager(cManager_)
			, m_pObject(0)
			, m_eUnfixMode(PhysicalFile::Page::UnfixMode::NotDirty)
			, m_ulCurrentKey(FileCommon::VectorKey::Undefined)
		{
		}

		AutoPageObject(PageManager& cManager_ ,ModUInt32 ulVectorKey_)
			: m_rPageManager(cManager_)
			, m_pObject( cManager_.attachNewObject(ulVectorKey_) )
			, m_eUnfixMode(PhysicalFile::Page::UnfixMode::NotDirty)
			, m_ulCurrentKey(ulVectorKey_)
		{
		}

		AutoPageObject(PageManager& cManager_ ,ModUInt32 ulVectorKey_ ,Buffer::Page::FixMode::Value eFixMode_)
			: m_rPageManager(cManager_)
			, m_pObject( cManager_.attachObject(ulVectorKey_, eFixMode_) )
			, m_eUnfixMode(PhysicalFile::Page::UnfixMode::NotDirty)
			, m_ulCurrentKey(ulVectorKey_)
		{
		}

		~AutoPageObject() throw()
		{
			detach();
		}

		Vector::Object* get() { return m_pObject; }
		operator Vector::Object*()   { return get(); }
		Vector::Object* operator->() { return get(); }
		Vector::Object& operator*() { return *get(); }

		void attach(ModUInt32 ulVectorKey_)
		{
			_SYDNEY_ASSERT(m_pObject==0);
			m_pObject = m_rPageManager.attachNewObject(ulVectorKey_);
			m_ulCurrentKey = ulVectorKey_;
		}

		void attach(ModUInt32 ulVectorKey_,Buffer::Page::FixMode::Value eFixMode_)
		{
			// Scan 時にページのアタッチ・デタッチが繰り返し発生するので、無駄をなくす
			if (m_pObject) {
				if (m_ulCurrentKey == ulVectorKey_)
				{
					return;//nop ∵ 同一ベクタキーでアタッチ済み
				}
				else if (m_pObject->getPage()->getID() == m_rPageManager.getPageID(ulVectorKey_))
				{
					// ベクタキーは異なるので、オブジェクトは
					// 新しい VectorKey で再アタッチしなければならない。
					// しかし、物理ページは同一なのでアタッチしない様にする。
					m_rPageManager.reattachObject(m_pObject, m_eUnfixMode, ulVectorKey_);
					m_ulCurrentKey = ulVectorKey_;
					return;
				}
				else
				{
					detach();
				}
			}

			_SYDNEY_ASSERT(m_pObject==0);
			m_pObject = m_rPageManager.attachObject(ulVectorKey_, eFixMode_);
			m_ulCurrentKey = ulVectorKey_;
		}

		void dirty() { m_eUnfixMode = PhysicalFile::Page::UnfixMode::Dirty; }

		void detach()
		{
			if (m_pObject) {
				//このメソッドは例外を投げてはいけない
				m_rPageManager.detachObject(m_pObject ,m_eUnfixMode );
				m_pObject = 0;
				m_ulCurrentKey = FileCommon::VectorKey::Undefined;
			}
		}

	private:
		PageManager& m_rPageManager;
		Vector::Object* m_pObject;
		PhysicalFile::Page::UnfixMode::Value m_eUnfixMode;
		ModUInt32 m_ulCurrentKey;

	private://auto のみで作成させる為の処置
		static void* operator new(size_t);
		static void operator delete(void*);
		static void* operator new[](size_t);
		static void operator delete[](void*);
	};
public:
	// コンストラクタ
	PageManager(
		const Trans::Transaction&	rTransaction_,
		FileParameter&				rFileParameter_,
		OpenParameter*				pOpenParameter_,
		PhysicalFile::File*			pPhysicalFile_,
		bool						bFirst_ = false);
	// デストラクタ
	~PageManager();

	// クローズする
	void close();

//	// トランザクション記述子への参照を取得する
//	const Trans::Transaction& getTransaction() const;

	// 物理ファイル記述子へのポインタを取得する
	PhysicalFile::File*& getFile();

	// 指定のVectorKeyに対応するオブジェクトをアタッチする
	Vector::Object* attachObject(ModUInt32 ulVectorKey_,
		Buffer::Page::FixMode::Value eFixMode_);

	// 指定のVectorKeyに対応するオブジェクトをアタッチする
	// UpdateIterator::insert()専用
	Vector::Object* attachNewObject(ModUInt32 ulVectorKey_);

	// 指定のVectorKeyに対応するオブジェクトをデタッチする
	void detachObject(Vector::Object*& rpObject_,
			PhysicalFile::Page::UnfixMode::Value eUnfixMode_);

	// 使用中の「次の」VectorKeyを返す
	ModUInt32 nextVectorKey(ModUInt32 ulVectorKey_,
							  bool bDirection_);

	// ファイル情報を初期化する
	void createFileInformation();

	// ファイル情報をアタッチする
	FileInformation* attachFileInformation
		(Buffer::Page::FixMode::Value eFixMode_);

	// ファイル情報をデタッチする
	void detachFileInformation
		(FileInformation*& rpFileInformation_);

	// VectorKeyに対応するページIDを得る
	ModUInt32 getPageID(ModUInt32 ulVectorKey_) const;

	// VectorKeyに対応するブロックIDを得る
	ModUInt32 getBlockID(ModUInt32 ulVectorKey_) const;

	// 指定のVectorKeyに対応するオブジェクトをデタッチする
	// ただし、物理ページのデタッチはしない
	bool detachObjectCore(PhysicalFile::Page*& pPage,
				Vector::Object*& rpObject_,
				PhysicalFile::Page::UnfixMode::Value eUnfixMode_);

	// 指定のVectorKeyに対応するオブジェクトをアタッチする
	// ただし、代わりとなる同一ページのオブジェクトが必要
	void reattachObject(Vector::Object*& rpObject_,
				PhysicalFile::Page::UnfixMode::Value eUnfixMode_,
				ModUInt32 ulVectorKey_);
private:

	// 指定した物理ページをattachする
	const PhysicalFile::Page*
	setCurrentPage(ModUInt32 ulPageID_,
						Buffer::Page::FixMode::Value eFixMode_);

	// 指定した物理ページをdetachする
	void unsetCurrentPage
			(const PhysicalFile::Page* pPage_,
			 PhysicalFile::Page::UnfixMode::Value eUnfixMode_,
			 bool bFreePage = false);

	// 引数に対応したオブジェクトを生成し、ポインタを返す
	Vector::Object* constructObject
		 (const PhysicalFile::Page* pPage_, ModUInt32 ulBlockID_);

	// 現在保持しているページの指定したブロックに対応する
	// ビットを調べる
	bool isValidBlock(const PhysicalFile::Page* pPage_,
						ModUInt32 ulBlockID_) const;

	// m_ulLastPageID の値が、正当でなければ初期化する。ページから読み込む。
	void initLastPageID();

	//// ここから変数

	// トランザクション記述子への参照(ページのアロケートの際に必要)
	const Trans::Transaction&		m_rTransaction;

	// ファイルパラメータへの参照
	FileParameter&			m_rFileParameter;

	// オープンパラメータへのポインタ
	OpenParameter*			m_pOpenParameter;

	// 物理ファイル記述子へのポインタ
	PhysicalFile::File*		m_pPhysicalFile;

	// 最終ページのページID
	ModUInt32				m_ulLastPageID;
	// m_ulLastPageID の値が、正当かどうか。
	bool					m_bLastPageIDValid;

	// Vector::Objectのキャッシュ
	Vector::Object*			m_pCacheObject;

	// PhysicalFile::Pageのキャッシュ
	PhysicalFile::Page*		m_pCachePage;

	// FileInformationのキャッシュ
	FileInformation*		m_pCacheFileInformation;
};

} // end of namespace Vector

_SYDNEY_END

#endif __SYDNEY_VECTOR_PAGEMANAGER_H

//
//	Copyright (c) 2000, 2001, 2002, 2003, 2004, 2006, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
