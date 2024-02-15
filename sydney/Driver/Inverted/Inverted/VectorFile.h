// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// VectorFile.h --
// 
// Copyright (c) 2002, 2003, 2004, 2005, 2007, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_INVERTED_VECTORFILE_H
#define __SYDNEY_INVERTED_VECTORFILE_H

#include "Inverted/Module.h"
#include "Inverted/File.h"
#include "Inverted/FakeError.h"
#include "Inverted/Page.h"
#include "ModMap.h"

_SYDNEY_BEGIN
_SYDNEY_INVERTED_BEGIN

//
//	TEMPLATE CLASS
//	Inverted::VectorFile -- ベクターファイル
//
//	NOTES
//	任意の型を格納するベクターファイル
//	ただし、現在の実装では、sizeof(VALUE)は4の倍数でなければならない。
//
//	ARGUMENTS
//	class VALUE
//		格納する値の型
//
template<class VALUE>
class VectorFile : public File
{
public:
	class Iterator;
	friend class Iterator;

	//
	//	CONST
	//	Inverted::VectorFile::_HEADER_SIZE
	//
	enum { _HEADER_SIZE = 256 };

	//
	//	STRUCT
	//	Inverted::VectorFile::Header
	//
	struct Header
	{
		// アロケートしたページの内最小のページID
		PhysicalFile::PageID m_uiMinimumPageID;
		// 最後にアロケートしたページID
		PhysicalFile::PageID m_uiLastPageID;
		// 登録したキーで最小のもの
		ModUInt32 m_uiMinimumKey;
		// 登録したキーで最大のもの
		ModUInt32 m_uiMaximumKey;
		// ページあたりの要素数
		ModUInt32 m_uiPageElementCount;
		// 要素のバイト数
		ModUInt32 m_uiElementSize;
		// 要素数
		ModUInt32 m_uiTotalCount;
		// ページを何で埋めるか
		unsigned char m_szChar;
	};

	//
	//	FUNCTION public
	//	Inverted::VectorFile::VectorFile -- コンストラクタ
	//
	//	NOTES
	//	コンストラクタ
	//
	//	ARGUMENTS
	//	Inverted::File::Type::Value eType_
	//		ファイル種別
	//
	//	RETURN
	//	なし
	//
	//	EXCEPTIONS
	//	なし
	//
	VectorFile(Type::Value eType_)
		: File(eType_, 0), m_pHeaderPage(0), m_pPageCache(0), m_pBufferCache(0)
	{
	}

	//
	//	FUNCTION public
	//	Inverted::VectorFile::~VectorFile -- デストラクタ
	//
	//	NOTES
	//	デストラクタ
	//
	//	ARGUMENTS
	//	なし
	//
	//	RETURN
	//	なし
	//
	//	EXCEPTIONS
	//	なし
	//
	virtual ~VectorFile()
	{
	}

	//
	//	FUNCTION public
	//	Inverted::VectorFile::create -- ファイルを作成する
	//
	//	NOTES
	//
	//	ARGUMENTS
	//	なし
	//
	//	RETURN
	//	なし
	//
	//	EXCEPTIONS
	//
	void create()
	{
		// まず下位を呼ぶ
		File::create();
		try
		{
			// ヘッダーページを初期化する
			initializeHeaderPage();
		}
		catch (...)
		{
			recoverAllPages();
			File::destroy();
			throw;
		}
	}

	//
	//	FUNCTION public
	//	Inverted::VectorFile::clear -- クリアする
	//
	//	NOTES
	//
	//	ARGUMENTS
	//	const Trans::Transaction& cTransaction_
	//		トランザクション
	//	bool bForce_
	//		強制モードかどうか
	//
	//	RETURN
	//	なし
	//
	//	EXCEPTIONS
	//
	void clear(const Trans::Transaction& cTransaction_, bool bForce_)
	{
		// まず下位を呼ぶ
		File::clear(cTransaction_, bForce_);
		// ヘッダーページを初期化する
		initializeHeaderPage();
	}

