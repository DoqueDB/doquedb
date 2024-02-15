// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Page.cpp -- 物理ページ関連の関数定義
// 
// Copyright (c) 2000, 2001, 2002, 2004, 2006, 2007, 2023 Ricoh Company, Ltd.
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

#include "Exception/NotSupported.h"

#include "Common/Assert.h"

#include "PhysicalFile/Page.h"
#include "PhysicalFile/File.h"
#include "PhysicalFile/DirectAreaPage.h"

#include "ModOsDriver.h"

_SYDNEY_USING
_SYDNEY_PHYSICALFILE_USING

///////////////////////////////////////////////////////////////////////////////
//
//	PhysicalFile::Pageクラスのpublicメンバ関数
//
///////////////////////////////////////////////////////////////////////////////

//	FUNCTION public
//	PhysicalFile::Page::operator void* --
//		void*へのキャスト演算子
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	void*
//		物理ページ先頭へのポインタ
//
//	EXCEPTIONS
//	なし

Page::operator void*() const
{
	; _SYDNEY_ASSERT(getFixMode() != Buffer::Page::FixMode::ReadOnly);

	return m_PhysicalPageTop;
}

//	FUNCTION public
//	PhysicalFile::Page::operator char* --
//		char*へのキャスト演算子
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	char*
//		物理ページ先頭へのポインタ
//
//	EXCEPTIONS
//	なし

Page::operator char*() const
{
	; _SYDNEY_ASSERT(getFixMode() != Buffer::Page::FixMode::ReadOnly);

	return m_PhysicalPageTop;
}

//
//	FUNCTION public
//	PhysicalFile::Page::write --
//		物理ページへデータを書き込む
//
//	NOTES
//	物理ページへデータを書き込む。
//	※ 空き領域管理機能付き物理ファイルでも、
//	　 利用者がこの関数により物理ページ内にデータを書き込むことは可能だが、
//	　 利用者が確保した物理エリア以外に書き込む事のないように注意が必要。
//
//	ARGUMENTS
//	const Trans::Transaction&		Transaction_
//		トランザクション記述子への参照
//	const void*						Buffer_
//		書き込むデータが格納されているバッファへのポインタ
//	const PhysicalFile::PageOffset	Offset_
//		物理ページ内の書き込み開始位置 [byte]
//	const PhysicalFile::PageSize	Size_
//		書き込みサイズ [byte]
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
Page::write(const Trans::Transaction&	Transaction_,
			const void*					Buffer_,
			const PageOffset			Offset_,
			const PageSize				Size_)
{
	; _SYDNEY_ASSERT(Offset_ >= 0 &&
					 (Offset_ + Size_ <= this->getUserAreaSize()));

	// 物理ページへデータを書き込む
	ModOsDriver::Memory::copy(this->m_PhysicalPageTop + Offset_,
							  Buffer_,
							  Size_);
}

//
//	FUNCTION public
//	PhysicalFile::Page::read --
//		物理ページからデータを読み込む
//
//	NOTES
//	物理ページからデータを読み込む。
//
//	ARGUMENTS
//	const Trans::Transaction&		Transaction_
//		トランザクション記述子への参照
//	const void*						Buffer_
//		物理ページから読み込んだデータを格納するバッファへのポインタ
//	const PhysicalFile::PageOffset	Offset_
//		物理ページ内の読み込み開始位置 [byte]
//	const PhysicalFile::PageSize	Size_
//		読み込みサイズ [byte]
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
Page::read(const Trans::Transaction&	Transaction_,
		   void*						Buffer_,
		   const PageOffset				Offset_,
		   const PageSize				Size_)
{
	; _SYDNEY_ASSERT(Offset_ >= 0 &&
					 (Offset_ + Size_ <= this->getUserAreaSize()));

	// 物理ページからデータを読み込む
	ModOsDriver::Memory::copy(Buffer_,
							  this->m_PhysicalPageTop + Offset_,
							  Size_);
}

//
//	FUNCTION public
//	PhysicalFile::Page::allocateArea --
//		物理エリアを確保する
//
//	NOTES
//	物理エリアを確保する。
//
//	※ 空き領域管理機能付き物理ファイルのみ利用可能。
//	　 （空き領域管理機能付き物理ファイル記述子クラスでのみ
//	　 　この関数をオーバーライドしている。）
//
//	ARGUMENTS
//	const Trans::Transaction&		Transaction_
//		トランザクション記述子への参照
//	const PhysicalFile::AreaSize	AreaSize_
//		確保する物理エリアのサイズ [byte]
//	const bool						WithCompaction_ = false
//		物理ページ内に物理エリアを確保するために十分な空き領域が
//		存在しない場合に、物理エリアの再配置(compaction)を
//		行うかどうか。
//			true  : 物理エリアの再配置を行う
//			false : 物理エリアの再配置を行わない
//
//	RETURN
//	PhysicalFile::AreaID
//		生成した物理エリアの識別子
//
//	EXCEPTIONS
//	なし
//
// virtual
AreaID
Page::allocateArea(const Trans::Transaction&	Transaction_,
				   const AreaSize				AreaSize_,
				   const bool					WithCompaction_)
{
	throw Exception::NotSupported(moduleName, srcFile, __LINE__);
}

