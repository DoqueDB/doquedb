// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// AreaManageFile.h --
//		空き領域管理機能付き物理ファイル関連のクラス定義、関数宣言
// 
// Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_PHYSICALFILE_AREAMANAGEFILE_H
#define __SYDNEY_PHYSICALFILE_AREAMANAGEFILE_H

#include "PhysicalFile/File.h"
#include "PhysicalFile/Area.h"

_SYDNEY_BEGIN

namespace PhysicalFile
{

class AreaManagePageHeader;

//
//	CLASS
//	PhysicalFile::AreaManageTable --
//		空き領域管理表クラス
//
//	NOTES
//	空き領域管理表クラス。
//	空き領域管理表は下図のような物理構造となっている。
//
//		┌──────────────────┐　　　　┬
//		│　　　　　　　　　　　　　　　　　　│　　　　│
//		│　　　　空き領域管理表ヘッダ　　　　│　　　　│
//		│　　　　　　　　　　　　　　　　　　│　　　　│
//		├──────────────────┤　　　　│
//		│　　　　　　　　　　　　　　　　　　│　　　　│
//		│　　　　　　　　　　　　　　　　　　│　　　　│
//		│　　　　　　　　　　　　　　　　　　│　　　　│
//		│　　　　　　　　　　　　　　　　　　│
//		│　　　　　　　　　　　　　　　　　　│　空き領域管理表
//		│　　　　　　　　　　　　　　　　　　│
//		│　　　　領域率ビットマップ　　　　　│　　　　│
//		│　　　　　　　　　　　　　　　　　　│　　　　│
//		│　　　　　　　　　　　　　　　　　　│　　　　│
//		│　　　　　　　　　　　　　　　　　　│　　　　│
//		│　　　　　　　　　　　　　　　　　　│　　　　│
//		│　　　　　　　　　　　　　　　　　　│　　　　│
//		│　　　　　　　　　　　　　　　　　　│　　　　│
//		└──────────────────┘　　　　┴
//
class AreaManageTable : public Common::Object
{
public:

	//
	//	CLASS
	//	PhysicalFile::AreaManageTable::Header --
	//		空き領域管理表ヘッダクラス
	//
	//	NOTES
	//	空き領域管理表ヘッダクラス。
	//	空き領域管理表ヘッダは下図のような物理構造となっている。
	//	使用中／未使用の物理ページ数はそれぞれ4バイトで記録するタイプと
	//	2バイトで記録するタイプがある。
	//	どちらで記録するかは、物理ファイルごとに、
	//	“1つの空き領域管理表で管理可能な物理ページ数”により決まる。
	//
	//		┌──────────────────┐　　　┬
	//		│　　　　使用中の物理ページ数　　　　│　　　│
	//		├──────────────────┤　　　│
	//		│　　　　未使用の物理ページ数　　　　│　　　│
	//		├──────────────────┤　┬　│
	//		│　　　　　　　　　　　　　　　　　　│　│
	//		│　未使用領域率別の物理ページ数配列　│　│　Ｂ
	//		│　　　　　　　　　　　　　　　　　　│
	//		├──────────────────┤　Ａ　│
	//		│　　　　　　　　　　　　　　　　　　│　　　│
	//		│　空き領域率別の物理ページ数配列　　│　│　│
	//		│　　　　　　　　　　　　　　　　　　│　│　│
	//		├──────────────────┤　┴　┴
	//		│　　　　　　　　　　　　　　　　　　│
	//		＝　　　　　　　　　　　　　　　　　　＝
	//
	//		Ａ：物理ページ数配列
	//		Ｂ：空き領域管理表ヘッダ
	//
	//	また、未使用領域率別／空き領域率別の物理ページ数配列はそれぞれ
	//	下図のような物理構造となっていて、
	//	これも各要素が4バイトで記録するタイプと
	//	2バイトで記録するタイプがある。
	//
	//		　［０］　［１］　［２］　［３］　［４］　［５］　［６］　［７］　
	//		┌───┬───┬───┬───┬───┬───┬───┬───┐
	//		│　　　│　５　│１０　│１５　│２０　│４０　│６０　│８０％│
	//		│　〜　│　〜　│　〜　│　〜　│　〜　│　〜　│　〜　│　〜　│
	//		│　４％│　９％│１４％│１９％│３９％│５９％│７９％│　　　│
	//		└───┴───┴───┴───┴───┴───┴───┴───┘
	//
	class Header : public Common::Object
	{
		friend class AreaManageFile;

