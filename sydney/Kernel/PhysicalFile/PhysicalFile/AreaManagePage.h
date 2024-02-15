// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// AreaManagePage.h --
//		空き領域管理機能付き物理ファイルの
//		物理ページ関連のクラス定義、関数宣言
// 
// Copyright (c) 2000, 2001, 2002, 2007, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_PHYSICALFILE_AREAMANAGEPAGE_H
#define __SYDNEY_PHYSICALFILE_AREAMANAGEPAGE_H

#include "PhysicalFile/AreaManageFile.h"
#include "PhysicalFile/Page.h"
#include "PhysicalFile/BitmapTable.h"

_SYDNEY_BEGIN

namespace PhysicalFile
{

//
//	CLASS
//	PhysicalFile::AreaManagePageHeader --
//		空き領域管理機能付き物理ファイルの物理ページヘッダクラス
//
//	NOTES
//	空き領域管理機能付き物理ファイルの物理ページヘッダクラス。
//	空き領域管理機能付き物理ファイルの物理ページヘッダは
//	下図のような物理構造となっている。
//	管理している物理エリア数以外の各情報は
//	それぞれ4バイトで記録するタイプと、
//	2バイトで記録するタイプがある。
//	どちらで記録するかは、物理ファイルごとに、
//	バージョンページサイズにより決まる。
//	管理している物理エリア数は、いずれの場合も
//	2バイトで記録する。
//
//		┌──────────────────┐　　　　┬
//		│　　　　　未使用領域サイズ　　　　　│　　　　│
//		├──────────────────┤　　　　│
//		│　　　　　空き領域サイズ　　　　　　│
//		├──────────────────┤　物理ページヘッダ
//		│　　　　空き領域オフセット　　　　　│
//		├──────────────────┤　　　　│
//		│　　　管理している物理エリア数　　　│　　　　│
//		├──────────────────┤　　　　┴
//		│　　　　　　　　　　　　　　　　　　│
//		＝　　　　　　　　　　　　　　　　　　＝
//
class AreaManagePageHeader : public Common::Object
{
	friend class AreaManageFile;
	friend class Content;

public:

	//
	//	ENUM public
	//	PhysicalFile::AreaManagePageHeader::Type --
	//		物理ページヘッダタイプ
	//
	//	NOTES
	//	空き領域管理機能付き物理ファイルの
	//	物理ページヘッダタイプ。
	//	物理ページヘッダの「管理している物理エリア数」以外の
	//	各項目は、物理ファイルごとに
	//	バージョンページサイズが65536バイト未満の場合には
	//	2バイトで記録し、
	//	65536ページ以上の場合には4バイトで記録する。
	//
	enum Type
	{
		SmallType = 0, // 各項目を2バイトで記録するタイプ
		LargeType,     // 各項目を4バイトで記録するタイプ
		UnknownType
	};

	// 物理ページヘッダサイズ [byte]
	static const PageSize	SmallSize;

	// 「未使用領域サイズ」のオフセット [byte]
	static const PageOffset	SmallUnuseAreaSizeOffset;

	// 「空き領域サイズ」のオフセット [byte]
	static const PageOffset	SmallFreeAreaSizeOffset;

	// 「空き領域オフセット」のオフセット [byte]
	static const PageOffset	SmallFreeAreaOffsetOffset;

	// 「管理している物理エリア数」のオフセット [byte]
	static const PageOffset	SmallManageAreaNumOffset;

	// 物理ページヘッダサイズ [byte]
	static const PageSize	LargeSize;

	// 「未使用領域サイズ」のオフセット [byte]
	static const PageOffset	LargeUnuseAreaSizeOffset;

	// 「空き領域サイズ」のオフセット [byte]
	static const PageOffset	LargeFreeAreaSizeOffset;

	// 「空き領域オフセット」のオフセット [byte]
	static const PageOffset	LargeFreeAreaOffsetOffset;

	// 「管理している物理エリア数」のオフセット [byte]
	static const PageOffset	LargeManageAreaNumOffset;