	//
	//	FUNCTION public
	//	Inverted::VectorFile::expand -- 要素が入る分-1のページを確保する
	//
	//	NOTES
	//
	//	ARGUMENTS
	//	ModUInt32 uiKey_
	//		キー
	//
	//	RETURN
	//	なし
	//
	//	EXCEPTIONS
	//
	void expand(ModUInt32 uiKey_)
	{
		setHeader();

		// 該当するページIDを求める
		PhysicalFile::PageID uiPageID = calcPageID(uiKey_);

		// uiPageIDは1以上の整数である
		if (m_pHeader->m_uiLastPageID < (uiPageID - 1))
		{
			int n = 0;
		
			while (m_pHeader->m_uiLastPageID < (uiPageID - 1))
			{
				// 1ページ以上の途中ページが必要

				// ページをアロケートする
				PhysicalFile::Page* pPage = allocatePage();
				++n;

				// ページの内容をクリアする
				Page::clear(pPage, m_pHeader->m_szChar);

				m_pHeader->m_uiLastPageID = pPage->getID();

				// ヘッダーページをdirtyにする
				m_pHeaderPage->dirty();

				// いきなり大きな uiKey_ が指定された場合に
				// メモリが不足する問題に対応する
		
				if ((n % 100) == 0)
				{
					// ここでflushしてしまう。戻せなくても問題ない。
					flushAllPages();

					// ヘッダーをセットしなおす
					setHeader();
				}
			}

			// 最後にflushする
			flushAllPages();
		}
	}

	//
	//	FUNCTION public
	//	Inverted::VectorFile::insert -- 要素を挿入する
	//
	//	NOTES
	//
	//	ARGUMENTS
	//	ModUInt32 uiKey_
	//		キー
	//	const VALUE& cValue_
	//		要素
	//
	//	RETURN
	//	なし
	//
	//	EXCEPTIONS
	//
	void insert(ModUInt32 uiKey_, const VALUE& cValue_)
	{
		setHeader();

		// 該当するページIDを求める
		PhysicalFile::PageID uiPageID = calcPageID(uiKey_);

		PhysicalFile::Page* pPage = 0;

		// 該当するページまで確保されているかチェックし、
		// 確保されていなかったら確保する
		while (m_pHeader->m_uiLastPageID < uiPageID)
		{
			; _INVERTED_FAKE_ERROR(VectorFile::allocatePage);

			// ページをアロケートする
			pPage = allocatePage();

			// ページの内容をクリアする
			Page::clear(pPage, m_pHeader->m_szChar);

			m_pHeader->m_uiLastPageID = pPage->getID();

			// ヘッダーページをdirtyにする
			m_pHeaderPage->dirty();
		}

		if (pPage == 0)
		{
			// アタッチする
			pPage = attachPhysicalPage(uiPageID);
		}

		// 該当する位置に挿入する
		ModUInt32 uiPosition = calcPageOffset(uiKey_);
		ModUInt32* pBuffer = getBuffer() + uiPosition;

		ModOsDriver::Memory::copy(pBuffer, &cValue_,
								  m_pHeader->m_uiElementSize);

		// これまで登録したキー値の最小or最大を超えていたら更新する
		if (m_pHeader->m_uiMinimumKey > uiKey_)
		{
			m_pHeader->m_uiMinimumKey = uiKey_;
		}
		else if (m_pHeader->m_uiMaximumKey < uiKey_)
		{
			m_pHeader->m_uiMaximumKey = uiKey_;
		}

		// エントリー数を更新する
		m_pHeader->m_uiTotalCount++;
		m_pHeaderPage->dirty();

		// ページをdirtyにする
		pPage->dirty();
	}

	//
	//	FUNCTION public
	//	Inverted::VectorFile::expunge -- エントリを削除する
	//
	//	NOTES
	//
	//	ARGUMENTS
	//	ModUInt32 uiKey_
	//		削除する要素のキー
	//
	//	RETURN
	//	なし
	//
	//	EXCEPTIONS
	//
	void expunge(ModUInt32 uiKey_)
	{
		setHeader();

		// 該当するページIDを求める
		PhysicalFile::PageID uiPageID = calcPageID(uiKey_);

		// 存在しないページの場合は無視する
		if (uiPageID > m_pHeader->m_uiLastPageID) return;

		// ページをアタッチする
		PhysicalFile::Page* pPage = attachPhysicalPage(uiPageID);

		// 該当する位置を求める
		ModUInt32 uiPosition = calcPageOffset(uiKey_);
		ModUInt32* pBuffer = getBuffer() + uiPosition;

		// 要素が格納されているかチェックする
		if (*pBuffer == ModUInt32Max)
		{
			// 格納されていない
			return;
		}

		// 削除する
		ModOsDriver::Memory::set(pBuffer, m_pHeader->m_szChar,
								 m_pHeader->m_uiElementSize);

		// エントリー数を更新する
		m_pHeader->m_uiTotalCount--;
		m_pHeaderPage->dirty();

		// ページをdirtyにする
		pPage->dirty();
	}

