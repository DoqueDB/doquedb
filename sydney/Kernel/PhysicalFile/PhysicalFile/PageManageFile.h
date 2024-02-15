// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// PageManageFile.h --
//		物理ページ管理機能付き物理ファイル関連のクラス定義、関数宣言
// 
// Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2013, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_PHYSICALFILE_PAGEMANAGEFILE_H
#define __SYDNEY_PHYSICALFILE_PAGEMANAGEFILE_H

#include "PhysicalFile/File.h"

_SYDNEY_BEGIN

namespace PhysicalFile
{

//
//	CLASS
//	PhysicalFile::PageTable --
//		物理ページ表クラス
//
//	NOTES
//	物理ページ表クラス。
//	物理ページ表は下図のような物理構造となっている。
//		┌──────────────────┐　　　┬
//		│　　　　　　　　　　　　　　　　　　│　　　│
//		│　　　　物理ページ表ヘッダ　　　　　│　　　│
//		│　　　　　　　　　　　　　　　　　　│　　　│
//		├──────────────────┤　　　│
//		│　　　　　　　　　　　　　　　　　　│　　　│
//		│　　　　　　　　　　　　　　　　　　│　　　│
//		│　　　　　　　　　　　　　　　　　　│　　　│
//		│　　　　　　　　　　　　　　　　　　│
//		│　　　　　　　　　　　　　　　　　　│　物理ページ表		  
//		│　　　　　　　　　　　　　　　　　　│
//		│　物理ページ使用状態ビットマップ　　│　　
//		│　　　　　　　　　　　　　　　　　　│　　　│
//		│　　　　　　　　　　　　　　　　　　│　　　│
//		│　　　　　　　　　　　　　　　　　　│　　　│
//		│　　　　　　　　　　　　　　　　　　│　　　│
//		│　　　　　　　　　　　　　　　　　　│　　　│
//		│　　　　　　　　　　　　　　　　　　│　　　│
//		└──────────────────┘　　　┴
//
class PageTable : public Common::Object
{
public:

	//
	//	CLASS
	//	PhysicalFile::PageTable::Header --
	//		物理ページ表ヘッダクラス
	//
	//	NOTES
	//	物理ページ表ヘッダクラス
	//	物理ページ表ヘッダは下図のような物理構造となっている。
	//	使用中／未使用の物理ページ数はそれぞれ4バイトで記録するタイプと
	//	2バイトで記録するタイプがある。
	//	どちらで記録するかは、物理ファイルごとに、
	//	“1つの物理ページ表で管理可能な物理ページ数”により決まる。
	//
	//		┌──────────────────┐　　　　┬
	//		│　　　　使用中の物理ページ数　　　　│
	//		├──────────────────┤　物理ページ表ヘッダ
	//		│　　　　未使用の物理ページ数　　　　│
	//		├──────────────────┤　　　　┴
	//		│　　　　　　　　　　　　　　　　　　│
	//		＝　　　　　　　　　　　　　　　　　　＝
	//
	class Header : public Common::Object
	{
		friend class PageManageFile;

	public:

		//
		//	ENUM public
		//	PhysicalFile::PageTable::Header::Type --
		//		物理ページ表ヘッダタイプ
		//
		//	NOTES
		//	物理ページ表ヘッダタイプ。
		//	物理ページ表の各項目は、物理ファイルごとに、
		//	1つの物理ページ表で管理可能な物理ページ数が
		//	65536ページ未満の場合には2バイトで記録し、
		//	65536ページ以上の場合には4バイトで記録する。
		//
		enum Type
		{
			SmallType = 0, // 各項目を2バイトで記録するタイプ
			LargeType,     // 各項目を4バイトで記録するタイプ
			UnknownType
		};

		// 物理ページ表ヘッダサイズ [byte]
		static const PageSize	SmallSize;

		// 物理ページ表ヘッダサイズ [byte]
		static const PageSize	LargeSize;

		//
		//	STRUCT public
		//	PhysicalFile::PageTable::Header::Item --
		//		物理ページ表ヘッダに記録されている項目
		//
		//	NOTES
		//	物理ページ表ヘッダに記録されている項目。
		//
		struct Item
		{
			// 使用中の物理ページ数
			PageNum	m_UsedPageNum;

			// 未使用の物理ページ数
			PageNum	m_UnusePageNum;
		};

