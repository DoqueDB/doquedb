// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// FileParameter.h -- ファイルパラメータクラスのヘッダファイル
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

#ifndef __SYDNEY_VECTOR_FILEPARAMETER_H
#define __SYDNEY_VECTOR_FILEPARAMETER_H

#include "ModVector.h"
#include "ModString.h"

#include "Vector/Module.h"
#include "Vector/FileOption.h"

#include "LogicalFile/FileID.h"
#include "PhysicalFile/File.h"
#include "Common/DataType.h"
#include "FileCommon/IDNumber.h"

_SYDNEY_BEGIN

namespace Vector
{

//	CLASS
//	Vector::FileParameter -- ファイルパラメータのクラス
//
//	NOTES
//	ベクターファイルのファイルパラメータに
//	効率良くアクセスできるようにするクラス。
//
//	ユーザーが指定していない情報を取得しようとするとデフォルト値が返る。
//
class SYD_VECTOR_FUNCTION_TESTEXPORT FileParameter : public Common::Object
{
public:
	// コンストラクタ
	FileParameter(const LogicalFile::FileID& rFileID_);
	// デストラクタ
	~FileParameter();

	// アクセサ

	// ファイルIDへの参照を返す
	const LogicalFile::FileID& getFileOption() const;

	// ベクタファイル格納先ディレクトリパスを取得
  	const ModUnicodeString& getDirectoryPath() const;

	// ベクタファイル格納先ディレクトリパスを設定(File::move()が使用)
  	void setDirectoryPath(const ModUnicodeString& cPath_);

	// 物理ファイル格納戦略を取得
	const PhysicalFile::File::StorageStrategy& getStorageStrategy() const;

	// 物理ファイルバッファリング戦略を取得
	const PhysicalFile::File::BufferingStrategy& getBufferingStrategy() const;

	// 物理ページサイズを取得
	PhysicalFile::PageSize getPhysicalPageSize() const;

	// 1物理ページあたりのブロック数を取得
	ModUInt32		getBlocksPerPage() const;

	// オブジェクト1個を格納するための領域サイズ(byte)を取得
	ModSize		getBlockSize() const;

	// ビットマップ領域へのオフセット(byte)を取得
	ModSize		getBitMapAreaOffset() const;

#ifdef OBSOLETE
	// 物理ページごとのビットマップ領域のサイズ(byte)を取得
	ModSize		getBitMapAreaSize() const;
#endif

	// ブロック領域へのオフセット(byte)を取得
	ModSize		getBlockAreaOffset() const;

	// 外向きの意味でのフィールド数を取得
	ModUInt32		getOuterFieldNumber() const;

	// 内向きの意味でのフィールド数を取得
	ModUInt32		getInnerFieldNumber() const;

	// 任意の外部仕様フィールドのフィールドデータ型を取得
	Common::DataType::Type getDataTypeForOuterFieldID
		(ModUInt32 ulOuterFieldID_) const;

	// 任意の内部仕様フィールドのフィールドデータ型を取得
	Common::DataType::Type getDataTypeForInnerFieldID
		(ModUInt32 ulInnerFieldID_) const;

	// 以下の2関数のOuterFieldID版は作る意味がないので作らない

#ifdef OBSOLETE
	// ひとつのフィールドの領域サイズを取得
	ModSize	getInnerFieldSize (ModUInt32 ulInnerFieldID_) const;
#endif

	// ひとつのフィールドのブロック先頭からのオフセットを取得
	ModOffset	getInnerFieldOffset (ModUInt32 ulInnerFieldID_) const;

	// 同値性の検査
	bool equals(const FileParameter& rOtherFileParameter_) const;

	// ページ当たりのブロック数などに関する情報を設定する
	void initializeBlockParameters(ModSize ulDataSize_);

	// 
	void setMounted();
	void unsetMounted();

#ifdef OBSOLETE
	bool isMounted() const;
#endif

	Lock::FileName getLockName() const { return m_cIDNumber.getLockName();}

	// ファイルドライバのバージョン
	void setVersion(int iVersion_);
	int getVersion() const;

	// ページサイズ
	void setPageSize(ModSize pageSize_);

private:
	//- 規約5-20に従い、コピーコンストラクタと代入演算子の使用を禁止する
	FileParameter(const FileParameter& dummy_);
	FileParameter& operator=(const FileParameter& dummy_);

	// フィールド情報の配列を初期化する
	void initializeFieldInfoArray(const LogicalFile::FileID& rFileID_);

	// // ここより変数

	// 物理ファイル格納戦略
	PhysicalFile::File::StorageStrategy		m_cStorageStrategy;

	// 物理ファイルバッファリング戦略
	PhysicalFile::File::BufferingStrategy	m_cBufferingStrategy;

	// 物理ページのうち、実際に利用できる部分のサイズ(!=m_ulPageSize)
	ModSize		m_ulDataSize;

	// 1物理ページあたりのブロック数
	ModUInt32	m_ulBlocksPerPage;

	// ビットマップエリアのページ先頭からのオフセット
	ModSize		m_ulBitMapAreaOffset;

	// ブロックエリアのページ先頭からのオフセット
	ModSize		m_ulBlockAreaOffset;

	// 物理ページごとのビットマップ領域のサイズ
	ModSize		m_ulBitMapAreaSize;

	// 1オブジェクトあたりの(外部的な意味での)フィールド数
	ModUInt32	m_ulOuterFieldNumber;

	// レコードオブジェクト1つあたりのサイズ(byte)
	// VectorKeyはファイルには記録しないのでサイズには含まれない
	ModSize		m_ulBlockSize;

	// 検索時に頻繁にアクセスされるフィールド情報を納める構造体
	struct FieldInfo // structで十分
	{
		Common::DataType::Type	m_iDataType;	 // フィールドのデータ型
		ModSize					m_ulFieldSize;	 // フィールドのサイズ(byte)
		ModOffset				m_ulFieldOffset; // フィールドのオフセット(byte)
	};

	// フィールド情報を格納するベクタ。要素数は外部的な意味でのフィールド数と同じ。
	FieldInfo*	m_pFieldInfoArrayWithOuterFieldID;

	// ファイルID
	LogicalFile::FileID			m_cFileID;

	// ファイル識別整数値
	const FileCommon::IDNumber	m_cIDNumber;
};

} // end of namespace Vector

_SYDNEY_END

#endif /* __SYDNEY_VECTOR_FILEPARAMETER_H */

//
//	Copyright (c) 2000, 2001, 2002, 2004, 2005, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
