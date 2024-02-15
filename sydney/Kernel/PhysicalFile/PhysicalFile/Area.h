// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Area.h -- 物理エリア関連のクラス定義、関数宣言
// 
// Copyright (c) 2000, 2001, 2002, 2004, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_PHYSICALFILE_AREA_H
#define __SYDNEY_PHYSICALFILE_AREA_H

#include "Common/Common.h"

#include "PhysicalFile/Module.h"
#include "PhysicalFile/Types.h"

_SYDNEY_BEGIN
_SYDNEY_PHYSICALFILE_BEGIN

//
//	CLASS
//	PhysicalFile::Area -- 物理エリアクラス
//
//	NOTES
//	物理エリアクラス。
//
class Area
{
	friend class Page;
	friend class AreaManagePage;

public:

	//
	//	STRUCT public
	//	PhysicalFile::Area::Information --
	//		物理エリア情報
	//
	//	NOTES
	//	物理エリア情報。
	//
	struct Information
	{
		// 物理エリアオフセット（物理ページ内でのオフセット） [byte]
		// ※ 実際に記録されているのは、
		// 　 物理ページ先頭からのオフセットではなく、
		// 　 バージョンページ先頭からのオフセットである。
		// 　 利用者に公開する際には、物理ページ先頭からの
		// 　 オフセットに変換して公開する。
		PageOffset	m_Offset;

		// 物理エリアサイズ [byte]
		AreaSize	m_Size;
	};

	//
	//	CLASS
	//	PhysicalFile::Area::Directory --
	//		物理エリア管理ディレクトリクラス
	//
	//	NOTES
	//	物理エリア管理ディレクトリクラス。
	//	物理エリア管理ディレクトリは下図のような物理構造となっている。
	//	物理エリア管理ディレクトリ内の連続する
	//	1個の物理エリア使用状態ビットマップと
	//	8個の物理エリア情報を物理エリア管理ブロックとする。
	//
	//		┌─────────────────┐　　　┬
	//		│　　　　　　　　　　　　　　　　　│　　　│
	//		＝　　　　　　　　　　　　　　　　　＝　　　│
	//		│　　　　　　　　　　　　　　　　　│　　　│
	//		├─────────────────┤　　　│
	//		│　　　　　　　　　　　　　　　　　│　　　│
	//		│　　　　　物理エリア情報　　　　　│　　　│
	//		│　　　　　　　　　　　　　　　　　│　　　│
	//		├─────────────────┤　　　│
	//		│　　　　　　　　　　　　　　　　　│　　　│
	//		│　　　　　物理エリア情報　　　　　│　　　│
	//		│　　　　　　　　　　　　　　　　　│　　　│
	//		├─────────────────┤　　　│
	//		│　　　　　　　　　　　　　　　　　│　　　│
	//		│　　　　　物理エリア情報　　　　　│　　　│
	//		│　　　　　　　　　　　　　　　　　│　　　│
	//		├─────────────────┤　　　│
	//		│　物理エリア使用状態ビットマップ　│　　　│
	//		├─────────────────┤　┬　│
	//		│　　　　　　　　　　　　　　　　　│　│　│
	//		│　　　　　物理エリア情報　　　　　│　│　│
	//		│　　　　　　　　　　　　　　　　　│　│　│
	//		├─────────────────┤　│　│
	//		│　　　　　　　　　　　　　　　　　│　│　│
	//		│　　　　　物理エリア情報　　　　　│　│　│
	//		│　　　　　　　　　　　　　　　　　│　│
	//		├─────────────────┤　│　Ｂ
	//		│　　　　　　　　　　　　　　　　　│　│
	//		│　　　　　物理エリア情報　　　　　│　│　│
	//		│　　　　　　　　　　　　　　　　　│　│　│
	//		├─────────────────┤　│　│
	//		│　　　　　　　　　　　　　　　　　│　│　│
	//		│　　　　　物理エリア情報　　　　　│　│　│
	//		│　　　　　　　　　　　　　　　　　│　│　│
	//		├─────────────────┤　　　│
	//		│　　　　　　　　　　　　　　　　　│　Ａ　│
	//		│　　　　　物理エリア情報　　　　　│　　　│
	//		│　　　　　　　　　　　　　　　　　│　│　│
	//		├─────────────────┤　│　│
	//		│　　　　　　　　　　　　　　　　　│　│　│
	//		│　　　　　物理エリア情報　　　　　│　│　│
	//		│　　　　　　　　　　　　　　　　　│　│　│
	//		├─────────────────┤　│　│
	//		│　　　　　　　　　　　　　　　　　│　│　│
	//		│　　　　　物理エリア情報　　　　　│　│　│
	//		│　　　　　　　　　　　　　　　　　│　│　│
	//		├─────────────────┤　│　│
	//		│　　　　　　　　　　　　　　　　　│　│　│
	//		│　　　　　物理エリア情報　　　　　│　│　│
	//		│　　　　　　　　　　　　　　　　　│　│　│
	//		├─────────────────┤　│　│
	//		│　物理エリア使用状態ビットマップ　│　│　│
	//		└─────────────────┘　┴　┴　← 物理ページ
	//		                                                   終端
	//
	//		Ａ：物理エリア管理ブロック
	//		Ｂ：物理エリア管理ディレクトリ
	//
	class Directory
	{
		friend class AreaManageFile;
		friend class Area;
		friend class Content;