//
//	FUNCTION public
//	PhysicalFile::Page::freeArea --
//		物理エリアを解放する
//
//	NOTES
//	物理エリアを解放する。
//
//	※ 空き領域管理機能付き物理ファイルのみ利用可能。
//	　 （空き領域管理機能付き物理ファイル記述子クラスでのみ
//	　 　この関数をオーバーライドしている。）
//
//	ARGUMENTS
//	const Trans::Transaction&	Transaction_
//		トランザクション記述子への参照
//	PhysicalFile::AreaID		AreaID_
//		解放する物理エリアの識別子
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
// virtual
void
Page::freeArea(const Trans::Transaction&		cTransaction_,
			   AreaID							uiAreaID_)
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

//
//	FUNCTION public
//	PhysicalFile::Page::reuseArea --
//		物理エリアを再利用する
//
//	NOTES
//	物理エリアを再利用する。
//
//	※ 空き領域管理機能付き物理ファイルのみ利用可能。
//	　 （空き領域管理機能付き物理ファイル記述子クラスでのみ
//	　 　この関数をオーバーライドしている。）
//
//	ARGUMENTS
//	const Trans::Transaction&	Transaction_
//		トランザクション記述子への参照
//	const PhysicalFile::AreaID	AreaID_
//		再利用する物理エリアの識別子
//
//	RETURN
//	PhysicalFile::AreaID
//		再利用した物理エリアの識別子
//
//	EXCEPTIONS
//	なし
//
// virtual
AreaID
Page::reuseArea(const Trans::Transaction&	Transaction_,
				const AreaID				AreaID_)
{
	throw Exception::NotSupported(moduleName, srcFile, __LINE__);
}

//
//	FUNCTION public
//	PhysicalFile::Page::writeArea --
//		物理エリアへデータを書き込む
//
//	NOTES
//	物理エリアへデータを書き込む。
//
//	※ 空き領域管理機能付き物理ファイルのみ利用可能。
//	　 （空き領域管理機能付き物理ファイル記述子クラスでのみ
//	　 　この関数をオーバーライドしている。）
//
//	ARGUMENTS
//	const Trans::Transaction&		Transaction_
//		トランザクション記述子への参照
//	const PhysicalFile::AreaID		AreaID_
//		データを書き込む物理エリアの識別子
//	const void*						Buffer_
//		書き込むデータが格納されているバッファへのポインタ
//	const PhysicalFile::AreaOffset	Offset_
//		物理エリア内の書き込み開始位置 [byte]
//	const PhysicalFile::AreaSize	Size_
//		書き込みサイズ [byte]
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
// virtual
void
Page::writeArea(const Trans::Transaction&	Transaction_,
				const AreaID				AreaID_,
				const void*					Buffer_,
				const AreaOffset			Offset_,
				const AreaSize				Size_)
{
	throw Exception::NotSupported(moduleName, srcFile, __LINE__);
}

//
//	FUNCTION public
//	PhysicalFile::Page::readArea --
//		物理エリアからデータを読み込む
//
//	NOTES
//	物理エリアからデータを読み込む。
//
//	※ 空き領域管理機能付き物理ファイルのみ利用可能。
//	　 （空き領域管理機能付き物理ファイル記述子クラスでのみ
//	　 　この関数をオーバーライドしている。）
//
//	ARGUMENTS
//	const Trans::Transaction&		Transaction_
//		トランザクション記述子への参照
//	const PhysicalFile::AreaID		AreaID_
//		データを読み込む物理エリアの識別子
//	void*							Buffer_
//		物理エリアから読み込んだデータを格納するバッファへのポインタ
//	const PhysicalFile::AreaOffset	Offset_
//		物理エリア内の読み込み開始位置 [byte]
//	const PhysicalFile::AreaSize	Size_
//		読み込みサイズ [byte]
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
// virtual
void
Page::readArea(const Trans::Transaction&	Transaction_,
			   const AreaID					AreaID_,
			   void*						Buffer_,
			   const AreaOffset				Offset_,
			   const AreaSize				Size_)
{
	throw Exception::NotSupported(moduleName, srcFile, __LINE__);
}

//	FUNCTION public
//	PhysicalFile::Page::fix --
//		バッファリング内容を返す
//
//	NOTES
//	物理ページのバッファリング内容を返す。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	PhysicalFile::Content
//		物理ページのバッファリング内容
//
//	EXCEPTIONS
//	なし

Content
Page::fix()
{
	return m_Content;
}

//
//	FUNCTION public
//	PhysicalFile::Page::fetchOutAreaInformation --
//		物理エリア情報を取り出す
//
//	NOTES
//	引数AreaID_で指定される物理エリアに関する情報を読み出して、
//	引数AreaInfo_へ設定する。
//
//	ARGUMENTS
//	const Trans::Transaction&			Transaction_
//		トランザクション記述子への参照への参照
//	const PhysicalFile::AreaID			AreaID_
//		物理エリア識別子
//	PhysicalFile::Area::Information&	AreaInfo_
//		物理エリア情報への参照
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
// virtual
void
Page::fetchOutAreaInformation(
	const Trans::Transaction&	Transaction_,
	const AreaID				AreaID_,
	Area::Information&			AreaInfo_) const
{
	throw Exception::NotSupported(moduleName, srcFile, __LINE__);
}

