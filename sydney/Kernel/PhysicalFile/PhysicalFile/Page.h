// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Page.h -- 物理ページ関連のクラス定義、関数宣言
// 
// Copyright (c) 2000, 2001, 2002, 2004, 2005, 2007, 2013, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_PHYSICALFILE_PAGE_H
#define __SYDNEY_PHYSICALFILE_PAGE_H

#include "PhysicalFile/Module.h"
#include "PhysicalFile/Area.h"
#include "PhysicalFile/Content.h"
#include "PhysicalFile/Types.h"
#include "PhysicalFile/DirectArea.h"

#include "Common/Object.h"
#include "Common/BitSet.h"

#include "ModVector.h"

_SYDNEY_BEGIN
_SYDNEY_PHYSICALFILE_BEGIN

class File;

//
//	CLASS
//	PhysicalFile::Page --
//		物理ページ記述子の基本クラス
//
//	NOTES
//	物理ページ記述子の基本クラス。
//
class SYD_PHYSICALFILE_FUNCTION Page : public Common::Object
{
	friend class File;
	friend class AreaManageFile;
	friend class PageManageFile;
	friend class DirectAreaFile;
	friend class Content;

public:

	//
	//	STRUCT public
	//	PhysicalFile::Page::UnfixMode --
	//		アンフィックスモード
	//
	//	NOTES
	//	物理ページのバッファリング内容のアンフィックスモード。
	//
	struct UnfixMode
	{
		//
		//	ENUM
		//	PhysicalFile::Page::UnfixMode::Value --
		//		アンフィックスモードの値
		//
		//	NOTES
		//	アンフィックスモードの値。
		//
		enum Value
		{
			NotDirty = 0,	// 物理ページを更新しなかった
			Dirty,			// 物理ページを更新した
			Omit			// 明示的にアンフィックスモードを指定しない
		};
	};

	//
	// メンバ関数
	//

	// void*へのキャスト演算子
	operator void*() const;

	// char*へのキャスト演算子
	operator char*() const;

	// const void*へのキャスト演算子
	operator const void*() const;

	// const char*へのキャスト演算子
	operator const char*() const;

	// アタッチ時に指定したフィックスモードを返す
	Buffer::Page::FixMode::Value getFixMode() const;

	// 物理ページへデータを書き込む
	void write(const Trans::Transaction&	Transaction_,
			   const void*					Buffer_,
			   const PageOffset				Offset_,
			   const PageSize				Size_);

	// 物理ページからデータを読み込む
	void read(const Trans::Transaction&	Transaction_,
			  void*						Buffer_,
			  const PageOffset			Offset_,
			  const PageSize			Size_);

	// 物理エリアを確保する(detachPageAllを呼ぶこと)
	virtual AreaID
		allocateArea(const Trans::Transaction&	Transaction_,
					 const AreaSize				AreaSize_,
					 const bool					WithCompaction_ = false);

	// 物理エリアを解放する(detachPageAllを呼ぶこと)
	virtual void freeArea(const Trans::Transaction&			cTransaction_,
						  AreaID							uiAreaID_);

	// 物理エリアを再利用する(detachPageAllを呼ぶこと)
	virtual AreaID reuseArea(const Trans::Transaction&	Transaction_,
							 const AreaID				AreaID_);
	
	// 物理エリアへデータを書き込む
	virtual void writeArea(const Trans::Transaction&	Transaction_,
						   const AreaID					AreaID_,
						   const void*					Buffer_,
						   const AreaOffset				Offset_,
						   const AreaSize				Size_);

	// 物理エリアからデータを読み込む
	virtual void readArea(const Trans::Transaction&	Transaction_,
						  const AreaID				AreaID_,
						  void*						Buffer_,
						  const AreaOffset			Offset_,
						  const AreaSize			Size_);

	// バッファリング内容を返す
	Content fix();

	// 物理エリア情報を取り出す
	virtual void fetchOutAreaInformation(
		const Trans::Transaction&	Transaction_,
		const AreaID				AreaID_,
		Area::Information&			AreaInfo_) const;

	// 物理エリア情報を取り出す
	virtual void fetchOutAreaInformation(
		const Content&		Content_,
		const AreaID		AreaID_,
		Area::Information&	AreaInfo_) const;

	// 物理エリア情報に記録されている物理エリアオフセットを返す
	// Get DirectArea's offset.
	virtual PageOffset getAreaOffset(AreaID	AreaID_) const;

	// 物理エリア情報に記録されている物理エリアサイズを返す
	// Get DirectArea's size.
	virtual AreaSize getAreaSize(AreaID	AreaID_) const;