	public:

		//
		//	ENUM public
		//	PhysicalFile::AreaManageTable::Header::Type --
		//		空き領域管理表ヘッダタイプ
		//
		//	NOTES
		//	空き領域管理表ヘッダタイプ。
		//	空き領域管理表の各項目は、物理ファイルごとに、
		//	1つの空き領域管理表で管理可能な物理ページ数が
		//	65536ページ未満の場合には2バイトで記録し、
		//	65536ページ以上の場合には4バイトで記録する。
		//
		enum Type
		{
			SmallType = 0, // 各項目を2バイトで記録するタイプ
			LargeType,     // 各項目を4バイトで記録するタイプ
			UnknownType
		};

		// 物理ページ数配列の要素数
		static const unsigned int	PageArrayElementNum;

		// 領域率から物理ページ数配列のインデックスへの変換表
		static const char			ToPageArrayIndex[101];

		// 空き領域管理表ヘッダサイズ [byte]
		static const PageSize	SmallSize;

		// 空き領域管理表ヘッダサイズ [byte]
		static const PageSize	LargeSize;

		// 未使用領域率別の物理ページ数配列開始位置 [byte]
		static const PageOffset	SmallUnuseAreaRatePageArrayOffset;

		// 未使用領域率別の物理ページ数配列開始位置 [byte]
		static const PageOffset	LargeUnuseAreaRatePageArrayOffset;

		// 空き領域率別の物理ページ数配列開始位置 [byte]
		static const PageOffset	SmallFreeAreaRatePageArrayOffset;

		// 空き領域率別の物理ページ数配列開始位置 [byte]
		static const PageOffset	LargeFreeAreaRatePageArrayOffset;

		//
		//	STRUCT public
		//	PhysicalFile::AreaManageTable::Header::Item --
		//		空き領域管理表ヘッダに記録されている項目
		//
		//	NOTES
		//	空き領域管理表ヘッダに記録されている項目。
		//	各項目が2バイトで記録されていようが4バイトで記録されていようが、
		//	メモリ上では4バイトで持つ。
		//
		struct Item
		{
			// 使用中の物理ページ数
			PageNum	m_UsedPageNum;

			// 未使用の物理ページ数
			PageNum	m_UnusePageNum;

			// 未使用領域率別の物理ページ数配列
			PageNum	m_PageNumByUnuseAreaRate[8];

			// 空き領域率別の物理ページ数配列
			PageNum	m_PageNumByFreeAreaRate[8];
		};

		//
		// 空き領域管理表ヘッダに記録されている各項目へ
		// アクセスするためのメソッド
		//

		// 使用中の物理ページ数を返す
		typedef PageNum (*GetUsedPageNumFunc)(const void*	HeaderTop_);

		// 未使用の物理ページ数を返す
		typedef PageNum (*GetUnusePageNumFunc)(const void*	HeaderTop_);

		// 未使用領域率別／空き領域率別いずれかの
		// 物理ページ数配列の開始位置を返す
		typedef
			PageOffset
			(*GetPageArrayOffsetFunc)(const bool	ByUnuseAreaRate_);

		// 指定未使用領域率／空き領域率以上の領域をもつ物理ページを
		// 管理しているかをチェックする
		typedef
			bool
			(*ExistManagePageFunc)(const void*			HeaderTop_,
								   const unsigned int	AreaRateValue_,
								   const bool			ByUnuseAreaRate_);

		// 使用中の物理ページ数を更新する
		typedef void (*UpdateUsedPageNumFunc)(void*		HeaderTop_,
											  const int	AddNum_);

		// 未使用の物理ページ数を更新する
		typedef void (*UpdateUnusePageNumFunc)(void*		HeaderTop_,
											   const int	AddNum_);

		// 未使用領域率別／空き領域率別の物理ページ数配列の1要素を更新する
		typedef
			void
			(*UpdatePageArrayElementFunc)(
				void*				HeaderTop_,
				const bool			ByUnuseAreaRate_,
				const unsigned int	AreaRateValue_,
				const bool			Increment_);

		// 未使用領域率別／空き領域率別の物理ページ数配列の1要素を
		// 上書きする
		typedef
			void
			(*OverwritePageArrayElementFunc)(
				void*				HeaderTop_,
				const bool			ByUnuseAreaRate_,
				const unsigned int	ArrayIndex_,
				const PageNum		PageNum_);