//
//	FUNCTION public
//	PhysicalFile::Page::fetchOutAreaInformation --
//		物理エリア情報を取り出す
//
//	NOTES
//	引数AreaID_で指定される物理エリアに関する情報を読み出して、
//	引数AreaInfo_へ設定する。
//
//	※ 空き領域管理機能付き物理ファイルのみ利用可能。
//	　 （空き領域管理機能付き物理ファイル記述子クラスでのみ
//	　 　この関数をオーバーライドしている。）
//
//	ARGUMENTS
//	const PhysicalFile::Content&		Content_
//		関数PhysicalFile::Page::fixで得られる
//		物理ページのバッファリング内容への参照
//	const PhysicalFile::AreaID			AreaID_
//		物理エリア識別子
//	PhysicalFile::Area::Information&	AreaInfo_
//		物理エリア情報への参照
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
// virtual
void
Page::fetchOutAreaInformation(const Content&		Content_,
							  const AreaID			AreaID_,
							  Area::Information&	AreaInfo_) const
{
	throw Exception::NotSupported(moduleName, srcFile, __LINE__);
}

//
//	FUNCTION public
//	PhysicalFile::Page::getAreaOffset --
//		物理エリア情報に記録されている物理エリアオフセットを返す
//		Get DirectArea's offset.
//
//	NOTES
//	物理エリア情報に記録されている物理エリアオフセットを返す。
//
//	※ 空き領域管理機能付き物理ファイルのみ利用可能。
//	　 （空き領域管理機能付き物理ファイル記述子クラスでのみ
//	　 　この関数をオーバーライドしている。）
//
//	ARGUMENTS
//	PhysicalFile::AreaID	AreaID_
//		物理エリア識別子
//
//	RETURN
//	PhysicalFile::PageOffset
//		物理エリアオフセット（物理ページ内での開始位置）
//
//	EXCEPTIONS
//	なし
//
PageOffset
Page::getAreaOffset(AreaID	AreaID_) const
{
	throw Exception::NotSupported(moduleName, srcFile, __LINE__);

	return 0;
}

//
//	FUNCTION public
//	PhysicalFile::Page::getAreaSize --
//		物理エリア情報に記録されている物理エリアサイズを返す
//		Get DirectArea's size.
//
//	NOTES
//	物理エリア情報に記録されている物理エリアサイズを返す。
//
//	※ 空き領域管理機能付き物理ファイルのみ利用可能。
//	　 （空き領域管理機能付き物理ファイル記述子クラスでのみ
//	　 　この関数をオーバーライドしている。）
//
//	ARGUMENTS
//	PhysicalFile::AreaID	AreaID_
//		物理エリア識別子
//
//	RETURN
//	PhysicalFile::AreaSize
//		物理エリアサイズ
//
//	EXCEPTIONS
//	なし
//
AreaSize
Page::getAreaSize(AreaID	AreaID_) const
{
	throw Exception::NotSupported(moduleName, srcFile, __LINE__);

	return 0;
}

//
//	FUNCTION public
//	PhysicalFile::Page::changeAreaSize --
//		物理エリアを拡大／縮小する
//
//	NOTES
//	物理エリアを拡大／縮小する。
//	縮小時には戻り値は常にtrueとなるが、
//	拡大時には拡大が可能で実際に拡大を行った場合にtrueとなる。
//
//	※ 空き領域管理機能付き物理ファイルのみ利用可能。
//	　 （空き領域管理機能付き物理ファイル記述子クラスでのみ
//	　 　この関数をオーバーライドしている。）
//
//	ARGUMENTS
//	const Trans::Transaction&		Transaction_
//		トランザクション記述子への参照
//	PhysicalFile::Content&			Content_
//		関数PhysicalFile::Page::fixで得られる
//		物理ページのバッファリング内容への参照
//	const PhysicalFile::AreaID		AreaID_
//		拡大または縮小する物理エリアの識別子
//	const PhysicalFile::AreaSize	Size_
//		拡大または縮小後の物理エリアのサイズ [byte]
//	const bool						DoCompaction_ = false
//		物理エリアの再配置も行うかどうか
//			true  : 必要に応じて、物理エリアの再配置も行う
//			false : 物理エリアの再配置は行わない
//
//	RETURN
//	bool
//		物理エリアを拡大／縮小したかどうか
//			true  : 物理エリアを拡大／縮小した
//			false : 物理エリアを拡大しなかった（できなかった）
//
//	EXCEPTIONS
//	なし
//
// virtual
bool
Page::changeAreaSize(const Trans::Transaction&	Transaction_,
					 Content&					Content_,
					 const AreaID				AreaID_,
					 const AreaSize				Size_,
					 const bool					DoCompaction_ // = false
					 )
{
	throw Exception::NotSupported(moduleName, srcFile, __LINE__);
}

