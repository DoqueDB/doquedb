// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// CompressedData.h -- 圧縮されたデータ関連のクラス定義、関数宣言
// 
// Copyright (c) 2001, 2002, 2004, 2023 Ricoh Company, Ltd.
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

#ifndef __TRMEISTER_COMMON_COMPRESSEDDATA_H
#define __TRMEISTER_COMMON_COMPRESSEDDATA_H

#include "Common/Module.h"
#include "Common/BinaryData.h"
#include "Common/ObjectPointer.h"

_TRMEISTER_BEGIN
_TRMEISTER_COMMON_BEGIN

//	CLASS
//	Common::CompressedData -- 圧縮されたデータを表すクラス
//
//	NOTES

class SYD_COMMON_FUNCTION CompressedData
{
public:
	// コンストラクタ(1)
	CompressedData();
	// コンストラクタ(2)
	CompressedData(ModSize iValueSize_);
	// コンストラクタ(3)
	CompressedData(const void* pData_, ModSize iCompressedSize_, ModSize iValueSize_);
	// コピーコンストラクタ
	explicit CompressedData(const CompressedData& cstrOther_);
	// デストラクタ
	virtual ~CompressedData();

	//シリアル化 -- CompressedData自体はExternalizableではない
	void serialize(ModArchive& cArchiver_);

	// 値を設定する
	void		setCompressedValue(const void* pCompressedData_, ModSize iCompressedSize_);
	void		setCompressedValue(const void* pCompressedData_, ModSize iCompressedSize_, ModSize iValueSize_);
	void		setCompressedValue(const void* pCompressedData_, ModSize iCompressedSize_, ModSize iValueSize_,
								   ModSize iAllocatedSize_);

	// 伸張されたときのサイズを得る
	ModSize		getValueSize() const;

	// 圧縮されたデータを得る
	const void*	getCompressedValue() const;
	// 圧縮されたサイズを得る
	ModSize		getCompressedSize() const;

	// 圧縮されているかを得る
	bool		isCompressed() const;

	// 圧縮すると、圧縮されるか
	static bool
	isCompressable(const void* p, ModSize size);
	// ある領域を圧縮し、その結果を他の領域へ格納する
	static void
	compress(void* dst, ModSize& dstSize, const void* src, ModSize srcSize);
	// ある領域を伸長し、その結果を他の領域へ格納する
	static void
	uncompress(void* dst, ModSize& dstSize, const void* src, ModSize srcSize);

protected:
	// 与えたデータを圧縮し格納する
	void		compress(const void* buf, ModSize iSize_);
	// 格納しているデータを伸張したものを得る
	void		uncompress(void* buf) const;
	// 圧縮データを得る
	const ObjectPointer<BinaryData>&
				getCompressedData() const;

	// 圧縮データを消去する
	void		clear();

	// 圧縮データを作り直す
	virtual ModSize
	reset() = 0;

	// 内容を複写する
	void		setValue(const CompressedData& cOther_);

private:
	//データ
	ModSize						m_iValueSize;
	ObjectPointer<BinaryData>	m_pCompressedData;
};

//	FUNCTION public
//	Common::CompressedData::CompressedData
//		-- コンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTION

inline
CompressedData::
CompressedData()
	: m_iValueSize(0),
	  m_pCompressedData(static_cast<BinaryData*>(0))
{ }

//	FUNCTION public
//	Common::CompressedData::CompressedData
//		-- コンストラクター
//
//	NOTES
//		このコンストラクターでオブジェクトを生成した場合、
//		他のメソッドを呼ぶ前にsetValueで値をセットする必要がある
//
//	ARGUMENTS
//		ModSize iValueSize_
//			伸張後のデータサイズ
//
//	RETURN
//		なし
//
//	EXCEPTION

inline
CompressedData::
CompressedData(ModSize iValueSize_)
	: m_iValueSize(iValueSize_),
	  m_pCompressedData(static_cast<BinaryData*>(0))
{ }