		// 使用中の物理ページ数と未使用の物理ページ数を取り出す
		typedef void (*FetchOutPageNumFunc)(const void*	HeaderTop_,
											PageNum&	UsedPageNum_,
											PageNum&	UnusePageNum_);

		// 未使用領域率別・空き領域率別2つの物理ページ数配列を取り出す
		typedef
			void
			(*FetchOutPageArrayBothFunc)(
				const void*	HeaderTop_,
				PageNum*	PageNumByUnuseAreaRate_,
				PageNum*	PageNumByFreeAreaRate_);

		// 未使用領域率別／空き領域率別いずれかの物理ページ数配列を取り出す
		typedef void (*FetchOutPageArrayFunc)(const void*	HeaderTop_,
											  const bool	ByUnuseAreaRate_,
											  PageNum*		PageArray_);

		//
		// アクセスする関数
		//

		// 使用中の物理ページ数を返す
		GetUsedPageNumFunc				GetUsedPageNum;

		// 未使用の物理ページ数を返す
		GetUnusePageNumFunc				GetUnusePageNum;

		// 未使用領域率別／空き領域率別いずれかの
		// 物理ページ数配列の開始位置を返す
		GetPageArrayOffsetFunc			GetPageArrayOffset;

		// 指定未使用領域率／空き領域率以上の領域をもつ物理ページを
		// 管理しているかをチェックする
		ExistManagePageFunc				ExistManagePage;

		// 使用中の物理ページ数を更新する
		UpdateUsedPageNumFunc			UpdateUsedPageNum;

		// 未使用の物理ページ数を更新する
		UpdateUnusePageNumFunc			UpdateUnusePageNum;

		// 未使用領域率別／空き領域率別の物理ページ数配列の1要素を更新する
		UpdatePageArrayElementFunc		UpdatePageArrayElement;

		// 未使用領域率別／空き領域率別の物理ページ数配列の1要素を
		// 上書きする
		OverwritePageArrayElementFunc	OverwritePageArrayElement;

		// 使用中の物理ページ数と未使用の物理ページ数を取り出す
		FetchOutPageNumFunc				FetchOutPageNum;

		// 未使用領域率別・空き領域率別2つの物理ページ数配列を取り出す
		FetchOutPageArrayBothFunc		FetchOutPageArrayBoth;

		// 未使用領域率別／空き領域率別いずれかの物理ページ数配列を取り出す
		FetchOutPageArrayFunc			FetchOutPageArray;

		//
		// メンバ関数
		//

		// コンストラクタ
		Header();

		// デストラクタ
		~Header();

		// 未使用領域率別／空き領域率別の物理ページ数配列を更新する
		void updatePageArray(void*				TablePointer_,
							 const bool			ByUnuseAreaRate_,
							 const unsigned int	BeforeAreaRateValue_,
							 const unsigned int	AfterAreaRateValue_);

		// 空き領域管理表ヘッダタイプを設定する
		void setType(const Type	Type_);

		//
		// データメンバ
		//

		// 空き領域管理表ヘッダタイプ
		Type	m_Type;

	private:

		//
		// メンバ関数
		//

		// アクセスする関数を設定する
		void setFunction();

	}; // end of class PhysicalFile::AreaManageTable::Header

	//
	//	CLASS
	//	PhysicalFile::AreaManageTable::Bitmap --
	//		領域率ビットマップクラス
	//
	//	NOTES
	//	領域率ビットマップクラス。
	//	領域率ビットマップは、管理している1物理ページに対して
	//	1バイト（8ビット）が割り当てられ、
	//	各ビットは下図のような意味合いを持つ。
	//	また、領域率[％]と領域率ビットマップの値（8ビット）との変換は
	//	PhysicalFile::BitmapTable内の各表を用いる。
	//
	//		┌─┬─┬─┬─┬─┬─┬─┬─┐
	//		│　│　│　│　│　│　│　│　│
	//		│７│６│５│４│３│２│１│０│
	//		│　│　│　│　│　│　│　│　│
	//		└┼┴┼┴┼┴┼┴┼┴┼┴┼┴┼┘
	//		　│　│　│　│　│　│　│　│
	//		　│　│　│　│　│　│　│　└─ 4％以下の未使用／空き領域を持つ
	//		　│　│　│　│　│　│　└─── 5〜9％　　　　　　　〃
	//		　│　│　│　│　│　└───── 10〜14％　　　　　　〃
	//		　│　│　│　│　└─────── 15〜19％　　　　　　〃
	//		　│　│　│　└───────── 20〜39％　　　　　　〃
	//		　│　│　└─────────── 40〜59％　　　　　　〃
	//		　│　└───────────── 60〜79％　　　　　　〃
	//	　	　└─────────────── 80％以上　　　　　　〃
	//
	class Bitmap : public Common::Object
	{
		friend class AreaManageFile;