		//
		// 物理ページ表ヘッダに記録されている各項目へ
		// アクセスするためのメソッド
		//

		// 使用中の物理ページ数を更新する
		typedef void (*UpdateUsedPageNumFunc)(void*		HeaderTop_,
											  const int	AddNum_);

		// 未使用の物理ページ数を更新する
		typedef void (*UpdateUnusePageNumFunc)(void*		HeaderTop_,
											   const int	AddNum_);

		// 物理ページ表ヘッダに記録されているすべての項目を取り出す
		typedef void (*FetchOutFunc)(const void*	HeaderTop_,
									 Item&			Item_);

		// 使用中の物理ページ数を返す
		typedef PageNum (*GetUsedPageNumFunc)(const void*	HeaderTop_);

		// 未使用の物理ページ数を返す
		typedef PageNum (*GetUnusePageNumFunc)(const void*	HeaderTop_);

		//
		// アクセスする関数
		//

		// 使用中の物理ページ数を更新する
		UpdateUsedPageNumFunc	UpdateUsedPageNum;

		// 未使用の物理ページ数を更新する
		UpdateUnusePageNumFunc	UpdateUnusePageNum;

		// 物理ページ表ヘッダに記録されているすべての項目を取り出す
		FetchOutFunc			FetchOut;

		// 使用中の物理ページ数を返す
		GetUsedPageNumFunc		GetUsedPageNum;

		// 未使用の物理ページ数を返す
		GetUnusePageNumFunc		GetUnusePageNum;

		//
		// メンバ関数
		//

		// コンストラクタ
		Header();

		// デストラクタ
		~Header();

		// 物理ページ表ヘッダタイプを設定する
		void setType(const Type	Type_);

		//
		// データメンバ
		//

		// 物理ページ表ヘッダタイプ
		Type	m_Type;

	private:

		//
		// メンバ関数
		//

		// アクセスする関数を設定する
		void setFunction();

	}; // end of class PhysicalFile::PageTable::Header

	//
	//	CLASS
	//	PhysicalFile::PageTable::Bitmap --
	//		物理ページ使用状態ビットマップクラス
	//
	//	NOTES
	//	物理ページ表内の物理ページ使用状態ビットマップクラス。
	//	物理ページ表で管理している1つの物理ページに対して
	//	1ビットが割り当てられ、使用中の物理ページではビットON、
	//	未使用の物理ページではビットOFFとなる。
	//
	class Bitmap : public Common::Object
	{
		friend class PageManageFile;

	public:

		//
		// メンバ関数
		//

		// 物理ページ使用状態ビットマップの開始位置を返す
		static PageOffset getOffset(const Header::Type	HeaderType_);

		// 物理ページ使用状態ビットマップのビット位置を設定する
		static void getBitPosition(const Header::Type	HeaderType_,
								   const PageID			PageID_,
								   const PageNum		PagePerManageTable_,
								   Version::Page::ID&	TableVersionPageID_,
								   PageOffset&			Offset_,
								   unsigned int&		BitNumber_);

		// 物理ページ使用状態ビットマップのビットを更新する
		static void overwriteValue(void*				TablePointer_,
								   const Header::Type	HeaderType_,
								   const PageID			PageID_,
								   const PageNum		PagePerManageTable_,
								   const bool			BitON_);

		// 使用中の物理ページかどうかをチェックする
		static bool isUsedPage(const void*			TablePointer_,
							   const Header::Type	HeaderType_,
							   const PageID			PageID_,
							   const PageNum		PagePerManageTable_);

	private:

		// 物理ページ使用状態ビットマップ開始位置 [byte]
		static const PageOffset	StartCaseBySmallHeader;

		// 物理ページ使用状態ビットマップ開始位置 [byte]
		static const PageOffset	StartCaseByLargeHeader;
	};

	//
	// メンバ関数
	//

	// 物理ページを管理している物理ページ表のバージョンページ識別子を返す
	static Version::Page::ID
		getVersionPageID(const PageID	PageID_,
						 const PageNum	PagePerManageTable_);

private:

}; // end of class PhysicalFile::PageTable

//
//	CLASS
//	PhysicalFile::PageManageFile --
//		物理ページ管理機能付き物理ファイルの記述子クラス
//
//	NOTES
//	物理ページ管理機能付き物理ファイルの記述子クラス。
//
class PageManageFile : public File
{
	friend class Manager;
	friend class File;

public:

