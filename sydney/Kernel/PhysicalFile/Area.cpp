// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Area.cpp -- 物理エリア関連の関数定義
// 
// Copyright (c) 2000, 2001, 2002, 2004, 2005, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "PhysicalFile";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "PhysicalFile/Area.h"
#include "PhysicalFile/AreaManagePage.h"

#include "Os/Memory.h"

_SYDNEY_USING
_SYDNEY_PHYSICALFILE_USING

///////////////////////////////////////////////////////////////////////////////
//
//	PhysicalFile::Areaクラスの定数など
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
//	PhysicalFile::Area::Directoryクラスの定数など
//
///////////////////////////////////////////////////////////////////////////////

//
//	CONST private
//	PhysicalFile::Area::Directory::AreaPerBlock --
//		1つの物理エリア管理ブロックで管理する物理エリア数
//
//	NOTES
//	1つの物理エリア管理ブロックで管理する物理エリア数。
//
// static
const AreaNum
Area::Directory::AreaPerBlock = 8;

namespace
{

namespace _SmallAreaDirectory
{

struct _Information
{
	ShortPageOffset m_Offset;
	ShortAreaSize	m_Size;
};

//
//	FUNCTION
//	_SmallAreaDirectory::overwriteInfo --
//		物理エリア情報を上書きする
//
//	NOTES
//	物理エリア情報を上書きする。
//
//	ARGUMENTS
//	void*									InfoTop_
//		物理エリア情報へのポインタ
//	const PhysicalFile::Area::Information&	AreaInfo_
//		物理エリア情報構造体への参照
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
overwriteInfo(void*						InfoTop_,
			  const Area::Information&	AreaInfo_)
{
	//【注意】	いい境界でアクセスできないので、
	//			バッファメモリを直接キャストできない

	_Information tmp;
	tmp.m_Offset = static_cast<ShortPageOffset>(AreaInfo_.m_Offset);
	tmp.m_Size = static_cast<ShortAreaSize>(AreaInfo_.m_Size);

	(void) Os::Memory::copy(InfoTop_, &tmp, sizeof(tmp));
}

//
//	FUNCTION
//	_SmallAreaDirectory::overwriteAreaOffset --
//		物理エリア情報に記録されている
//		「物理エリアオフセット」を上書きする
//
//	NOTES
//	物理エリア情報に記録されている
//	「物理エリアオフセット」を上書きする。
//
//	ARGUMENTS
//	void*							InfoTop_
//		物理エリア情報へのポインタ
//	const PhysicalFile::PageOffset	AreaOffset_
//		物理エリアオフセット
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
overwriteAreaOffset(void*				InfoTop_,
					const PageOffset	AreaOffset_)
{
	//【注意】	いい境界でアクセスできないので、
	//			バッファメモリを直接キャストできない

	_Information tmp;
	tmp.m_Offset = static_cast<ShortPageOffset>(AreaOffset_);

	(void) Os::Memory::copy(InfoTop_, &tmp.m_Offset, sizeof(tmp.m_Offset));
}

//
//	FUNCTION
//	_SmallAreaDirectory::overwriteAreaSize --
//		物理エリア情報に記録されている
//		「物理エリアサイズ」を上書きする
//
//	NOTES
//	物理エリア情報に記録されている
//	「物理エリアサイズ」を上書きする。
//
//	ARGUMENTS
//	void*							InfoTop_
//		物理エリア情報へのポインタ
//	const PhysicalFile::AreaSize	AreaSize_
//		物理エリアサイズ
//
//	RETURN
//	
//
//	EXCEPTIONS
//	なし
//
void
overwriteAreaSize(void*				InfoTop_,
				  const AreaSize	AreaSize_)
{
	//【注意】	いい境界でアクセスできないので、
	//			バッファメモリを直接キャストできない

	_Information tmp;
	tmp.m_Size = static_cast<ShortAreaSize>(AreaSize_);

	(void) Os::Memory::copy(
		static_cast<char*>(InfoTop_) + sizeof(tmp.m_Offset),
		&tmp.m_Size, sizeof(tmp.m_Size));
}

//
//	FUNCTION
//	_SmallAreaDirectory::fetchOutInfo -- 物理エリア情報を取り出す
//
//	NOTES
//	物理エリア情報を取り出す。
//
//	ARGUMENTS
//	const void*							InfoTop_
//		物理エリア情報へのポインタ
//	PhysicalFile::Area::Information&	AreaInfo_
//		物理エリア情報構造体への参照
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
fetchOutInfo(const void*		InfoTop_,
			 Area::Information&	AreaInfo_)
{
	//【注意】	いい境界でアクセスできないので、
	//			バッファメモリを直接キャストできない

	_Information tmp;
	(void) Os::Memory::copy(&tmp, InfoTop_, sizeof(tmp));

	AreaInfo_.m_Offset = tmp.m_Offset;
	AreaInfo_.m_Size = tmp.m_Size;
}

//
//	FUNCTION
//	_SmallAreaDirectory::getAreaOffset --
//		物理エリア情報に記録されている
//		「物理エリアオフセット」を返す
//
//	NOTES
//	物理エリア情報に記録されている
//	「物理エリアオフセット」を返す。
//
//	ARGUMENTS
//	const void*	InfoTop_
//		物理エリア情報へのポインタ
//
//	RETURN
//	PhysicalFile::PageOffset
//		物理エリアオフセット [byte]
//
//	EXCEPTIONS
//	なし
//
PageOffset
getAreaOffset(const void*	InfoTop_)
{
	//【注意】	いい境界でアクセスできないので、
	//			バッファメモリを直接キャストできない

	_Information tmp;
	(void) Os::Memory::copy(&tmp.m_Offset, InfoTop_, sizeof(tmp.m_Offset));

	return tmp.m_Offset;
}

//
//	FUNCTION
//	_SmallAreaDirectory::getAreaSize --
//		物理エリア情報に記録されている
//		「物理エリアサイズ」を返す
//
//	NOTES
//	物理エリア情報に記録されている
//	「物理エリアサイズ」を返す。
//
//	ARGUMENTS
//	const void*	InfoTop_
//		物理エリア情報へのポインタ
//
//	RETURN
//	PhysicalFile::AreaSize
//		物理エリアサイズ [byte]
//
//	EXCEPTIONS
//	なし
//
AreaSize
getAreaSize(const void*	InfoTop_)
{
	//【注意】	いい境界でアクセスできないので、
	//			バッファメモリを直接キャストできない

	_Information tmp;
	(void) Os::Memory::copy(
		&tmp.m_Size,
		static_cast<const char*>(InfoTop_) + sizeof(tmp.m_Offset),
		sizeof(tmp.m_Size));

	return tmp.m_Size;
}

} // end of namespace _SmallAreaDirectory

namespace _LargeAreaDirectory
{

//
//	FUNCTION
//	_LargeAreaDirectory::overwriteInfo --
//		物理エリア情報を上書きする
//
//	NOTES
//	物理エリア情報を上書きする。
//
//	ARGUMENTS
//	void*									InfoTop_
//		物理エリア情報へのポインタ
//	const PhysicalFile::Area::Information&	AreaInfo_
//		物理エリア情報構造体への参照
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
overwriteInfo(void*						InfoTop_,
			  const Area::Information&	AreaInfo_)
{
	//【注意】	いい境界でアクセスできないので、
	//			バッファメモリを直接キャストできない

	(void) Os::Memory::copy(InfoTop_, &AreaInfo_, sizeof(AreaInfo_));
}

//
//	FUNCTION
//	_LargeAreaDirectory::overwriteAreaOffset --
//		物理エリア情報に記録されている
//		「物理エリアオフセット」を上書きする
//
//	NOTES
//	物理エリア情報に記録されている
//	「物理エリアオフセット」を上書きする。
//
//	ARGUMENTS
//	void*							InfoTop_
//		物理エリア情報へのポインタ
//	const PhysicalFile::PageOffset	AreaOffset_
//		物理エリアオフセット
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
overwriteAreaOffset(void*				InfoTop_,
					const PageOffset	AreaOffset_)
{
	//【注意】	いい境界でアクセスできないので、
	//			バッファメモリを直接キャストできない

	(void) Os::Memory::copy(InfoTop_, &AreaOffset_, sizeof(AreaOffset_));
}

//
//	FUNCTION
//	_LargeAreaDirectory::overwriteAreaSize --
//		物理エリア情報に記録されている
//		「物理エリアサイズ」を上書きする
//
//	NOTES
//	物理エリア情報に記録されている
//	「物理エリアサイズ」を上書きする。
//
//	ARGUMENTS
//	void*							InfoTop_
//		物理エリア情報へのポインタ
//	const PhysicalFile::AreaSize	AreaSize_
//		物理エリアサイズ
//
//	RETURN
//	
//
//	EXCEPTIONS
//	なし
//
void
overwriteAreaSize(void*				InfoTop_,
				  const AreaSize	AreaSize_)
{
	//【注意】	いい境界でアクセスできないので、
	//			バッファメモリを直接キャストできない

	(void) Os::Memory::copy(
		static_cast<char*>(InfoTop_) + sizeof(PageOffset),
		&AreaSize_, sizeof(AreaSize_));
}

//
//	FUNCTION
//	_LargeAreaDirectory::fetchOutInfo -- 物理エリア情報を取り出す
//
//	NOTES
//	物理エリア情報を取り出す。
//
//	ARGUMENTS
//	const void*							InfoTop_
//		物理エリア情報へのポインタ
//	PhysicalFile::Area::Information&	AreaInfo_
//		物理エリア情報構造体への参照
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
fetchOutInfo(const void*		InfoTop_,
			 Area::Information&	AreaInfo_)
{
	//【注意】	いい境界でアクセスできないので、
	//			バッファメモリを直接キャストできない

	(void) Os::Memory::copy(&AreaInfo_, InfoTop_, sizeof(AreaInfo_));
}

//
//	FUNCTION
//	_LargeAreaDirectory::getAreaOffset --
//		物理エリア情報に記録されている
//		「物理エリアオフセット」を返す
//
//	NOTES
//	物理エリア情報に記録されている
//	「物理エリアオフセット」を返す。
//
//	ARGUMENTS
//	const void*	InfoTop_
//		物理エリア情報へのポインタ
//
//	RETURN
//	PhysicalFile::PageOffset
//		物理エリアオフセット [byte]
//
//	EXCEPTIONS
//	なし
//
PageOffset
getAreaOffset(const void*	InfoTop_)
{
	//【注意】	いい境界でアクセスできないので、
	//			バッファメモリを直接キャストできない

	Area::Information tmp;
	(void) Os::Memory::copy(&tmp.m_Offset, InfoTop_, sizeof(tmp.m_Offset));

	return tmp.m_Offset;
}

//
//	FUNCTION
//	_LargeAreaDirectory::getAreaSize --
//		物理エリア情報に記録されている
//		「物理エリアサイズ」を返す
//
//	NOTES
//	物理エリア情報に記録されている
//	「物理エリアサイズ」を返す。
//
//	ARGUMENTS
//	const void*	InfoTop_
//		物理エリア情報へのポインタ
//
//	RETURN
//	PhysicalFile::AreaSize
//		物理エリアサイズ [byte]
//
//	EXCEPTIONS
//	なし
//
AreaSize
getAreaSize(const void*	InfoTop_)
{
	//【注意】	いい境界でアクセスできないので、
	//			バッファメモリを直接キャストできない

	Area::Information tmp;
	(void) Os::Memory::copy(
		&tmp.m_Size,
		static_cast<const char*>(InfoTop_) + sizeof(tmp.m_Offset),
		sizeof(tmp.m_Size));

	return tmp.m_Size;
}

} // end of namespace _LargeAreaDirectory

} // end of global namespace

