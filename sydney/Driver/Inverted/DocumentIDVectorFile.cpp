// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// DocumentIDVectorFile.cpp --
// 
// Copyright (c) 2002, 2004, 2005, 2023 Ricoh Company, Ltd.
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

namespace {
const char srcFile[] = __FILE__;
const char moduleName[] = "Inverted";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "Inverted/DocumentIDVectorFile.h"
#include "Inverted/Parameter.h"

#include "Os/Limits.h"

_SYDNEY_USING
_SYDNEY_INVERTED_USING

namespace
{
	//
	//	VARIABLE
	//
	const Os::Ucs2 _pszPath[] = {'D','o','c','I','D',0};

	//
	//	VARIABLE
	//
	ParameterInteger64 _cMaxDocumentLength("Inverted_MaxDocumentLength",
										   "3G");

}

//
//	FUNCTION public
//	Inverted::DocumentIDVectorFile::DocumentIDVectorFile -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	const Inverted::FileID& cFileID_
//		転置ファイルパラメータ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
DocumentIDVectorFile::DocumentIDVectorFile(const FileID& cFileID_,
										   bool batch_)
	: VectorFile<ModPair<ModUInt32, ModUInt32> >(Type::DocumentIDVector),
	  m_iUnitCount(cFileID_.getDistribute()),
	  m_bDistribution(cFileID_.isDistribution())
{
	// 物理ファイルをアタッチする
	attach(cFileID_,
		   cFileID_.getPageSize(),
		   cFileID_.getPath(),
		   Os::Path(_pszPath),
		   batch_);
}

//
//	FUNCTION public
//	Inverted::DocumentIDVectorFile::~DocumentIDVectorFile -- デストラクタ
//
//	NOTES
//	デストラクタ。
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
DocumentIDVectorFile::~DocumentIDVectorFile()
{
	// 物理ファイルをデタッチする
	detach();
}

//
//	FUNCTION public
//	Inverted::DocumentIDVectorFile::create -- 作成する
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
void
DocumentIDVectorFile::create()
{
	// まず下位から
	VectorFile<ModPair<ModUInt32, ModUInt32> >::create();
	// ヘッダーを初期化する
	initializeHeaderPage();
}

//
//	FUNCTION public
//	Inverted::DocumentIDVectorFile::move -- ファイルを移動する
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction& cTransaction_
//		トランザクション
//	const Os::Path& cFilePath_
//		パス
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
DocumentIDVectorFile::move(const Trans::Transaction& cTransaction_,
							  const Os::Path& cFilePath_)
{
	File::move(cTransaction_, cFilePath_, Os::Path(_pszPath));
}

//
//	FUNCTION public
//	Inverted::DocumentIDVectorFile::clear -- クリアする
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
void
DocumentIDVectorFile::clear(const Trans::Transaction& cTransaction_,
							bool bForce_)
{
	// まず下位から
	VectorFile<ModPair<ModUInt32, ModUInt32> >::clear(cTransaction_, bForce_);
	// ヘッダーを初期化する
	initializeHeaderPage();
}

//
//	FUNCTION public
//	Inverted::DocumentIDVectorFile::insert -- 挿入する
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32 uiDocumentID_
//		文書ID
//	ModInt32 iUnit_
//		ユニット番号
//	ModUInt32 uiRowID_
//		タプルID
//	ModUInt32 uiDocumentLength_
//		文書長
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
DocumentIDVectorFile::insert(ModUInt32 uiDocumentID_, ModInt32 iUnit_,
							 ModUInt32 uiRowID_, ModUInt32 uiDocumentLength_)
{
	// ベクターファイルに挿入する
	VectorFile<ModPair<ModUInt32, ModUInt32> >::insert(uiDocumentID_,
		ModPair<ModUInt32, ModUInt32>(uiRowID_, uiDocumentLength_));

	// ヘッダーページをdirtyにする
	dirtyHeaderPage();

	// ヘッダーを更新する
	Header* pHeader = syd_reinterpret_cast<Header*>(getHeader());
	pHeader->m_uiDocumentCount++;
	pHeader->m_ulTotalDocumentLength += uiDocumentLength_;
	pHeader->m_uiLastDocumentID = uiDocumentID_;
		// 新たに登録されるエントリは必ず
		// これまでより大きな文書IDである

	if (m_bDistribution)
	{
		// ユニットの文書長を更新する
		Unit* p = getUnitArray();
		p[iUnit_].m_uiDocumentCount++;
		p[iUnit_].m_ulTotalDocumentLength += uiDocumentLength_;
	}
}

//
//	FUNCTION public
//	Inverted::DocumentIDVectorFile::expunge -- エントリを削除する
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32 uiDocumentID_
//		文書ID
//	ModInt32 uiUnit_
//		ユニット番号
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
DocumentIDVectorFile::expunge(ModUInt32 uiDocumentID_, ModInt32 iUnit_)
{
	// ベクターファイルから削除する
	ModPair<ModUInt32, ModUInt32> cData;
	if (VectorFile<ModPair<ModUInt32, ModUInt32> >::expunge(uiDocumentID_, cData) == true)
	{
		// ヘッダーをdirtyにする
		dirtyHeaderPage();

		// 削除されたので、ヘッダーを更新する
		Header* pHeader = syd_reinterpret_cast<Header*>(getHeader());
		pHeader->m_uiDocumentCount--;
		pHeader->m_ulTotalDocumentLength -= cData.second;

		if (m_bDistribution)
		{
			// ユニットの文書長を更新する
			Unit* p = getUnitArray();
			p[iUnit_].m_uiDocumentCount--;
			p[iUnit_].m_ulTotalDocumentLength -= cData.second;
		}
	}
}

//
//	FUNCTION public
//	Inverted::DocumentIDVectorFile::find -- 検索する
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32 uiDocumentID_
//		文書ID
//	ModUInt32& uiRowID_
//		タプルID
//	ModUInt32& uiDocumentLength_
//		文書長
//
//	RETURN
//	bool
//		検索にヒットした場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
DocumentIDVectorFile::find(ModUInt32 uiDocumentID_,
						   ModUInt32& uiRowID_, ModUInt32& uiDocumentLength_)
{
	// ベクターファイルを検索する
	ModPair<ModUInt32, ModUInt32> cData;
	if (VectorFile<ModPair<ModUInt32, ModUInt32> >::find(uiDocumentID_, cData) == false)
		// ヒットしなかった
		return false;

	uiRowID_ = cData.first;
	uiDocumentLength_ = cData.second;

	return true;
}

//
//	FUNCTION public
//	Inverted::DocumentIDVectorFile::getLastDocumentID -- 最終文書IDを得る
//
//	NOTES
//
//	ARGUMENS
//	なし
//
//	RETURN
//	ModUInt32
//		最終文書ID
//
//	EXCEPTIONS
//
ModUInt32
DocumentIDVectorFile::getLastDocumentID()
{
	// ヘッダーを得る
	Header* pHeader = syd_reinterpret_cast<Header*>(getHeader());
	return pHeader->m_uiLastDocumentID;
}

//
//	FUNCTION public
//	Inverted::DocumentIDVectorFile::getMaximumDocumentID -- 最大文書IDを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModUInt32
//		最大文書ID
//
//	EXCEPTIONS
//
ModUInt32
DocumentIDVectorFile::getMaximumDocumentID()
{
	return getMaximumKey();
}

//
//	FUNCTION public
//	Inverted::DocumentIDVectorFile::getMinimumDocumentID -- 最小文書IDを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModUInt32
//		最小文書ID
//
//	EXCEPTIONS
//
ModUInt32
DocumentIDVectorFile::getMinimumDocumentID()
{
	return getMinimumKey();
}

//
//	FUNCTION public
//	Inverted::DocumentIDVectorFile::getTotalDocumentLength -- 総文書長を得る
//
//	NOTES
//
//	ARGUMENS
//	なし
//
//	RETURN
//	ModUInt64
//		総文書長
//
//	EXCEPTIONS
//
ModUInt64
DocumentIDVectorFile::getTotalDocumentLength()
{
	// ヘッダーを得る
	Header* pHeader = syd_reinterpret_cast<Header*>(getHeader());
	return pHeader->m_ulTotalDocumentLength;
}

//
//	FUNCTION public
//	Inverted::DocumentIDVectorFile::getDocumentCount -- 文書数を得る
//
//	NOTES
//
//	ARGUMENS
//	なし
//
//	RETURN
//	ModUInt32
//		文書数
//
//	EXCEPTIONS
//
ModUInt32
DocumentIDVectorFile::getDocumentCount()
{
	// ヘッダーを得る
	Header* pHeader = syd_reinterpret_cast<Header*>(getHeader());
	return pHeader->m_uiDocumentCount;
}

//
//	FUNCTION public
//	Inverted::DocumentIDVectorFile::getListCount -- 転置リスト数を得る
//
//	NOTES
//
//	ARGUMENS
//	なし
//
//	RETURN
//	ModUInt32
//		転置リスト数
//
//	EXCEPTIONS
//
ModUInt32
DocumentIDVectorFile::getListCount()
{
	// ヘッダーを得る
	Header* pHeader = syd_reinterpret_cast<Header*>(getHeader());
	return pHeader->m_uiListCount;
}

//
//	FUNCTION public
//	Inverted::DocumentIDVectorFile::incrementListCount
//		-- 転置リスト数を1つ増やす
//
//	NOTES
//
//	ARGUMENS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
DocumentIDVectorFile::incrementListCount(int element)
{
	// ヘッダーページをdirtyにする
	dirtyHeaderPage();
	// ヘッダーを更新する
	Header* pHeader = syd_reinterpret_cast<Header*>(getHeader());
	pHeader->m_uiListCount++;

	if (m_bDistribution)
	{
		Unit* p = getUnitArray();
		p[element].m_uiListCount++;
	}
}

//
//	FUNCTION public
//	INverted::DocumentIDVectorFile::getMaxUnitCount -- 最大ユニット数を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModInt32
//		最大可能ユニット数
//
//	EXCEPTIONS
//
ModInt32
DocumentIDVectorFile::getMaxUnitCount()
{
	ModInt32 c = 1;
	if (m_bDistribution)
	{
		// ヘッダーを得る
		Header* pHeader = syd_reinterpret_cast<Header*>(getHeader());
		c = pHeader->m_iUnitCount;
	}
	return c;
}

//
//	FUNCTION public
//	Inverted::DocumentIDVectorFile::getUnitTotalDocumentLength
//		-- 転置ユニットの文書数を得る
//
//	NOTES
//
//	ARGUMENTS
//	int element_
//		ユニット番号
//
//	RETURN
//	ModUInt32
//		該当ユニットの文書数
//
//	EXCEPTIONS
//
ModUInt32
DocumentIDVectorFile::getUnitDocumentCount(int element_)
{
	ModUInt32 count = 0;
	
	if (m_bDistribution)
	{
		Unit* p = getUnitArray();
		count = p[element_].m_uiDocumentCount;
	}
	else
	{
		count = getDocumentCount();
	}
	return count;
}

//
//	FUNCTION public
//	Inverted::DocumentIDVectorFile::getUnitTotalDocumentLength
//		-- 転置ユニットの総文書長を得る
//
//	NOTES
//
//	ARGUMENTS
//	int element_
//		ユニット番号
//
//	RETURN
//	ModUInt64
//		該当ユニットの総文書長
//
//	EXCEPTIONS
//
ModUInt64
DocumentIDVectorFile::getUnitTotalDocumentLength(int element_)
{
	Unit* p = getUnitArray();
	return p[element_].m_ulTotalDocumentLength;
}

//
//	FUNCTION public
//	Inverted::DocumentIDVectorFile::checkInsertUnit
//		-- 挿入ユニット番号をチェックし、必要なら変更する
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		挿入するユニット番号が変更された場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
DocumentIDVectorFile::checkInsertUnit()
{
	bool result = false;
	if (m_bDistribution)
	{
		Header* pHeader = syd_reinterpret_cast<Header*>(getHeader());
		Unit* p = getUnitArray();

		ModUInt64 maxLen = pHeader->m_ulMaxDocumentLength;
		if (p[pHeader->m_iInsertUnit].m_ulTotalDocumentLength > maxLen)
		{
			// 現在の挿入ユニットは上限を超えている
			// -> 挿入できるユニットを探す

			int i = 0;
			for (; i < pHeader->m_iUnitCount; ++i)
			{
				if (p[i].m_ulTotalDocumentLength < (maxLen / 10 * 9))
				{
					// 既存のユニットで空きがあるものが見つかった
					break;
				}
			}

			// ヘッダーページをdirtyにする
			dirtyHeaderPage();
			result = true;
		
			if (i == pHeader->m_iUnitCount)
			{
				// すべて超えているので、最大文書長を倍にする

				i = 0;
				while (p[i].m_ulTotalDocumentLength > maxLen)
					maxLen *= 2;
				pHeader->m_ulMaxDocumentLength = maxLen;
			}

			// ヘッダーに設定する
			pHeader->m_iInsertUnit = i;
		}
	}
	return result;
}

//
//	FUNCTION public
//	Inverted::DocumentIDVectorFile::isInserted -- データが挿入されているかどうか
//
//	NOTES
//
//	ARGUMENTS
//	int element_
//		ユニット番号
//
//	RETURN
//	bool
//		データが挿入されている場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
DocumentIDVectorFile::isInserted(int element_)
{
	if (m_bDistribution)
	{
		return (getUnitDocumentCount(element_) != 0);
	}
	else
	{
		return (getDocumentCount() != 0);
	}
}

//
//	FUNCTION public
//	Inverted::DocumentIDVectorFile::clearUnit
//		-- ユニット情報をクリアする
//
//	NOTES
//
//	ARGUMENTS
//	int element_
// 		クリアするユニット番号
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
DocumentIDVectorFile::clearUnit(int element_)
{
	if (m_bDistribution)
	{
		// ヘッダーページをdirtyにする
		dirtyHeaderPage();

		// ユニット情報を得る
		Unit* p = getUnitArray();
		p += element_;
		
		// ヘッダーを更新する
		Header* pHeader = syd_reinterpret_cast<Header*>(getHeader());
		pHeader->m_uiDocumentCount -= p->m_uiDocumentCount;
		pHeader->m_ulTotalDocumentLength -= p->m_ulTotalDocumentLength;
		pHeader->m_uiListCount -= p->m_uiListCount;

		// ユニット情報をクリアする
		Os::Memory::reset(p, sizeof(Unit));
	}
}

//
//	FUNCTION public
//	Inverted::DocumentIDVectorFile::getInsertUnit
//		-- 挿入に利用するユニット番号を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModInt32
//		挿入に利用するユニット番号
//
//	EXCEPTIONS
//
ModInt32
DocumentIDVectorFile::getInsertUnit()
{
	ModInt32 i = 0;
	if (m_bDistribution)
	{
		// ヘッダーを得る
		Header* pHeader = syd_reinterpret_cast<Header*>(getHeader());
		i = pHeader->m_iInsertUnit;
	}
	return i;
}

//
//	FUNCTION public
//	Inverted::DocumentIDVectorFile::search -- 検索する
//
//	NOTES
//	ModInvertedDocumentLengthFileのためのメソッド。Modの検索時に使用される
//
//	ARGUMENTS
//	const ModUInt32 uiDocumentID_
//		文書ID
//	ModSize& uiDocumentLength_
//		文書長
//
//	RETURN
//	ModBoolean
//		ヒットした場合はModTrue、それ以外の場合はModFalse
//
//	EXCEPTIONS
//
ModBoolean
DocumentIDVectorFile::search(const ModUInt32 uiDocumentID_,
							 ModSize& uiDocumentLength_) const
{
	if (isMounted())
	{
		ModUInt32 uiRowID;
		if (const_cast<DocumentIDVectorFile*>(this)
			->find(uiDocumentID_, uiRowID, uiDocumentLength_) == true)
			return ModTrue;
	}
	return ModFalse;
}

//
//	FUNCTION public
//	Inverted::DocumentIDVectorFile::getAverageDocumentLength -- 平均文書長を得る
//
//	NOTES
//	ModInvertedDocumentLengthFileのためのメソッド。Modの検索時に使用される
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModSize
//		平均文書長
//
//	EXCEPTIONS
//
ModSize
DocumentIDVectorFile::getAverageDocumentLength() const
{
	ModUInt64 total = 0;
	ModUInt64 count = 0;
	if (isMounted())
	{
		total
			= const_cast<DocumentIDVectorFile*>(this)->getTotalDocumentLength();
		count = const_cast<DocumentIDVectorFile*>(this)->getDocumentCount();
	}
	if (count) total /= count;
	return static_cast<ModSize>(total);
}

//
//	FUNCTION private
//	Inverted::DocumentIDVectorFile::initializeHeaderPage
//		-- ヘッダーページを初期化する
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
void
DocumentIDVectorFile::initializeHeaderPage()
{
	// ヘッダーを初期化する
	Header* pHeader = syd_reinterpret_cast<Header*>(getHeader());
	pHeader->m_uiLastDocumentID = 0;
	pHeader->m_uiDocumentCount = 0;
	pHeader->m_ulTotalDocumentLength = 0;
	pHeader->m_uiVersion = 1;
	pHeader->m_uiListCount = 0;
	pHeader->m_iUnitCount = m_iUnitCount;
	pHeader->m_iInsertUnit = 0;
	pHeader->m_ulMaxDocumentLength = _cMaxDocumentLength.get();

	Unit* p = getUnitArray();
	Os::Memory::reset(p, sizeof(Unit)*m_iUnitCount);

	// ヘッダーページをdirtyにする
	dirtyHeaderPage();
}

//
//	FUNCTION public
//	Inverted::DocumentIDVectorFile::getUnitArray
//		-- ユニットごとの文書長配列の先頭を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Inverted::DocumentIDVectorFile::Unit*
//		ユニットごとの情報の先頭
//
//	EXCEPTIONS
//
DocumentIDVectorFile::Unit*
DocumentIDVectorFile::getUnitArray()
{
	return syd_reinterpret_cast<Unit*>(
		getHeader() + _SUBCLASS_HEADER_SIZE/sizeof(ModUInt32));
}

//
//	Copyright (c) 2002, 2004, 2005, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