	//
	// メンバ関数
	//

	// 物理ページ記述子を破棄し、ページ内容を元に戻す
	void recoverPage(Page*&	Page_);

	// 生成されている全物理ページ記述子を破棄し、ページ内容を元に戻す
	void recoverPageAll();

	// 物理ページ記述子を生成する(detachPageAllを呼ぶこと)
	// FixMode が ReadOnly のときスレッドセーフ
	Page*
		attachPage(
			const Trans::Transaction&			Transaction_,
			const PageID						PageID_,
			const Buffer::Page::FixMode::Value	FixMode_,
			const Buffer::ReplacementPriority::Value
												ReplacementPriority_ =
										Buffer::ReplacementPriority::Low);
	
	// 物理ページ記述子を破棄する
	// FixMode が ReadOnly のときスレッドセーフ
	void
		detachPage(
			Page*&					Page_,
			Page::UnfixMode::Value	UnfixMode_ = Page::UnfixMode::Omit,
			const bool				SavePage_ = false);

	// 物理ページデータサイズを返す
	PageSize getPageDataSize(const AreaNum	DummyAreaNum_ = 1) const;
#ifdef OBSOLETE
	// 利用者が確保済のおおよその領域サイズを返す
	FileSize getAllocatedSize(const Trans::Transaction&	Transaction_);
#endif // OBSOLETE
	// 先頭の使用中の物理ページの識別子を返す(detachPageAllを呼ぶこと)
	PageID getTopPageID(const Trans::Transaction&	Transaction_);

	// 最後の使用中の物理ページの識別子を返す(detachPageAllを呼ぶこと)
	PageID getLastPageID(const Trans::Transaction&	Transaction_);

	// 次の使用中の物理ページの識別子を返す(detachPageAllを呼ぶこと)
	PageID getNextPageID(const Trans::Transaction&	Transaction_,
						 const PageID				PageID_);

	// 前の使用中の物理ページの識別子を返す(detachPageAllを呼ぶこと)
	PageID getPrevPageID(const Trans::Transaction&	Transaction_,
						 const PageID				PageID_);

#ifdef DEBUG

	Version::Page::ID getTableID(const PageID	PageID_);

	void
		getTableHeader(
			const Trans::Transaction&	Transaction_,
			const Version::Page::ID		TableVersionPageID_,
			PageNum&					UsedPageNum_,
			PageNum&					UnusePageNum_,
			PageNum*					DummyPageNumByUnuseAreaRate_,
			PageNum*					DummyPageNumByFreeAreaRate_);

	void
		getTableBitmap(
			const Trans::Transaction&	Transaction_,
			const Version::Page::ID		TableVersionPageID_,
			unsigned char*				BitmapBuffer_);

#endif // DEBUG

private:

	//
	// メンバ関数
	//

	// コンストラクタ
	PageManageFile(const File::StorageStrategy&		FileStorageStrategy_,
				   const File::BufferingStrategy&	BufferingStrategy_,
				   const Lock::FileName*			LockName_,
				   bool								batch_);

	// デストラクタ
	virtual ~PageManageFile();

	// 物理ページ記述子を生成する
	// FixMode が ReadOnly のときスレッドセーフ
	virtual Page*
		attachPage(
			const Trans::Transaction&					Transaction_,
			File*										File_,
			const PageID								PageID_,
			const Buffer::Page::FixMode::Value			FixMode_,
			const Buffer::ReplacementPriority::Value	ReplacementPriority_);
	
	// Allocate Page instance.
	// File's pure virtual function
	Page* allocatePageInstance(
		const Trans::Transaction&			cTransaction_,
		PageID								uiPageID_,
		Buffer::Page::FixMode::Value		eFixMode_,
		Admin::Verification::Progress*		pProgress_ = 0,
		Buffer::ReplacementPriority::Value	eReplacementPriority_
											= Buffer::ReplacementPriority::Low);
	
	// 物理ファイル生成時の初期化
	void initialize(const Trans::Transaction&	Trans_,
					void*						FileHeader_);

	// 最後の使用中の物理ページの識別子を返す
	PageID getLastPageID(const Trans::Transaction&	Transaction_,
						 PageNum					usedPageNum_,
						 PageNum					unusePageNum_);