///////////////////////////////////////////////////////////////////////////////
//
//	PhysicalFile::Area::Directoryクラスのpublicメンバ関数
//
///////////////////////////////////////////////////////////////////////////////

//
//	FUNCTION public
//	PhysicalFile::Area::Directory::Directory -- コンストラクタ
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
//	なし
//
Area::Directory::Directory()
	: m_VersionPageSize(0),
	  m_VersionPageDataSize(0)
{
}

//
//	FUNCTION public
//	PhysicalFile::Area::Directory::~Directory -- デストラクタ
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
Area::Directory::~Directory()
{
}

//
//	FUNCTION public
//	PhysicalFile::Area::Directory::intialize -- 初期化
//
//	NOTES
//	コンストラクタ。
//
//	ARGUMENTS
//	PhysicalFile::PageSize	VersionPageSize_
//		バージョンページサイズ [byte]
//	PhysicalFile::PageSize	VersionPageDataSize_
//		バージョンページデータサイズ [byte]
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
Area::Directory::initialize(PageSize	VersionPageSize_,
							PageSize	VersionPageDataSize_)
{
	m_VersionPageSize = VersionPageSize_;
	m_VersionPageDataSize = VersionPageDataSize_;
	
	if ((VersionPageSize_ & ConstValue::PageSizeUpperMask) > 0)	{

		// 指定サイズが 65536 バイトを超えた（要素は４バイト）

		this->m_InfoType = Area::Directory::LargeInfoType;
		this->m_InfoSize = sizeof(Information);
	} else {

		// 指定サイズが 65536 バイトを超えなかった（要素は２バイト）

		this->m_InfoType = Area::Directory::SmallInfoType;
		this->m_InfoSize = sizeof(_SmallAreaDirectory::_Information);
	}

	// ヘッダサイズ
	m_BlockSize = 1 + (AreaPerBlock * this->m_InfoSize);
//				 ~~~ 物理エリア使用状態ビットマップのサイズ [byte]

	this->setFunction();
}