	public:

		//
		//	ENUM public
		//	PhysicalFile::Area::Directory::InfoType --
		//		物理エリア情報タイプ
		//
		//	NOTES
		//	物理エリア情報タイプ。
		//	物理エリア情報の各項目は、物理ファイルごとに
		//	バージョンページサイズが
		//	65536バイト未満の場合には2バイトで記録し、
		//	65536バイト以上の場合には4バイトで記録する。
		//
		enum InfoType
		{
			SmallInfoType = 0, // 各項目を2バイトで記録するタイプ
			LargeInfoType,     // 各項目を4バイトで記録するタイプ
			UnknownInfoType
		};

		//
		// 物理エリア情報に記録されている各項目へ
		// アクセスするためのメソッド
		//

		// 物理エリア情報を上書きする
		typedef void (*OverwriteInfoFunc)(void*					InfoTop_,
										  const Information&	AreaInfo_);

		// 物理エリア情報に記録されている
		// 「物理エリアオフセット」を上書きする
		typedef
			void (*OverwriteAreaOffsetFunc)(void*				InfoTop_,
											const PageOffset	AreaOffset_);

		// 物理エリア情報に記録されている「物理エリアサイズ」を上書きする
		typedef
			void (*OverwriteAreaSizeFunc)(void*				InfoTop_,
										  const AreaSize	AreaSize_);

		// 物理エリア情報を取り出す
		typedef void (*FetchOutInfoFunc)(const void*	InfoTop_,
										 Information&	AreaInfo_);

		// 物理エリア情報に記録されている「物理エリアオフセット」を返す
		typedef PageOffset (*GetAreaOffsetFunc)(const void*	InfoTop_);

		// 物理エリア情報に記録されている「物理エリアサイズ」を返す
		typedef AreaSize (*GetAreaSizeFunc)(const void*	InfoTop_);

		//
		// アクセスする関数
		//

		// 物理エリア情報を上書きする
		OverwriteInfoFunc		OverwriteInfo;

		// 物理エリア情報に記録されている
		// 「物理エリアオフセット」を上書きする
		OverwriteAreaOffsetFunc	OverwriteAreaOffset;

		// 物理エリア情報に記録されている「物理エリアサイズ」を上書きする
		OverwriteAreaSizeFunc	OverwriteAreaSize;

		// 物理エリア情報を取り出す
		FetchOutInfoFunc		FetchOutInfo;

		// 物理エリア情報に記録されている「物理エリアオフセット」を返す
		GetAreaOffsetFunc		GetAreaOffset;

		// 物理エリア情報に記録されている「物理エリアサイズ」を返す
		GetAreaSizeFunc			GetAreaSize;

		//
		// メンバ関数
		//

		// コンストラクタ
		Directory();

		// デストラクタ
		virtual ~Directory();

		// 初期化
		void initialize(PageSize VersionPageSize_,
						PageSize VersionPageDataSize_);

		// 物理エリア管理ディレクトリサイズを返す
		PageSize getSize(const AreaNum	AreaNum_) const;
		static PageSize getSize(PageSize VersionPageSize_,
								const AreaNum AreaNum_);

		// 物理エリア情報を上書きする
		void overwriteInfo(void*				PageTop_,
						   const AreaID			AreaID_,
						   const Information&	AreaInfo_);

		// 物理エリア管理ディレクトリに物理エリア情報を追加する
		void appendInformation(void*				PageTop_,
							   const Information&	AreaInfo_);

		// 物理エリア情報に記録されている「物理エリアオフセット」を上書きする
		void overwriteAreaOffset(void*				PageTop_,
								 const AreaID		AreaID_,
								 const PageOffset	AreaOffset_);

		// 物理エリア情報に記録されている「物理エリアサイズ」を上書きする
		void overwriteAreaSize(void*			PageTop_,
							   const AreaID		AreaID_,
							   const AreaSize	AreaSize_);

		// 物理エリア使用状態ビットマップのビットをON/OFFする
		void overwriteBitValue(void*		PageTop_,
							   const AreaID	AreaID_,
							   const bool	BitON_);

		// 使用中の物理エリアかどうかをチェックする
		bool isUsedArea(const void*		PageTop_,
						const AreaID	AreaID_) const;