	// 物理エリアを拡大／縮小する(detachPageAllを呼ぶこと)
	virtual bool changeAreaSize(
		const Trans::Transaction&	Transaction_,
		Content&					Content_,
		const AreaID				AreaID_,
		const AreaSize				Size_,
		const bool					DoCompaction_ = false);

	// 物理エリアを拡大／縮小する(detachPageAllを呼ぶこと)
	virtual bool changeAreaSize(
		const Trans::Transaction&		cTransaction_,
		AreaID							uiAreaID_,
		AreaSize						uiSize_,
		bool							bDoCompaction_ = false);

	// 物理エリアを再配置する(detachPageAllを呼ぶこと)
	virtual void compaction(const Trans::Transaction&	Transaction_,
							Content&					Content_);

	// 物理エリアを再配置する(detachPageAllを呼ぶこと)
	virtual void compaction(const Trans::Transaction&	Transaction_);

	// 先頭の使用中物理エリアの識別子を返す
	virtual AreaID getTopAreaID(const Content&	Content_) const;

	// 先頭の使用中物理エリアの識別子を返す
	virtual AreaID getTopAreaID(
		const Trans::Transaction&	Transaction_) const;

	// 最後の使用中物理エリアの識別子を返す
	virtual AreaID getLastAreaID(const Content&	Content_) const;

	// 最後の使用中物理エリアの識別子を返す
	virtual AreaID getLastAreaID(
		const Trans::Transaction&	Transaction_) const;

	// 次の使用中物理エリアの識別子を返す
	virtual AreaID getNextAreaID(const Content&	Content_,
								 const AreaID	AreaID_) const;

	// 次の使用中物理エリアの識別子を返す
	virtual AreaID getNextAreaID(const Trans::Transaction&	Transaction_,
								 const AreaID				AreaID_) const;

	// 前の使用中物理エリアの識別子を返す
	virtual AreaID getPrevAreaID(const Content&	Content_,
								 const AreaID	AreaID_) const;

	// 前の使用中物理エリアの識別子を返す
	virtual AreaID getPrevAreaID(const Trans::Transaction&	Transaction_,
								 const AreaID				AreaID_) const;

	// 物理ファイル記述子を返す
	virtual File* getFile() const = 0;

	// 物理ページ識別子を返す
	PageID getID() const;

	// 未使用領域サイズを返す
	virtual PageSize getUnuseAreaSize(
		const Content&	Content_,
		const AreaNum	AreaNum_ = 1) const;

	// 未使用領域サイズを返す
	virtual PageSize getUnuseAreaSize(
		const Trans::Transaction&	Transaction_,
		const AreaNum				AreaNum_ = 1) const;

	// 空き領域サイズを返す
	virtual PageSize getFreeAreaSize(
		const Content&	Content_,
		const AreaNum	AreaNum_ = 1) const;

	// 空き領域サイズを返す
	virtual PageSize getFreeAreaSize(
		const Trans::Transaction&	Transaction_,
		const AreaNum				AreaNum_ = 1) const;

	// 物理ページの更新を通知する
	void dirty();

	// アンフィックスモードを返す
	UnfixMode::Value getUnfixMode() const;

	// 物理ページデータサイズを返す
	virtual PageSize getPageDataSize(const AreaNum	AreaNum_ = 1) const = 0;

	virtual void detach(){ --m_ReferenceCounter; };

#ifdef DEBUG

	virtual void
		getPageHeader(const Trans::Transaction&	Transaction_,
					  PageSize&					UnuseAreaSize_,
					  unsigned int&				UnuseAreaRate_,
					  PageSize&					FreeAreaSize_,
					  unsigned int&				FreeAreaRate_,
					  PageOffset&				FreeAreaOffset_,
					  AreaNum&					ManageAreaNum_);

	virtual void
		getAreaDirectory(const Trans::Transaction&	Transaction_,
						 unsigned char*				AreaUseFlag_,
						 Area::Information*			AreaInfo_);
#endif

protected:

	//
	// メンバ関数
	//

	// コンストラクタ
	Page(const Trans::Transaction&					Transaction_,
		 File*										File_,
		 const PageID								PageID_,
		 const Buffer::Page::FixMode::Value			FixMode_,
		 const Buffer::ReplacementPriority::Value	ReplacementPriority_ =
									Buffer::ReplacementPriority::Middle);

	// コンストラクタ
	Page(const Trans::Transaction&			Transaction_,
		 File*								File_,
		 const PageID						PageID_,
		 const Buffer::Page::FixMode::Value	FixMode_,
		 Admin::Verification::Progress&		Progress_);

	// デストラクタ
	virtual ~Page();