//
//	FUNCTION public
//	PhysicalFile::Area::Directory::getSize --
//		物理エリア管理ディレクトリサイズを返す
//
//	NOTES
//	引数AreaNum_で指定される物理エリアを管理している状態の
//	物理エリア管理ディレクトリサイズを返す。
//
//	ARGUMENTS
//	const PhysicalFile::AreaNum	AreaNum_
//		物理エリア数
//
//	RETURN
//	PhysicalFile::PageSize
//		物理エリア管理ディレクトリサイズ [byte]
//
//	EXCEPTIONS
//	なし
//
PageSize
Area::Directory::getSize(const AreaNum	AreaNum_) const
{
	// 物理エリア管理ブロック数を求める
	AreaNum	blockNum =
		AreaNum_ / PhysicalFile::Area::Directory::AreaPerBlock;

	// 物理エリア管理ブロックサイズを求める
	PageSize	blockSize = blockNum * this->m_BlockSize;

	// 余りの物理エリア情報数を求める
	AreaNum	modulusAreaNum =
		AreaNum_ % PhysicalFile::Area::Directory::AreaPerBlock;

	// 余りの物理エリア情報サイズを求める
	PageSize	modulusSize = 0;
	if (modulusAreaNum > 0)
	{
		modulusSize = 1 + (modulusAreaNum * this->m_InfoSize);
	}

	// 物理エリア管理ディレクトリサイズを返す
	return blockSize + modulusSize;
}