	public:

		//
		// メンバ関数
		//

		// 領域率ビットマップの開始位置を返す
		static PageOffset getOffset(const Header::Type	HeaderType_);

		// 領域率ビットマップオフセットを返す
		static PageOffset getOffset(const Header::Type	HeaderType_,
									const PageID		PageID_,
									const PageNum		PagePerManageTable_);

		// 指定未使用領域率／空き領域率以上の領域をもつ物理ページを検索する
		static bool searchPage(const void*			TablePointer_,
							   const Header::Type	HeaderType_,
							   const unsigned int	AreaRateValue_,
							   PageID&				PageIDInTable_,
							   const PageID			LastPageIDInTable_,
							   const bool			ByUnuseAreaRate_);

		// 領域率ビットマップの値を上書きする
		static void overwriteValue(void*				TablePointer_,
								   const Header::Type	HeaderType_,
								   const PageID			PageID_,
								   const PageNum		PagePerManageTable_,
								   const unsigned char	Value_);

		// 領域率ビットマップの値を返す
		static unsigned char
			getValue(const void*		TablePointer_,
					 const Header::Type	HeaderType_,
					 const PageID		PageID_,
					 const PageNum		PagePerManageTable_);

	private:

		// 領域率ビットマップ開始位置 [byte]
		static const PageOffset	StartCaseBySmallHeader;

		// 領域率ビットマップ開始位置 [byte]
		static const PageOffset	StartCaseByLargeHeader;
	};

	//
	// メンバ関数
	//

	// 物理ページを管理している空き領域管理表のバージョンページ識別子を返す
	static Version::Page::ID
		getVersionPageID(const PageID	PageID_,
						 const PageNum	PagePerManageTable_);

private:

}; // end of class PhysicalFile::AreaManageTable

//
//	CLASS
//	PhysicalFile::AreaManageFile --
//		空き領域管理機能付き物理ファイルの記述子クラス
//
//	NOTES
//	空き領域管理機能付き物理ファイルの記述子クラス。
//
class AreaManageFile : public File
{
	friend class Manager;
	friend class File;
	friend class AreaManagePage;

public:

	//
	// メンバ関数
	//

	// 物理ページ記述子を破棄し、ページ内容を元に戻す
	void recoverPage(const Trans::Transaction&	Transaction_,
					 Page*&						Page_);

	// 生成されている全物理ページ記述子を破棄し、ページ内容を元に戻す
	void recoverPageAll(const Trans::Transaction&	Transaction_);

	// 物理ページを高速検索可能な閾値を返す [byte]
	PageSize getPageSearchableThreshold() const;

	// 物理ページを検索する
	PageID searchFreePage(const Trans::Transaction&	Transaction_,
						  const PageSize			Size_,
						  const PageID				PageID_,
						  const bool				IsUnuseArea_,
						  const AreaNum				AreaNum_ = 1);

	// 物理ページを検索する
	Page* searchFreePage2(const Trans::Transaction&		Transaction_,
						  PageSize						Size_,
						  Buffer::Page::FixMode::Value	eFixMode_,
						  PageID						PageID_,
						  bool							IsUnuseArea_,
						  AreaNum						AreaNum_ = 1);

	// 物理ページデータサイズを返す
	PageSize getPageDataSize(const AreaNum	AreaNum_ = 1) const;
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
			PageNum*					PageNumByUnuseAreaRate_,
			PageNum*					PageNumByFreeAreaRate_);

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
	AreaManageFile(const File::StorageStrategy&		FileStorageStrategy_,
				   const File::BufferingStrategy&	BufferingStrategy_,
				   const Lock::FileName*			LockName_,
				   bool								batch_);

	// デストラクタ
	virtual ~AreaManageFile();

	// 物理ファイル生成時の初期化
	void initialize(const Trans::Transaction&	Trans_,
					void*						FileHeader_);