//
//	FUNCTION public
//	PhysicalFile::Page::changeAreaSize --
//		物理エリアを拡大／縮小する
//
//	NOTES
//	物理エリアを拡大／縮小する。
//	縮小時には戻り値は常にtrueとなるが、
//	拡大時には拡大が可能で実際に拡大を行った場合にtrueとなる。
//
//	※ 空き領域管理機能付き物理ファイルのみ利用可能。
//	　 （空き領域管理機能付き物理ファイル記述子クラスでのみ
//	　 　この関数をオーバーライドしている。）
//
//	ARGUMENTS
//	const Trans::Transaction&		Transaction_
//		トランザクション記述子への参照
//	const PhysicalFile::AreaID		AreaID_
//		拡大または縮小する物理エリアの識別子
//	const PhysicalFile::AreaSize	Size_
//		拡大または縮小後の物理エリアのサイズ [byte]
//	const bool						DoCompaction_ = false
//		物理エリアの再配置も行うかどうか
//			true  : 必要に応じて、物理エリアの再配置も行う
//			false : 物理エリアの再配置は行わない
//
//	RETURN
//	bool
//		物理エリアを拡大／縮小したかどうか
//			true  : 物理エリアを拡大／縮小した
//			false : 物理エリアを拡大しなかった（できなかった）
//
//	EXCEPTIONS
//	なし
//
// virtual
bool
Page::changeAreaSize(const Trans::Transaction&		cTransaction_,
					 AreaID							uiAreaID_,
					 AreaSize						uiSize_,
					 bool							bDoCompaction_) // = false
{
	throw Exception::NotSupported(moduleName, srcFile, __LINE__);
}

//
//	FUNCTION public
//	PhysicalFile::Page::compaction --
//		物理エリアを再配置する
//
//	NOTES
//	物理エリアを再配置する。
//	この関数を呼び出して物理エリアの再配置を行うと、
//	以前に解放された物理エリアは再利用不可能となる。
//
//	※ 空き領域管理機能付き物理ファイルのみ利用可能。
//	　 （空き領域管理機能付き物理ファイル記述子クラスでのみ
//	　 　この関数をオーバーライドしている。）
//
//	ARGUMENTS
//	const Trans::Transaction&	Transaction_
//		トランザクション記述子への参照
//	PhysicalFile::Content&		Content_
//		関数PhysicalFile::Page::fixで得られる
//		物理ページのバッファリング内容への参照
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
// virtual
void
Page::compaction(const Trans::Transaction&	Transaction_,
				 Content&					Content_)
{
	throw Exception::NotSupported(moduleName, srcFile, __LINE__);
}

//
//	FUNCTION public
//	PhysicalFile::Page::compaction --
//		物理エリアを再配置する
//
//	NOTES
//	物理エリアを再配置する。
//	この関数を呼び出して物理エリアの再配置を行うと、
//	以前に解放された物理エリアは再利用不可能となる。
//
//	※ 空き領域管理機能付き物理ファイルのみ利用可能。
//	　 （空き領域管理機能付き物理ファイル記述子クラスでのみ
//	　 　この関数をオーバーライドしている。）
//
//	ARGUMENTS
//	const Trans::Transaction&	Transaction_
//		トランザクション記述子への参照
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
// virtual
void
Page::compaction(const Trans::Transaction&	Transaction_)
{
	throw Exception::NotSupported(moduleName, srcFile, __LINE__);
}

//
//	FUNCTION public
//	PhysicalFile::Page::getTopAreaID --
//		先頭の使用中物理エリアの識別子を返す
//
//	NOTES
//	物理ページ内で（利用者が確保し、）使用中の物理エリアのうち、
//	物理エリア識別子順での先頭物理エリアの識別子を返す。
//	（物理的に先頭の物理エリアの識別子ではない。）
//	該当する物理エリアが物理ページ内に存在しない場合には、
//	PhysicalFile::ConstValue::UndefinedAreaIDを返す。
//
//	※ 空き領域管理機能付き物理ファイルのみ利用可能。
//	　 （空き領域管理機能付き物理ファイル記述子クラスでのみ
//	　 　この関数をオーバーライドしている。）
//
//	ARGUMENTS
//	const PhysicalFile::Content&	Content_
//		関数PhysicalFile::Page::fixで得られる
//		物理ページのバッファリング内容への参照
//
//	RETURN
//	PhysicalFile::AreaID
//		物理エリア識別子順での先頭の使用中物理エリアの識別子
//
//	EXCEPTIONS
//	なし
//
// virtual
AreaID
Page::getTopAreaID(const Content&	Content_) const
{
	throw Exception::NotSupported(moduleName, srcFile, __LINE__);
}

//
//	FUNCTION public
//	PhysicalFile::Page::getTopAreaID --
//		先頭の使用中物理エリアの識別子を返す
//
//	NOTES
//	物理ページ内で（利用者が確保し、）使用中の物理エリアのうち、
//	物理エリア識別子順での先頭物理エリアの識別子を返す。
//	（物理的に先頭の物理エリアの識別子ではない。）
//	該当する物理エリアが物理ページ内に存在しない場合には、
//	PhysicalFile::ConstValue::UndefinedAreaIDを返す。
//
//	※ 空き領域管理機能付き物理ファイルのみ利用可能。
//	　 （空き領域管理機能付き物理ファイル記述子クラスでのみ
//	　 　この関数をオーバーライドしている。）
//
//	ARGUMENTS
//	const Trans::Transaction&	Transaction_
//		トランザクション記述子への参照
//
//	RETURN
//	PhysicalFile::AreaID
//		物理エリア識別子順での先頭の使用中物理エリアの識別子
//
//	EXCEPTIONS
//	なし
//
// virtual
AreaID
Page::getTopAreaID(const Trans::Transaction&	Transaction_) const
{
	throw Exception::NotSupported(moduleName, srcFile, __LINE__);
}