//
//	FUNCTION public
//	PhysicalFile::Area::Directory::getSize --
//		物理エリア管理ディレクトリサイズを返す
//
//	NOTES
//	引数AreaNum_で指定される物理エリアを管理している状態の
//	物理エリア管理ディレクトリサイズを返す。
//
//	ARGUMENTS
//	PhysicalFile::PageSize	VersionPageSize_
//		バージョンページサイズ [byte]
//	const PhysicalFile::AreaNum	AreaNum_
//		物理エリア数
//
//	RETURN
//	PhysicalFile::PageSize
//		物理エリア管理ディレクトリサイズ [byte]
//
//	EXCEPTIONS
//	なし
//
PageSize
Area::Directory::getSize(PageSize		VersionPageSize_,
						 const AreaNum	AreaNum_)
{
	// BlockSizeとInfoSizeを決定する
	PageSize BlockSize;
	PageSize InfoSize;
	
	if ((VersionPageSize_ & ConstValue::PageSizeUpperMask) > 0)
	{
		InfoSize = sizeof(Information);
	}
	else
	{
		InfoSize = sizeof(_SmallAreaDirectory::_Information);
	}

	BlockSize = 1 + (AreaPerBlock * InfoSize);
	
	// 物理エリア管理ブロック数を求める
	AreaNum	blockNum =
		AreaNum_ / PhysicalFile::Area::Directory::AreaPerBlock;

	// 物理エリア管理ブロックサイズを求める
	PageSize	blockSize = blockNum * BlockSize;

	// 余りの物理エリア情報数を求める
	AreaNum	modulusAreaNum =
		AreaNum_ % PhysicalFile::Area::Directory::AreaPerBlock;

	// 余りの物理エリア情報サイズを求める
	PageSize	modulusSize = 0;
	if (modulusAreaNum > 0)
	{
		modulusSize = 1 + (modulusAreaNum * InfoSize);
	}

	// 物理エリア管理ディレクトリサイズを返す
	return blockSize + modulusSize;
}