	// Allocate Page instance.
	// File's pure virtual function
	Page* allocatePageInstance(
		const Trans::Transaction&			cTransaction_,
		PageID								uiPageID_,
		Buffer::Page::FixMode::Value		eFixMode_,
		Admin::Verification::Progress*		pProgress_ = 0,
		Buffer::ReplacementPriority::Value	eReplacementPriority_
											= Buffer::ReplacementPriority::Low);
	
	// 最後の使用中の物理ページの識別子を返す
	PageID getLastPageID(const Trans::Transaction&	Transaction_,
						 PageNum					usedPageNum_,
						 PageNum					unusePageNum_);

	// 空き領域管理表から
	// 使用中の物理ページ数と未使用の物理ページ数を取り出す
	void fetchOutPageNumFromManageTable(const void*	TablePointer_,
										PageNum&	UsedPageNum_,
										PageNum&	UnusePageNum_) const;

	// 空き領域管理表に記録されている使用中の物理ページ数を更新する
	void updateUsedPageNum(void*		TablePointer_,
						   const int	AddNum_);

	// 空き領域管理表に記録されている未使用の物理ページ数を更新する
	void updateUnusePageNum(void*		TablePointer_,
							const int	AddNum_);

	// 使用中の物理ページかどうかをチェックする
	bool isUsedPage(const void*		TablePointer_,
					const PageID	PageID_) const;

	// 1つの空き領域管理表で管理可能な物理ページ数を返す
	PageNum getPagePerManageTable() const;

	// 次に割り当てる物理ページを検索する
	PageID searchNextAssignPage(const void*	TablePointer_) const;

	// 物理ページを初期化する
	void initializePage(void*	PagePointer_);

	// 空き領域管理表のバージョンページ識別子を返す
	Version::Page::ID
		getManageTableVersionPageID(const PageID	PageID_) const;

	// 空き領域管理表を更新する
	void updateManageTable(void*			TablePointer_,
						   const PageID		PageID_,
						   const PageNum	PageNum_,
						   const bool		ForReuse_,
						   const void*		PagePointer_ = 0);

	// 空き領域管理表内の領域率ビットマップを更新する
	void updateAreaBitmap(void*					TablePointer_,
						  const PageID			PageID_,
						  const unsigned char	BitmapValue_);

	// 未使用領域率別／空き領域率別の物理ページ数配列を更新する
	void updatePageArray(void*				TablePointer_,
						 const bool			ByUnuseAreaRate_,
						 const unsigned int	AreaRateValue_,
						 const bool			Increment_);

	// 空き領域管理表を修復する
	void
		recoverAreaManageTable(
			const Trans::Transaction&	Transaction_,
			const Version::Page::ID		TableVersionPageID_);

	// 空き領域管理表を修復する
	void
		recoverAreaManageTables(
			const Trans::Transaction&		Transaction_,
			ModVector<Version::Page::ID>&	TableVersionPageIDs_);

	// 未使用領域サイズ／空き領域サイズを領域率に変換する
	unsigned int convertToAreaRate(const PageSize	AreaSize_) const;

	// 物理ページデータサイズを返す
	static PageSize
		getPageDataSize(const Os::Memory::Size	VersionPageSize_,
						const AreaNum			AreaNum_);

	//
	// 整合性検査のためのメソッド
	//

	// 物理エリア識別子を記録するためのビットマップの列を初期化する
	void initializeAreaIDBitmap(const PageNum	ManagePageNum_);

	// 物理エリア識別子を記録するためのビットマップの列を解放する
	void terminateAreaIDBitmap(const PageNum	ManagePageNum_);

	// 物理エリア識別子を記録するためのビットマップを設定する
	void setAreaIDBitmap(const PageID	PageID_,
						 const AreaNum	AreaNum_,
						 const AreaID*	AreaIDs_);

	// 利用者と自身の物理エリアの使用状況が一致するかどうかをチェックする
	void correspondUseArea(const Trans::Transaction&		Transaction_,
						   Admin::Verification::Progress&	Progress_);

	// 利用者と自身の物理エリアの使用状況が一致するかどうかをチェックする
	void correspondUseArea(const Trans::Transaction&		Transaction_,
						   const PhysicalFile::PageID		PageID_,
						   void*							TableTop_,
						   Admin::Verification::Progress&	Progress_);