		// 物理エリア情報を取り出す
		void fetchOutInfo(const void*	PageTop_,
						  const AreaID	AreaID_,
						  Information&	AreaInfo_) const;

		// 物理エリア情報に記録されている「物理エリアオフセット」を返す
		PageOffset getAreaOffset(const void*	PageTop_,
								 const AreaID	AreaID_) const;

		// 物理エリア情報に記録されている「物理エリアサイズ」を返す
		AreaSize getAreaSize(const void*	PageTop_,
							 const AreaID	AreaID_) const;

		// 上書き可能な物理エリア情報の物理エリア識別子を返す
		AreaID getOverwriteAreaID(const void*	PageTop_,
								  const AreaNum	ManageAreaNum_) const;

		// 上書き可能な物理エリア情報数を返す
		AreaNum getOverwriteAreaNum(const void*		PageTop_,
									const AreaNum	ManageAreaNum_) const;

	private:

		// 1つの物理エリア管理ブロックで管理する物理エリア数
		static const AreaNum			AreaPerBlock;

		//
		// メンバ関数
		//

		// 物理エリア情報にアクセスするためのメソッドを設定する
		void setFunction();

		// 物理エリア使用状態ビットマップのオフセットを返す
		PageOffset getBitmapOffset(const AreaID	AreaID_) const;

		// 物理エリア使用状態ビットマップのビット番号を返す
		unsigned int getBitNumber(const AreaID	AreaID_) const;

		// 物理エリア情報のオフセットを返す
		PageOffset getInfoOffset(const AreaID	AreaID_) const;

		// バージョンページサイズ [byte]
		PageSize			m_VersionPageSize;

		// バージョンページデータサイズ [byte]
		PageSize			m_VersionPageDataSize;

		// 物理エリア管理ブロックサイズ [byte]
		PageSize			m_BlockSize;

		// 物理エリア情報サイズ [byte]
		PageSize			m_InfoSize;

		// 物理エリア情報タイプ
		InfoType			m_InfoType;

	}; // end of class PhysicalFile::Area::Directory

}; // end of class PhysicalFile::Area

//
//	FUNCTION private
//	PhysicalFile::Area::Directory::getBitmapOffset --
//		物理エリア使用状態ビットマップのオフセットを返す
//
//	NOTES
//	物理エリア使用状態ビットマップのオフセットを返す。
//
//	ARGUMENTS
//	const PhysicalFile::AreaID	AreaID_
//		物理エリア識別子
//
//	RETURN
//	PhysicalFile::PageOffset
//		物理エリア使用状態ビットマップのオフセット [byte]
//
//	EXCEPTIONS
//	なし
//
inline
PageOffset
Area::Directory::getBitmapOffset(const AreaID	AreaID_) const
{
	return
		static_cast<PageOffset>(
			this->m_VersionPageDataSize -
			1 -
			AreaID_ /
			PhysicalFile::Area::Directory::AreaPerBlock *
			this->m_BlockSize);
}

//
//	FUNCTION private
//	PhysicalFile::Area::Directory::getBitNumber --
//		物理エリア使用状態ビットマップのビット番号を返す
//
//	NOTES
//	物理エリア使用状態ビットマップのビット番号を返す。
//
//	ARGUMENTS
//	const PhysicalFile::AreaID	AreaID_
//		物理エリア識別子
//
//	RETURN
//	unsigned int
//		物理エリア使用状態ビットマップのビット番号
//
//	EXCEPTIONS
//	なし
//
inline
unsigned int
Area::Directory::getBitNumber(const AreaID	AreaID_) const
{
	return AreaID_ % PhysicalFile::Area::Directory::AreaPerBlock;
}

//
//	FUNCTION private
//	PhysicalFile::Area::Directory::getInformationOffset --
//		物理エリア管理情報のオフセットを返す
//
//	NOTES
//	引数AreaID_で示される物理エリアに関する物理エリア情報が
//	記録されている領域の、物理ページ先頭からのバイト数を返す。
//
//	ARGUMENTS
//	const PhysicalFile::AreaID	AreaID_
//		物理エリア識別子
//
//	RETURN
//	PhysicalFile::PageOffset
//		物理エリア管理情報のオフセット [byte]
//
//	EXCEPTIONS
//	なし
//
inline
PageOffset
Area::Directory::getInfoOffset(const AreaID	AreaID_) const
{
	return
		static_cast<PageOffset>(
			this->getBitmapOffset(AreaID_) -
			(this->getBitNumber(AreaID_) + 1) *
			this->m_InfoSize);
}

_SYDNEY_PHYSICALFILE_END
_SYDNEY_END

#endif //__SYDNEY_PHYSICALFILE_AREA_H

//
//	Copyright (c) 2000, 2001, 2002, 2004, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