	//
	//	STRUCT public
	//	PhysicalFile::AreaManagePageHeader::Item --
	//		空き領域管理機能付き物理ファイルの
	//		物理ページヘッダに記録されている項目
	//
	//	NOTES
	//	空き領域管理機能付き物理ファイルの
	//	物理ページヘッダに記録されている項目。
	//
	struct Item
	{
		// 未使用領域サイズ
		PageSize	m_UnuseAreaSize;

		// 空き領域サイズ
		PageSize	m_FreeAreaSize;

		// 空き領域オフセット
		PageOffset	m_FreeAreaOffset;

		// 管理している物理エリア数
		AreaNum		m_ManageAreaNum;
	};

	//
	// 物理ページヘッダに記録されている各項目へ
	// アクセスするためのメソッド
	//

	// 物理ページヘッダを上書きする
	typedef void (*OverwriteFunc)(void*			HeaderTop_,
								  const Item&	Item_);

	// 未使用領域サイズを上書きする
	typedef
		void (*OverwriteUnuseAreaSizeFunc)(void*			HeaderTop_,
										   const PageSize	UnseAreaSize_);

#ifdef OBSOLETE
	// 空き領域サイズを上書きする
	typedef
		void (*OverwriteFreeAreaSizeFunc)(void*				HeaderTop_,
										  const PageSize	FreeAreaSize_);
#endif // OBSOLETE

#ifdef OBSOLETE
	// 空き領域オフセットを上書きする
	typedef
		void
		(*OverwriteFreeAreaOffsetFunc)(void*			HeaderTop_,
									   const PageOffset	FreeAreaOffset_);
#endif // OBSOLETE

#ifdef OBSOLETE
	// 管理している物理エリア数を更新する
	typedef void (*UpdateManageAreaNumFunc)(void*		HeaderTop_,
											const bool	Increment_);
#endif // OBSOLETE

	// 物理ページヘッダに記録されているすべての項目を取り出す
	typedef void (*FetchOutFunc)(const void*	HeaderTop_,
								 Item&			Item);

	// 未使用領域サイズ・空き領域サイズを取り出す
	typedef void (*FetchOutAreaSizeFunc)(const void*	HeaderTop_,
										 PageSize&		UnuseAreaSize_,
										 PageSize&		FreeAreaSize_);

	// 未使用領域サイズを返す
	typedef PageSize (*GetUnuseAreaSizeFunc)(const void*	HeaderTop_);

	// 空き領域サイズを返す
	typedef PageSize (*GetFreeAreaSizeFunc)(const void*	HeaderTop_);

#ifdef OBSOLETE
	// 空き領域オフセットを返す
	typedef PageOffset (*GetFreeAreaOffsetFunc)(const void*	HeaderTop_);
#endif // OBSOLETE

	// 管理している物理エリア数を返す
	typedef AreaNum (*GetManageAreaNumFunc)(const void*	HeaderTop_);

	//
	// アクセスする関数
	//

	// 物理ページヘッダを上書きする
	OverwriteFunc				Overwrite;

	// 未使用領域サイズを上書きする
	OverwriteUnuseAreaSizeFunc	OverwriteUnuseAreaSize;

#ifdef OBSOLETE
	// 空き領域サイズを上書きする
	OverwriteFreeAreaSizeFunc	OverwriteFreeAreaSize;
#endif // OBSOLETE

#ifdef OBSOLETE
	// 空き領域オフセットを上書きする
	OverwriteFreeAreaOffsetFunc	OverwriteFreeAreaOffset;
#endif // OBSOLETEv

#ifdef OBSOLETE
	// 管理している物理エリア数を更新する
	UpdateManageAreaNumFunc		UpdateManageAreaNum;
#endif // OBSOLETE

	// 物理ページヘッダに記録されているすべての項目を取り出す
	FetchOutFunc				FetchOut;

	// 未使用領域サイズ・空き領域サイズを取り出す
	FetchOutAreaSizeFunc		FetchOutAreaSize;

	// 未使用領域サイズを返す
	GetUnuseAreaSizeFunc		GetUnuseAreaSize;