//
//	FUNCTION public
//	PhysicalFile::Area::Directory::overwriteInfo --
//		物理エリア情報を上書きする
//
//	NOTES
//	物理エリア情報を上書きする。
//
//	ARGUMENTS
//	void*									PagePointer_
//		物理ページのバッファリング内容へのポインタ
//	const PhysicalFile::AreaID				AreaID_
//		物理エリア識別子
//	const PhysicalFile::Area::Information&	AreaInfo_
//		物理エリア情報構造体への参照
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
Area::Directory::overwriteInfo(
	void*						PageTop_,
	const AreaID				AreaID_,
	const Area::Information&	AreaInfo_)
{
	// 物理エリア情報のオフセットを得る
	PageOffset	infoOffset = this->getInfoOffset(AreaID_);
	
	// 物理エリア情報へのポインタを設定する
	void*	infoTop = static_cast<char*>(PageTop_) + infoOffset;

	(*(this->OverwriteInfo))(infoTop, AreaInfo_);
}

//
//	FUNCTION public
//	PhysicalFile::Area::Directory::appendInformation --
//		物理エリア管理ディレクトリに物理エリア情報を追加する
//
//	NOTES
//	物理エリア管理ディレクトリに物理エリア情報を追加する。
//	物理エリア使用状態ビットマップの更新は行わない。
//
//	ARGUMENTS
//	void*									PageTop_
//		物理ページのバッファリング内容へのポインタ
//	const PhysicalFile::Area::Information&	AreaInfo_
//		物理エリア情報構造体への参照
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
Area::Directory::appendInformation(void*					PageTop_,
								   const Area::Information&	AreaInfo_)
{
	// 物理ページヘッダに記録されている
	// 「管理している物理エリア数」を得る
	AreaManagePageHeader pageHeader(this->m_VersionPageSize);
	
	AreaNum	areaNum = (*pageHeader.GetManageAreaNum)(PageTop_);

	// 物理エリア情報を書き込む
	this->overwriteInfo(PageTop_,
						static_cast<AreaID>(areaNum),
						AreaInfo_);
}

//
//	FUNCTION public
//	PhysicalFile::Area::Directory::overwriteAreaOffset --
//		物理エリア情報に記録されている「物理エリアオフセット」を上書きする
//
//	NOTES
//	物理エリア情報に記録されている「物理エリアオフセット」を上書きする。
//
//	ARGUMENTS
//	void*							PageTop_
//		物理ページのバッファリング内容へのポインタ
//	const PhysicalFile::AreaID		AreaID_
//		物理エリア識別子
//	const PhysicalFile::PageOffset	AreaOffset_
//		物理エリアオフセット [byte]
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
Area::Directory::overwriteAreaOffset(void*				PageTop_,
									 const AreaID		AreaID_,
									 const PageOffset	AreaOffset_)
{
	// 物理エリア情報のオフセットを得る
	PageOffset	infoOffset = this->getInfoOffset(AreaID_);
	
	// 物理エリア情報へのポインタを設定する
	void*	infoTop = static_cast<char*>(PageTop_) + infoOffset;

	(*(this->OverwriteAreaOffset))(infoTop, AreaOffset_);
}

