// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:
//
// ModInvertedOverflowFileHeader.h -- オーバーフローファイルの定義
// 
// Copyright (c) 1999, 2002, 2023 Ricoh Company, Ltd.
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

#ifndef	__ModInvertedOverflowFileHeader_H__
#define __ModInvertedOverflowFileHeader_H__

#include "ModError.h"
#include "ModOs.h"
#include "ModMessage.h"
#include "ModOstrStream.h"
#include "ModSerial.h"

#include "ModInvertedManager.h"
#include "ModInvertedTypes.h"

class ModInvertedOverflowPage;
class ModInvertedFileCatalogData;

//
// TYPEDEF
// ModInvertedOverflowPageID -- 転置ファイル用オーバーフローファイルのページID
//
// NOTES
// OverflowFileのページIDは、物理ファイル番号とそのファイルにおける物
// 理ページIDの2つの要素から成っている。ファイル番号が上位のビットに位
// 置し、物理ページIDが下位のビットに割当てられている
//
// +----------------------+------------------------------------+
// |       ファイル番号   |      ページID                      |
// +----------------------+------------------------------------+
//
typedef unsigned long ModInvertedOverflowPageID;


//
// CLASS
// ModInvertedOverflowFileHeader -- 転置ファイル用オーバーフローファイルのヘッダー
//
// NOTES
// オーバーフローファイル全体の管理情報を保持する。ただし、この構造体はリーフ
// ファイルのヘッダーページに置かれる。
//
class ModInvertedOverflowFileHeader : public ModSerializer
{
    friend class ModInvertedFile;
    friend class ModInvertedOverflowFile;
    friend class ModInvertedFileCatalogData;
public:
	typedef ModInvertedOverflowPage Page;
	typedef ModInvertedOverflowPageID PageID;
	typedef ModInvertedFileCatalogData CatalogData;

	//
	// ENUM
	// ModInvertedOverflowFileNum -- OverflowFileのファイル数に関連する列挙型
	//
	// NOTES
	// この列挙型のメンバであるdefaultMaxFileNumはOverflowファイルの最
	// 大ファイル数を示す。またこの値はOverflowFileの物理ファイルIDを
	// 格納しておく配列メンバ変数 fileID の要素数を決定するのに使われ
	// ている。そのため const 変数ではなく、enum とした。
	//		ModPhysicalFileID fileID[defaultMaxFileNum];
	//
	enum ModInvertedOverflowFileNum {
		defaultMaxFileNum = 32
	};

	// コンストラクタ
	ModInvertedOverflowFileHeader();

	// シリアライズ
    void serialize(ModArchive& archiver_);

	ModInvertedOverflowFileHeader& operator=(
		const ModInvertedOverflowFileHeader&);

	// ページID変換用関数
	PageID getOverflowPageID(const int, const ModPhysicalPageID) const;
	ModPhysicalPageID getPhysicalPageID(const PageID) const;
	ModSize getFileIndex(const PageID) const;

	// 物理ページの大きさ（キロバイト単位）
	void setPageSize(const ModSize);
	ModSize getPageSize() const;
	// 物理ファイルの上限（キロバイト単位）
	void setMaxFileSize(const ModFileSize);
	ModFileSize getMaxFileSize() const;
	// 物理ファイル数の上限
	ModSize calcMaxFileNum( /* const ModSize */ )/* const*/;
	static ModSize calcMaxFileNum(const ModSize);
	void setMaxFileNum(const ModSize);
	ModSize getMaxFileNum() const;
	// 使用済物理ファイル数
	void setUsedFileNum(const ModSize);
	ModSize getUsedFileNum() const;
	// 個別の物理ファイルのページ数の上限
	void setMaxPageID(const ModPhysicalPageID);
	ModPhysicalPageID getMaxPageID() const;
	// 識別子
	void setIdentifier(const char);
	char getIdentifier() const;

	// n 番目のファイルID
	void setFileId(const ModSize, const ModPhysicalFileID);
	ModPhysicalFileID getFileId(const ModSize) const;

	void setCatalogData(const CatalogData& data);

	// 出力用関数
	friend ModMessageStream& operator<<(ModMessageStream&,
									   const ModInvertedOverflowFileHeader&);
	friend ModOstream& operator<<(ModOstream&,
								  const ModInvertedOverflowFileHeader&);

protected:
	static const ModSize headerNum;