	// 空き領域サイズを返す
	GetFreeAreaSizeFunc			GetFreeAreaSize;

#ifdef OBSOLETE
	// 空き領域オフセットを返す
	GetFreeAreaOffsetFunc		GetFreeAreaOffset;
#endif // OBSOLETE

	// 管理している物理エリア数を返す
	GetManageAreaNumFunc		GetManageAreaNum;
	
	//
	// メンバ関数
	//

	// コンストラクタ
	AreaManagePageHeader(const PageSize VersionPageSize_);

	// デストラクタ
	virtual ~AreaManagePageHeader();

	// 物理ページヘッダを初期化する
	static void initialize(void*			PagePointer_,
						   const PageSize	VersionPageSize_,
						   const PageSize	VersionPageDataSize_);

	// 物理ページヘッダサイズを返す [byte]
	PageSize getSize() const;
	static PageSize getSize(PageSize	VersionPageSize_);

	// 管理している物理エリア数を返す
	static AreaNum getManageAreaNum(PageSize	VersionPageSize_,
									const void*	HeaderTop_);

	//
	// データメンバ
	//

	// 物理ページヘッダタイプ
	Type		m_Type;

private:

	//
	// メンバ関数
	//

	// アクセスする関数を設定する
	void setFunction();

	//
	// データメンバ
	//

	// 物理ページヘッダサイズ
	PageSize	m_Size;

}; // end of class PhysicalFile::AreaManagePageHeader

//
//	CLASS
//	PhysicalFile::AreaManagePage --
//		空き領域管理機能付き物理ファイルの物理ページ記述子クラス
//
//	NOTES
//	空き領域管理機能付き物理ファイルの物理ページ記述子クラス。
//
class AreaManagePage : public Page
{
	friend class File;
	friend class AreaManageFile;

public:

	//
	// メンバ関数
	//

	// 物理エリアを確保する
	AreaID allocateArea(const Trans::Transaction&	Transaction_,
						const AreaSize				AreaSize_,
						const bool					WithCompaction_ = false);

	// 物理エリアを解放する
	void freeArea(const Trans::Transaction&	Transaction_,
				  AreaID					AreaID_);

	// 物理エリアを再利用する
	AreaID reuseArea(const Trans::Transaction&	Transaction_,
					 const AreaID				AreaID_);

	// 物理エリアへデータを書き込む
	void writeArea(const Trans::Transaction&	Transaction_,
				   const AreaID					AreaID_,
				   const void*					Buffer_,
				   const AreaOffset				Offset_,
				   const AreaSize				Size_);

	// 物理エリアからデータを読み込む
	void readArea(const Trans::Transaction&	Transaction_,
				  const AreaID				AreaID_,
				  void*						Buffer_,
				  const AreaOffset			Offset_,
				  const AreaSize			Size_);

	// 物理エリア情報を取り出す
	void
		fetchOutAreaInformation(
			const Trans::Transaction&	Transaction_,
			const AreaID				AreaID,
			Area::Information&			AreaInformation) const;

	// 物理エリア情報を取り出す
	void
		fetchOutAreaInformation(
			const Content&		Content_,
			const AreaID		AreaID_,
			Area::Information&	AreaInfo_) const;

	// 物理エリア情報に記録されている物理エリアオフセットを返す
	PageOffset getAreaOffset(AreaID	AreaID_) const;

	// 物理エリア情報に記録されている物理エリアサイズを返す
	AreaSize getAreaSize(AreaID	AreaID_) const;

	// 物理エリアを拡大／縮小する
	bool
		changeAreaSize(
			const Trans::Transaction&	Transaction_,
			Content&					Content_,
			const AreaID				AreaID_,
			const AreaSize				Size_,
			const bool					DoCompaction_ = false);

	// 物理エリアを拡大／縮小する
	bool
		changeAreaSize(
			const Trans::Transaction&	cTransaction_,
			AreaID						uiAreaID_,
			AreaSize					uiSize_,
			bool						bDoCompaction_ = false);