	// 空き領域管理機能付き物理ファイルの整合性検査を行う
	void checkPhysicalFile(const Trans::Transaction&		Transaction_,
						   Admin::Verification::Progress&	Progress_);

	// 管理物理ページ総数一致検査および使用中物理ページ総数一致検査
	void checkPageNumInFile(const Trans::Transaction&		Transaction_,
							const bool						IsManage_,
							Admin::Verification::Progress&	Progress_);

	// 空き領域管理表ごとの物理ページ数一致検査
	void checkPageNumInTable(const Trans::Transaction&		Transaction_,
							 Admin::Verification::Progress&	Progress_);

	// 物理ページ未使用領域サイズ一致検査および
	// 物理ページ空き領域サイズ一致検査
	void checkAreaSize(const Trans::Transaction&		Transaction_,
					   const bool						IsUnuse_,
					   Admin::Verification::Progress&	Progress_);

	// 物理エリア情報検査
	void checkPhysicalArea(const Trans::Transaction&		Transaction_,
						   Admin::Verification::Progress&	Progress_);

	// Area::Directoryを得る
	Area::Directory& getDirectory() { return m_Directory; }

#ifdef OBSOLETE
	// searchFreePageで見るべき管理テーブルを列挙する
	ModSize getLookManageTable(Version::Page::ID uiFirst_,
							   Version::Page::ID uiLast_,
							   Version::Page::ID* pLookPage_,
							   ModSize iSize_);
#endif // OBSOLETE

	//
	// 静的データメンバ
	//

	// 物理ページを高速検索可能な閾値となる検索基準領域率 [%]
	static const unsigned int	PageSearchableThreshold; // = 80[%]

	//
	// 非静的データメンバ
	//

	// 1つの空き領域管理表で管理可能な物理ページ数
	PageNum					m_PagePerManageTable;

	// 物理ページ内の使用率上限 [%]
	unsigned int			m_PageUseRate;

	// 空き領域管理表ヘッダ
	AreaManageTable::Header	m_TableHeader;

	//
	// 整合性検査のためのデータメンバ
	//

	// 物理エリア識別子を記録するためのビットマップの列へのポインタ
	Common::BitSet**		m_AreaIDs;

	// 物理エリア識別子を記録するためのビットマップの列の確保サイズ [byte]
	ModSize					m_AreaIDsSize;

	// 各物理ページごとの最終使用物理エリア識別子を記録するための列
	// へのポインタ
	AreaID*					m_LastAreaIDs;

	// 各物理ページごとの最終使用物理エリア識別子を記録するための列の
	// 確保サイズ [byte]
	ModSize					m_LastAreaIDsSize;

	// Area::Directory
	Area::Directory			m_Directory;

	// AreaManagePageHeader
	AreaManagePageHeader*	m_PageHeader;

	// searchFreePage内部でattachPageしたページを返したかどうか
	bool m_bAttachedForSearchFreePage;

}; // end of class PhysicalFile::AreaManageFile

//
//	FUNCTION public
//	PhysicalFile::AreaManageFile::getPageDataSize --
//		物理ページデータサイズを返す
//
//	NOTES
//	物理ページデータサイズを返す。
//
//	ARGUMENTS
//	const PhysicalFile::AreaNum	AreaNum_ = 1
//		生成物理エリア数
//
//	RETURN
//	PhysicalFile::PageSize
//		物理ページデータサイズ [byte]
//
//	EXCEPTIONS
//	なし
//	※ 版管理マネージャ以下のモジュールからの例外は
//	　 そのまま利用者に投げられる可能性がある
//
inline
PageSize
AreaManageFile::getPageDataSize(const AreaNum	AreaNum_ // = 1
								) const
{
	return this->m_UserAreaSizeMax - m_Directory.getSize(AreaNum_);
}

//
//	FUNCTION private
//	PhysicalFile::AreaManageFile::getPagePerManageTable --
//		1つの空き領域管理表で管理可能な物理ページ数を返す
//
//	NOTES
//	1つの空き領域管理表で管理可能な物理ページ数を返す。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	PhysicalFile::PageNum
//		1つの空き領域管理表で管理可能な物理ページ数
//
//	EXCEPTIONS
//	なし
//
inline
PageNum
AreaManageFile::getPagePerManageTable() const
{
	return m_PagePerManageTable;
}

} // end of namespace PhysicalFile

_SYDNEY_END

#endif //__SYDNEY_PHYSICALFILE_AREAMANAGEFILE_H

//
//	Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