	void setPageBits();

private:
	ModSize pageSize;					// 物理ページの大きさ（キロバイト単位）
	ModFileSize maxFileSize;			// 物理ファイルの上限（キロバイト単位）
	ModPhysicalPageID maxPageID;		// 個別の物理ファイルのページ数の上限
	ModSize maxFileNum;					// 物理ファイル数の上限
	ModSize usedFileNum;				// 使用済物理ファイル数
	ModPhysicalFileID fileID[defaultMaxFileNum];
										// n 番目のファイルID
	char identifier;					// OverflowFileの識別子（文字）

	ModSize pageBits;					// オーバーフローページIDにおける
										// ページ識別に用いるビット数
										// ※ 出力関数/シリアライズでは出力しない
};


class ModInvertedFile;

//
// FUNCTION
// ModInvertedOverflowFileHeader::serialize -- シリアライズ
//
// NOTES
// オーバーフローファイルヘッダーをシリアライズする。
//
// ARGUMENTS
// ModArchive& archiver_
//		アーカイバ
//
// RETURN
// なし
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
inline void
ModInvertedOverflowFileHeader::serialize(ModArchive& archiver_)
{
	try {
		archiver_(pageSize);
		archiver_(maxFileSize);
		archiver_(maxFileNum);
		archiver_(maxPageID);
		archiver_(usedFileNum);
		archiver_(fileID, maxFileNum);
		archiver_(identifier);

	} catch (ModException& exception) {
		ModRethrow(exception);

	} catch(...) {
/* purecov:begin deadcode */
		ModUnexpectedThrow(ModModuleInvertedFile);
/* purecov:end */
	}
}

inline ModInvertedOverflowFileHeader& 
ModInvertedOverflowFileHeader::operator=(const ModInvertedOverflowFileHeader& o)
{
	pageSize = o.pageSize;
	maxFileSize = o.maxFileSize;
	maxFileNum = o.maxFileNum;
	maxPageID = o.maxPageID;
	usedFileNum = o.usedFileNum;

	for (ModSize i(0); i < maxFileNum; ++i) {
		fileID[i] = o.fileID[i];
	}

	identifier = o.identifier;

	return *this;
}


//
// FUNCTION protected
// ModInvertedOverflowFileHeader::setPageBits -- ページIDにおけるページ用のビット数の計算
//
// NOTES
// ページ用ビット数とは次のように定義される
//
// +-----------------------------------------------------------+
// |	ModInvertedOverflowPageID (unsigned long)			   |
// +----------------------+------------------------------------+
// |<- 物理ファイル番号 ->|<-    最大ページIDを表わすの      ->|
// |                      |      に必要なbit数                 |
// |                      |      (ページ用ビット数:pageBits)   |
//
// この様に定義される“ページ用ビット数”を計算し、メンバ変数 pageBits 
// にセットする。
//
// ARGUMENTS
// なし
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
inline void
ModInvertedOverflowFileHeader::setPageBits()
{
	; ModAssert(getMaxPageID() > 0);
	this->pageBits = 1;
	ModSize tmp(getMaxPageID() - 1);
	while (tmp > 1) {
		++this->pageBits;
		tmp >>= 1;
	}
}

//
// FUNCTION
// ModInvertedOverflowFileHeader::getOverflowPageId -- オーバーフローページIDの計算
//
// NOTES public
// オーバーフローファイルを構成する物理ファイルの番号(fileIndex)とそのファイル
// における物理ページID(pageId)から、オーバーフローファイルとしてのページIDを
// 計算する。
// （ビット構成を固定としてしまえば、static 関数にしてもよい）
//
// ARGUMENTS
//	const int fileIndex
//		ファイル番号
//	const ModPhysicalPageID pageId
//		物理ページID
//
// RETURN
//	ModInvertedOverflowPageID
//
// EXCEPTIONS
// なし
//
inline ModInvertedOverflowPageID
ModInvertedOverflowFileHeader::getOverflowPageID(
	const int fileIndex, const ModPhysicalPageID pageId) const
{
	; ModAssert((int)pageId < (1<<pageBits));
	return (fileIndex<<pageBits) + (pageId & ( (1<<pageBits)-1 ) );
}

