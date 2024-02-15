// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// FileParameter.h -- Ｂ＋木ファイルパラメータクラスのヘッダーファイル
// 
// Copyright (c) 2000, 2001, 2002, 2004, 2023, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_BTREE_FILEPARAMETER_H
#define __SYDNEY_BTREE_FILEPARAMETER_H

#include "Btree/Module.h"
#include "Btree/KeyPosType.h"

#include "LogicalFile/FileID.h"
#include "PhysicalFile/File.h"

_SYDNEY_BEGIN

namespace FileCommon
{
	class IDNumber;
	class HintArray;
}

_SYDNEY_BTREE_BEGIN

//
//	CLASS
//	Btree::FileParameter -- Ｂ＋木ファイルパラメータクラス
//
//	NOTES
//	Ｂ＋木ファイルオブジェクトを生成してから破棄するまでの間に使用する
//	パラメータのクラス。
//	FileID (LogicalFile::FileID) を扱いやすいようにパラメータ値を保持しておく。
//	※ なぜLogicalFile::FileIDを継承しているか…
//		File::create時の戻り値として利用しているため。
//
class FileParameter : public LogicalFile::FileID
{
public:

	// コンストラクタ
	FileParameter(const LogicalFile::FileID&	FileID_);

	// デストラクタ
	~FileParameter();

	// ファイルパラメータを初期化する
	void initialize(const LogicalFile::FileID&	FileID_);

	// ファイルパラメータの後処理をする
	void terminate();

	// フィールド値の記録サイズを返す [byte]
	Os::Memory::Size getFieldArchiveSize(const int	FieldIndex_) const;

	// ファイル格納先ディレクトリを変更する
	void changeBtreeFilePath(const ModUnicodeString&	Path_);

	//
	// データメンバ
	//

	//
	// 静的データメンバ
	//

	// 可変長フィールドを外置きオブジェクトに記録するかどうかの閾値
	// （可変長フィールド最大長[byte]）
	//     8バイト以下：外置きオブジェクトにはならない
	//     9バイト以上：外置きオブジェクトになる
	static const Os::Memory::Size	VariableFieldInsideThreshold; // = 8

	//
	// 非静的データメンバ
	//

	// 物理ページサイズ
	Os::Memory::Size				m_PhysicalPageSize;

	// ツリーファイルの格納戦略
	PhysicalFile::File::StorageStrategy
									m_TreeFileStorageStrategy;

	// バリューファイルの格納戦略
	PhysicalFile::File::StorageStrategy
									m_ValueFileStorageStrategy;

	// バッファリング戦略
	PhysicalFile::File::BufferingStrategy
									m_BufferingStrategy;

	// 1ノードあたりのキー数
	ModUInt32						m_KeyPerNode;

	//
	//	STRUCT
	//	Btree::FileParameter::UniqueType -- ユニークタイプ
	//
	//	NOTES
	//	ユニークタイプ。
	//
	struct UniqueType
	{
		//
		//	ENUM
		//	Btree::FileParameter::UniqueType::Value -- ユニークタイプ
		//
		//	NOTES
		//	ユニークタイプ。
		//
		enum Value
		{
			NotUnique = 0, // ユニーク指定なし
			Object,        // オブジェクト値でユニーク
			Key,           // キー値でユニーク
			Undefined
		};
	};

	// ユニークタイプ
	UniqueType::Value				m_UniqueType;

	// フィールド数
	int								m_FieldNum;

	// キーフィールド数
	int								m_KeyNum;

	// バリューフィールド数
	int								m_ValueNum;

	// 先頭バリューフィールドのインデックス
	int								m_TopValueFieldIndex;

	// フィールドタイプ配列
	Common::DataType::Type*			m_FieldTypeArray;

	// フィールドタイプ配列の確保サイズ [byte]
	ModSize							m_FieldTypeArrayAllocateSize;

	// 固定長フィールドフラグ配列
	bool*							m_IsFixedFieldArray;

	// 固定長フィールドフラグ配列の確保サイズ [byte]
	ModSize							m_IsFixedFieldArrayAllocateSize;

	// フィールド最大長配列（可変長フィールド用）
	Os::Memory::Size*				m_FieldMaxLengthArray;

	// フィールド最大長配列の確保サイズ [byte]
	ModSize							m_FieldMaxLengthArrayAllocateSize;

	// 外置きフィールドフラグ配列
	bool*							m_FieldOutsideArray;

	// 外置きフィールドフラグ配列の確保サイズ [byte]
	ModSize							m_FieldOutsideArrayAllocateSize;

	// 配列フィールドフラグ配列
	bool*							m_IsArrayFieldArray;

	// 配列フィールドフラグ配列の確保サイズ [byte]
	ModSize							m_IsArrayFieldArrayAllocateSize;