	// 物理ページ表から使用中の物理ページ数・未使用の物理ページ数を取り出す
	void fetchOutPageNumFromManageTable(const void*	TablePointer_,
										PageNum&	UsedPageNum_,
										PageNum&	UnusePageNum_) const;

	// 物理ページ表に記録されている使用中の物理ページ数を更新する
	void updateUsedPageNum(void*		TablePointer_,
						   const int	AddNum_);

	// 物理ページ表に記録されている未使用の物理ページ数を更新する
	void updateUnusePageNum(void*		TablePointer_,
							const int	AddNum_);

	// 使用中の物理ページかどうかをチェックする
	bool isUsedPage(const void*		TablePointer_,
					const PageID	PageID_) const;

	// 1つの物理ページ表で管理可能な物理ページ数を返す
	PageNum getPagePerManageTable() const;

	// 次に割り当てる物理ページを検索する
	PageID searchNextAssignPage(const void*	TablePointer_) const;

	// 物理ページ表のバージョンページ識別子を返す
	Version::Page::ID getManageTableVersionPageID(
		const PageID	PageID_) const;

	// 物理ページ表を更新する
	void updateManageTable(void*			TablePointer_,
						   const PageID		PageID_,
						   const PageNum	PageNum_,
						   const bool		ForReuse_,
						   const void*		DummyPagePointer_ = 0);

	// 物理ページ使用状態ビットマップを更新する
	void updatePageBitmap(void*			TablePointer_,
						  const PageID	PageID_,
						  const bool	BitON_);

	// 物理ページデータサイズを返す
	static PageSize
		getPageDataSize(const Os::Memory::Size	VersionPageSize_,
						const AreaNum			DummyAreaNum_);
	//
	// 整合性検査のためのメソッド
	//

	// 物理ページ管理機能付き物理ファイルの整合性検査を行う
	void checkPhysicalFile(const Trans::Transaction&		Transaction_,
						   Admin::Verification::Progress&	Progress_);

	// 管理物理ページ総数一致検査および使用中物理ページ数一致検査
	void checkPageNumInFile(const Trans::Transaction&		Transaction_,
							const bool						IsManage_,
							Admin::Verification::Progress&	Progress_);

	// 物理ページ表ごとの使用中物理ページ数一致検査および
	// 物理ページ表ごとの未使用物理ページ数一致検査
	void checkPageNumInTable(const Trans::Transaction&		Transaction_,
							 const bool						IsUsed_,
							 Admin::Verification::Progress&	Progress_);

	//
	// データメンバ
	//

	// 1つの物理ページ表で管理可能な物理ページ数
	PageNum				m_PagePerManageTable;

	// 物理ページ表ヘッダ
	PageTable::Header	m_TableHeader;

}; // end of class PhysicalFile::PageManageFile

//
//	FUNCTION public
//	PhysicalFile::PageManageFile::getPageDataSize --
//		物理ページデータサイズを返す
//
//	NOTES
//	物理ページデータサイズを返す。
//
//	ARGUMENTS
//	const PhysicalFile::AreaNum	DummyAreaNum_ = 1
//		ダミーの物理エリア数
//		※ 空き領域管理機能付き物理ファイルと
//		　 インタフェースを共通化したために存在する。
//
//	RETURN
//	PhysicalFile::PageSize
//		物理ページデータサイズ [byte]
//
//	EXCEPTIONS
//	なし
//
inline
PageSize
PageManageFile::getPageDataSize(const AreaNum	DummyAreaNum_ // = 1
								) const
{
	return this->m_UserAreaSizeMax;
}

//
//	FUNCTION private
//	PhysicalFile::PageManageFile::getPagePerManageTable --
//		1つの物理ページ表で管理可能な物理ページ数を返す
//
//	NOTES
//	1つの物理ページ表で管理可能な物理ページ数を返す。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	PhysicalFile::PageNum
//		1つの物理ページ表で管理可能な物理ページ数
//
//	EXCEPTIONS
//	なし
//
inline
PageNum
PageManageFile::getPagePerManageTable() const
{
	return m_PagePerManageTable;
}

} // end of namespace PhysicalFile

_SYDNEY_END

#endif //__SYDNEY_PHYSICALFILE_PAGEMANAGEFILE_H

//
//	Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
