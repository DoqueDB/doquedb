// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// NonManageFile.h --
//		管理機能なし物理ファイル関連のクラス定義、関数宣言
// 
// Copyright (c) 2002, 2003, 2006, 2007, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_PHYSICALFILE_NONMANAGEFILE_H
#define __SYDNEY_PHYSICALFILE_NONMANAGEFILE_H

#include "PhysicalFile/File.h"

_SYDNEY_BEGIN

namespace PhysicalFile
{

//
//	CLASS
//	PhysicalFile::NonManageFile --
//		管理機能なし物理ファイルの記述子クラス
//
//	NOTES
//
class NonManageFile : public File
{
	friend class Manager;
	friend class File;

public:

	//
	// メンバ関数
	//

	// 物理ページ記述子を破棄し、ページ内容を元に戻す
	void recoverPage(Page*&	Page_);

	// 使用中の総バージョンページサイズを得る
	FileSize
	getUsedSize(const Trans::Transaction& trans);
#ifdef OBSOLETE
	// 確保中の総バージョンページサイズを得る
	FileSize
	getTotalSize(const Trans::Transaction& trans);
#endif

	// 物理ページデータサイズを返す
	PageSize getPageDataSize(const AreaNum	DummyAreaNum_ = 1) const;
#ifdef OBSOLETE
	// 利用者が確保済のおおよその領域サイズを返す
	FileSize getAllocatedSize(const Trans::Transaction&	Transaction_);
#endif // OBSOLETE
	// 物理ページ記述子を生成する
	Page* attachPage(const Trans::Transaction&					Transaction_,
					 const Buffer::Page::FixMode::Value			FixMode_,
					 const Buffer::ReplacementPriority::Value	ReplacementPriority_ =
																	Buffer::ReplacementPriority::Low);

	// 先頭の使用中の物理ページの識別子を返す
	PageID getTopPageID(const Trans::Transaction&	Transaction_);

	// 最後の使用中の物理ページの識別子を返す
	PageID getLastPageID(const Trans::Transaction&	Transaction_);

	//
	// 整合性検査のためのメソッド
	//

	// 物理ページ記述子を生成する（バージョンページの整合性検査付き）
	Page* verifyPage(const Trans::Transaction&			Transaction_,
					 const Buffer::Page::FixMode::Value	FixMode_,
					 Admin::Verification::Progress&		Progress_);

private:

	//
	// メンバ関数
	//

	// コンストラクタ
	NonManageFile(const File::StorageStrategy&		FileStorageStrategy_,
				  const File::BufferingStrategy&	BufferingStrategy_,
				  const Lock::FileName*				LockName_,
				  bool								batch_);

	// デストラクタ
	virtual ~NonManageFile();

	// Allocate Page instance.
	// File's pure virtual function
	Page* allocatePageInstance(
		const Trans::Transaction&			cTransaction_,
		PageID								uiPageID_,
		Buffer::Page::FixMode::Value		eFixMode_,
		Admin::Verification::Progress*		pProgress_ = 0,
		Buffer::ReplacementPriority::Value	eReplacementPriority_
											= Buffer::ReplacementPriority::Low);
	
	// 物理ページデータサイズを返す
	static PageSize
		getPageDataSize(const Os::Memory::Size	VersionPageSize_,
						const AreaNum			DummyAreaNum_);

	// 物理ページ識別子からバージョンページ識別子へ変換する
	Version::Page::ID convertToVersionPageID(const PageID	PageID_) const;

	//
	// 整合性検査のためのメソッド
	//

	// 管理機能なし物理ファイルの整合性検査を行う
	void checkPhysicalFile(const Trans::Transaction&		Transaction_,
						   Admin::Verification::Progress&	Progress_);

}; // end of class PhysicalFile::NonManageFile

//	FUNCTION public
//	PhysicalFile::NonManageFile::getUsedSize --
//		物理ファイルが使用中の総バージョンページサイズを得る
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction&		trans
//			サイズを求めるトランザクションのトランザクション記述子
//
//	RETURN
//		求めたサイズ(B 単位)
//
//	EXCEPTIONS

inline
FileSize
NonManageFile::getUsedSize(const Trans::Transaction& trans)
{
	return m_VersionPageDataSize;
}

#ifdef OBSOLETE
//	FUNCTION public
//	PhysicalFile::File::getTotalSize --
//		物理ファイルが確保中の総バージョンページサイズを得る
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction&		trans
//			サイズを求めるトランザクションのトランザクション記述子
//
//	RETURN
//		求めたサイズ(B 単位)
//
//	EXCEPTIONS

inline
FileSize
NonManageFile::getTotalSize(const Trans::Transaction& trans)
{
	return getUsedSize(trans);
}
#endif

//
//	FUNCTION public
//	PhysicalFile::NonManageFile::getPageDataSize --
//		物理ページデータサイズを返す
//
//	NOTES
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
NonManageFile::getPageDataSize(const AreaNum	DummyAreaNum_ // = 1
							   ) const
{
	return this->m_UserAreaSizeMax;
}

#ifdef OBSOLETE
//
//	FUNCTION public
//	PhysicalFile::NonManageFile::getAllocatedSize --
//		利用者が確保済のおおよその領域サイズを返す
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction&	Transaction_
//		トランザクション記述子への参照
//
//	RETURN
//	PhysicalFile::FileSize
//		利用者が確保済のおおよその領域サイズ [byte]
//
//	EXCEPTIONS
//	なし
//
inline
FileSize
NonManageFile::getAllocatedSize(const Trans::Transaction&	Transaction_)
{
	return this->m_UserAreaSizeMax;
}
#endif // OBSOLETE

} // end of namespace PhysicalFile

_SYDNEY_END

#endif //__SYDNEY_PHYSICALFILE_NONMANAGEFILE_H

//
//	Copyright (c) 2002, 2003, 2006, 2007, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
