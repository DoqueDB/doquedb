// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// OpenParameter.h -- Ｂ＋木ファイルオープンパラメータクラスのヘッダーファイル
// 
// Copyright (c) 2000,2001,2002, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_BTREE_OPENPARAMETER_H
#define __SYDNEY_BTREE_OPENPARAMETER_H

#include "Btree/Module.h"

#include "Common/IntegerArrayData.h"
#include "Common/DataArrayData.h"
#include "LogicalFile/TreeNodeInterface.h"

#include "FileCommon/OpenMode.h"

#include "Btree/FileParameter.h"

_SYDNEY_BEGIN

namespace LogicalFile
{
class OpenOption;
}

namespace Btree
{

//
//	CLASS
//	Btree::OpenParameter -- Ｂ＋木ファイルオープンパラメータクラス
//
//	NOTES
//	Ｂ＋木ファイルをオープンしてからクローズするまでの間に使用する
//	パラメータのクラス。
//
class SYD_BTREE_FUNCTION OpenParameter : public Common::Object
{
public:

	// コンストラクタ
	OpenParameter();

	// コンストラクタ
	OpenParameter(const FileParameter&				cFileParameter_,
				  const LogicalFile::OpenOption&	cOpenOption_);

	OpenParameter(const FileParameter&	FileParam_,
				  const OpenParameter&	Original_);

	// デストラクタ
	~OpenParameter();

	// Ｂ＋木ファイルオープンパラメータを設定する
	void set(const FileParameter&			cFileParameter_,
			 const LogicalFile::OpenOption&	cOpenOption_);

	//
	// データメンバ
	//

	// オープンモード
	FileCommon::OpenMode::Mode	m_iOpenMode;

	// 見積りフラグ
	bool						m_bEstimate;

	// ビットセットフラグ
	//   false : 通常のオブジェクト取得モード
	//   true  : ビットセットオブジェクト取得モード
	bool						m_bGetByBitSet;

	// フィールド選択フラグ
	bool						m_bFieldSelect;

	// 処理対象フィールドインデックス配列
	// ※ 他の論理ファイルドライバが指定するために用意
	Common::IntegerArrayData	m_cTargetFieldIndexArray;

	// 処理対象フィールドインデックス配列
	int*						m_TargetFieldIndexArray;

	// 処理対象フィールド数
	int							m_TargetFieldNum;

	// 処理対象フィールドインデックス配列の確保サイズ
	ModSize						m_TargetFieldIndexArrayAllocateSize;

	// オブジェクトIDフィールドが処理対象フィールドかどうか
	//		true  : 処理対象フィールド
	//		false : 処理対象フィールドではない
	bool						m_SelectObjectID;

	// キーフィールドに処理対象フィールドが含まれているかどうか
	//		true  : 含まれている
	//		false : 含まれていない
	bool						m_ExistTargetFieldInKey;

	// バリューフィールドに処理対象フィールドが含まれているかどうか
	//		true  : 含まれている
	//		false : 含まれていない
	bool						m_ExistTargetFieldInValue;

	//
	//	STRUCT
	//	Btree:OpenParameter::SearchCondition -- 検索条件用パラメータ
	//
	//	NOTES
	//	検索条件用パラメータ。
	//
	struct SearchCondition
	{
		// 検索対象キーフィールドインデックス配列
		Common::IntegerArrayData	m_cFieldIndexArray;

		// 検索開始条件配列
		Common::DataArrayData		m_cStartArray;

		// 検索開始比較演算子配列
		// ※ Common::IntegerArrayData だが、各要素には
		//    LogicalFile::TreeNodeInterface::Type をキャストして
		//    設定する。
		Common::IntegerArrayData	m_cStartOpeArray;

		// 検索終了条件配列
		Common::DataArrayData		m_cStopArray;

		// 検索終了比較演算子配列
		// ※ 同上
		Common::IntegerArrayData	m_cStopOpeArray;

		//
		// 以下は、likeによる文字列検索用のメンバ
		//
		SearchCondition() //初期化（他のメンバーには、コンストラクタが定義済み）
			: m_SetEscape(false)
			, m_Escape(0)
			, m_VoidSearch(false)
		{}
		// likeでの検索の際のエスケープ文字が指定されているかどうか
		bool						m_SetEscape;

		// likeでの検索の際のエスケープ文字
		ModUnicodeChar				m_Escape;

		// 検索結果が'φ'となる問い合わせ
		bool						m_VoidSearch;
	};

	// 検索条件
	struct Btree::OpenParameter::SearchCondition	m_cSearchCondition;

	//
	//	ENUM
	//	Btree::OpenParameter::SortKeyType -- ソートキーの種別
	//
	//	NOTES
	//	ソートキーの種別。
	//	キーフィールドあるいはオブジェクトID
	//
	enum SortKeyType
	{
		KeyField = 0, // キーフィールド
		ObjectID      // オブジェクトID
	};

	// ソートキーフラグ
	SortKeyType					m_iSortKeyType;

	// オブジェクト取得ソート順
	//   false : オブジェクトソート順
	//   true  : オブジェクトソート順と逆順
	bool						m_bSortReverse;

	//
	//	ENUM
	//	Btree::OpenParameter::ReadSubMode -- オブジェクト取得サブモード列挙子
	//
	//	NOTES
	//	オブジェクト取得サブモード列挙子。
	//
	enum ReadSubMode
	{
		ScanRead = 0,
		FetchRead
	};

	// オブジェクト取得サブモード
	ReadSubMode					m_iReadSubMode;

	// Fetch 対象フィールドインデックス配列
	Common::IntegerArrayData	m_cFetchFieldIndexArray;

private:

	// 処理対象フィールドインデックス配列を設定する
	void setTargetFields(const FileParameter&	FileParam_);

	// 処理対象フィールドインデックス配列を解放する
	void clearTargetFields();

	// 検索条件パラメータを設定する
	void setSearchCondition(const FileParameter&			cFileParameter_,
							const LogicalFile::OpenOption&	cOpenOption_);

	// 検索条件とソートキーの整合性をチェックする
	void checkSortKey() const;

	// 検索条件となるコモンデータを返す
	static Common::Data*
		getSearchCondition(const FileParameter&		cFileParameter_,
						   const int				iSearchFieldIndex_,
						   const ModUnicodeString&	cstrValueString_);

	// 検索条件の比較演算子を返す
	static LogicalFile::TreeNodeInterface::Type
		getCompareOperator(
			int	iOperatorValue_);


}; // end of class Btree::OpenParameter

} // end of namespace Btree

_SYDNEY_END

#endif // __SYDNEY_BTREE_OPENPARAMETER_H

//
//	Copyright (c) 2000,2001,2002, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