	// 再利用
	virtual	void
	reset(const Trans::Transaction&				Transaction_,
		  PageID								PageID_,
		  Buffer::Page::FixMode::Value			FixMode_,
		  Buffer::ReplacementPriority::Value	ReplacementPriority_ =
			   						Buffer::ReplacementPriority::Middle) = 0;
	void reset(const Trans::Transaction&			Transaction_,
			   File*								File_,
			   PageID								PageID_,
			   Buffer::Page::FixMode::Value			FixMode_,
			   Buffer::ReplacementPriority::Value	ReplacementPriority_);

	//
	// データメンバ
	//

	// 物理ページ識別子
	PageID							m_ID;
	// フィックスモード
	Buffer::Page::FixMode::Value	m_FixMode;

	// 物理ファイル記述子
	// superクラスでFile*を持ってしまうと、
	// subクラスでキャストしてからFileのsubクラスの関数を呼ばなくてはならない
	//File*							m_File;

	// バージョンページのバッファリング内容
	Version::Page::Memory			m_Memory;
	// 物理ページのバッファリング内容
	Content							m_Content;

	// 物理ページに対応するバージョンページの
	// バッファリング内容の先頭アドレス
	char*							m_VersionPageTop;
	// 物理ページのバッファリング内容の先頭アドレス
	char*							m_PhysicalPageTop;

	//
	// 以下のメンバは、物理ファイル記述子が管理する
	//

	// 参照カウンタ
	int								m_ReferenceCounter;

	// アタッチ中の物理ページ記述子をつなぐリンク
	Page*							m_Next;
	Page*							m_Prev;

	// デタッチした物理ページ記述子をつなぐリンク
	Page*							m_Free;

	// アンフィックスモード
	UnfixMode::Value				m_UnfixMode;

private:

	//
	// メンバ関数
	//

	// 利用者に公開する領域のサイズを返す
#ifdef DEBUG
	virtual PageSize getUserAreaSize() const = 0;
#endif

	//
	// 整合性検査のためのメソッド
	//

	// 利用者と自身の物理エリアの使用状況が一致するかどうかをチェックする
	virtual void
		correspondUseArea(const Trans::Transaction&			Transaction_,
						  void*								TableTop_,
						  Common::BitSet&					AreaIDs_,
						  const AreaID						LastAreaID_,
						  Admin::Verification::Progress&	Progress_);

	// 物理エリア情報検査
	virtual void
		checkPhysicalArea(
			Admin::Verification::Progress&	Progress_) const;

}; // end of class PhysicalFile::Page

//
//	FUNCTION public
//	PhysicalFile::Page::operator const void* --
//		const void*へのキャスト演算子
//
//	NOTES
//	const void*へのキャスト演算子。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	const void*
//		物理ページ先頭へのポインタ
//
//	EXCEPTIONS
//	なし
//
inline
Page::operator const void*() const
{
	return this->m_PhysicalPageTop;
}

//
//	FUNCTION public
//	PhysicalFile::Page::operator const char* --
//		const char*へのキャスト演算子
//
//	NOTES
//	const char*へのキャスト演算子。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	const char*
//		物理ページ先頭へのポインタ
//
//	EXCEPTIONS
//	なし
//
inline
Page::operator const char*() const
{
	return this->m_PhysicalPageTop;
}

//	FUNCTION public
//	PhysicalFile::Page::getFixMode --
//		アタッチ時に指定したフィックスモードを返す
//
//	NOTES
//	アタッチ時に指定したフィックスモードを返す。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Buffer::Page::FixMode::Value
//		フィックスモード
//
//	EXCEPTIONS
//	なし

inline
Buffer::Page::FixMode::Value
Page::getFixMode() const
{
	return this->m_FixMode;
}

//
//	FUNCTION public
//	PhysicalFile::Page::getID --
//		物理ページ識別子を返す
//
//	NOTES
//	物理ページ識別子を返す。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	PhysicalFile::PageID
//		物理ページ識別子
//
//	EXCEPTIONS
//	なし
//
inline
PageID
Page::getID() const
{
	return this->m_ID;
}

//
//	FUNCTION public
//	PhysicalFile::Page::dirty --
//		物理ページの更新を通知する
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
inline
void
Page::dirty()
{
	this->m_UnfixMode = UnfixMode::Dirty;
}

//
//	FUNCTION public
//	PhysicalFile::Page::getUnfixMode --
//		アンフィックスモードを返す
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	PhysicalFile::Page::UnfixMode::Value
//		アンフィックスモード
//
//	EXCEPTIONS
//	なし
//
inline
Page::UnfixMode::Value
Page::getUnfixMode() const
{
	return m_UnfixMode;
}

_SYDNEY_PHYSICALFILE_END
_SYDNEY_END

#endif //__SYDNEY_PHYSICALFILE_PAGE_H

//
//	Copyright (c) 2000, 2001, 2002, 2004, 2005, 2007, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