	//
	//	FUNCTION public
	//	Inverted::VectorFile::expunge -- エントリを削除する
	//
	//	NOTES
	//
	//	ARGUMENTS
	//	ModUInt32 uiKey_
	//		キー
	//	VALUE& cValue_
	//		要素
	//
	//	RETURN
	//	bool
	//		削除した場合はtrue、それ以外の場合はfalse
	//
	//	EXCEPTIONS
	//
	bool expunge(ModUInt32 uiKey_, VALUE& cValue_)
	{
		setHeader();

		// 該当するページIDを求める
		PhysicalFile::PageID uiPageID = calcPageID(uiKey_);

		// 存在しないページの場合は無視する
		if (uiPageID > m_pHeader->m_uiLastPageID) return false;

		// ページをアタッチする
		PhysicalFile::Page* pPage = attachPhysicalPage(uiPageID);

		// 該当する位置を求める
		ModUInt32 uiPosition = calcPageOffset(uiKey_);
		ModUInt32* pBuffer = getBuffer() + uiPosition;

		// 要素が格納されているかチェックする
		if (*pBuffer == ModUInt32Max)
		{
			// 格納されていない
			return false;
		}

		// 要素をコピーする
		ModOsDriver::Memory::copy(&cValue_, pBuffer,
								  m_pHeader->m_uiElementSize);

		// 削除する
		ModOsDriver::Memory::set(pBuffer, m_pHeader->m_szChar,
								 m_pHeader->m_uiElementSize);

		// エントリー数を更新する
		m_pHeader->m_uiTotalCount--;
		m_pHeaderPage->dirty();

		// ページをdirtyにする
		pPage->dirty();

		return true;
	}

	//
	//	FUNCTION public
	//	Inverted::VectorFile::find -- 検索する
	//
	//	NOTES
	//
	//	ARGUMENTS
	//	ModUInt32 uiKey_
	//		キー
	//	VALUE& cValue_
	//		検索した要素
	//
	//	RETURN
	//	bool
	//		検索にヒットした場合はtrue、それ以外の場合はfalse
	//
	//	EXCEPTIONS
	//
	bool find(ModUInt32 uiKey_, VALUE& cValue_)
	{
		setHeader();

		// 該当するページIDを求める
		PhysicalFile::PageID uiPageID = calcPageID(uiKey_);

		// 存在しないページの場合は無視する
		if (uiPageID > m_pHeader->m_uiLastPageID) return false;

		// ページをアタッチする
		attachPhysicalPage(uiPageID);

		// 該当する位置を求める
		ModUInt32 uiPosition = calcPageOffset(uiKey_);
		ModUInt32* pBuffer = getBuffer() + uiPosition;

		// 要素が格納されているかチェックする
		if (*pBuffer == ModUInt32Max)
		{
			// 格納されていない
			return false;
		}

		// 要素をコピーする
		ModOsDriver::Memory::copy(&cValue_, pBuffer,
								  m_pHeader->m_uiElementSize);

		return true;
	}

	//
	//	FUNCTION public
	//	Inverted::VectorFile::flushAllPages -- 変更内容をフラッシュする
	//
	//	NOTES
	//
	//	ARGUMENTS
	//	なし
	//
	//	RETURN
	//	なし
	//
	//	EXCEPTIONS
	//
	void flushAllPages()
	{
		if (m_pHeaderPage) detachPhysicalPage(m_pHeaderPage);
		if (m_pPageCache) detachPhysicalPage(m_pPageCache);
		PageMap::Iterator i = m_PageMap.begin();
		for (; i != m_PageMap.end(); ++i)
		{
			detachPhysicalPage((*i).second);
		}
		m_PageMap.erase(m_PageMap.begin(), m_PageMap.end());
		File::flushAllPages();
	}