//
// FUNCTION public
// ModInvertedOverflowFileHeader::getPhysicalPageID -- 物理ページIDの取得
//
// NOTES
// オーバーフローファイルとしてのページIDから、対応する物理ファイルにおける
// 物理ページID(pageId)を計算して返す。
// (ModInvertedOverflowPageID の 下位ビット部分)
// （ビット構成を固定としてしまえば、static 関数にしてもよい）
//
// ARGUMENTS
//	const ModInvertedOverflowPageID pageId
//		OverflowFileのページID
//
// RETURN
//	ModPhysicalPageID
//
// EXCEPTIONS
// なし
//
inline ModPhysicalPageID
ModInvertedOverflowFileHeader::getPhysicalPageID(
	const ModInvertedOverflowPageID pageId) const
{
	return pageId & ( (1<<pageBits) - 1 );
}

//
// FUNCTION public
// ModInvertedOverflowFileHeader::getFileIndex -- ファイル番号の取得
//
// NOTES
// オーバーフローファイルとしてのページIDから、対応する物理ファイル番号
// (fileIndex)を計算する。
// (ModInvertedOverflowPageID の 上位ビット部分)
// （ビット構成を固定としてしまえば、static 関数にしてもよい）
//
// ARGUMENTS
//	const ModInvertedOverflowPageID pageId
//		OverflowFileのページID
//
// RETURN
//	ModSize ファイル番号
//
// EXCEPTIONS
// なし
//
inline ModSize
ModInvertedOverflowFileHeader::getFileIndex(
	const ModInvertedOverflowPageID pageId) const
{
	return pageId>>pageBits;
}

//
// FUNCTION public
// ModInvertedOverflowFileHeader::setPageSize -- 物理ページの大きさをセットする
//
// NOTES
// 物理ページの大きさ（キロバイト単位）をメンバ変数 pageSize 
// へセットする
//
// ARGUMENTS
//	const ModSize val
//		物理ページの大きさ
//
// RETURN
//	なし
//
// EXCEPTIONS
// なし
//
inline void
ModInvertedOverflowFileHeader::setPageSize(const ModSize val)
{
	this->pageSize = val;
}

//
// FUNCTION public
// ModInvertedOverflowFileHeader::getPageSize -- 物理ページの大きさを得る
//
// NOTES
//
// ARGUMENTS
// メンバ変数 pageSize を返す。
//
// RETURN
//	ModSize  物理ページの大きさ
//
// EXCEPTIONS
// なし
//
inline ModSize
ModInvertedOverflowFileHeader::getPageSize() const
{
	return this->pageSize;
}


//
// FUNCTION public
// ModInvertedOverflowFileHeader::setMaxFileSize -- 物理ファイルの上限をセットする
//
// NOTES
// 物理ファイルの上限（キロバイト単位）をメンバ変数 maxFileSize にセットする
//
// ARGUMENTS
//	const ModFileSize val
//		物理ファイルの上限
//
// RETURN
//	なし
//
// EXCEPTIONS
// なし
//
inline void
ModInvertedOverflowFileHeader::setMaxFileSize(const ModFileSize val)
{
	this->maxFileSize = val;
}

//
// FUNCTION public
// ModInvertedOverflowFileHeader::getMaxFileSize -- 物理ファイルの上限を得る
//
// NOTES
// maxFileSizeを返す
//
// ARGUMENTS
//	なし
//
// RETURN
//	ModFileSize  物理ファイルの上限
//
// EXCEPTIONS
// なし
//
inline ModFileSize
ModInvertedOverflowFileHeader::getMaxFileSize() const
{
	return this->maxFileSize;
}

//
// FUNCTION public
// ModInvertedOverflowFileHeader::calcMaxFileNum -- 物理ファイル数の上限を計算する
//
// NOTES
// 最大ページIDを表わすのに必要なビット数からくる制限(※)と、
// defaultMaxFileNum を比較して小さい方をOverflowFileの最大物理ファイ
// ル数とし、その値を返す。
//
// ※ OverflowFileの物理ファイル番号は ModInvertedOverflowPageID の上 
//    位ビット部分を使い、ページIDは下位ビット部分を使うのでこのよう
//    な制限が生じる。
//
// ARGUMENTS
//	なし
//
// RETURN
//	ModSize  物理ファイル数の上限
//
// EXCEPTIONS
// なし
//
inline ModSize
ModInvertedOverflowFileHeader::calcMaxFileNum()
{
	// ページIDの上限から定まるファイル数を得るためのビット数(pageBits)計算
	setPageBits();

	if ((1<<(32 - pageBits)) >= defaultMaxFileNum) {
		return defaultMaxFileNum;
	}
	return 1<<(32 - pageBits);
}

