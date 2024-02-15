// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Estimator.h -- 見積りクラスのヘッダーファイル
// 
// Copyright (c) 2000, 2001, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_BTREE_ESTIMATOR_H
#define __SYDNEY_BTREE_ESTIMATOR_H

#include "Common/Object.h"

#include "PhysicalFile/Types.h"

#include "Os/Memory.h"

_SYDNEY_BEGIN

namespace Trans
{
class Transaction;
}

namespace PhysicalFile
{
class File;
}

namespace Btree
{

class FileInformation;
class FileParameter;
class OpenParameter;
class ValueFile;

//
//	CLASS
//	Btree::Estimator -- 見積りクラス
//
//	NOTES
//	見積りクラス。
//
class Estimator
{
public:

	// コンストラクタ
	//Estimator();

	// デストラクタ
	//~Estimator();

	// Ｂ＋木ファイルサイズを返す [byte]
	static ModUInt64
		getSize(const Trans::Transaction*	Transaction_,
				PhysicalFile::File*			TreePhysicalFile_,
				PhysicalFile::File*			ValuePhysicalFile_);

	// Ｂ＋木ファイルに挿入されているオブジェクト数を返す
	static ModInt64 getCount(const Trans::Transaction*	Transaction_,
							 PhysicalFile::File*		TreePhysicalFile_);

	// 検索時のオーバヘッドコストを返す [秒]
	static double
		getOverhead(const Trans::Transaction*	Transaction_,
					PhysicalFile::File*			TreePhysicalFile_,
					const FileParameter&		FileParam_,
					const OpenParameter&		OpenParam_);

	// オブジェクトへアクセスする際のプロセスコストを返す [秒]
	static double
		getProcessCost(const Trans::Transaction*	Transaction_,
					   PhysicalFile::File*			TreePhysicalFile_,
					   ValueFile*					ValueFile_,
					   const FileParameter&			FileParam_,
					   const OpenParameter&			OpenParam_);

private:

	// リーフページのキーオブジェクト検索のオーバヘッドコストを返す [秒]
	static double
		getOverheadAtSearch(
			const Trans::Transaction*	Transaction_,
			PhysicalFile::File*			TreePhysicalFile_,
			const FileParameter&		FileParam_,
			const OpenParameter&		OpenParam_);

	// キーフィールドの記録サイズを返す [byte]
	static Os::Memory::Size
		getKeyFieldArchiveSize(
			const Trans::Transaction*	Transaction_,
			PhysicalFile::File*			TreePhysicalFile_,
			const ModUInt64				ObjectNum_,
			const FileParameter&		FileParam_);

	// オブジェクトに無制限可変長フィールドが含まれているかを知らせる
	static bool
		existUnlimitedVariableField(
			const FileParameter&	FileParam_,
			bool					bIsKeyObject_);

	// リーフページ数を設定する
	static void
		setLeafPageNumber(
			const Trans::Transaction*	Transaction_,
			PhysicalFile::File*			TreePhysicalFile_,
			ModUInt32&					LeafPageNum_,
			ModUInt32&					PhysicalPageNum_);

	// リーフページ全てのの空き領域合計サイズを返す [byte]
	static Os::Memory::Size
		getLeafPageFreeAreaSize(
			const Trans::Transaction*	Transaction_,
			PhysicalFile::File*			TreePhysicalFile_);

	// 可変長フィールドの記録サイズを返す [byte]
	static Os::Memory::Size
		getOutsideVariableFieldArchiveSize(
			const FileParameter&	FileParam_,
			bool					IsKeyObject_);

	// オブジェクト取得の際のプロセスコストを返す [秒]
	static double
		getProcessCostAtRead(
			const Trans::Transaction*	Transaction_,
			PhysicalFile::File*			TreePhysicalFile_,
			ValueFile*					ValueFile_,
			const FileParameter&		FileParam_,
			const OpenParameter&		OpenParam_);

	// バリューオブジェクト読み込み時間を返す [秒]
	static double
		getValueObjectReadTime(
			const Trans::Transaction*	Transaction_,
			ValueFile*					ValueFile_,
			const ModUInt64				ObjectNum_,
			const FileParameter&		FileParam_);

	// バリューフィールドの記録サイズを返す [byte]
	static Os::Memory::Size
		getValueFieldArchiveSize(
			const Trans::Transaction*	Transaction_,
			ValueFile*					ValueFile_,
			const ModUInt64				ObjectNum_,
			const FileParameter&		FileParam_);

	// 全てのバリューページの空き領域合計サイズを返す [byte]
	static Os::Memory::Size
		getValuePageFreeAreaSize(
			const Trans::Transaction*	Transaction_,
			PhysicalFile::File*			ValuePhysicalFile_);

	// バリューページ数を返す
	static PhysicalFile::PageNum
		getValuePageNum(
			const Trans::Transaction*	Transaction_,
			PhysicalFile::File*			ValuePhysicalFile_);

	// 1 秒間にファイルからメモリに転送できるバイト数を返す [byte/sec]
	static int getCostFileToMemory();

#ifdef OBSOLETE
	// 1 秒間にメモリからファイルに転送できるバイト数を返す [byte/sec]
	static int getCostMemoryToFile();
#endif //OBSOLETE

#ifdef OBSOLETE
	// 1 秒間にメモリからメモリに転送できるバイト数を返す [byte/sec]
	static int getCostMemoryToMemory();
#endif //OBSOLETE

}; // end of class Btree::Estimator

} // end of namespace Btree

_SYDNEY_END

#endif // __SYDNEY_BTREE_ESTIMATOR_H

//
//	Copyright (c) 2000, 2001, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