	// 要素タイプ配列（配列フィールド用）
	Common::DataType::Type*			m_ElementTypeArray;

	// 要素タイプ配列の確保サイズ [byte]
	ModSize							m_ElementTypeArrayAllocateSize;

	// 最大要素数配列（配列フィールド用）
	int*							m_ElementMaxNumArray;

	// 最大要素崇拝列の確保サイズ [byte]
	ModSize							m_ElementMaxNumArrayAllocateSize;

	// 固定長要素フラグ配列（配列フィールド用）
	bool*							m_IsFixedElementArray;

	// 固定長要素フラグ配列の確保サイズ [byte]
	ModSize							m_IsFixedElementArrayAllocateSize;

	// 要素最大長配列（配列フィールド用）
	Os::Memory::Size*				m_ElementMaxLengthArray;

	// 要素最大長配列の確保サイズ [byte]
	ModSize							m_ElementMaxLengthArrayAllocateSize;

	struct SortOrder
	{
		enum Value
		{
			Ascending = 0, // 昇順
			Descending,    // 降順
			Undefined
		};
	};

	// キーフィールドソート順配列
	SortOrder::Value*				m_KeyFieldSortOrderArray;

	// キーフィールドソート順配列の確保サイズ [byte]
	ModSize							m_KeyFieldSortOrderArrayAllocateSize;

	// キーフィールド比較結果との乗数配列
	int*							m_MultiNumberArray;

	// キーフィールド比較結果との乗数配列の確保サイズ [byte]
	ModSize							m_MultiNumberArrayAllocateSize;

	// キー値記録先
	KeyPosType::Value				m_KeyPosType;

	// キー情報に記録するキー値の記録サイズ
	// （キー値をキー情報に記録する場合にのみ有効）
	Os::Memory::Size				m_KeySize;

	// 外置きキーフィールドが存在するかどうか
	//		true  : 外置きキーフィールドが存在する
	//		false : 外置きキーフィールドが存在しない
	bool							m_ExistOutsideFieldInKey;

	// 外置きバリューフィールドが存在するかどうか
	//		true  : 外置きバリューフィールドが存在する
	//		false : 外置きバリューフィールドが存在しない
	bool							m_ExistOutsideFieldInValue;

	// 可変長キーフィールドが存在するかどうか
	//		true  : 可変長キーフィールドが存在する
	//		false : 可変長キーフィールドが存在しない
	bool							m_ExistVariableFieldInKey;

	// 可変長バリューフィールドが存在するかどうか
	//		true  : 可変長バリューフィールドが存在する
	//		false : 可変長バリューフィールドが存在しない
	bool							m_ExistVariableFieldInValue;

	// 外置きキーフィールドの最大長の合計
	Os::Memory::Size				m_OutsideKeyFieldTotalLengthMax;

	//
	// ※ 配列フィールドはキーフィールドに指定できないので、
	// 　 "m_ExistArrayFieldInKey" というのはいらない。
	//

	// 配列バリューフィールドが存在するかどうか
	//		true  : 配列バリューフィールドが存在する
	//		false : 配列バリューフィールドが存在しない
	bool							m_ExistArrayFieldInValue;

	// 代表キーオブジェクトの記録サイズ [byte]
	Os::Memory::Size				m_DirectKeyObjectSize;

	// 代表バリューオブジェクトの記録サイズ [byte]
	Os::Memory::Size				m_DirectValueObjectSize;

	// ノード内のキー分割率 [%]
	int								m_NodeKeyDivideRate;

	// ノードマージチェックの閾値（キー数）
	ModUInt32						m_NodeMergeCheckThreshold;

	// ノードマージ実行の閾値（キー数）
	ModUInt32						m_NodeMergeExecuteThreshold;

	// 論理ファイルIDから一意な整数値を得るためのオブジェクトへのポインタ
	FileCommon::IDNumber*			m_IDNumber;

private:

	// ファイル格納先ディレクトリパスを設定する
	void setBtreeFilePath(const LogicalFile::FileID&	FileID_,
						  ModUnicodeString&			Path_);

	// 物理ページサイズを設定する
	void setPhysicalPageSize(const LogicalFile::FileID&	FileID_);

	// 一時ファイルかどうかを設定する
	bool setTemporary(const LogicalFile::FileID&	FileID_);

	// 読み込み専用かどうかを設定する
	bool setReadOnly(const LogicalFile::FileID&	FileID_);

	// マウントされているかどうかを設定する
	bool setMounted(const LogicalFile::FileID&	FileID_);

	// ツリーファイル格納戦略を設定する
	void setTreeFileStorageStrategy(const ModUnicodeString&	Path_,
									const bool				IsTemp_,
									const bool				Mounted_);