//
//	FUNCTION public
//	PhysicalFile::Area::Directory::overwriteAreaSize --
//		物理エリア情報に記録されている「物理エリアサイズ」を上書きする
//
//	NOTES
//	物理エリア情報に記録されている「物理エリアサイズ」を上書きする。
//
//	ARGUMENTS
//	void*							PageTop_
//		物理ページのバッファリング内容へのポインタ
//	const PhysicalFile::AreaID		AreaID_
//		物理エリア識別子
//	const PhysicalFile::AreaSize	AreaSize_
//		物理エリアサイズ [byte]
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
Area::Directory::overwriteAreaSize(void*			PageTop_,
								   const AreaID		AreaID_,
								   const AreaSize	AreaSize_)
{
	// 物理エリア情報のオフセットを得る
	PageOffset	infoOffset = this->getInfoOffset(AreaID_);

	// 物理エリア情報へのポインタを設定する
	void*	infoTop = static_cast<char*>(PageTop_) + infoOffset;

	(*(this->OverwriteAreaSize))(infoTop, AreaSize_);
}

//
//	FUNCTION public
//	PhysicalFile::Area::Directory::overwriteBitValue --
//		物理エリア使用状態ビットマップのビットをON/OFFする
//
//	NOTES
//	物理エリア管理ディレクトリ内の「物理エリア使用状態ビットマップ」の
//	ビットをON/OFFする。
//	設定するビットは、引数AreaID_で指定される物理エリアを管理している
//	ただ1つのビットである。
//
//	ARGUMENTS
//	void*						PageTop_
//		物理ページのバッファリング内容へのポインタ
//	const PhysicalFile::AreaID	AreaID_
//		物理エリア識別子
//	const bool					BitON_
//		ビットをONするかどうか
//			true  : ビットをONにする
//			false : ビットをOFFにする
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
Area::Directory::overwriteBitValue(void*		PageTop_,
								   const AreaID	AreaID_,
								   const bool	BitON_)
{
	// 物理エリア使用状態ビットマップのオフセットを得る
	PageOffset	bitmapOffset = this->getBitmapOffset(AreaID_);

	// 物理エリア使用状態ビットマップへのポインタを設定する
	unsigned char*	bitmapWritePos =
		static_cast<unsigned char*>(PageTop_) + bitmapOffset;

	// 物理エリア使用状態ビットマップのビット番号を得る
	unsigned int	bitNumber = this->getBitNumber(AreaID_);

	// マスクデータを設定する
	unsigned char	maskData = 1 << bitNumber;

	if (BitON_)
	{
		// 物理エリア使用状態ビットマップのビットをONにする
		*bitmapWritePos |= maskData;
	}
	else
	{
		// 物理エリア使用状態ビットマップのビットをOFFにする
		*bitmapWritePos &= ~maskData;
	}
}

//
//	FUNCTION public
//	PhysicalFile::Area::Directory::isUsedArea --
//		使用中の物理エリアかどうかをチェックする
//
//	NOTES
//	引数AreaID_が使用中の物理エリアの識別子かどうかをチェックする。
//
//	ARGUMENTS
//	const void*					PageTop_
//		物理ページのバッファリング内容へのポインタ
//	const PhysicalFile::AreaID	AreaID_
//		チェックする物理エリアの識別子
//
//	RETURN
//	bool
//		使用中の物理エリアかどうか
//			true  : 引数AreaID_が使用中の物理エリアの識別子である
//			false : 引数AreaID_が未使用の物理エリアの識別子である
//
//	EXCEPTIONS
//	なし
//
bool
Area::Directory::isUsedArea(const void*		PageTop_,
							const AreaID	AreaID_) const
{
	// 物理エリア使用状態ビットマップのオフセットを得る
	PageOffset	bitmapOffset = this->getBitmapOffset(AreaID_);

	// 物理エリア使用状態ビットマップへのポインタを設定する
	const unsigned char*	bitmapReadPos =
		static_cast<const unsigned char*>(PageTop_) + bitmapOffset;

	// 物理エリア使用状態ビットマップのビット番号を得る
	unsigned int	bitNumber = this->getBitNumber(AreaID_);

	// マスクデータを設定する
	unsigned char	maskData = 1 << bitNumber;

	return (*bitmapReadPos & maskData) != 0;
}