//
//	FUNCTION public
//	PhysicalFile::Page::getLastAreaID --
//		最後の使用中物理エリアの識別子を返す
//
//	NOTES
//	物理ページ内で（利用者が確保し、）使用中の物理エリアのうち、
//	物理エリア識別子順での最終物理エリアの識別子を返す。
//	（物理的に最後の物理エリアの識別子ではない。）
//	該当する物理エリアが物理ページ内に存在しない場合には、
//	PhysicalFile::ConstValue::UndefinedAreaIDを返す。
//
//	※ 空き領域管理機能付き物理ファイルのみ利用可能。
//	　 （空き領域管理機能付き物理ファイル記述子クラスでのみ
//	　 　この関数をオーバーライドしている。）
//
//	ARGUMENTS
//	const PhysicalFile::Content&	Content_
//		関数PhysicalFile::Page::fixで得られる
//		物理ページのバッファリング内容への参照
//
//	RETURN
//	PhysicalFile::AreaID
//		物理エリア識別子順での最後の使用中物理エリアの識別子
//
//	EXCEPTIONS
//	なし
//
// virtual
AreaID
Page::getLastAreaID(const Content&	Content_) const
{
	throw Exception::NotSupported(moduleName, srcFile, __LINE__);
}

//
//	FUNCTION public
//	PhysicalFile::Page::getLastAreaID --
//		最後の使用中物理エリアの識別子を返す
//
//	NOTES
//	物理ページ内で（利用者が確保し、）使用中の物理エリアのうち、
//	物理エリア識別子順での最終物理エリアの識別子を返す。
//	（物理的に最後の物理エリアの識別子ではない。）
//	該当する物理エリアが物理ページ内に存在しない場合には、
//	PhysicalFile::ConstValue::UndefinedAreaIDを返す。
//
//	※ 空き領域管理機能付き物理ファイルのみ利用可能。
//	　 （空き領域管理機能付き物理ファイル記述子クラスでのみ
//	　 　この関数をオーバーライドしている。）
//
//	ARGUMENTS
//	const Trans::Transaction&	Transaction_
//		トランザクション記述子への参照
//
//	RETURN
//	PhysicalFile::AreaID
//		物理エリア識別子順での最後の使用中物理エリアの識別子
//
//	EXCEPTIONS
//	なし
//
// virtual
AreaID
Page::getLastAreaID(const Trans::Transaction&	Transaction_) const
{
	throw Exception::NotSupported(moduleName, srcFile, __LINE__);
}

//
//	FUNCTION public
//	PhysicalFile::Page::getNextAreaID --
//		次の使用中物理エリアの識別子を返す
//
//	NOTES
//	引数AreaID_で指定される物理エリアの、
//	物理エリア識別子順での次の使用中物理エリアの識別子を返す。
//	該当する物理エリアが物理ページ内に存在しない場合には、
//	PhysicalFile::ConstValue::UndefinedAreaIDを返す。
//
//	※ 空き領域管理機能付き物理ファイルのみ利用可能。
//	　 （空き領域管理機能付き物理ファイル記述子クラスでのみ
//	　 　この関数をオーバーライドしている。）
//
//	ARGUMENTS
//	const PhysicalFile::Content&	Content_
//		関数PhysicalFile::Page::fixで得られる
//		物理ページのバッファリング内容への参照
//	const PhysicalFile::AreaID		AreaID_
//		物理エリア識別子
//
//	RETURN
//	PhysicalFile::AreaID
//		物理エリア識別子順での次の使用中物理エリアの識別子
//
//	EXCEPTIONS
//	なし
//
// virtual
AreaID
Page::getNextAreaID(const Content&	Content_,
					const AreaID	AreaID_) const
{
	throw Exception::NotSupported(moduleName, srcFile, __LINE__);
}

//
//	FUNCTION public
//	PhysicalFile::Page::getNextAreaID --
//		次の使用中物理エリアの識別子を返す
//
//	NOTES
//	引数AreaID_で指定される物理エリアの、
//	物理エリア識別子順での次の使用中物理エリアの識別子を返す。
//	該当する物理エリアが物理ページ内に存在しない場合には、
//	PhysicalFile::ConstValue::UndefinedAreaIDを返す。
//
//	※ 空き領域管理機能付き物理ファイルのみ利用可能。
//	　 （空き領域管理機能付き物理ファイル記述子クラスでのみ
//	　 　この関数をオーバーライドしている。）
//
//	ARGUMENTS
//	const Trans::Transaction&	Transaction_
//		トランザクション記述子への参照
//	const PhysicalFile::AreaID	AreaID_
//		物理エリア識別子
//
//	RETURN
//	PhysicalFile::AreaID
//		物理エリア識別子順での次の使用中物理エリアの識別子
//
//	EXCEPTIONS
//	なし
//
// virtual
AreaID
Page::getNextAreaID(const Trans::Transaction&	Transaction_,
					const AreaID				AreaID_) const
{
	throw Exception::NotSupported(moduleName, srcFile, __LINE__);
}