inline /*static*/ ModSize
ModInvertedOverflowFileHeader::calcMaxFileNum(const ModSize maxPageID)
{
	ModSize pageBits(1);
	ModSize tmp(maxPageID - 1);

	// maxPageID を表わすのに必要なビット数を計算する
	while (tmp > 1) {
		++pageBits;
		tmp >>= 1;
	}
	// 32ビットから pageBits 分を差し引いたbitで表わせる値と 
	// defaultMaxFileNum を比較し小さい方を返す
	if ( ( 1<<(32 - pageBits) )
		>= ModInvertedOverflowFileHeader::defaultMaxFileNum) {
		return ModInvertedOverflowFileHeader::defaultMaxFileNum;
	}
	return 1<<(32 - pageBits);
}


//
// FUNCTION public
// ModInvertedOverflowFileHeader::setMaxFileNum -- 物理ファイル数の上限をセット
//
// NOTES
// OverflowFileの物理ファイル数の上限をメンバ変数 maxFileNum にセット
// するOverflowFileの物理ファイル数の上限は calcMaxFileNum() メンバ関
// 数により計算できる。
//
// ARGUMENTS
//	const ModSize val
//		OverflowFileの物理ファイル数の上限
//
// RETURN
//	なし
//
// EXCEPTIONS
// なし
//
inline void
ModInvertedOverflowFileHeader::setMaxFileNum(const ModSize val)
{
	this->maxFileNum = val;
}

//
// FUNCTION public
// ModInvertedOverflowFileHeader::getMaxFileNum -- 物理ファイル数の上限を得る
//
// NOTES
// メンバ変数 maxFileNum を返す。
//
// ARGUMENTS
//	なし
//
// RETURN
//	ModSize  OverflowFile物理ファイル数
//
// EXCEPTIONS
// なし
//
inline ModSize
ModInvertedOverflowFileHeader::getMaxFileNum() const
{
	return this->maxFileNum;
}

//
// FUNCTION public
// ModInvertedOverflowFileHeader::setUsedFileNum -- 使用済物理ファイル数のセット
//
// NOTES
// OverflowFileの使用済物理ファイル数をメンバ変数 usedFileNum にセットする
//
// ARGUMENTS
//	const ModSize val
//		OverflowFileの使用済物理ファイル数
//
// RETURN
//	なし
//
// EXCEPTIONS
// なし
//
inline void
ModInvertedOverflowFileHeader::setUsedFileNum(const ModSize val)
{
	this->usedFileNum = val;
}

//
// FUNCTION public
// ModInvertedOverflowFileHeader::getUsedFileNum -- 使用済物理ファイル数を得る
//
// NOTES
// メンバ変数 usedFileNum を返す。OverflowFileの使用済物理ファイル数を返す。
//
// ARGUMENTS
//	なし
//
// RETURN
//	ModSize  OverflowFileの使用済物理ファイル数
//
// EXCEPTIONS
// なし
//
inline ModSize
ModInvertedOverflowFileHeader::getUsedFileNum() const
{
	return this->usedFileNum;
}

// 
//
// FUNCTION public
// ModInvertedOverflowFileHeader::setMaxPageID -- 個別物理ファイルのページ数の上限をセット
//
// NOTES
// OverflowFileの個別物理ファイルのページ数の上限をメンバ変数 
// maxPageID にセットする
//
// ARGUMENTS
//	const ModPhysicalPageID val
//		OverflowFileの個別物理ファイルのページ数の上限
//
// RETURN
//	なし
//
// EXCEPTIONS
// なし
//
inline void
ModInvertedOverflowFileHeader::setMaxPageID(const ModPhysicalPageID val)
{
	this->maxPageID = val;
}