//
//	FUNCTION public
//	PhysicalFile::Area::Directory::fetchOutInfo --
//		物理エリア情報を取り出す
//
//	NOTES
//	物理エリア情報に記録されているすべての情報を取り出し、
//	引数AreaInfo_に設定する。
//
//	ARGUMENTS
//	const void*							PageTop_
//		物理ページのバッファリング内容へのポインタ
//	const PhysicalFile::AreaID			AreaID_
//		物理エリア識別子
//	PhysicalFile::Area::Information&	AreaInfo_
//		物理エリア情報構造体への参照
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
Area::Directory::fetchOutInfo(const void*			PageTop_,
							  const AreaID			AreaID_,
							  Area::Information&	AreaInfo_) const
{
	// 物理エリア情報オフセットを得る
	PageOffset	infoOffset = this->getInfoOffset(AreaID_);

	// 物理エリア情報へのポインタを設定する
	const void*	infoTop = static_cast<const char*>(PageTop_) + infoOffset;

	(*(this->FetchOutInfo))(infoTop, AreaInfo_);
}

//
//	FUNCTION public
//	PhysicalFile::Area::Directory::getAreaOffset --
//		物理エリア情報に記録されている「物理エリアオフセット」を返す
//
//	NOTES
//	物理エリア情報に記録されている「物理エリアオフセット」を返す。
//
//	ARGUMENTS
//	const void*					PageTop_
//		物理ページのバッファリング内容へのポインタ
//	const PhysicalFlie::AreaID	AreaID_
//		物理エリア識別子
//
//	RETURN
//	PhysicalFile::PageOffset
//		物理エリアオフセット [byte]
//
//	EXCEPTIONS
//	なし
//
PageOffset
Area::Directory::getAreaOffset(const void*	PageTop_,
							   const AreaID	AreaID_) const
{
	// 物理エリア情報オフセットを得る
	PageOffset	infoOffset = this->getInfoOffset(AreaID_);

	// 物理エリア情報へのポインタを設定する
	const void*	infoTop = static_cast<const char*>(PageTop_) + infoOffset;

	return (*(this->GetAreaOffset))(infoTop);
}

//
//	FUNCTION public
//	PhysicalFile::Area::Directory::getAreaSize --
//		物理エリア情報に記録されている「物理エリアサイズ」を返す
//
//	NOTES
//	物理エリア情報に記録されている「物理エリアサイズ」を返す。
//
//	ARGUMENTS
//	const void*					PageTop_
//		物理ページのバッファリング内容へのポインタ
//	const PhysicalFile::AreaID	AreaID_
//		物理エリア識別子
//
//	RETURN
//	PhysicalFile::AreaSize
//		物理エリアサイズ [byte]
//
//	EXCEPTIONS
//	なし
//
AreaSize
Area::Directory::getAreaSize(const void*	PageTop_,
							 const AreaID	AreaID_) const
{
	// 物理エリア情報オフセットを得る
	PageOffset	infoOffset = this->getInfoOffset(AreaID_);

	// 物理エリア情報へのポインタを設定する
	const void*	infoTop = static_cast<const char*>(PageTop_) + infoOffset;

	return (*(this->GetAreaSize))(infoTop);
}

