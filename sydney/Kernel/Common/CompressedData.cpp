// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// CompressedData.cpp -- 圧縮されたデータ関連の関数定義
// 
// Copyright (c) 2001, 2002, 2003, 2006, 2009, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Common";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Common/Assert.h"
#include "Common/CompressedData.h"
#include "Common/SystemParameter.h"
#include "Common/Thread.h"
#include "Common/UnicodeString.h"

#include "Exception/BadArgument.h"
#include "Exception/MemoryExhaust.h"
#include "Exception/Unexpected.h"
#include "Os/AutoCriticalSection.h"
#include "Os/Memory.h"

#include "ModDefaultManager.h"

#include "zlib.h"

_TRMEISTER_USING
_TRMEISTER_COMMON_USING

namespace
{
	Os::CriticalSection _criticalSection;

	const ModUnicodeString _pszUncompressedSizeKey(
		_TRMEISTER_U_STRING("Common_CompressThreshold"));
	const ModSize _iDefaultCompressThreshold = 1024;
	ModSize _iCompressThreshold = 0;

	void
	_initialize()
	{
		if (_iCompressThreshold == 0) {
			// むやみにクリティカルセクションに入らないように
			// 値がセットされているかを調べてから入る
			Os::AutoCriticalSection m(_criticalSection);

			// 上記のif文を2つ以上のスレッドが突破しているかもしれないので
			// ここで再度チェックする
			if (_iCompressThreshold == 0) {
				int iValue = 0;
				if (SystemParameter::getValue(
						_pszUncompressedSizeKey, iValue)) {
					if (iValue > 2) {
						_iCompressThreshold = static_cast<ModSize>(iValue);

					} else {
						// 最小値は2
						_iCompressThreshold = 2;
					}
				} else {
					_iCompressThreshold = _iDefaultCompressThreshold;
				}
			}
		}
	}

	void
	_compress(void* destination, ModSize& destinationLength,
			  const void* source, const ModSize sourceLength)
	{
		uLongf len = destinationLength;
		const int error = ::compress((Byte*)destination, &len,
									 (const Bytef*)source, sourceLength);
		switch(error) {
		case Z_OK:
			break;
		case Z_MEM_ERROR:
			_TRMEISTER_THROW0(Exception::MemoryExhaust);
		case Z_BUF_ERROR:
			_TRMEISTER_THROW0(Exception::BadArgument);
		default:
			_TRMEISTER_THROW0(Exception::Unexpected);
		}
		destinationLength = static_cast<ModSize>(len);
	}

	void
	_uncompress(void* destination, ModSize& destinationLength,
				const void* source, const ModSize sourceLength)
	{
		uLongf len = destinationLength;
		const int error = ::uncompress((Byte*)destination, &len,
									   (const Bytef*)source, sourceLength);
		switch(error) {
		case Z_OK:
			break;
		case Z_MEM_ERROR:
			_TRMEISTER_THROW0(Exception::MemoryExhaust);
		case Z_BUF_ERROR:
		case Z_DATA_ERROR:
			_TRMEISTER_THROW0(Exception::BadArgument);
		default:
			_TRMEISTER_THROW0(Exception::Unexpected);
		}
		destinationLength = static_cast<ModSize>(len);
	}
} // namespace

//	FUNCTION pubic
//	Common::CompressedData::serialize -- シリアル化
//
//	NOTES
//	シリアル化を行う
//
//	ARGUMENTS
//	ModArchive& cArchiver_
//		アーカイバー
//
//	RETURN
//	なし
//
//	EXCEPTIONS

void
CompressedData::
serialize(ModArchive& cArchiver_)
{
	ModSize iValueSize;
	ModSize iCompressedSize;
	if (cArchiver_.isStore())
	{
		iValueSize = getValueSize();
		iCompressedSize = getCompressedSize();
		cArchiver_ << iValueSize;
		cArchiver_ << iCompressedSize;
	}
	else
	{
		cArchiver_ >> iValueSize;
		cArchiver_ >> iCompressedSize;
		m_iValueSize = iValueSize;
		m_pCompressedData = new BinaryData();
	}
	if (iCompressedSize) {
		; _TRMEISTER_ASSERT(m_pCompressedData.get());
		m_pCompressedData->serialize(cArchiver_);
	}
}