//
//	FUNCTION public
//	PhysicalFile::Page::getPrevAreaID --
//		前の使用中物理エリアの識別子を返す
//
//	NOTES
//	引数AreaID_で指定される物理エリアの、
//	物理エリア識別子順での前の使用中物理エリアの識別子を返す。
//	該当する物理エリアが物理ページ内に存在しない場合には、
//	PhysicalFile::ConstValue::UndefinedAreaIDを返す。
//
//	※ 空き領域管理機能付き物理ファイルのみ利用可能。
//	　 （空き領域管理機能付き物理ファイル記述子クラスでのみ
//	　 　この関数をオーバーライドしている。）
//
//	ARGUMENTS
//	const PhysicalFile::Content&	Content_
//		関数PhysicalFile::Page::fixで得られる
//		物理ページのバッファリング内容への参照
//	const PhysicalFile::AreaID		AreaID_
//		物理エリア識別子
//
//	RETURN
//	PhysicalFile::AreaID
//		物理エリア識別子順での前の使用中物理エリアの識別子
//
//	EXCEPTIONS
//	なし
//
// virtual
AreaID
Page::getPrevAreaID(const Content&	Content_,
					const AreaID	AreaID_) const
{
	throw Exception::NotSupported(moduleName, srcFile, __LINE__);
}

//
//	FUNCTION public
//	PhysicalFile::Page::getPrevAreaID --
//		前の使用中物理エリアの識別子を返す
//
//	NOTES
//	引数AreaID_で指定される物理エリアの、
//	物理エリア識別子順での前の使用中物理エリアの識別子を返す。
//	該当する物理エリアが物理ページ内に存在しない場合には、
//	PhysicalFile::ConstValue::UndefinedAreaIDを返す。
//
//	※ 空き領域管理機能付き物理ファイルのみ利用可能。
//	　 （空き領域管理機能付き物理ファイル記述子クラスでのみ
//	　 　この関数をオーバーライドしている。）
//
//	ARGUMENTS
//	const Trans::Transaction&	Transaction_
//		トランザクション記述子への参照
//	const PhysicalFile::AreaID	AreaID_
//		物理エリア識別子
//
//	RETURN
//	PhysicalFile::AreaID
//		物理エリア識別子順での前の使用中物理エリアの識別子
//
//	EXCEPTIONS
//	なし
//
// virtual
AreaID
Page::getPrevAreaID(const Trans::Transaction&	Transaction_,
					const AreaID				AreaID_) const
{
	throw Exception::NotSupported(moduleName, srcFile, __LINE__);
}

//
//	FUNCTION public
//	PhysicalFile::Page::getUnuseAreaSize --
//		未使用領域サイズを返す
//
//	NOTES
//	物理ページ内の未使用領域サイズを返す
//
//	※ 空き領域管理機能付き物理ファイルのみ利用可能。
//	　 （空き領域管理機能付き物理ファイル記述子クラスでのみ
//	　 　この関数をオーバーライドしている。）
//
//	ARGUMENTS
//	const PhysicalFile::Content&	Content_
//		関数PhysicalFile::Page::fixで得られる
//		物理ページのバッファリング内容への参照
//	const PhysicalFile::AreaNum		AreaNum_ = 1
//		生成物理エリア数
//		（物理ページ内にいくつの物理エリアを生成するか）
//
//	RETURN
//	PhysicalFile::PageSize
//		物理ページ内の未使用領域サイズ [byte]
//
//	EXCEPTIONS
//	なし
//
// virtual
PageSize
Page::getUnuseAreaSize(const Content&	Content_,
					   const AreaNum	AreaNum_) const
{
	throw Exception::NotSupported(moduleName, srcFile, __LINE__);
}

//
//	FUNCTION public
//	PhysicalFile::Page::getUnuseAreaSize --
//		未使用領域サイズを返す
//
//	NOTES
//	物理ページ内の未使用領域サイズを返す
//
//	※ 空き領域管理機能付き物理ファイルのみ利用可能。
//	　 （空き領域管理機能付き物理ファイル記述子クラスでのみ
//	　 　この関数をオーバーライドしている。）
//
//	ARGUMENTS
//	const Trans::Transaction&	Transaction_
//		トランザクション記述子への参照
//	const PhysicalFile::AreaNum	AreaNum_ = 1
//		生成物理エリア数
//		（物理ページ内にいくつの物理エリアを生成するか）
//
//	RETURN
//	PhysicalFile::PageSize
//		物理ページ内の未使用領域サイズ [byte]
//
//	EXCEPTIONS
//	なし
//
// virtual
PageSize
Page::getUnuseAreaSize(const Trans::Transaction&	Transaction_,
					   const AreaNum				AreaNum_) const
{
	throw Exception::NotSupported(moduleName, srcFile, __LINE__);
}