//
//	FUNCTION public
//	PhysicalFile::Area::Directory::getOverwriteAreaID --
//		上書き可能な物理エリア情報の物理エリア識別子を返す
//
//	NOTES
//	上書き可能な物理エリア情報の物理エリア識別子を返す。
//	ただし、上書き可能な物理エリア情報が物理ページ内に
//	存在しない場合には、PhysicalFile::ConstValue::UndefinedAreaID
//	を返す。
//
//	ARGUMENTS
//	const void*					PageTop_
//		物理ページのバッファリング内容へのポインタ
//	const PhysicalFile::AreaNum	ManageAreaNum_
//		物理ページ内で管理している物理エリア数
//
//	RETURN
//	PhysicalFile::AreaID
//		上書き可能な物理エリア情報の物理エリア識別子
//		または、PhysicalFile::ConstValue::UndefinedAreaID
//
//	EXCEPTIONS
//	なし
//
AreaID
Area::Directory::getOverwriteAreaID(const void*		PageTop_,
									const AreaNum	ManageAreaNum_) const
{
	if (ManageAreaNum_ == 0)
	{
		// 物理ページ内で管理している物理エリアが存在しない…

		return ConstValue::UndefinedAreaID;
	}

	AreaID	lastAreaID = ManageAreaNum_ - 1;

	for (AreaID areaID = 0; areaID <= lastAreaID; areaID++)
	{
		if (this->isUsedArea(PageTop_, areaID) == false &&
			this->getAreaOffset(PageTop_, areaID) ==
			ConstValue::UndefinedAreaOffset)
		{
			// 上書き可能なエリア情報…

			return areaID;
		}
	}

	return ConstValue::UndefinedAreaID;
}

//
//	FUNCTION public
//	PhysicalFile::Area::Directory::getOverwriteAreaNum --
//		上書き可能な物理エリア情報数を返す
//
//	NOTES
//	上書き可能な物理エリア情報数を返す。
//
//	ARGUMENTS
//	const void*					PageTop_
//		物理ページのバッファリング内容へのポインタ
//	const PhysicalFile::AreaNum	ManageAreaNum_
//		物理ページ内で管理している物理エリア数
//
//	RETURN
//	PhysicalFile::AreaNum
//		上書き可能な物理エリア情報数
//
//	EXCEPTIONS
//	なし
//
// static
AreaNum
Area::Directory::getOverwriteAreaNum(const void*	PageTop_,
									 const AreaNum	ManageAreaNum_) const
{
	if (ManageAreaNum_ == 0)
	{
		// 物理ページ内で管理している物理エリアが存在しない…

		return 0;
	}

	AreaNum	overwriteAreaNum = 0;

	AreaID	lastAreaID = ManageAreaNum_ - 1;

	for (AreaID areaID = 0; areaID <= lastAreaID; areaID++)
	{
		if (this->isUsedArea(PageTop_, areaID) == false &&
			this->getAreaOffset(PageTop_, areaID) ==
			ConstValue::UndefinedAreaOffset)
		{
			// 上書き可能なエリア情報…

			overwriteAreaNum++;
		}
	}

	return overwriteAreaNum;
}

///////////////////////////////////////////////////////////////////////////////
//
//	PhysicalFile::Area::Directoryクラスのprivateメンバ関数
//
///////////////////////////////////////////////////////////////////////////////

//
//	FUNCTION private
//	PhysicalFile::Area::Directory::setFunction --
//		物理エリア情報にアクセスするためのメソッドを設定する
//
//	NOTES
//	物理エリア情報にアクセスするためのメソッドを設定する。
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
void
Area::Directory::setFunction()
{
	if (this->m_InfoType == Area::Directory::SmallInfoType)
	{
		this->OverwriteInfo =
			_SmallAreaDirectory::overwriteInfo;

		this->OverwriteAreaOffset =
			_SmallAreaDirectory::overwriteAreaOffset;

		this->OverwriteAreaSize =
			_SmallAreaDirectory::overwriteAreaSize;

		this->FetchOutInfo =
			_SmallAreaDirectory::fetchOutInfo;

		this->GetAreaOffset =
			_SmallAreaDirectory::getAreaOffset;

		this->GetAreaSize =
			_SmallAreaDirectory::getAreaSize;
	}
	else
	{
		this->OverwriteInfo =
			_LargeAreaDirectory::overwriteInfo;

		this->OverwriteAreaOffset =
			_LargeAreaDirectory::overwriteAreaOffset;

		this->OverwriteAreaSize =
			_LargeAreaDirectory::overwriteAreaSize;

		this->FetchOutInfo =
			_LargeAreaDirectory::fetchOutInfo;

		this->GetAreaOffset =
			_LargeAreaDirectory::getAreaOffset;

		this->GetAreaSize =
			_LargeAreaDirectory::getAreaSize;
	}
}

//
//	Copyright (c) 2000, 2001, 2002, 2004, 2005, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