	// バリューファイル格納戦略を設定する
	void setValueFileStorageStrategy(const ModUnicodeString&	Path_,
									 const bool					IsTemp_,
									 const bool					Moutned_);

	// バッファリング戦略を設定する
	void setBufferingStrategy(const bool	IsTemp_,
							  const bool	ReadOnly_);

	// ユニークタイプを設定する
	void setUniqueType(const LogicalFile::FileID&	FileID_);

	// フィールドパラメータを設定する
	void setFieldParam(const LogicalFile::FileID&	FileID_);

	// フィールド数などを設定する
	void setFieldNum(const LogicalFile::FileID&	FileID_);

	// フィールドデータタイプを返す
	Common::DataType::Type
		getFieldType(const LogicalFile::FileID&	FileID_,
					 const int					FieldIndex_);

	// 可変長フィールドパラメータを設定する
	void setVariableFieldParam(const LogicalFile::FileID&	FileID_,
							   const int				FieldIndex_);

	// 配列フィールドパラメータを設定する
	void setArrayFieldParam(const LogicalFile::FileID&	FileID_,
							const int					FieldIndex_);

	// フィールドソートパラメータを設定する
	void setSortParam(const LogicalFile::FileID&	FileID_,
					  const int					FieldIndex_);

	// オブジェクトの記録フォーマットを設定する
	void setObjectForm();

	// 固定長キー値の記録サイズを返す
	Os::Memory::Size getFixedKeyRealSize() const;

	// リーフページのキーオブジェクトのために物理ページサイズを更新する
	void resetPageSizeForLeafKey();

	// バリューオブジェクトのために物理ページサイズを更新する
	void resetPageSizeForValue();

	// 代表キーオブジェクトの記録サイズを返す [byte]
	Os::Memory::Size getDirectKeyObjectSize() const;

	// 代表バリューオブジェクトの記録サイズを返す [byte]
	Os::Memory::Size getDirectValueObjectSize() const;

	// ファイルヒントを解析する
	void analyzeFileHint(const LogicalFile::FileID&	FileID_);

	// ファイルヒントに設定されている整数値を得る
	static bool getFileHintValue(const FileCommon::HintArray& hintArray_,
								 const ModUnicodeString& FileHint_,
								 const char* const	HintKey_,
								 int&				HintValue_);

	// 各配列へのポインタを設定する
	void setArrayPointer();

	// ローカルは配列を初期化する
	void initializeLocalArray();

	// 次数を設定する
	void setKeyPerNode(const LogicalFile::FileID&	FileID_);

	// デフォルトの次数を設定する
	void setDefaultKeyPerNode();

	// 次数のデフォルト値を返す
	ModUInt32 getDefaultKeyPerNode() const;

	// サポートしているフィールドデータタイプかどうかを知らせる
	bool
	isSupportFieldType(Common::DataType::Type FieldType_) const;

	// ページサイズを2倍にして、それを矯正する。
	void
	resizePhysicalPageSize();

	//
	// データメンバ
	//

	//
	// 静的データメンバ
	//

	// データメンバの各配列をヒープ領域に確保するかどうかの閾値
	// （フィールド数）
	//     10フィールド以下：ローカルな配列を使用
	//     11フィールド以上：ヒープ領域に配列を確保
	static const int		LocalLimit; // = 10

	//
	// 非静的データメンバ
	//

	// フィールドタイプ配列
	Common::DataType::Type	m_FieldTypeLocalArray[10];

	// 固定長フィールドフラグ配列
	bool					m_IsFixedFieldLocalArray[10];

	// フィールド最大長配列（可変長フィールド用）
	Os::Memory::Size		m_FieldMaxLengthLocalArray[10];

	// 外置きフィールドフラグ配列
	bool					m_FieldOutsideLocalArray[10];

	// 配列フィールドフラグ配列
	bool					m_IsArrayFieldLocalArray[10];

	// 要素タイプ配列（配列フィールド用）
	Common::DataType::Type	m_ElementTypeLocalArray[10];

	// 最大要素数配列（配列フィールド用）
	int						m_ElementMaxNumLocalArray[10];

	// 固定長要素フラグ配列（配列フィールド用）
	bool					m_IsFixedElementLocalArray[10];

	// 要素最大長配列（配列フィールド用）
	Os::Memory::Size		m_ElementMaxLengthLocalArray[10];

	// キーフィールドソート順配列
	SortOrder::Value		m_KeyFieldSortOrderLocalArray[10];

	// キーフィールド比較結果との乗数配列
	int						m_MultiNumberLocalArray[10];

}; // end of class Btree::FileParameter

_SYDNEY_BTREE_END
_SYDNEY_END

#endif // __SYDNEY_BTREE_FILEPARAMETER_H

//
//	Copyright (c) 2000, 2001, 2002, 2004, 2023, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