	// 物理エリアを再配置する
	void compaction(const Trans::Transaction&	Transaction_,
					Content&					Content_);

	// 物理エリアを再配置する
	void compaction(const Trans::Transaction&	Transaction_);

	// 先頭の使用中物理エリアの識別子を返す
	AreaID getTopAreaID(const Content&	Content_) const;

	// 先頭の使用中物理エリアの識別子を返す
	AreaID getTopAreaID(const Trans::Transaction&	Transaction_) const;

	// 最後の使用中物理エリアの識別子を返す
	AreaID getLastAreaID(const Content&	Content_) const;

	// 最後の使用中物理エリアの識別子を返す
	AreaID getLastAreaID(const Trans::Transaction&	Transaction_) const;

	// 次の使用中物理エリアの識別子を返す
	AreaID getNextAreaID(const Content&	Content_,
						 const AreaID	AreaID_) const;

	// 次の使用中物理エリアの識別子を返す
	AreaID getNextAreaID(const Trans::Transaction&	Transaction_,
						 const AreaID				AreaID_) const;

	// 前の使用中物理エリアの識別子を返す
	AreaID getPrevAreaID(const Content&	Content_,
						 const AreaID	AreaID_) const;

	// 前の使用中物理エリアの識別子を返す
	AreaID getPrevAreaID(const Trans::Transaction&	Transaction_,
						 const AreaID				AreaID_) const;

	// 未使用領域サイズを返す
	PageSize getUnuseAreaSize(const Content&	Content_,
							  const AreaNum		AreaNum_ = 1) const;

	// 未使用領域サイズを返す
	PageSize getUnuseAreaSize(const Trans::Transaction&	Transaction_,
							  const AreaNum				AreaNum_ = 1) const;

	// 空き領域サイズを返す
	PageSize getFreeAreaSize(const Content&	Content_,
							 const AreaNum	AreaNum_ = 1) const;

	// 空き領域サイズを返す
	PageSize getFreeAreaSize(const Trans::Transaction&	Transaction_,
							 const AreaNum				AreaNum_ = 1) const;

	// 物理ページデータサイズを返す
	PageSize getPageDataSize(const AreaNum	AreaNum_ = 1) const
		{ return m_File->getPageDataSize(AreaNum_); };

	// 物理ファイル記述子を返す
	File* getFile() const { return m_File; };

#ifdef DEBUG

	void getPageHeader(const Trans::Transaction&	Transaction_,
					   PageSize&					UnuseAreaSize_,
					   unsigned int&				UnuseAreaRate_,
					   PageSize&					FreeAreaSize_,
					   unsigned int&				FreeAreaRate_,
					   PageOffset&					FreeAreaOffset_,
					   AreaNum&						ManageAreaNum_);

	void getAreaDirectory(const Trans::Transaction&	Transaction_,
						  unsigned char*			AreaUseFlag_,
						  Area::Information*		AreaInfo_);

#endif

private:

	//
	// メンバ関数
	//

	// コンストラクタ
	AreaManagePage(
		const Trans::Transaction&					Transaction_,
		AreaManageFile*								File_,
		const PageID								PageID_,
		const Buffer::Page::FixMode::Value			FixMode_,
		const Buffer::ReplacementPriority::Value	ReplacementPriority_ =
									Buffer::ReplacementPriority::Middle);

	// コンストラクタ
	AreaManagePage(
		const Trans::Transaction&			Transaction_,
		AreaManageFile*						File_,
		const PageID						PageID_,
		const Buffer::Page::FixMode::Value	FixMode_,
		Admin::Verification::Progress&		Progress_);

	// デストラクタ
	virtual ~AreaManagePage();

	// メモリー再利用
	void reset(
		const Trans::Transaction&			Transaction_,
		PageID								PageID_,
		Buffer::Page::FixMode::Value		FixMode_,
		Buffer::ReplacementPriority::Value	ReplacementPriority_ =
									Buffer::ReplacementPriority::Middle);

	// 利用者に公開する領域のサイズを返す
#ifdef DEBUG
	PageSize getUserAreaSize() const;
#endif