//
// FUNCTION public
// ModInvertedOverflowFileHeader::getMaxPageID -- 個別物理ファイルのページ数の上限を得る
//
// NOTES
// OverflowFileの個別物理ファイルのページ数の上限を得る。メンバ変数 
// maxPageID を返す。
//
// ARGUMENTS
//	なし
//
// RETURN
//	ModPhysicalPageID  個別物理ファイルのページ数の上限
//
// EXCEPTIONS
// なし
//
inline ModPhysicalPageID
ModInvertedOverflowFileHeader::getMaxPageID() const
{
	return this->maxPageID;
}

//
// FUNCTION public
// ModInvertedOverflowFileHeader::setFileId -- 物理ファイルIDをセット
//
// NOTES
// 引数 index で表わされるOverflowFileのファイル番号に対応する物理
// ファイルIDをメンバ変数 fileID[] にセットする。
//
// ARGUMENTS
// const ModSize index
//		OverflowFileのファイル番号
// const ModPhysicalFileID val
//		物理ファイルID
//
// RETURN
//	なし
//
// EXCEPTIONS
// なし
//
inline void
ModInvertedOverflowFileHeader::setFileId(const ModSize index,
										 const ModPhysicalFileID val)
{
	this->fileID[index] = val;
}

//
// FUNCTION public
// ModInvertedOverflowFileHeader::getFileId -- 物理ファイルIDを得る
//
// NOTES
// 引数 index で表わされるOverflowFile番号に対応する物理ファイルIDを返す。
//
// ARGUMENTS
//	const ModSize index
//		OverflowFile番号
//
// RETURN
//	ModPhysicalFileID  OverflowFileの物理ファイルID
//
// EXCEPTIONS
// なし
//
inline ModPhysicalFileID
ModInvertedOverflowFileHeader::getFileId(const ModSize index) const
{
	return this->fileID[index];
}


//
// FUNCTION public
// ModInvertedOverflowFileHeader::setIdentifier -- 識別子の設定
//
// NOTES
// 識別子を設定する。
//
// ARGUMENTS
// const char identifier_
//		識別子
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
inline void
ModInvertedOverflowFileHeader::setIdentifier(const char identifier_)
{
	identifier = identifier_;
}


//
// FUNCTION public
// ModInvertedOverflowFileHeader::getIdentifier -- 識別子の取得
//
// NOTES
// 識別子を取得する。
//
// ARGUMENTS
// なし
//
// RETURN
// 識別子
//
// EXCEPTIONS
// なし
//
inline char
ModInvertedOverflowFileHeader::getIdentifier() const
{
	return identifier;
}


//
// FUNCTION
// operator<< -- オーバーフローファイルヘッダーのストリームへの出力演算子
//
// NOTES
// オーバーフローファイルヘッダーをメッセージストリーム／ストルストリームに
// 出力する。
//
// ARGUMENTS
// ModMessageStream& stream
//		出力するメッセージストリーム
// const ModInvertedOverflowFileHeader& header
//		オーバーフローファイルヘッダー
//
// RETURN
// メッセージストリーム
//
// EXCEPTIONS
// なし
//
inline ModMessageStream&
operator<<(ModMessageStream& stream, const ModInvertedOverflowFileHeader& header)
{
	stream << "pageSize: " << header.pageSize
		   << ", maxFileSize: " << header.maxFileSize
		   << ", maxPageID: " << header.maxPageID
		   << ", maxFileNum: " << header.maxFileNum
		   << ", usedFileNum: " << header.usedFileNum
		   << ", fileID:";

	for (ModSize i(0); i < header.usedFileNum; ++i) {
		stream << " [" << i << "]: " << header.fileID[i];
	}

	return stream;
}

inline ModOstream&
operator<<(ModOstream& stream, const ModInvertedOverflowFileHeader& header)
{
	stream << "pageSize: " << header.pageSize
		   << ", maxFileSize: " << header.maxFileSize
		   << ", maxPageID: " << header.maxPageID
		   << ", maxFileNum: " << header.maxFileNum
		   << ", usedFileNum: " << header.usedFileNum
		   << ", fileID:";

	for (ModSize i(0); i < header.usedFileNum; ++i) {
		stream << " [" << i << "]: " << header.fileID[i];
	}

	return stream;
}

#endif // __ModInvertedOverflowFileHeader_H__

//
// Copyright (c) 1999, 2002, 2023 Ricoh Company, Ltd.
// All rights reserved.
//