//	FUNCTION public
//	Common::CompressedData::setCompressedValue -- データを設定する
//
//	NOTES
//		伸張後のデータサイズはコンストラクターなどで設定される必要がある
//
//	ARGUMENTS
//		const void* pCompressedData_
//			圧縮済みのデータへのポインタ
//		ModSize iCompressedSize_
//			圧縮済みのデータサイズ
//
//	RETURN
//	なし
//
//	EXCEPTIONS

void
CompressedData::
setCompressedValue(const void* pCompressedData_, ModSize iCompressedSize_)
{
	m_pCompressedData = new BinaryData(pCompressedData_, iCompressedSize_);
}

//	FUNCTION public
//	Common::CompressedData::setCompressedValue -- データを設定する
//
//	NOTES
//
//	ARGUMENTS
//		const void* pCompressedData_
//			圧縮済みのデータへのポインタ
//		ModSize iCompressedSize_
//			圧縮済みのデータサイズ
//		ModSize iValueSize_
//			伸張後のデータサイズ
//
//	RETURN
//	なし
//
//	EXCEPTIONS

void
CompressedData::
setCompressedValue(const void* pCompressedData_, ModSize iCompressedSize_, ModSize iValueSize_)
{
	setCompressedValue(pCompressedData_, iCompressedSize_);
	m_iValueSize = iValueSize_;
}

//	FUNCTION public
//	Common::CompressedData::setCompressedValue -- データを設定する
//
//	NOTES
//		バッファをコピーせずにそのままの領域を使う
//
//	ARGUMENTS
//		const void* pCompressedData_
//			圧縮済みのデータへのポインタ
//		ModSize iCompressedSize_
//			圧縮済みのデータサイズ
//		ModSize iValueSize_
//			伸張後のデータサイズ
//		ModSize iAllocatedSize_
//			伸張後のデータサイズ
//
//	RETURN
//	なし
//
//	EXCEPTIONS

void
CompressedData::
setCompressedValue(const void* pCompressedData_, ModSize iCompressedSize_,
				   ModSize iValueSize_, ModSize iAllocatedSize_)
{
	m_pCompressedData = new BinaryData(pCompressedData_, iCompressedSize_, false, iAllocatedSize_);
	m_iValueSize = iValueSize_;
}

//	FUNCTION public
//	Common::CompressedData::isCompressable -- 圧縮すると、圧縮されるか
//
//	NOTES
//
//	ARGUMENTS
//		void*				p
//			圧縮しようとしている領域の先頭アドレス
//		ModSize				size
//			圧縮しようとしている領域のサイズ(B 単位)
//
//	RETURN
//		true
//			圧縮される
//		false
//			圧縮されない
//
//	EXCEPTIONS

// static
bool
CompressedData::isCompressable(const void* p, ModSize size)
{
	_initialize();

	return size >= _iCompressThreshold;
}


//	FUNCTION public
//	Common::CompressedData::compress --
//		ある領域を圧縮し、その結果を他の領域に格納する
//
//	NOTES
//
//	ARGUMENTS
//		void*			dst
//			圧縮結果を格納する領域の先頭アドレス
//		ModSize&		dstSize
//			圧縮結果を格納する領域のサイズ(B 単位)で、
//			実行が成功すると、圧縮結果のサイズが格納される
//		void*			src
//			圧縮する領域の先頭アドレス
//		ModSize			srcSize
//			圧縮する領域のサイズ(B 単位)
//
//	RETURN
//		なし
//
//	EXCEPTIONS

// static
void
CompressedData::compress(void* dst, ModSize& dstSize,
						 const void* src, ModSize srcSize)
{
	if (isCompressable(src, srcSize)) {

		// 圧縮すると、圧縮されるサイズの領域を
		// 圧縮しようとしているので、実際に圧縮する

		_compress(dst, dstSize, src, srcSize);

	} else if (dstSize >= srcSize) {

		// 圧縮しても、圧縮されないサイズの領域を圧縮しようとしたときは、
		// 圧縮しようとした領域をそのまま複写する

		(void) Os::Memory::copy(dst, src, srcSize);
		dstSize = srcSize;
	} else

		// 複写先の領域のサイズが圧縮しようとした領域のサイズより小さい

		_TRMEISTER_THROW0(Exception::MemoryExhaust);
}