	//
	//	FUNCTION public
	//	Inverted::VectorFile::recoverAllPages -- 変更内容を破棄する
	//
	//	NOTES
	//
	//	ARGUMENTS
	//	なし
	//
	//	RETURN
	//	なし
	//
	//	EXCEPTIONS
	//
	void recoverAllPages()
	{
		if (m_pHeaderPage) recoverPhysicalPage(m_pHeaderPage);
		if (m_pPageCache) recoverPhysicalPage(m_pPageCache);
		PageMap::Iterator i = m_PageMap.begin();
		for (; i != m_PageMap.end(); ++i)
		{
			recoverPhysicalPage((*i).second);
		}
		m_PageMap.erase(m_PageMap.begin(), m_PageMap.end());
		File::recoverAllPages();
	}

	//
	//	FUNCTION public
	//	Inverted::VectorFile::saveAllPages -- 変更内容を保存する
	//
	//	NOTES
	//
	//	ARGUMENTS
	//	なし
	//
	//	RETURN
	//	なし
	//
	//	EXCEPTIONS
	//
	void saveAllPages()
	{
		// ベクターの場合はflushしちゃう
		flushAllPages();
	}

	//
	//	FUNCTION public
	//	Inverted::VectorFile::getMinimumKey -- 最小のキー値を得る
	//
	//	NOTES
	//
	//	ARGUMENTS
	//	なし
	//
	//	RETURN
	//	ModUInt32
	//		登録されている最小のキー値
	//
	//	EXCEPTIONS
	//
	ModUInt32 getMinimumKey()
	{
		setHeader();
		return m_pHeader->m_uiMinimumKey;
	}

	//
	//	FUNCTION public
	//	Inverted::VectorFile::getMaximumKey -- 最大のキー値を得る
	//
	//	NOTES
	//
	//	ARGUMENTS
	//	なし
	//
	//	RETURN
	//	ModUInt32
	//		登録されている最大のキー値
	//
	//	EXCEPTIONS
	//
	ModUInt32 getMaximumKey()
	{
		setHeader();
		return m_pHeader->m_uiMaximumKey;
	}

	//
	//	FUNCTION public
	//	Inverted::VectorFile::getCount -- 要素数を得る
	//
	//	NOTES
	//
	//	ARGUMENTS
	//	なし
	//
	//	RETURN
	//	ModUInt32
	//		登録されている要素の数
	//
	//	EXCEPTIONS
	//
	ModUInt32 getCount()
	{
		setHeader();
		return m_pHeader->m_uiTotalCount;
	}
	//
	//	次の値を得る
	//
	void next(ModUInt32& uiKey_, VALUE& cValue_)
	{
		setHeader();

		if (uiKey_ == ModUInt32Max) return;

		// とりあえずキー値を１つインクリメントする
		uiKey_++;

		while (uiKey_ <= m_pHeader->m_uiMaximumKey)
		{
			// attachするページを得る
			PhysicalFile::PageID uiPageID = calcPageID(uiKey_);
			// ページをattachする
			attachPhysicalPage(uiPageID);
			// オフセットを計算する
			ModUInt32 uiOffset = calcPageOffset(uiKey_);

			ModUInt32* pBuffer = getBuffer() + uiOffset;
			
			// 存在しているかチェックする
			if (*pBuffer != ModUInt32Max)
			{
				// 存在している
				ModOsDriver::Memory::copy(&cValue_, pBuffer,
										  m_pHeader->m_uiElementSize);
				break;
			}

			uiKey_++;
		}

		if (uiKey_ > m_pHeader->m_uiMaximumKey) uiKey_ = ModUInt32Max;
	}

protected:
	//
	//	FUNCTION protected
	//	Inverted::VectorFile::allocatePage -- ページを確保する
	//
	//	NOTES
	//
	//	ARGUMENTS
	//	なし
	//
	//	RETURN
	//	PhysicalFile::Page*
	//		新しく確保したページ
	//
	//	EXCEPTIONS
	//
	PhysicalFile::Page* allocatePage()
	{
		if (m_pPageCache)
		{
			if (m_pPageCache->getUnfixMode()
				== PhysicalFile::Page::UnfixMode::Dirty)
			{
				// dirtyなので、とりあえずMAPに格納する
				m_PageMap.insert(m_pPageCache->getID(), m_pPageCache);
			}
			else
			{
				// dirtyじゃないので、detachする
				detachPhysicalPage(m_pPageCache);
			}
			m_pPageCache = 0;
		}
		
		m_pBufferCache = 0;

		m_pPageCache = File::allocatePage();

		return m_pPageCache;
	}
	