//
//	FUNCTION public
//	PhysicalFile::Page::getFreeAreaSize --
//		空き領域サイズを返す
//
//	NOTES
//	物理ページ内の空き領域サイズを返す
//
//	※ 空き領域管理機能付き物理ファイルのみ利用可能。
//	　 （空き領域管理機能付き物理ファイル記述子クラスでのみ
//	　 　この関数をオーバーライドしている。）
//
//	ARGUMENTS
//	const PhysicalFile::Content&	Content_
//		関数PhysicalFile::Page::fixで得られる
//		物理ページのバッファリング内容への参照
//	const PhysicalFile::AreaNum		AreaNum_ = 1
//		生成物理エリア数
//		（物理ページ内にいくつの物理エリアを生成するか）
//
//	RETURN
//	PhysicalFile::PageSize
//		物理ページ内の空き領域サイズ [byte]
//
//	EXCEPTIONS
//	なし
//
// virtual
PageSize
Page::getFreeAreaSize(const Content&	Content_,
					  const AreaNum		AreaNum_) const
{
	throw Exception::NotSupported(moduleName, srcFile, __LINE__);
}

//
//	FUNCTION public
//	PhysicalFile::Page::getFreeAreaSize --
//		空き領域サイズを返す
//
//	NOTES
//	物理ページ内の空き領域サイズを返す
//
//	※ 空き領域管理機能付き物理ファイルのみ利用可能。
//	　 （空き領域管理機能付き物理ファイル記述子クラスでのみ
//	　 　この関数をオーバーライドしている。）
//
//	ARGUMENTS
//	const Trans::Transaction&	Transaction_
//		トランザクション記述子への参照
//	const PhysicalFile::AreaNum	AreaNum_ = 1
//		生成物理エリア数
//		（物理ページ内にいくつの物理エリアを生成するか）
//
//	RETURN
//	PhysicalFile::PageSize
//		物理ページ内の空き領域サイズ [byte]
//
//	EXCEPTIONS
//	なし
//
// virtual
PageSize
Page::getFreeAreaSize(const Trans::Transaction&	Transaction_,
					  const AreaNum				AreaNum_) const
{
	throw Exception::NotSupported(moduleName, srcFile, __LINE__);
}

#ifdef DEBUG

// virtual
void
Page::getPageHeader(const Trans::Transaction&	Transaction_,
					PageSize&					UnuseAreaSize_,
					unsigned int&				UnuseAreaRate_,
					PageSize&					FreeAreaSize_,
					unsigned int&				FreeAreaRate_,
					PageOffset&					FreeAreaOffset_,
					AreaNum&					ManageAreaNum_)
{
	; _SYDNEY_ASSERT(false);
}

// virtual
void
Page::getAreaDirectory(const Trans::Transaction&	Transaction_,
					   unsigned char*				AreaUseFlag_,
					   Area::Information*			AreaInfo_)
{
	; _SYDNEY_ASSERT(false);
}

#endif

///////////////////////////////////////////////////////////////////////////////
//
//	PhysicalFile::Pageクラスのprotectedメンバ関数
//
///////////////////////////////////////////////////////////////////////////////

//	FUNCTION protected
//	PhysicalFile::Page::Page -- コンストラクタ
//
//	NOTES
//	コンストラクタ。
//
//	ARGUMENTS
//	const Trans::Transaction&					Transaction_
//		トランザクション記述子への参照
//	const PhysicalFile::File*					File_
//		物理ファイル記述子
//	const PhysicalFile::PageID					PageID_
//		物理ページ識別子
//	const Buffer::Page::FixMode::Value			FixMode_
//		フィックスモード
//	const Buffer::ReplacementPriority::Value	ReplacementPriority_
//		バッファリング内容の破棄されにくさ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//	※ 版管理マネージャ以下のモジュールからの例外は
//	　 そのまま利用者に投げられる可能性がある

Page::Page(const Trans::Transaction&				Transaction_,
		   File*									File_,
		   const PageID								PageID_,
		   const Buffer::Page::FixMode::Value		FixMode_,
		   const Buffer::ReplacementPriority::Value	ReplacementPriority_)
	: m_ID(PageID_),
	  m_FixMode(FixMode_),
	  m_Memory(Version::Page::fix(Transaction_,
								  *File_->m_VersionFile,
								  File_->convertToVersionPageID(m_ID),
								  FixMode_,
								  ReplacementPriority_)),
	  m_Content(&m_Memory,
				File_->m_Type,
				FixMode_,
				File_->m_VersionPageSize,
				File_->m_VersionPageDataSize),
	  m_PhysicalPageTop((FixMode_ == Buffer::Page::FixMode::ReadOnly) ?
						const_cast<char*>(static_cast<const Content&>(m_Content).operator const char*()) :
						m_Content.operator char*()),
	  m_ReferenceCounter(0),
	  m_Next(0),
	  m_Prev(0),
	  m_Free(0),
	  m_UnfixMode(UnfixMode::Omit)
{}