	// 未使用領域サイズ／空き領域サイズを領域率に変換する
	void convertToAreaRate(const PageSize		AreaSize_,
						   unsigned int&		AreaRateValue_,
						   BitmapTable::Rate&	AreaRate_) const;

	// 物理エリアを拡大／縮小する
	bool
		changeAreaSize(
			const Trans::Transaction&	Transaction_,
			void*						PagePointer_,
			const AreaID				AreaID_,
			const AreaSize				Size_,
			const bool					DoCompaction_ = false);

	// 物理エリアを拡大する
	bool expandArea(const Trans::Transaction&	Transaction_,
					void*						PagePointer_,
					const AreaID				AreaID_,
					const AreaSize				BeforeAreaSize_,
					const AreaSize				AfterAreaSize_,
					const bool					DoCompaction_);

	// 物理エリアを縮小する
	bool reduceArea(const Trans::Transaction&	Transaction_,
					void*						PagePointer_,
					const AreaID				AreaID_,
					const AreaSize				Size_);

	// 物理エリアを再配置する
	void compaction(const Trans::Transaction&	Transaction_,
					void*						PagePointer_);

	// 先頭の使用中物理エリアの識別子を返す
	AreaID getTopAreaID(const void*	PagePointer_) const;

	// 最後の使用中物理エリアの識別子を返す
	AreaID getLastAreaID(const void*	PagePointer_) const;

	// 次の使用中物理エリアの識別子を返す
	AreaID getNextAreaID(const void*	PagePointer_,
						 const AreaID	AreaID_) const;

	// 前の使用中物理エリアの識別子を返す
	AreaID getPrevAreaID(const void*	PagePointer_,
						 const AreaID	AreaID_) const;

	// 未使用領域サイズを返す
	PageSize getUnuseAreaSize(const void*	PagePointer_,
							  const AreaNum	AreaNum_) const;

	// 空き領域サイズを返す
	PageSize getFreeAreaSize(const void*	PagePointer_,
							 const AreaNum	AreaNum_) const;

	//
	// 整合性検査のためのメソッド
	//

	// 利用者と自身の物理エリアの使用状況が一致するかどうかをチェックする
	void correspondUseArea(const Trans::Transaction&		Transaction_,
						  void*								TableTop_,
						  Common::BitSet&					AreaIDs_,
						  const AreaID						LastAreaID_,
						  Admin::Verification::Progress&	Progress_);

	// 物理エリアの使用状態を修復する
	void correctUseArea(const AreaID					AreaID_,
						void*							TableTop_,
						Admin::Verification::Progress&	Progress_);

	// 物理エリア情報検査
	void
		checkPhysicalArea(
			Admin::Verification::Progress&	Progress_) const;

	//
	//	メンバ変数
	//

	// 物理ファイル記述子
	AreaManageFile*				m_File;

	// 空き領域管理表クラスのインスタンスへのポインタ
	AreaManageTable::Header*	m_TableHeader;

	// ヘッダークラス
	// 	( Sat, 21 Apr 2001 追加)
	AreaManagePageHeader&		m_Header;

	// 空き領域管理表を修復する必要があるかどうか
	bool						m_NecessaryRecoverTable;

	// Area::Directory
	Area::Directory&			m_Directory;

}; // end of class PhysicalFile::AreaManagePage

//	FUNCTION public
//	PhysicalFile::AreaManagePage::getAreaOffset --
//		物理エリア情報に記録されている物理エリアオフセットを返す
//
//	NOTES
//	物理エリア情報に記録されている物理エリアオフセットを返す。
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

inline
PageOffset
AreaManagePage::getAreaOffset(AreaID	AreaID_) const
{
	return
		m_Directory.getAreaOffset(this->m_VersionPageTop, AreaID_) -
			m_Header.getSize();
}

} // end of namespace PhysicalFile

_SYDNEY_END

#endif //__SYDNEY_PHYSICALFILE_AREAMANAGEPAGE_H

//
//	Copyright (c) 2000, 2001, 2002, 2007, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