	//
	//	FUNCTION protected
	//	Inverted::VectorFile::attachPage -- ページにアタッチする
	//
	//	NOTES
	//
	//	ARGUMENTS
	//	PhysicalFile::PageID uiPageID_
	//		物理ページID
	//
	//	RETURN
	//	PhysicalFile::Page*
	//		アタッチしたページ
	//
	//	EXCEPTIONS
	//
	PhysicalFile::Page* attachPhysicalPage(PhysicalFile::PageID uiPageID_)
	{
		// まずキャッシュしているページと同じかどうかチェックする
		if (m_pPageCache)
		{
			if (m_pPageCache->getID() == uiPageID_)
				return m_pPageCache;
			if (m_pPageCache->getUnfixMode()
				== PhysicalFile::Page::UnfixMode::Dirty)
			{
				// dirtyなので、とりあえずMAPに格納する
				m_PageMap.insert(m_pPageCache->getID(), m_pPageCache);
			}
			else
			{
				// dirtyじゃないので、detachする
				detachPhysicalPage(m_pPageCache);
			}
			m_pPageCache = 0;
		}
		
		m_pBufferCache = 0;
		
		// 次に、MAPにあるかどうかチェックする
		PageMap::Iterator i = m_PageMap.find(uiPageID_);
		if (i != m_PageMap.end())
		{
			// みつかった
			m_pPageCache = (*i).second;
			// MAPから削除する
			m_PageMap.erase(i);
			
			return m_pPageCache;
		}

		// どこにもないので、新たに確保する
		m_pPageCache = File::attachPhysicalPage(uiPageID_);

		return m_pPageCache;
	}

	//
	//	FUNCTION protected
	//	Inverted::VectorFile::getHeader -- サブクラス用のヘッダを得る
	//
	//	NOTES
	//
	//	ARGUMENTS
	//	なし
	//
	//	RETURN
	//	ModUInt32*
	//		サブクラス用のヘッダーへのポインタ
	//
	//	EXCEPTIONS
	//
	ModUInt32* getHeader()
	{
		setHeader();
		return m_pSubClassHeader;
	}

	//
	//	FUNCTION protected
	//	Inverted::VectorFIle::dirtyHeaderPage -- ヘッダーページをdirtyにする
	//
	//	NOTES
	//
	//	ARGUMENTS
	//	なし
	//
	//	RETURN
	//	なし
	//
	//	EXCEPTIONS
	//
	void dirtyHeaderPage()
	{
		setHeader();
		m_pHeaderPage->dirty();
	}

	//
	//	FUNCTION protected
	//	Inverted::VectorFile::getBuffer
	//		-- 現在attachしているページのバッファを得る
	//
	//	NOTES
	//
	//	ARGUMENTS
	//	なし
	//
	//	RETURN
	//	ModUInt32*
	//		現在attachしているページのバッファ
	//
	//	EXCEPTIONS
	//
	ModUInt32* getBuffer()
	{
		if (m_pBufferCache == 0)
		{
			m_pBufferCache = Page::getBuffer(m_pPageCache);
		}
		return m_pBufferCache;
	}

private:
	//
	//	FUNCTION private
	//	VectorFile::initializeHeaderPage -- ヘッダーページを初期化する
	//
	//	NOTES
	//
	//	ARGUMENTS
	//	なし
	//
	//	RETURN
	//	なし
	//
	//	EXCEPTIONS
	//
	void initializeHeaderPage()
	{
		// ヘッダーのページを確保する
		PhysicalFile::Page* pPhysicalFile = File::allocatePage();
		// ヘッダーを設定する
		setHeader(pPhysicalFile);

		// 内容を設定する
		m_pHeader->m_uiMinimumPageID = 0;
		m_pHeader->m_uiLastPageID = 0;
		m_pHeader->m_uiMinimumKey = 0;
		m_pHeader->m_uiMaximumKey = 0;
		m_pHeader->m_uiPageElementCount = getPageDataSize() / sizeof(VALUE);
		m_pHeader->m_uiElementSize = sizeof(VALUE);
		m_pHeader->m_uiTotalCount = 0;
		m_pHeader->m_szChar = 0xff;

		// ヘッダーをdirtyにする
		m_pHeaderPage->dirty();
	}