//	FUNCTION public
//	Common::CompressedData::uncompress --
//		ある領域を伸長し、その結果を他の領域に格納する
//
//	NOTES
//
//	ARGUMENTS
//		void*			dst
//			伸長結果を格納する領域の先頭アドレス
//		ModSize&		dstSize
//			伸長結果を格納する領域のサイズ(B 単位)で、
//			実行が成功すると、伸長結果のサイズが格納される
//		void*			src
//			伸長する領域の先頭アドレス
//		ModSize			srcSize
//			伸長する領域のサイズ(B 単位)
//
//	RETURN
//		なし
//
//	EXCEPTIONS

// static
void
CompressedData::uncompress(void* dst, ModSize& dstSize,
						   const void* src, ModSize srcSize)
{
	_uncompress(dst, dstSize, src, srcSize);
}

//	FUNCTION protected
//	Common::CompressedData::compress -- 圧縮する
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

void
CompressedData::compress(const void* buf, ModSize size)
{
	if (!isCompressable(buf, size)) {
		// 圧縮の必要がなければそのままセットする
		setCompressedValue(buf, size, size);

	} else {
		m_iValueSize = size;
		ModSize iBufferSize = size - 1;
		void* pszDestination = ModDefaultManager::allocate(iBufferSize);

		try {
			ModSize iCompressedSize = iBufferSize;
			_compress(pszDestination, iCompressedSize, buf, size);
			// 圧縮したデータをバイナリデータとして保持する
			// ★注意★
			// 内部でallocateしないモードのコンストラクターを使う
			// 以降の処理ではpszDestinationをfreeしてはいけない

			m_pCompressedData = new BinaryData(pszDestination, iCompressedSize, false, iBufferSize);

		} catch (Exception::BadArgument&) {
			ModDefaultManager::free(pszDestination, iBufferSize);

			// 例外がBadArgumentなら圧縮しても元よりも小さくならなかったということなので
			// 圧縮しないで保持する
			setCompressedValue(buf, size);
			Thread::resetErrorCondition();

		} catch (...) {
			ModDefaultManager::free(pszDestination, iBufferSize);
			_TRMEISTER_RETHROW;
		}
	}
}

//	FUNCTION protected
//	Common::CompressedData::uncompress --
//		格納しているデータを伸張したものを得る
//
//	NOTES
//
//	ARGUMENTS
//		void*		buf
//			伸張した結果を格納する領域の先頭アドレス
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
CompressedData::uncompress(void* buf) const
{
	if (isCompressed()) {

		// 格納しているデータは圧縮されているので、伸張する

		ModSize uncompressedSize = getValueSize();
		_uncompress(buf, uncompressedSize,
					m_pCompressedData->getValue(),
					m_pCompressedData->getSize());

		; _TRMEISTER_ASSERT(getValueSize() == uncompressedSize);

	} else if (m_pCompressedData.get())

		// 格納しているデータは圧縮されていないので、
		// そのままコピーする

		Os::Memory::copy(
			buf, m_pCompressedData->getValue(), m_pCompressedData->getSize());
	else

		// データを格納していない
		_TRMEISTER_THROW0(Exception::BadArgument);
}

//	FUNCTION protected
//	Common::CompressedData::setValue -- 内容を複写する
//
//	NOTES
//
//	ARGUMENTS
//		const CompressedData& cOther_
//			複写もとのデータ
//
//	RETURN
//	なし
//
//	EXCEPTIONS

void
CompressedData::
setValue(const CompressedData& cOther_)
{
	m_iValueSize = cOther_.m_iValueSize;
	m_pCompressedData = cOther_.m_pCompressedData;
}

//
//	Copyright (c) 2001, 2002, 2003, 2006, 2009, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