//	FUNCTION protected
//	PhysicalFile::Page::Page -- コンストラクタ
//
//	NOTES
//	コンストラクタ。
//
//	ARGUMENTS
//	const Trans::Transaction&			Transaction_
//		トランザクション記述子への参照
//	PhysicalFile::File*					File_
//		物理ファイル記述子
//	const PhysicalFile::PageID			PageID_
//		物理ページ識別子
//	const Buffer::Page::FixMode::Value	FixMode_
//		フィックスモード
//	Admin::Verification::Progress&		Progress_
//		整合性検査の途中経過への参照
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//	※ 版管理マネージャ以下のモジュールからの例外は
//	　 そのまま利用者に投げられる可能性がある

Page::Page(const Trans::Transaction&			Transaction_,
		   File*								File_,
		   const PageID							PageID_,
		   const Buffer::Page::FixMode::Value	FixMode_,
		   Admin::Verification::Progress&		Progress_)
	: m_ID(PageID_),
	  m_FixMode(FixMode_),
	  m_Memory(Version::Page::verify(Transaction_,
									 *File_->m_VersionFile,
									 File_->convertToVersionPageID(m_ID),
									 FixMode_,
									 Progress_)),
	  m_Content(&m_Memory,
				File_->m_Type,
				FixMode_,
				File_->m_VersionPageSize,
				File_->m_VersionPageDataSize),
	  m_PhysicalPageTop((FixMode_ == Buffer::Page::FixMode::ReadOnly) ?
						const_cast<char*>(static_cast<const Content&>(m_Content).operator const char*()) :
						m_Content.operator char*()),
	  m_ReferenceCounter(0),
	  m_Next(0),
	  m_Prev(0),
	  m_Free(0),
	  m_UnfixMode(UnfixMode::Omit)
{}

//
//	FUNCTION protected
//	PhysicalFile::Page::reset -- メモリーの再利用
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Page::reset(const Trans::Transaction&			Transaction_,
			File*								File_,
			PageID								PageID_,
			Buffer::Page::FixMode::Value		FixMode_,
			Buffer::ReplacementPriority::Value	ReplacementPriority_)
{
	m_ID = PageID_;
	m_FixMode = FixMode_;
	// 各サブクラスで、適切なFileのサブクラスを設定する。
	//m_File = File_;
	m_Memory = Version::Page::fix(Transaction_,
								  *File_->m_VersionFile,
								  File_->convertToVersionPageID(m_ID),
								  FixMode_,
								  ReplacementPriority_);
	m_Content.reset(&m_Memory,
					File_->m_Type,
					FixMode_,
					File_->m_VersionPageSize,
					File_->m_VersionPageDataSize);
	m_PhysicalPageTop = (FixMode_ == Buffer::Page::FixMode::ReadOnly)
		? const_cast<char*>(static_cast<const Content&>(m_Content).operator const char*())
		: m_Content.operator char*();
	m_ReferenceCounter = 0;
	m_Next = 0;
	m_Prev = 0;
	m_Free = 0;
	m_UnfixMode = UnfixMode::Omit;
}

//	FUNCTION protected
//	PhysicalFile::Page::~Page --
//		デストラクタ
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

Page::~Page()
{}

//
//	FUNCTION private
//	PhysicalFile::Page::correspondUseArea --
//		利用者と自身の物理エリアの
//		使用状況が一致するかどうかをチェックする
//
//	NOTES
//	利用者と自身の物理エリアの使用状況が一致するかどうかをチェックする。
//
//	※ 空き領域管理機能付き物理ファイルのみ利用可能。
//	　 （空き領域管理機能付き物理ファイル記述子クラスでのみ
//	　 　この関数をオーバーライドしている。）
//
//	ARGUMENTS
//	const Trans::Transaction&		Transaction_
//		トランザクション記述子への参照
//	void*							TableTop_
//		空き領域管理表先頭へのポインタ
//	Common::BitSet&					AreaIDs_
//		物理エリア識別子が記録されているビットマップへの参照
//	const PhysicalFile::AreaID		LastAreaID_
//		利用者がその物理ページ内で最後の使用中とした物理エリアの識別子
//	Admin::Verification::Progress&	Progress_
//		整合性検査の途中経過への参照
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
// virtual
void
Page::correspondUseArea(const Trans::Transaction&		Transaction_,
						void*							TableTop_,
						Common::BitSet&					AreaIDs_,
						const AreaID					LastAreaID_,
						Admin::Verification::Progress&	Progress_)
{
	throw Exception::NotSupported(moduleName, srcFile, __LINE__);
}

//
//	FUNCTION private
//	PhysicalFile::Page::checkPhysicalArea -- 物理エリア情報検査
//
//	NOTES
//	物理エリア情報の整合性検査および
//	物理エリアの重複検査を行う。
//
//	※ 空き領域管理機能付き物理ファイルのみ利用可能。
//	　 （空き領域管理機能付き物理ファイル記述子クラスでのみ
//	　 　この関数をオーバーライドしている。）
//
//	ARGUMENTS
//	Admin::Verification::Progress&	Progress_
//		整合性検査の途中経過への参照
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
// virtual
void
Page::checkPhysicalArea(Admin::Verification::Progress&	Progress_) const
{
	throw Exception::NotSupported(moduleName, srcFile, __LINE__);
}

//
//	Copyright (c) 2000, 2001, 2002, 2004, 2006, 2007, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