//	FUNCTION public
//	Common::CompressedData::CompressedData
//		-- コンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		const void* pData_
//			圧縮済みデータへのポインター
//		ModSize iCompressedSize_
//			圧縮済みデータのサイズ
//		ModSize iValueSize_
//			伸張後のデータサイズ
//
//	RETURN
//		なし
//
//	EXCEPTION

inline
CompressedData::
CompressedData(const void* pData_, ModSize iCompressedSize_, ModSize iValueSize_)
	: m_iValueSize(0),
	  m_pCompressedData(static_cast<BinaryData*>(0))
{
	setCompressedValue(pData_, iCompressedSize_, iValueSize_);
}

//	FUNCTION public
//	Common::CompressedData::CompressedData
//		-- コピーコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		const Common::CompressedData& cstrOther_
//			コピー元のデータ
//
//	RETURN
//		なし
//
//	EXCEPTION

inline
CompressedData::
CompressedData(const CompressedData& cOther_)
{
	setValue(cOther_);
}

//	FUNCTION public
//	Common::CompressedData::~CompressedData
//		-- デストラクター
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTION

inline
CompressedData::
~CompressedData()
{
	m_iValueSize = 0;
	m_pCompressedData = static_cast<BinaryData*>(0);
}

//	FUNCTION public
//	Common::CompressedData::getValueSize
//		-- 解凍されたときのサイズを得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//
//	EXCEPTION

inline
ModSize
CompressedData::
getValueSize() const
{
	return m_iValueSize;
}

//	FUNCTION public
//	Common::CompressedData::getCompressedValue
//		-- 圧縮されたデータを得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		圧縮されたデータの先頭へのポインタ
//		圧縮されていなければ0が返る
//
//	EXCEPTION

inline
const void*
CompressedData::
getCompressedValue() const
{
	if (getValueSize() && !m_pCompressedData.get()) {
		// サイズが0ではなくて圧縮データがないなら作り直す必要がある
		const_cast<CompressedData*>(this)->reset();
	}
	return m_pCompressedData.get()
		? m_pCompressedData->getValue()
		: 0;
}

//	FUNCTION public
//	Common::CompressedData::getCompressedSize
//		-- 圧縮されたデータのサイズを得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		圧縮されたデータのサイズ
//		圧縮されていなければ0が返る
//
//	EXCEPTION

inline
ModSize
CompressedData::
getCompressedSize() const
{
	if (getValueSize() && !m_pCompressedData.get()) {
		// サイズが0ではなくて圧縮データがないなら作り直す必要がある
		const_cast<CompressedData*>(this)->reset();
	}
	return m_pCompressedData.get()
		? m_pCompressedData->getSize()
		: 0;
}

//	FUNCTION public
//	Common::CompressedData::isCompressed
//		-- 圧縮されているかを得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		圧縮されていればtrueが返る
//
//	EXCEPTION

inline
bool
CompressedData::
isCompressed() const
{
	return (m_pCompressedData.get() != 0
			&& m_pCompressedData->getSize() < m_iValueSize);
}

//	FUNCTION protected
//	Common::CompressedData::clear
//		-- 圧縮データを消去する
//
//	NOTES
//		非圧縮データが変更されるなどして圧縮データが無効になったときに用いる
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTION

inline
void
CompressedData::
clear()
{
	m_pCompressedData = static_cast<BinaryData*>(0);
}

//	FUNCTION protected
//	Common::CompressedData::getCompressedData
//		-- 圧縮データを得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		圧縮されているデータのObjectPointer
//
//	EXCEPTION

inline
const ObjectPointer<BinaryData>&
CompressedData::
getCompressedData() const
{
	return m_pCompressedData;
}

_TRMEISTER_COMMON_END
_TRMEISTER_END

#endif // __TRMEISTER_COMMON_COMPRESSEDDATA_H

//
//	Copyright (c) 2001, 2002, 2004, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