	//
	//	FUNCTION private
	//	VectorFile::setHeader -- ヘッダーを設定する
	//
	//	NOTES
	//
	//	ARGUMENTS
	//	PhysicalFile::Page* pPhysicalPage_ (default 0)
	//
	//	RETURN
	//	なし
	//
	//	EXCEPTIONS
	//
	void setHeader(PhysicalFile::Page* pPhysicalPage_ = 0)
	{
		if (m_pHeaderPage == 0)
		{
			// 先頭のページをアタッチする
			m_pHeaderPage = (pPhysicalPage_) ? pPhysicalPage_
							 : File::attachPhysicalPage(
								 0,
								 Buffer::ReplacementPriority::Middle);
		}

		// ヘッダーを設定する
		ModUInt32* pBuffer = Page::getBuffer(m_pHeaderPage);
		m_pHeader = syd_reinterpret_cast<Header*>(pBuffer);
		m_pSubClassHeader = pBuffer + _HEADER_SIZE/sizeof(ModUInt32);
	}

	// キー値からページIDを計算する
	PhysicalFile::PageID calcPageID(ModUInt32 uiKey_) const
	{
		return uiKey_ / m_pHeader->m_uiPageElementCount + 1;
	}

	// キー値からページオフセットを計算する
	ModUInt32 calcPageOffset(ModUInt32 uiKey_) const
	{
		return (uiKey_ % m_pHeader->m_uiPageElementCount) * 
					(m_pHeader->m_uiElementSize / sizeof(ModUInt32));
	}


	// ヘッダーページ
	PhysicalFile::Page* m_pHeaderPage;

	// ヘッダー
	Header* m_pHeader;

	// サブクラス用のヘッダー領域
	ModUInt32* m_pSubClassHeader;

	// 直前にattachしたページ
	PhysicalFile::Page* m_pPageCache;
	// 直前にattachしたページのバッファ
	ModUInt32* m_pBufferCache;

	// dirtyなページをためておくマップ
	typedef ModMap<PhysicalFile::PageID, PhysicalFile::Page*, ModLess<PhysicalFile::PageID> > PageMap;

	PageMap m_PageMap;

public:
	//
	//	CLASS
	//	Inverted::VectorFile::Iterator -- ベクターファイルのiterator
	//
	//	NOTES
	//
	class Iterator
	{
	public:
		//
		//	STRUCT
		//	Iterator::Data
		//
		struct Data
		{
			ModUInt32	key;	// キー
			VALUE		value;	// 値
		};

		//
		//	コンストラクタ
		//
		Iterator(VectorFile<VALUE>& cFile_, ModUInt32 uiKey_, VALUE cValue_)
			: m_cFile(cFile_)
		{
			m_cData.key = uiKey_;
			m_cData.value = cValue_;
		}

		//
		//	比較演算子
		//
		bool operator!=(const Iterator& cOther_)
		{
			return m_cData.key != cOther_.m_cData.key;
		}
		bool operator<(const Iterator& cOther_)
		{
			return m_cData.key < cOther_.m_cData.key;
		}

		//
		//	インクリメントする
		//
		Iterator& operator++()
		{
			m_cFile.next(m_cData.key, m_cData.value);
			return *this;
		}

		//
		//	参照する
		//
		Data& operator*()
		{
			return m_cData;
		}

	private:
		// ベクターファイル
		VectorFile<VALUE>& m_cFile;

		// データ
		Data m_cData;
	};

	//
	//	先頭のIteratorを得る
	//
	Iterator begin()
	{
		setHeader();
		ModUInt32 uiKey = m_pHeader->m_uiMinimumKey;
		VALUE cValue;
		if (find(m_pHeader->m_uiMinimumKey, cValue) == false)
		{
			// 先頭のキーが削除されている-> 次を探す
			next(uiKey, cValue);
		}
		return Iterator(*this, uiKey, cValue);
	}

	//
	//	最後のIteratorを得る
	//
	Iterator end()
	{
		return Iterator(*this, ModUInt32Max, VALUE());
	}
};

_SYDNEY_INVERTED_END
_SYDNEY_END

#endif //__SYDNEY_INVERTED_VECTORFILE_H

//
//	Copyright (c) 2002, 2003, 2004, 2005, 2007, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
