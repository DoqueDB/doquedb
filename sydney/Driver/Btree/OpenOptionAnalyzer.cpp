// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// OpenOptionAnalyzer.cpp --
//		Ｂ＋木ファイルオープンオプション解析器クラスの実現ファイル
// 
// Copyright (c) 2000, 2001, 2002, 2004, 2006, 2009, 2023 Ricoh Company, Ltd.
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

namespace
{
const char	srcFile[] = __FILE__;
const char	moduleName[] = "Btree";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "Btree/OpenOptionAnalyzer.h"

#include "Exception/BadArgument.h"
#include "Exception/Unexpected.h"

#include "Common/Assert.h"
#include "Common/IntegerArrayData.h"
#include "Common/UnicodeString.h"

#include "FileCommon/DataManager.h"

#include "Btree/OpenOption.h"
#include "Btree/MultiSearchConditionItem.h"
#include "Btree/FileParameter.h"

_SYDNEY_USING
_SYDNEY_BTREE_USING

//////////////////////////////////////////////////
//
//	PUBLIC METHOD
//
//////////////////////////////////////////////////

//
//	FUNCTION public
//	Btree::OpenOptionAnalyzer::getSearchParameter --
//		検索オープンパラメータを設定する
//
//	NOTES
//	検索オープンパラメータを設定する。
//	Btree::File::getSearchParameter の下請け関数。
//
//	ARGUMENTS
//	const LogicalFile::TreeNodeInterface*	Condition_
//		検索条件オブジェクトへのポインタ
//	LogicalFile::OpenOption&				OpenOption_
//		Ｂ＋木ファイルオープンオプションオブジェクトへの参照
//	const Btree::FileParameter&				FileParam_
//		Ｂ＋木ファイルパラメータオブジェクトへの参照
//
//	RETURN
//	bool
//		指定した検索条件でオブジェクトの高速検索が可能かどうか
//			true  : 指定した検索条件でオブジェクトの高速検索が可能
//			false : 指定した検索条件ではオブジェクトの高速検索が不可能（※）
//			        ※検索結果が 'φ’になってしまう場合には true を返す。
//			        　これは、OpenOption の内容を判断して決定する。
//
//	EXCEPTIONS
//	なし
//
// static
bool
OpenOptionAnalyzer::getSearchParameter(
	const LogicalFile::TreeNodeInterface*	Condition_,
	LogicalFile::OpenOption&				OpenOption_,
	const FileParameter&					FileParam_)
{
	// Ｂ＋木ファイルは、初回File::get()時に、
	// 検索条件と一致するすべてのオブジェクトを保持しない。
	// …ということを利用者に知らせる。
	OpenOption_.setBoolean( _SYDNEY_OPEN_PARAMETER_KEY(FileCommon::OpenOption::CacheAllObject::Key), false );

	bool ret = false;
	if (Condition_ == 0)
	{
		// 検索条件が設定されていない…

		//
		// Ｂ＋木ファイルの場合、
		// 検索条件が設定されていないのは、Scanモードと解釈する。
		//

		//
		// Scanモード
		//
		ret = 
			OpenOptionAnalyzer::setScanSearchParameter(OpenOption_);
	}
	else
	{
		LogicalFile::TreeNodeInterface::Type	nodeType = Condition_->getType();

		if (nodeType == LogicalFile::TreeNodeInterface::Fetch)
		{
			//
			// Fetchモード
			//
			ret = 
				OpenOptionAnalyzer::setFetchSearchParameter(Condition_,
															   OpenOption_,
															   FileParam_);
		}
		else if (nodeType == LogicalFile::TreeNodeInterface::Like)
		{
			//
			// likeでの先頭文字列キーフィールドへの検索
			//
			ret = 
				OpenOptionAnalyzer::setLikeSearchParameter(Condition_,
															  OpenOption_,
															  FileParam_);
		}

		//
		// Searchモード
		//
		else if (nodeType == LogicalFile::TreeNodeInterface::Equals ||
			nodeType == LogicalFile::TreeNodeInterface::GreaterThan ||
			nodeType == LogicalFile::TreeNodeInterface::GreaterThanEquals ||
			nodeType == LogicalFile::TreeNodeInterface::LessThan ||
			nodeType == LogicalFile::TreeNodeInterface::LessThanEquals)
		{
			ret = 
				OpenOptionAnalyzer::setSingleSearchParameter(
					Condition_,
					OpenOption_,
					FileParam_);
		}
		else if (nodeType == LogicalFile::TreeNodeInterface::EqualsToNull)
		{
			ret = 
				OpenOptionAnalyzer::setEqualsToNullSearchParameter(
					Condition_,
					OpenOption_,
					FileParam_);
		}
		else if (nodeType == LogicalFile::TreeNodeInterface::And ||
				 nodeType == LogicalFile::TreeNodeInterface::List)
		{
			; _SYDNEY_ASSERT(FileParam_.m_KeyNum >= 1);

			int	targetFieldNum = FileParam_.m_KeyNum + 1;
			//        オブジェクト ID フィールドの分 ~~~

			MultiSearchConditionItem*	conditions =
				new MultiSearchConditionItem[targetFieldNum];//Vector にすべきでは？
			try {
				ret =
					OpenOptionAnalyzer::setMultiSearchParameter(
						Condition_,
						OpenOption_,
						FileParam_,
						conditions,
						targetFieldNum);
			} catch (...) {
				delete [] conditions;//Vector にすると、ModAutoPointerが使える。
				_SYDNEY_RETHROW;
			}
			delete [] conditions;//Vector にすると、ModAutoPointerが使える。
		}
		else
		{
#ifdef DEBUG
			SydDebugMessage << "Unknown node type. (=" << (int)nodeType << ")" << ModEndl;
#endif

			return false;//サポート外（必ず false）
		}
	}
	return ret;
}

//
//	FUNCTION public
//	Btree::OpenOptionAnalyzer::getTargetParameter --
//		Btree::File::getProjectionParameter
//		Btree::File::getUpdateParameter
//		の下請け関数
//
//	NOTES
//	引数TargetFields_で指定されている 1 つ以上の
//	フィールドインデックスを読みとり、
//	オブジェクト取得時には、該当するフィールドのみで
//	オブジェクトを構成するようにオープンオプションを設定する。
//	例えば、
//		・オブジェクト ID フィールド（これは、必ず存在する）
//		・キーフィールド × 2 (key#1, key#2)
//		・バリューフィールド × 3 (value#1, value#2, value#3)
//	で構成されるオブジェクトを挿入するためのＢ＋木ファイルから
//	オブジェクトを取得する際に、key#2, value#3 のみを
//	取得するのであれば、引数TargetFields_は、下図のように設定する。
//
//	      cProjection_
//	   ┌───────┐
//	   │     ┌──┐ │
//	   │ [0] │  2 │ │
//	   │     ├──┤ │
//	   │ [1] │  5 │ │
//	   │     └──┘ │
//	   └───────┘
//
//	Btree::File::getProjectionParameter
//	Btree::File::getUpdateParameter
//	の下請け関数。
//
//	ARGUMENTS
//	const Common::IntegerArrayData&		TargetFields_
//		フィールドインデックス配列オブジェクトへの参照
//	LogicalFile::OpenOption&			OpenOption_
//		Ｂ＋木ファイルオープンオプションオブジェクトへの参照
//	const FileCommon::OpenMode::Mode	OpenMode_
//		オープンモード
//	const Btree::FileParameter&			FileParam_
//		Ｂ＋木ファイルパラメータオブジェクトへの参照
//
//	RETURN
//	bool
//		Ｂ＋木ファイルの場合、すべてのフィールドを返すことが可能なので、
//		常に true を返す。（ true を返せないようなときはエラーである。）
//
//	EXCEPTIONS
//	BadArgument
//		不正な引数
//
// static
bool
OpenOptionAnalyzer::getTargetParameter(
	const Common::IntegerArrayData&		TargetFields_,
	LogicalFile::OpenOption&			OpenOption_,
	const FileCommon::OpenMode::Mode	OpenMode_,
	const FileParameter&				FileParam_)
{
	if ( OpenOption_.getBoolean(_SYDNEY_BTREE_OPEN_PARAMETER_KEY(Btree::OpenOption::GetByBitSet::Key)) )
	{
		; _SYDNEY_ASSERT(TargetFields_.getCount() == 1);

		int	bitSetFieldIndex = TargetFields_.getElement(0);

		Common::DataType::Type	bitSetFieldType = *(FileParam_.m_FieldTypeArray + bitSetFieldIndex);

		; _SYDNEY_ASSERT( bitSetFieldType == Common::DataType::UnsignedInteger );
	}

	//
	// 引数TargetFields_の要素数がおかしくないかチェック
	//

	; _SYDNEY_ASSERT(FileParam_.m_FieldNum >= 2);

	int	targetFieldNum = TargetFields_.getCount();

	if (targetFieldNum == 0 || targetFieldNum > FileParam_.m_FieldNum)
	{
		throw Exception::BadArgument(moduleName, srcFile, __LINE__);
	}

	//
	// 引数TargetFields_の各要素で指定される
	// フィールドインデックスがおかしくないかチェック
	// ※ チェックしながら引数 OpenOption_ にパラメータを
	//    設定していけないので、先にチェックする。
	//

	int	i;

	for (i = 0; i < targetFieldNum; ++i)
	{
		int	fieldIndex = TargetFields_.getElement(i);

		if (fieldIndex > FileParam_.m_FieldNum)
		{
			throw Exception::BadArgument(moduleName, srcFile, __LINE__);
		}

		if (OpenMode_ == FileCommon::OpenMode::Update && fieldIndex == 0)
		{
			//
			// ※ オブジェクト ID の更新はできない。
			//

#ifdef DEBUG

			SydDebugMessage << "Object id is can't update." << ModEndl;

#endif

			return false;
		}
	}

	//
	// ここまで来れば、おかしなフィールド指定はない
	//

	//
	// この関数が、関数 getProjectionParameter から呼ばれた場合、
	// Read/Search モードで Ｂ＋木ファイルをオープンするということ。
	// ということは、先に関数 getSearchParameter が呼ばれているかも
	// 知れない。その場合、オープンモードは既に設定されているはず。
	// もし、既にオープンモードが設定されているのであれば、
	// ここでは、オープンモードは設定しない。
	//

	int	currentOpenMode = OpenOption_.getInteger( _SYDNEY_OPEN_PARAMETER_KEY(FileCommon::OpenOption::OpenMode::Key) );

	if (OpenMode_ == FileCommon::OpenMode::Read &&
		currentOpenMode != FileCommon::OpenOption::OpenMode::Read &&
		currentOpenMode != FileCommon::OpenOption::OpenMode::Search)
	{
		//
		// 挿入／更新／削除いずれかのモードが既に設定されているのであれば、
		// それはエラー
		//

		if (currentOpenMode == FileCommon::OpenOption::OpenMode::Update)
		{
			throw Exception::BadArgument(moduleName, srcFile, __LINE__);
		}

		// オープンオプションにオープンモードパラメータを設定
		OpenOption_.setInteger(_SYDNEY_OPEN_PARAMETER_KEY(FileCommon::OpenOption::OpenMode::Key), FileCommon::OpenOption::OpenMode::Read);
	}

	//
	// この関数が、関数 Btree::File::getUpdateParameter から呼ばれた場合、
	// Update モードで Ｂ＋木ファイルをオープンするということ。
	//

	if (OpenMode_ == FileCommon::OpenMode::Update)
	{
		// オープンオプションにオープンモードパラメータを設定
		OpenOption_.setInteger(_SYDNEY_OPEN_PARAMETER_KEY(FileCommon::OpenOption::OpenMode::Key), FileCommon::OpenOption::OpenMode::Update);
	}

	// オープンオプションにフィールド選択指定パラメータを設定
	OpenOption_.setBoolean(_SYDNEY_OPEN_PARAMETER_KEY(FileCommon::OpenOption::FieldSelect::Key), true);

	// オープンオプションに処理対象フィールド数パラメータを設定
	OpenOption_.setInteger(_SYDNEY_OPEN_PARAMETER_KEY(FileCommon::OpenOption::TargetFieldNumber::Key), targetFieldNum);

	for (i = 0; i < targetFieldNum; ++i)
	{
		// オープンオプションに
		// 処理対象フィールドインデックスパラメータを設定
		int	targetFieldIndex = TargetFields_.getElement(i);
		OpenOption_.setInteger(_SYDNEY_OPEN_PARAMETER_FORMAT_KEY(FileCommon::OpenOption::TargetFieldIndex::Key, i), targetFieldIndex);
	}

	return true;
}

//
//	FUNCTION public
//	Btree::OpenOptionAnalyzer::getSortParameter --
//		ソート順パラメータを設定する
//
//	NOTES
//	オブジェクト取得時のソート順パラメータを設定する。
//	Btree::File::getSortParameter の下請け関数。
//
//	ARGUMENTS
//	const Common::IntegerArrayData&	Keys_
//		ソート順を指定するフィールドインデックスの列への参照
//	const Common::IntegerArrayData&	Orders_
//		引数Keys_で指定されたフィールドのソート順の列への参照
//	LogicalFile::OpenOption&		OpenOption_
//		Ｂ＋木ファイルオープンオプションオブジェクトへの参照
//	const Btree::FileParameter&		FileParam_
//		Ｂ＋木ファイルパラメータオブジェクトへの参照
//
//	RETURN
//	bool
//		指定されたソート順でオブジェクトを返せる場合には true を、
//		そうでない場合には false を返す。
//
//	EXCEPTIONS
//	BadArgument
//		不正な引数
//
// static
bool
OpenOptionAnalyzer::getSortParameter(
	const Common::IntegerArrayData&	Keys_,
	const Common::IntegerArrayData&	Orders_,
	LogicalFile::OpenOption&		OpenOption_,
	const FileParameter&			FileParam_)
{
	int	sortKey;

	bool	sortReverse = false;

	if (Keys_.getCount() != Orders_.getCount() || Keys_.getCount() < 1)
	{
		throw Exception::BadArgument(moduleName, srcFile, __LINE__);
	}

	//
	// getSearchParameterが先に呼ばれた形跡があれば
	// 指定ソート順との整合性のチェックをする
	//

	bool	establishedSearchParam = false;
	bool	objectIDSearch = false;
	OpenOptionAnalyzer::checkSearchField(OpenOption_,
										 establishedSearchParam,
										 objectIDSearch);

	if (Keys_.getElement(0) == 0)
	{
		// オブジェクト ID がソートキー…

		if (Keys_.getCount() > 1)
		{

#ifdef DEBUG

			SydDebugMessage << "Illegal key." << ModEndl;

#endif

			return false;
		}

		if (establishedSearchParam && objectIDSearch == false)
		{

#ifdef DEBUG

			SydDebugMessage << "Illegal key." << ModEndl;

#endif

			return false;
		}

		sortKey = OpenOption::SortKey::ObjectID;

		//
		// オブジェクトの取得順を指定する
		//   true  : オブジェクトIDの降順でオブジェクトを取得
		//   false : オブジェクトIDの昇順でオブジェクトを取得
		//

		if (Orders_.getElement(0) == 1)
		{
			sortReverse = true;
		}
	}
	else
	{
		// キーフィールドがソートキー…

		if (establishedSearchParam && objectIDSearch)
		{

#ifdef DEBUG

			SydDebugMessage << "Illegal key." << ModEndl;

#endif

			return false;
		}

		//
		// すべてのキーフィールドについて
		// ソート順を指定していなければならない
		// → …ことはない
		//

		; _SYDNEY_ASSERT(FileParam_.m_KeyNum >= 1);

#if 0
		if (Keys_.getCount() != FileParam_.m_KeyNum)
		{

#ifdef DEBUG

			SydDebugMessage << "Illegal key." << ModEndl;

#endif

			return false;
		}
#endif

		sortKey = OpenOption::SortKey::KeyField;

		FileParameter::SortOrder::Value	firstKeySortOrder =
			*(FileParam_.m_KeyFieldSortOrderArray + 1);

		; _SYDNEY_ASSERT(
			firstKeySortOrder != FileParameter::SortOrder::Undefined);

		int	intFirstKeySortOrder =
			(firstKeySortOrder == FileParameter::SortOrder::Ascending) ?
				0 : 1;

		if (intFirstKeySortOrder != Orders_.getElement(0))
		{
			sortReverse = true;
		}

		int	elementNum = Keys_.getCount();

		for (int i = 1; i < elementNum; ++i)
		{
			int	keyFieldIndex = Keys_.getElement(i);

			FileParameter::SortOrder::Value	keySortOrder =
				*(FileParam_.m_KeyFieldSortOrderArray + keyFieldIndex);

			; _SYDNEY_ASSERT(
				keySortOrder != FileParameter::SortOrder::Undefined);

			int	intKeySortOrder =
				(keySortOrder == FileParameter::SortOrder::Ascending) ?
					0 : 1;

			if (sortReverse == false)
			{
				if (Orders_.getElement(i) != intKeySortOrder)
				{

#ifdef DEBUG

					SydDebugMessage << "Can't set sort order." << ModEndl;

#endif

					return false;
				}
			}
			else
			{
				if (Orders_.getElement(i) == intKeySortOrder)
				{

#ifdef DEBUG

					SydDebugMessage << "Can't set sort order." << ModEndl;

#endif

					return false;
				}
			}
		}

	} // end else

	//
	// パラメータ設定
	//

	// "SortKey" パラメータ
	OpenOption_.setInteger(_SYDNEY_BTREE_OPEN_PARAMETER_KEY(Btree::OpenOption::SortKey::Key), sortKey);

	// "SortReverse" パラメータ
	OpenOption_.setBoolean(_SYDNEY_BTREE_OPEN_PARAMETER_KEY(Btree::OpenOption::SortReverse::Key), sortReverse);

	return true;
}

//////////////////////////////////////////////////
//
//	PRIVATE METHOD
//
//////////////////////////////////////////////////

//
//	FUNCTION private
//	Btree::OpenOptionAnalyzer::setScanSearchParameter --
//		Scanモード検索オープンパラメータを設定する
//
//	NOTES
//	Scanモード検索オープンパラメータを設定する。
//
//	ARGUMENTS
//	LogicalFile::OpenOption&	OpenOption_
//		Ｂ＋木ファイルオープンオプションオブジェクトへの参照
//
//	RETURN
//	bool
//		指定した検索条件でオブジェクトの高速検索が可能かどうか
//		※ Scanモードが指定された場合、File::getSearchParameter()は、
//		　 必ずtrueを返す。（高速検索可能である。）
//			true  : 指定した検索条件でオブジェクトの高速検索が可能
//			false : 指定した検索条件ではオブジェクトの高速検索が不可能
//
//	EXCEPTIONS
//	なし
//
// static
bool
OpenOptionAnalyzer::setScanSearchParameter(
	LogicalFile::OpenOption&	OpenOption_)
{
	//
	// ※ Scanモードのオープンモードは"Read"とする。
	// 　 同じ"Read"モードのFetchモードとは
	// 　 オブジェクト取得サブモードパラメータで区別する。
	//

	//
	// "OpenMode"パラメータ
	//
	OpenOption_.setInteger( _SYDNEY_OPEN_PARAMETER_KEY(FileCommon::OpenOption::OpenMode::Key), _SYDNEY_OPEN_PARAMETER_VALUE(FileCommon::OpenOption::OpenMode::Read));

	//
	// "ReadSubMode"パラメータ
	//
	OpenOption_.setInteger( _SYDNEY_OPEN_PARAMETER_KEY(FileCommon::OpenOption::ReadSubMode::Key), _SYDNEY_OPEN_PARAMETER_VALUE(FileCommon::OpenOption::ReadSubMode::Scan));

	return true;
}

//
//	FUNCTION private
//	Btree::OpenOptionAnalyzer::setFetchSearchParameter -- 
//		Fetchモード検索オープンパラメータを設定する
//
//	NOTES
//	Fetchモード検索オープンパラメータを設定する。
//
//	ARGUMENTS
//	const LogicalFile::TreeNodeInterface*	Condition_
//		Fetch検索条件オブジェクトへのポインタ
//	LogicalFile::OpenOption&				OpenOption_
//		Ｂ＋木ファイルオープンオプションオブジェクトへの参照
//	const Btree::FileParameter&				FileParam_
//		Ｂ＋木ファイルパラメータオブジェクトへの参照
//
//	RETURN
//	bool
//		指定した検索条件でオブジェクトの高速検索が可能かどうか
//			true  : 指定した検索条件でオブジェクトの高速検索が可能
//			false : 指定した検索条件ではオブジェクトの高速検索が不可能
//
//	EXCEPTIONS
//	なし
//
// static
bool
OpenOptionAnalyzer::setFetchSearchParameter(
	const LogicalFile::TreeNodeInterface*	Condition_,
	LogicalFile::OpenOption&				OpenOption_,
	const FileParameter&					FileParam_)
{
	//
	// 既に検索条件用のパラメータが設定されている場合と
	// されていない場合では、許可する条件が異なる。
	// 既に設定されている場合（Search + Fetchモード）は、
	// 無意味な条件以外は基本的に何でもOKとする。
	//

	bool	fetchOnly = true;

	int	openMode;

	if (OpenOption_.getInteger(_SYDNEY_OPEN_PARAMETER_KEY(FileCommon::OpenOption::OpenMode::Key), openMode) &&
		openMode == FileCommon::OpenOption::OpenMode::Search)
	{
		fetchOnly = false;
	}

	//
	// 引数Condition_のチェックと
	// Fetch対象フィールドのインデックスを設定する
	//

	Common::IntegerArrayData	fetchFieldIndexArray;

	if (OpenOptionAnalyzer::setFetchFieldIndexArray(Condition_,
													fetchFieldIndexArray,
													FileParam_,
													OpenOption_,
													fetchOnly)
		== false)
	{

#ifdef DEBUG

		SydDebugMessage << "Can't set fetch field." << ModEndl;

#endif

		return false;
	}

	// オープンオプションに Fetch 検索パラメータを設定する
	OpenOptionAnalyzer::setFetchSearchParameter(fetchFieldIndexArray,
												OpenOption_,
												fetchOnly);

	return true;
}

//
//	FUNCTION private
//	Btree::OpenOptionAnalyzer::setLikeSearchParameter --
//		likeでの先頭文字列キーフィールドへの検索のための
//		オープンパラメータを設定する
//
//	NOTES
//	likeでの先頭文字列キーフィールドへの検索のための
//	オープンパラメータを設定する。
//
//	ARGUMENTS
//	const LogicalFile::TreeNodeInterface*	LikeNode_
//		like検索条件が設定されているツリーノードへのポインタ
//	LogicalFile::OpenOption&				OpenOption_
//		Ｂ＋木ファイルオープンオプションオブジェクトへの参照
//	const Btree::FileParameter&				FileParam_
//		Ｂ＋木ファイルパラメータオブジェクトへの参照
//
//	RETURN
//	bool
//		指定した検索条件でオブジェクトの高速検索が可能かどうか
//			true  : 指定した検索条件でオブジェクトの高速検索が可能
//			false : 指定した検索条件ではオブジェクトの高速検索が不可能
//
//	EXCEPTIONS
//	なし
//
// static
bool
OpenOptionAnalyzer::setLikeSearchParameter(
	const LogicalFile::TreeNodeInterface*	LikeNode_,
	LogicalFile::OpenOption&				OpenOption_,
	const FileParameter&					FileParam_)
{
	if (*(FileParam_.m_FieldTypeArray + 1) != Common::DataType::String)
	{
		// 先頭キーフィールドがCommon::DataType::String型の
		// フィールドではない…

		// それはできない。

#ifdef DEBUG

		SydDebugMessage << "Can't like search." << ModEndl;

#endif

		return false;
	}

	if (LikeNode_->getOperandSize() != 2)
	{
		// Likeノードのオペランドは2つのはず。
		// 内訳…
		//   ① Fieldノード
		//   　 （フィールドインデックスが記録されている）
		//   ② ConstantValueノードまたはVariableノード
		//   　 （パターン文字列が記録されている）

#ifdef DEBUG

		SydDebugMessage << "Illegal number of operands." << ModEndl;

#endif

		return false;
	}

	//
	// 2つのオペランド、どちらがFieldノードで
	// どちらがConstantValueノードかをチェックする。
	//

	int	patternNodeIndex = 1;

	LogicalFile::TreeNodeInterface::Type	firstOperandType =
		LikeNode_->getOperandAt(0)->getType();

	if (firstOperandType == LogicalFile::TreeNodeInterface::ConstantValue ||
		firstOperandType == LogicalFile::TreeNodeInterface::Variable)
	{
		patternNodeIndex = 0;
	}

	int	fieldNodeIndex = (patternNodeIndex == 1) ? 0 : 1;

	//
	// Fieldノードに先頭キーフィールドのインデックスが
	// 記録されているかをチェックする。
	//

	const LogicalFile::TreeNodeInterface*	fieldNode =
		LikeNode_->getOperandAt(fieldNodeIndex);

	if (fieldNode->getType() != LogicalFile::TreeNodeInterface::Field)
	{
		// Fieldノードがちゃんと設定されていない…

#ifdef DEBUG

		SydDebugMessage << "Illegal field node." << ModEndl;

#endif

		return false;
	}

	int	fieldIndex = ModUnicodeCharTrait::toInt(fieldNode->getValue());

	if (fieldIndex != 1)
	{
		// 先頭キーフィールド以外が指定されている…

#ifdef DEBUG

		SydDebugMessage << "Illegal field index." << ModEndl;

#endif

		return false;
	}

	//
	// パターン文字列が記録されているであろうノードが
	// ConstantValueノードまたはVariableノードかどうかをチェックする。
	//

	const LogicalFile::TreeNodeInterface*	patternNode =
		LikeNode_->getOperandAt(patternNodeIndex);

	LogicalFile::TreeNodeInterface::Type	patternNodeType =
		patternNode->getType();

	if (patternNodeType != LogicalFile::TreeNodeInterface::ConstantValue &&
		patternNodeType != LogicalFile::TreeNodeInterface::Variable)
	{
#ifdef DEBUG

		SydDebugMessage << "Illegal pattern node." << ModEndl;

#endif

		return false;
	}

	ModUnicodeString	patternString = patternNode->getValue();

	if (patternString.getLength() == 0)
	{
#ifdef DEBUG

		SydDebugMessage << "Illegal pattern string." << ModEndl;
#endif

		return false;
	}

	//
	// 先頭1文字が'%'か'_'じゃないかチェックする。
	// （いきなりワイルドカードを使われると、高速で検索できない。）
	//

	if (patternString[0] == Common::UnicodeChar::usPercent ||
		patternString[0] == Common::UnicodeChar::usLowLine)
	{
#ifdef DEBUG

		SydDebugMessage << "Can't like search." << ModEndl;

#endif

		return false;
	}

	//
	// エスケープ文字が指定されていれば、それもチェックする。
	//

	int	optionNum = LikeNode_->getOptionSize();

	bool	setEscape = (optionNum > 0);

	ModUnicodeString	escapeString;

	if (optionNum > 0)
	{
		if (optionNum != 1)
		{
			// そんなにオプションを指定されても、
			// 今のB+木ファイルでは、エスケープ文字しか
			// 認識できないです…

#ifdef DEBUG

			SydDebugMessage << "Illegal number of option." << ModEndl;

#endif

			return false;
		}

		const LogicalFile::TreeNodeInterface*	escapeNode =
			LikeNode_->getOptionAt(0);

		//
		// エスケープ文字が記録されているであるノードが
		// ConstantValueノードまたはVariableノードかどうかをチェックする。
		//

		LogicalFile::TreeNodeInterface::Type	escapeNodeType =
			escapeNode->getType();

		if (escapeNodeType !=
			LogicalFile::TreeNodeInterface::ConstantValue &&
			escapeNodeType !=
			LogicalFile::TreeNodeInterface::Variable)
		{

#ifdef DEBUG

			SydDebugMessage << "Illegal escape node." << ModEndl;

#endif

			return false;
		}

		escapeString = escapeNode->getValue();
	}

	//
	// オープンオプションにパラメータを設定する
	//

	// "OpenMode"パラメータ
	OpenOption_.setInteger(_SYDNEY_OPEN_PARAMETER_KEY(FileCommon::OpenOption::OpenMode::Key), _SYDNEY_OPEN_PARAMETER_VALUE(FileCommon::OpenOption::OpenMode::Search));

	// "SearchFieldNumber"パラメータ
	OpenOption_.setInteger(_SYDNEY_BTREE_OPEN_PARAMETER_KEY(Btree::OpenOption::SearchFieldNumber::Key), 1);

	// "SearchFieldIndex[0]"パラメータ
	OpenOption_.setInteger(_SYDNEY_BTREE_OPEN_PARAMETER_FORMAT_KEY(Btree::OpenOption::SearchFieldIndex::Key, 0), 1);

	// "SearchStart[0]"パラメータ
	OpenOption_.setString(_SYDNEY_BTREE_OPEN_PARAMETER_FORMAT_KEY(Btree::OpenOption::SearchStart::Key, 0), patternString);

	// "SearchStartOpe[0]"パラメータ
	OpenOption_.setInteger(_SYDNEY_BTREE_OPEN_PARAMETER_FORMAT_KEY(Btree::OpenOption::Ope::StartKey, 0), _SYDNEY_OPEN_PARAMETER_VALUE(Btree::OpenOption::Ope::Like));

	if (setEscape)
	{
		OpenOption_.setString(_SYDNEY_BTREE_OPEN_PARAMETER_KEY(Btree::OpenOption::Escape::Key), escapeString);
	}

	return true;
}

//
//	FUNCTION private
//	Btree::OpenOptionAnalyzer::isBinaryDataField --
//		Common::BinaryData 型のフィールドかどうかをチェックする
//
//	NOTES
//	Ｂ＋木ファイルに挿入されている（する）オブジェクトの指定フィールドが
//	Common::BinaryData 型のフィールドかどうかをチェックする。
//	Ｂ＋木ファイルは、Common::BinaryData 型のフィールドに対する
//	検索はできないため。
//
//	ARGUMENTS
//	const Btree::FileParameter&	FileParam_
//		Ｂ＋木ファイルパラメータオブジェクトへの参照
//	const int					FieldIndex_
//		フィールドインデックス
//
//	RETURN
//	bool
//		引数FieldIndex_で指定されたフィールドが
//		Common::BinaryData 型フィールドの場合には true を、
//		そうでない場合には false を返す。
//
//	EXCEPTIONS
//	なし
//
// static
bool
OpenOptionAnalyzer::isBinaryDataField(
	const FileParameter&	FileParam_,
	const int				FieldIndex_)
{
	; _SYDNEY_ASSERT(FieldIndex_ < FileParam_.m_FieldNum);

	Common::DataType::Type	fieldType =
		*(FileParam_.m_FieldTypeArray + FieldIndex_);

	return (fieldType == Common::DataType::Binary);
}

//
//	FUNCTION private
//	Btree::OpenOptionAnalyzer::setFetchFieldIndexArray --
//		Fetch対象フィールドのインデックスを設定する
//
//	NOTES
//	Fetch対象フィールドのインデックスを設定する。
//
//	ARGUMENTS
//	const LogicalFile::TreeNodeInterface*	Condition_
//		検索条件オブジェクトへのポインタ
//	Common::IntegerArrayData&				FetchFieldIndexArray_
//		Fettch 対象フィールドインデックス配列への参照
//	const Btree::FileParameter&				FileParam_
//		Ｂ＋木ファイルパラメータオブジェクトへの参照
//	LogicalFile::OpenOption&				OpenOption_
//		Ｂ＋木ファイルオープンオプションオブジェクトへの参照
//	const bool								FetchOnly_
//		Fetch モードのみかどうか
//			true  : Fetchのみ
//			false : Search + Fetch
//
//	RETURN
//	bool
//		指定した検索条件でオブジェクトの高速検索が可能かどうか
//			true  : 指定した検索条件でオブジェクトの高速検索が可能
//			false : 指定した検索条件ではオブジェクトの高速検索が不可能
//
//	EXCEPTIONS
//	なし
//
// static
bool
OpenOptionAnalyzer::setFetchFieldIndexArray(
	const LogicalFile::TreeNodeInterface*	Condition_,
	Common::IntegerArrayData&				FetchFieldIndexArray_,
	const FileParameter&					FileParam_,
	const LogicalFile::OpenOption&			OpenOption_,
	const bool								FetchOnly_)
{
	; _SYDNEY_ASSERT(Condition_->getOptionSize() == 2);

	const LogicalFile::TreeNodeInterface*	fetchedColumns =
		Condition_->getOptionAt(0);

	if (fetchedColumns == 0)
	{

#ifdef DEBUG

		SydDebugMessage << "Illegal tree node." << ModEndl;

#endif

		return false;
	}

	int	operandSize = fetchedColumns->getOperandSize();

	if(operandSize == 0)
	{

#ifdef DEBUG

		SydDebugMessage << "Illegal tree node." << ModEndl;

#endif

		return false;
	}

	; _SYDNEY_ASSERT(FileParam_.m_FieldNum >= 2);

	if (FetchOnly_ == false)
	{
		// Search + Fetchモード…

		return
			OpenOptionAnalyzer::setFetchFieldIndexArray(
				fetchedColumns,
				FetchFieldIndexArray_,
				FileParam_,
				OpenOption_);
	}

	//
	// Fetchモード
	//

	//
	// File::getSearchParameter()よりも先に
	// File::getSortParameter()が呼ばれているのであれば、
	// ソートキーとFetch対象フィールドの整合性の
	// チェックを行なう。
	//

	bool	establishedSortKey = false;
	bool	sortKeyIsObjectID = false;

	OpenOptionAnalyzer::checkSortKey(OpenOption_,
									 establishedSortKey,
									 sortKeyIsObjectID);

	bool	setTop = false;

	for (int i = 0; i < operandSize; ++i)
	{
		const LogicalFile::TreeNodeInterface*	columnNode =
			fetchedColumns->getOperandAt(i);

		int	fieldIndex =
			ModUnicodeCharTrait::toInt(columnNode->getValue());

		if (i == 0 && fieldIndex == 0)
		{
			// オブジェクトIDでFetch…

			if (establishedSortKey && sortKeyIsObjectID == false)
			{
				// ソートキーとの整合性が合わない…

				return false;
			}

			//
			// オブジェクトIDでFetchするならば
			// キーフィールドも同時に指定されていないことを
			// 確認する
			//

			if (operandSize > 1)
			{
				// オブジェクトIDフィールドとは
				// 別のフィールドも指定されているらしい…

				//
				// オブジェクトIDは、
				// 単独でなければFetchできない。
				//

#ifdef DEBUG

				SydDebugMessage
					<< "Can't fetch by object id & key." << ModEndl;

#endif

				return false;
			}

			FetchFieldIndexArray_.setElement(0, 0);

			break;
		}

		// キーフィールド（キー値）でFetch…

		if (establishedSortKey && sortKeyIsObjectID)
		{
			// ソートキーとの整合性が合わない…

			return false;
		}

		if (fieldIndex > FileParam_.m_FieldNum)
		{
			// おかしなフィールドインデックス…

#ifdef DEBUG

			SydDebugMessage << "Illegal fetch field index." << ModEndl;

#endif

			return false;
		}

		if (OpenOptionAnalyzer::isBinaryDataField(FileParam_,
												  fieldIndex))
		{
			// Common::BinaryDataフィールドが指定されている…

			//
			// Common::BinaryDataフィールドではFetchできない。
			//

#ifdef DEBUG

			SydDebugMessage
				<< "Can't saerch by binary field." << ModEndl;

#endif

			return false;
		}

		if (fieldIndex == i + 1)
		{
			if (setTop == false)
			{
				setTop = true;
			}
		}
		else if (setTop == false)
		{
			//
			// いきなり途中のフィールドから
			// 指定しようとしても困る。
			// せめて、先頭キーフィールド 1 つくらいは
			// 指定してもらわないと…。
			//

#ifdef DEBUG

			SydDebugMessage << "Illegal fetch field index." << ModEndl;

#endif

			return false;
		}

		FetchFieldIndexArray_.setElement(i, fieldIndex);

	} // end for i

	return true;
}

//
//	FUNCTION private
//	Btree::OpenOptionAnalyzer::setFetchFieldIndexArray --
//		Search + Fetchモード時のFetch対象フィールドのインデックスを設定する
//
//	NOTES
//	Search + Fetchモード時のFetch対象フィールドのインデックスを設定する。
//
//	ARGUMENTS
//	const LogicalFile::TreeNodeInterface*	FetchedColumns_
//		Fetch対象フィールドインデックスが設定されている
//		検索条件オブジェクトへのポインタ
//	Common::IntegerArrayData&				FetchFieldIndexArray_
//		Fetch対象フィールドインデックス配列への参照
//	const Btree::FileParameter&				FileParam_
//		Ｂ＋木ファイルパラメータオブジェクトへの参照
//	const LogicalFile::OpenOption&				OpenOption_
//		Ｂ＋木ファイルオープンオプションオブジェクトへの参照
//
//	RETURN
//	bool
//		指定した検索条件でオブジェクトの高速検索が可能かどうか
//			true  : 指定した検索条件でオブジェクトの高速検索が可能
//			false : 指定した検索条件ではオブジェクトの高速検索が不可能
//
//	EXCEPTIONS
//	なし
//
// static
bool
OpenOptionAnalyzer::setFetchFieldIndexArray(
	const LogicalFile::TreeNodeInterface*	FetchedColumns_,
	Common::IntegerArrayData&				FetchFieldIndexArray_,
	const FileParameter&					FileParam_,
	const LogicalFile::OpenOption&			OpenOption_)
{
	//
	// Search + Fetch モード
	//

	//
	// もう既にSearchモードが設定されているのであれば、
	// ソートキーとの整合性チェックはすんでいるはずなので、
	// ここでは行なわない。
	//

	//
	// 不正なフィールドインデックスがないかチェックするために
	// オープンオプション（引数OpenOption_）に設定されている（はずの）
	// 検索対象キーフィールドに関するパラメータを読み込み、
	// 検索対象キーフィールドインデックスの配列を生成する。
	//

	Common::IntegerArrayData	searchFieldIndexArray;

	int	searchFieldNum = -1;

	bool	find = OpenOption_.getInteger(_SYDNEY_BTREE_OPEN_PARAMETER_KEY(OpenOption::SearchFieldNumber::Key), searchFieldNum);
	if (searchFieldNum == 0) return true; //検索結果が‘φ’

	; _SYDNEY_ASSERT(find);
	; _SYDNEY_ASSERT(searchFieldNum >= 1);

	int	i;

	for (i = 0; i < searchFieldNum; ++i)
	{
		int	searchFieldIndex = -1;
		find = OpenOption_.getInteger(_SYDNEY_BTREE_OPEN_PARAMETER_FORMAT_KEY(Btree::OpenOption::SearchFieldIndex::Key, i), searchFieldIndex);

		; _SYDNEY_ASSERT(find);
		; _SYDNEY_ASSERT(searchFieldIndex >= 1);

		searchFieldIndexArray.setElement(i, searchFieldIndex);
	}

	int	operandSize = FetchedColumns_->getOperandSize();

	if (operandSize == 0)
	{
		// Fetch対象フィールドが指定されていない！？…

#ifdef DEBUG

		SydDebugMessage << "Illegal tree node." << ModEndl;

#endif

		return false;
	}

	; _SYDNEY_ASSERT(FileParam_.m_FieldNum >= 2);

	for (i = 0; i < operandSize; ++i)
	{
		const LogicalFile::TreeNodeInterface*	columnNode =
			FetchedColumns_->getOperandAt(i);

		int	fetchFieldIndex =
			ModUnicodeCharTrait::toInt(columnNode->getValue());

		if (fetchFieldIndex == 0)
		{
			// オブジェクトIDフィールドが指定された…

			//
			// Search+Fetchでは、
			// “オブジェクトIDでFetch”はできない。
			//

#ifdef DEBUG

			SydDebugMessage
				<< "Can't search by key + fetch by object id." << ModEndl;

#endif

			return false;
		}

		if (OpenOptionAnalyzer::isBinaryDataField(FileParam_,
												  fetchFieldIndex))
		{
			// Common::BinaryDataフィールドが指定された…

			//
			// Common::BinaryDataフィールドでのFetchはできない
			//

#ifdef DEBUG

			SydDebugMessage << "Can't search by binary field." << ModEndl;

#endif

			return false;
		}

		FetchFieldIndexArray_.setElement(i, fetchFieldIndex);
	}

	return true;
}

//
//	FUNCTION private
//	Btree::OpenOptionAnalyzer::checkSortKey --
//		既にオープンオプションにソート順パラメータが
//		設定されているかをチェックする
//
//	NOTES
//	既に引数OpenOption_（オープンオプション）に
//	ソート順パラメータが設定されているかをチェックする。
//
//	ARGUMENTS
//	const LogicalFile::OpenOption&	OpenOption_
//		Ｂ＋木ファイルオープンオプションオブジェクトへの参照
//	bool&						EstablishedSortKey_
//		ソートキーが確定しているかどうか（本関数が設定する）
//	bool&						SortKeyIsObjectID_
//		ソートキーがオブジェクトIDかどうか（本関数が設定する）
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
// static
void
OpenOptionAnalyzer::checkSortKey(
	const LogicalFile::OpenOption&	OpenOption_,
	bool&							EstablishedSortKey_,
	bool&							SortKeyIsObjectID_)
{
	int	paramValue;

	if (OpenOption_.getInteger(_SYDNEY_BTREE_OPEN_PARAMETER_KEY(Btree::OpenOption::SortKey::Key), paramValue))
	{
		// ソート順パラメータが設定されている…

		EstablishedSortKey_ = true;

		if (paramValue == OpenOption::SortKey::ObjectID)
		{
			// ソートキーがオブジェクトID…

			SortKeyIsObjectID_ = true;
		}
		else if (paramValue == OpenOption::SortKey::KeyField)
		{
			// ソートキーがキーフィールド…

			SortKeyIsObjectID_ = false;
		}
		else
		{
			; _SYDNEY_ASSERT(false);
			throw Exception::Unexpected(moduleName, srcFile, __LINE__);
		}
	}
	else
	{
		// ソート順パラメータが設定されていない…

		EstablishedSortKey_ = false;
		SortKeyIsObjectID_ = false;
	}
}

//
//	FUNCTION private
//	Btree::OpenOptionAnalyzer::checkSearchField --
//		検索条件とソート順の整合性をチェックする
//
//	NOTES
//	検索条件とソート順の整合性をチェックする。
//
//	ARGUMENTS
//	const LogicalFile::OpenOption&	OpenOption_
//		Ｂ＋木ファイルオープンオプションオブジェクトへの参照
//	bool&						EstablishedSearchParam_
//		検索用のパラメータが引数 OpenOption_ に設定されているかどうか
//	bool&						ObjectIDSearch_
//		オブジェクト ID による検索かどうか
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	BadArgument
//		不正な引数
//
// static
void
OpenOptionAnalyzer::checkSearchField(
	const LogicalFile::OpenOption&	OpenOption_,
	bool&							EstablishedSearchParam_,
	bool&							ObjectIDSearch_)
{
	EstablishedSearchParam_ = false;
	ObjectIDSearch_ = false;

	int	paramValue = -1;

	if (OpenOption_.getInteger(_SYDNEY_BTREE_OPEN_PARAMETER_KEY(Btree::OpenOption::SearchFieldNumber::Key), paramValue))
	{
		if (paramValue < 0)
		{
			throw Exception::BadArgument(moduleName, srcFile, __LINE__);
		}

		EstablishedSearchParam_ = true;

		paramValue = -1;
		if (OpenOption_.getInteger(_SYDNEY_BTREE_OPEN_PARAMETER_FORMAT_KEY(Btree::OpenOption::SearchFieldIndex::Key, 0), paramValue) == false || paramValue < 1)
		{
			throw Exception::BadArgument(moduleName, srcFile, __LINE__);
		}

		if (paramValue == 0)
		{
			ObjectIDSearch_ = true;
		}
	}

	paramValue = -1;

	if (OpenOption_.getInteger(_SYDNEY_BTREE_OPEN_PARAMETER_KEY(Btree::OpenOption::FetchFieldNumber::Key) ,paramValue))
	{
		if (paramValue < 1)
		{
			throw Exception::BadArgument(moduleName, srcFile, __LINE__);
		}

		paramValue = -1;
		if (OpenOption_.getInteger(_SYDNEY_BTREE_OPEN_PARAMETER_FORMAT_KEY(Btree::OpenOption::FetchFieldIndex::Key, 0), paramValue) == false || paramValue < 0)
		{
			throw Exception::BadArgument(moduleName, srcFile, __LINE__);
		}

		if (paramValue == 0)
		{
			if (EstablishedSearchParam_ && ObjectIDSearch_ == false)
			{
				throw Exception::BadArgument(moduleName, srcFile, __LINE__);
			}

			ObjectIDSearch_ = true;
		}
		else
		{
			if (EstablishedSearchParam_ && ObjectIDSearch_)
			{
				throw Exception::BadArgument(moduleName, srcFile, __LINE__);
			}
		}

		EstablishedSearchParam_ = true;
	}
}

//
//	FUNCTION private
//	Btree::OpenOptionAnalyzer::setFetchSearchParameter --
//		オープンオプションにFetch検索パラメータを設定する
//
//	NOTES
//	オープンオプションにFetch検索パラメータを設定する。
//
//	ARGUMENTS
//	const Common::IntegerArrayData&	FetchFieldIndexArray_
//		Fetch対象フィールドインデックス配列への参照
//	LogicalFile::OpenOption&		OpenOption_
//		Ｂ＋木ファイルオープンオプションオブジェクトへの参照
//	const bool						FetchOnly_
//		Fetchのみかどうか
//			true  : Fetchのみ
//			false : Search + Fetch
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
// static
void
OpenOptionAnalyzer::setFetchSearchParameter(
	const Common::IntegerArrayData&	FetchFieldIndexArray_,
	LogicalFile::OpenOption&		OpenOption_,
	const bool						FetchOnly_)
{
	//
	// ※ Search + Fetchモードの場合は"OpenMode"パラメータは設定しない
	//

	if (FetchOnly_)
	{
		//
		// ※ Fetch モードのオープンモードは"Read"とする。
		//    同じ"Read"モードのScanモードとは
		//    オブジェクト取得サブモードパラメータで区別する
		//

		//
		// "OpenMode"パラメータ
		//

		OpenOption_.setInteger( _SYDNEY_OPEN_PARAMETER_KEY(FileCommon::OpenOption::OpenMode::Key), _SYDNEY_OPEN_PARAMETER_VALUE(FileCommon::OpenOption::OpenMode::Read));
	}

	//
	// "ReadSubMode"パラメータ
	//

	OpenOption_.setInteger( _SYDNEY_OPEN_PARAMETER_KEY(FileCommon::OpenOption::ReadSubMode::Key), _SYDNEY_OPEN_PARAMETER_VALUE(FileCommon::OpenOption::ReadSubMode::Fetch));

	//
	// "FetchFieldNumber"パラメータ
	//

	int	fieldNum = FetchFieldIndexArray_.getCount();

	OpenOption_.setInteger(_SYDNEY_BTREE_OPEN_PARAMETER_KEY(Btree::OpenOption::FetchFieldNumber::Key), fieldNum);

	for (int i = 0; i < fieldNum; ++i)
	{
		//
		// "FetchFieldIndex[%d]"パラメータ
		//
		int	fetchFieldIndex = FetchFieldIndexArray_.getElement(i);
		OpenOption_.setInteger(_SYDNEY_BTREE_OPEN_PARAMETER_FORMAT_KEY(Btree::OpenOption::FetchFieldIndex::Key, i), fetchFieldIndex);
	}
}

//
//	FUNCTION private
//	Btree::OpenOptionAnalyzer::setSingleSearchParameter --
//		単一条件によるSearchモードオープンパラメータを設定する
//
//	NOTES
//	単一条件によるSearchモードオープンパラメータを設定する。
//
//	ARGUMENTS
//	const LogicalFile::TreeNodeInterface*	Condition_
//		検索条件オブジェクトへのポインタ
//	LogicalFile::OpenOption&				OpenOption_
//		Ｂ＋木ファイルオープンオプションオブジェクトへの参照
//	const Btree::FileParameter&				FileParam_
//		Ｂ＋木ファイルパラメータオブジェクトへの参照
//
//	RETURN
//	bool
//		指定した検索条件でオブジェクトの高速検索が可能かどうか
//			true  : 指定した検索条件でオブジェクトの高速検索が可能
//			false : 指定した検索条件ではオブジェクトの高速検索が不可能
//
//	EXCEPTIONS
//	なし
//
// static
bool
OpenOptionAnalyzer::setSingleSearchParameter(
	const LogicalFile::TreeNodeInterface*	Condition_,
	LogicalFile::OpenOption&				OpenOption_,
	const FileParameter&					FileParam_)
{
	if (Condition_->getOperandSize() != 2)
	{
		//
		// 単一条件のオペランド数は 2 でなくてはいけない
		//

#ifdef DEBUG

		SydDebugMessage << "Illegal search condition." << ModEndl;

#endif

		return false;
	}

	//
	// Fieldノードのバリューである
	// フィールドインデックスをチェックする。
	//

	int	fieldNodeIndex = -1;

	bool	constantValueOperandTypeChecked = false;

	const LogicalFile::TreeNodeInterface*	columnNode =
		Condition_->getOperandAt(0);

	LogicalFile::TreeNodeInterface::Type	operandType =
		columnNode->getType();

	if (operandType == LogicalFile::TreeNodeInterface::Field)
	{
		fieldNodeIndex = 0;
	}
	else if (operandType == LogicalFile::TreeNodeInterface::ConstantValue ||
			 operandType == LogicalFile::TreeNodeInterface::Variable)
	{
		fieldNodeIndex = 1;

		constantValueOperandTypeChecked = true;

		columnNode = Condition_->getOperandAt(1);

		if (columnNode->getType() != LogicalFile::TreeNodeInterface::Field)
		{
			// どっちのオペランドもFieldノードではない…

#ifdef DEBUG

			SydDebugMessage << "Illegal search condition." << ModEndl;

#endif

			return false;
		}
	}

	if (fieldNodeIndex < 0)
	{
		// 認識できないノードがあったらしい…

#ifdef DEBUG

		SydDebugMessage << "Illegal search condition." << ModEndl;

#endif

		return false;
	}

	//
	// 単一条件は、先頭キーフィールド（フィールドインデックス = 1）
	// に対してのみ有効である。
	//
	int	fieldIndex = ModUnicodeCharTrait::toInt(columnNode->getValue());

	if (fieldIndex != 1)
	{
		// 先頭キーフィールド以外のフィールドが指定された…

#ifdef DEBUG

		if (fieldIndex == 0)
		{
			// オブジェクトIDフィールドが指定された…

			//
			// “オブジェクトIDでFetch”はできるけれども、
			// “オブジェクトIDでSearch”はできない。
			//

			SydDebugMessage << "Can't search by object ID." << ModEndl;
		}
		else
		{
			SydDebugMessage << "Illegal search field index." << ModEndl;
		}

#endif

		return false;
	}

	; _SYDNEY_ASSERT(OpenOptionAnalyzer::isBinaryDataField(FileParam_,
														   fieldIndex)
					 == false);

	//
	// File::getSearchParameter()よりも先に
	// File::getSortParameter()が呼ばれているのであれば、
	// ソートキーと検索対象フィールドの整合性のチェックを行なう。
	//

	bool	establishedSortKey = false;
	bool	sortKeyIsObjectID = false;

	OpenOptionAnalyzer::checkSortKey(OpenOption_,
									 establishedSortKey,
									 sortKeyIsObjectID);

	if (sortKeyIsObjectID)
	{
		// ソートキーがオブジェクトID…

		//
		// “オブジェクトIDでSearch”はできない。
		//

#ifdef DEBUG

		SydDebugMessage << "Can't search by object ID." << ModEndl;
#endif

		return false;
	}

	int	constantValueIndex = (fieldNodeIndex == 0) ? 1 : 0;

	const LogicalFile::TreeNodeInterface*	constantValueOperand =
		Condition_->getOperandAt(constantValueIndex);

	operandType = constantValueOperand->getType();

	if (constantValueOperandTypeChecked == false &&
		(operandType != LogicalFile::TreeNodeInterface::ConstantValue &&
		 operandType != LogicalFile::TreeNodeInterface::Variable))
	{

#ifdef DEBUG

		SydDebugMessage << "Illegal node type." << ModEndl;

#endif

		return false;
	}

	ModUnicodeString	searchValueString =
		constantValueOperand->getValue();

	LogicalFile::TreeNodeInterface::Type	searchOpe =
		Condition_->getType();

	int searchOpeValue = -1;

	if (searchOpe == LogicalFile::TreeNodeInterface::Equals)
	{
		searchOpeValue = OpenOption::Ope::Equals;
	}
	else if (searchOpe == LogicalFile::TreeNodeInterface::GreaterThan)
	{
		searchOpeValue = OpenOption::Ope::GreaterThan;
	}
	else if (searchOpe == LogicalFile::TreeNodeInterface::GreaterThanEquals)
	{
		searchOpeValue = OpenOption::Ope::GreaterThanEquals;
	}
	else if (searchOpe == LogicalFile::TreeNodeInterface::LessThan)
	{
		searchOpeValue = OpenOption::Ope::LessThan;
	}
	else if (searchOpe == LogicalFile::TreeNodeInterface::LessThanEquals)
	{
		searchOpeValue = OpenOption::Ope::LessThanEquals;
	}
	else
	{
		//
		// 引数Condition_のノードタイプのチェックをしてから
		// 当関数が呼ばれたわけだから
		// ここにはこないはず。
		//
		; _SYDNEY_ASSERT(false);
		throw Exception::Unexpected(moduleName, srcFile, __LINE__);
	}

	//
	// パラメータ設定
	//

	//
	// "OpenMode"パラメータ
	//
	OpenOption_.setInteger( _SYDNEY_OPEN_PARAMETER_KEY(FileCommon::OpenOption::OpenMode::Key), _SYDNEY_OPEN_PARAMETER_VALUE(FileCommon::OpenOption::OpenMode::Search));

	//
	// "SearchFieldNumber"パラメータ
	//
	OpenOption_.setInteger(_SYDNEY_BTREE_OPEN_PARAMETER_KEY(Btree::OpenOption::SearchFieldNumber::Key), 1);

	//
	// "SearchFieldIndex[0]"パラメータ
	//
	OpenOption_.setInteger(_SYDNEY_BTREE_OPEN_PARAMETER_FORMAT_KEY(Btree::OpenOption::SearchFieldIndex::Key, 0), fieldIndex);

	//
	// "SearchStart[0]"パラメータ
	//
	OpenOption_.setString(_SYDNEY_BTREE_OPEN_PARAMETER_FORMAT_KEY(Btree::OpenOption::SearchStart::Key, 0), searchValueString);

	//
	// "SearchStartOpe[0]"パラメータ
	//
	OpenOption_.setInteger(_SYDNEY_BTREE_OPEN_PARAMETER_FORMAT_KEY(Btree::OpenOption::Ope::StartKey, 0), _SYDNEY_OPEN_PARAMETER_VALUE(searchOpeValue));

	return true;
}

//
//	FUNCTION private
//	Btree::OpenOptionAnalyzer::setEqualsToNullSearchParameter --
//		「キー ＝ ヌル値」の Search モードオープンパラメータを設定する
//
//	NOTES
//	「キー ＝ ヌル値」の Search モードオープンパラメータを設定する。
//
//	ARGUMENTS
//	const LogicalFile::TreeNodeInterface*	Condition_
//		検索条件オブジェクトへのポインタ
//	LogicalFile::OpenOption&				OpenOption_
//		Ｂ＋木ファイルオープンオプションオブジェクトへの参照
//	const Btree::FileParameter&				FileParam_
//		Ｂ＋木ファイルパラメータオブジェクトへの参照
//
//	RETURN
//	bool
//		呼出側が指定した検索条件で高速検索が可能ならば true を、
//		不可能ならば false を返す。
//
//	EXCEPTIONS
//	なし
//
// static
bool
OpenOptionAnalyzer::setEqualsToNullSearchParameter(
	const LogicalFile::TreeNodeInterface*	Condition_,
	LogicalFile::OpenOption&				OpenOption_,
	const FileParameter&					FileParam_)
{
	//
	// EqualsToNullの単一条件のオペランド数は、
	// 1つでなくてはいけない。
	//

	if (Condition_->getOperandSize() != 1)
	{

#ifdef DEBUG

		SydDebugMessage << "Illegal search condition." << ModEndl;

#endif

		return false;
	}

	//
	// しかも、そのオペランドはFieldノードでなくてはいけない。
	//

	const LogicalFile::TreeNodeInterface*	columnNode = Condition_->getOperandAt(0);

	if (columnNode->getType() != LogicalFile::TreeNodeInterface::Field)
	{

#ifdef DEBUG

		SydDebugMessage << "Illegal field node type." << ModEndl;

#endif

		return false;
	}

	//
	// EqualsToNullの単一条件は
	//   ・先頭キーフィールド（フィールドインデックス = 1 ）
	// に対してのみ有効である。
	//
	int	fieldIndex = ModUnicodeCharTrait::toInt(columnNode->getValue());

	if (fieldIndex != 1)
	{

#ifdef DEBUG

		SydDebugMessage << "Illegal key field index." << ModEndl;

#endif

		return false;
	}

	; _SYDNEY_ASSERT( OpenOptionAnalyzer::isBinaryDataField(FileParam_, fieldIndex) == false );

	//
	// File::getSearchParameter()よりも先に
	// File::getSortParameter()が呼ばれているのであれば、
	// ソートキーと検索対象フィールドの整合性のチェックを行なう。
	//

	bool	establishedSortKey = false;
	bool	sortKeyIsObjectID = false;
	OpenOptionAnalyzer::checkSortKey(OpenOption_, establishedSortKey, sortKeyIsObjectID);

	if (sortKeyIsObjectID)
	{
		//
		// “オブジェクトIDでSearch”はできない。
		// しかも、オブジェクトIDがヌル値などということはありえない。
		//

#ifdef DEBUG

		SydDebugMessage << "Can't search by object ID." << ModEndl;

#endif

		return false;
	}

	//
	// パラメータ設定
	//

	//
	// "OpenMode"パラメータ
	//
	OpenOption_.setInteger(_SYDNEY_OPEN_PARAMETER_KEY(FileCommon::OpenOption::OpenMode::Key), FileCommon::OpenOption::OpenMode::Search);

	//
	// "SearchFieldNumber"パラメータ
	//
	OpenOption_.setInteger(_SYDNEY_BTREE_OPEN_PARAMETER_KEY(Btree::OpenOption::SearchFieldNumber::Key), 1);

	//
	// "SearchFieldIndex[0]"パラメータ
	//
	OpenOption_.setInteger(_SYDNEY_BTREE_OPEN_PARAMETER_FORMAT_KEY(Btree::OpenOption::SearchFieldIndex::Key, 0), 1);

	//
	// "SearchStart[0]"パラメータ
	//
	OpenOption_.setString(_SYDNEY_BTREE_OPEN_PARAMETER_FORMAT_KEY(Btree::OpenOption::SearchStart::Key, 0), "0");

	//
	// "SearchStartOpe[0]"パラメータ
	//
	OpenOption_.setInteger(_SYDNEY_BTREE_OPEN_PARAMETER_FORMAT_KEY(Btree::OpenOption::Ope::StartKey, 0), OpenOption::Ope::EqualsToNull);

	return true;
}

//
//	FUNCTION private
//	Btree::OpenOptionAnalyzer::setMultiSearchParameter --
//		複合検索条件によるSearchモードオープンパラメータを設定する
//
//	NOTES
//	複合検索条件によるSearchモードオープンパラメータを設定する。
//
//	ARGUMENTS
//	const LogicalFile::TreeNodeInterface*	Condition_
//		検索条件オブジェクトへのポインタ
//	LogicalFile::OpenOption&				OpenOption_
//		Ｂ＋木ファイルオープンオプションオブジェクトへの参照
//	const Btree::FileParameter&				FileParam_
//		Ｂ＋木ファイルパラメータオブジェクトへの参照
//	Btree::MultiSearchConditionItem*		Conditions_
//		複合検索条件解析データ配列へのポインタ
//	const int								TargetFieldNum_
//		検索対象フィールド数
//
//	RETURN
//	bool
//		指定した検索条件でオブジェクトの高速検索が可能かどうか
//			true  : 指定した検索条件でオブジェクトの高速検索が可能
//			        ※検索結果が‘φ’になる場合も含める
//			false : 指定した検索条件ではオブジェクトの高速検索が不可能
//
//	EXCEPTIONS
//	なし
//
// static
bool
OpenOptionAnalyzer::setMultiSearchParameter(
	const LogicalFile::TreeNodeInterface*	Condition_,
	LogicalFile::OpenOption&				OpenOption_,
	const FileParameter&					FileParam_,
	MultiSearchConditionItem*				Conditions_,
	int										TargetFieldNum_)
{
	// 複合検索条件を組み立てる
	OpenOptionAnalyzer::SearchResult ret;
	ret = OpenOptionAnalyzer::setMultiSearchParameter_Multi(Condition_,
															FileParam_,
															Conditions_,
															TargetFieldNum_);
	if ( ret == ILLEGAL ) return false; // 検索不可能

	// 複合検索条件をチェックする
	if ( ret == REGULAR ) {
		ret = OpenOptionAnalyzer::checkMultiSearchCondition(Conditions_,
															TargetFieldNum_,
															OpenOption_,
															FileParam_);
		if ( ret == ILLEGAL ) return false; // 検索不可能
	}

	//
	// ここまでで、キーフィールドに対する条件（Conditions_）が整った。
	// OpenOption_ に値を設定する。
	//
	if ( ret == REGULAR ) {
		setMultiSearchParameter_setOption(OpenOption_, FileParam_, Conditions_, TargetFieldNum_);
	} else {
		setMultiSearchParameter_setOptionNULL(OpenOption_);
	}

	return true; // 検索可能（‘φ’も含める）
}

//
//	FUNCTION private
//	Btree::OpenOptionAnalyzer::setMultiSearchParameter_Multi --
//		setMultiSearchParameter の内部構成メソッド
//
//	NOTES
//		複合検索条件によるSearchモードオープンパラメータを検査し、
//		複合検索条件解析データ配列を構築する。
//
//	ARGUMENTS
//	const LogicalFile::TreeNodeInterface*	Condition_
//		検索条件オブジェクトへのポインタ
//	LogicalFile::OpenOption&				OpenOption_
//		Ｂ＋木ファイルオープンオプションオブジェクトへの参照
//	const Btree::FileParameter&				FileParam_
//		Ｂ＋木ファイルパラメータオブジェクトへの参照
//	Btree::MultiSearchConditionItem*		Conditions_
//		複合検索条件解析データ配列へのポインタ
//	const int								TargetFieldNum_
//		検索対象フィールド数
//
//	RETURN
//	SearchResult
//		指定した検索条件でのオブジェクトの検索可能性判定
//			REGULAR: 指定した検索条件でオブジェクトの高速検索が可能
//			ILLEGAL: 指定した検索条件ではオブジェクトの高速検索が不可能
//			NULLOUT: 検索結果が 'φ’（空集合）になる
//
//	EXCEPTIONS
//	なし
//
// static
OpenOptionAnalyzer::SearchResult
OpenOptionAnalyzer::setMultiSearchParameter_Multi(
	const LogicalFile::TreeNodeInterface*	Condition_,
	const FileParameter&					FileParam_,
	MultiSearchConditionItem*				Conditions_,
	int										TargetFieldNum_)
{
	OpenOptionAnalyzer::SearchResult ret = REGULAR;
	int	operandSize = Condition_->getOperandSize();

	//
	// TreeNodeInterface の内容の検査およびキーフィールドへの検索条件配列への変換
	//
	for (int i = 0; i < operandSize; ++i)
	{
		const LogicalFile::TreeNodeInterface*	operand =
			Condition_->getOperandAt(i);

		LogicalFile::TreeNodeInterface::Type	operandType =
			operand->getType();

		switch (operandType)
		{
		default:// 異常値
#ifdef DEBUG
			SydDebugMessage << "Unknown node type." << ModEndl;
#endif
			return ILLEGAL;
		case LogicalFile::TreeNodeInterface::And:
		case LogicalFile::TreeNodeInterface::List:
			// 複合条件
			ret = OpenOptionAnalyzer::setMultiSearchParameter_Multi(	// 再帰評価する。
																	operand, //枝ノード
																	FileParam_,
																	Conditions_,//[OUT] キーフィールドへの検索条件配列
																	TargetFieldNum_);
			if (ret != REGULAR)// 有効な検索でない
			{
				return ret;// 再帰を中止
			}
			break;
		case LogicalFile::TreeNodeInterface::Equals:
		case LogicalFile::TreeNodeInterface::GreaterThan:
		case LogicalFile::TreeNodeInterface::GreaterThanEquals:
		case LogicalFile::TreeNodeInterface::LessThan:
		case LogicalFile::TreeNodeInterface::LessThanEquals:
		case LogicalFile::TreeNodeInterface::EqualsToNull:
			// 単一条件
			ret = OpenOptionAnalyzer::setMultiSearchParameter_Single(operand, //枝ノード
																	 FileParam_,
																	 Conditions_,//[OUT] キーフィールドへの検索条件配列
																	 TargetFieldNum_);
			if (ret != REGULAR)// 有効な検索でない
			{
				return ret;//再帰を中止
			}
			break;
		}
	}

	return ret;// デフォルトは REGULAR
}


//
//	FUNCTION private
//	Btree::OpenOptionAnalyzer::setMultiSearchParameter_Single --
//		setMultiSearchParameter の内部構成メソッド
//
//	NOTES
//		単純検索条件によるSearchモードオープンパラメータを検査し、
//		複合検索条件解析データ配列を構築する。
//
//	ARGUMENTS
//	const LogicalFile::TreeNodeInterface*	Condition_
//		検索条件オブジェクトへのポインタ
//	LogicalFile::OpenOption&				OpenOption_
//		Ｂ＋木ファイルオープンオプションオブジェクトへの参照
//	const Btree::FileParameter&				FileParam_
//		Ｂ＋木ファイルパラメータオブジェクトへの参照
//	Btree::MultiSearchConditionItem*		Conditions_
//		複合検索条件解析データ配列へのポインタ
//	const int								TargetFieldNum_
//		検索対象フィールド数
//
//	RETURN
//	SearchResult
//		指定した検索条件でのオブジェクトの検索可能性判定
//			REGULAR: 指定した検索条件でオブジェクトの高速検索が可能
//			ILLEGAL: 指定した検索条件ではオブジェクトの高速検索が不可能
//			NULLOUT: 検索結果が 'φ’（空集合）になる
//
//	EXCEPTIONS
//	なし
//
// static
OpenOptionAnalyzer::SearchResult
OpenOptionAnalyzer::setMultiSearchParameter_Single(
			const LogicalFile::TreeNodeInterface*	Condition_
			,const FileParameter&					FileParam_
			,MultiSearchConditionItem*				Conditions_		//[OUT] キーフィールドへの検索条件配列
			,int									TargetFieldNum_)
{
	OpenOptionAnalyzer::SearchResult ret = REGULAR;

	// 検索対象となるフィールドインデックスを取得する
	int	searchFieldIndex =
		ModUnicodeCharTrait::toInt(
			OpenOptionAnalyzer::getOperandValue(
				Condition_,
				LogicalFile::TreeNodeInterface::Field));

	if (searchFieldIndex < 1 || TargetFieldNum_ <= searchFieldIndex)
	{
		// 範囲外の検索対象
#ifdef DEBUG
		if (searchFieldIndex == 0)	// 特に Object ID のとき
		{
			SydDebugMessage << "Can't search by object ID." << ModEndl;
		}
		else
		{
			SydDebugMessage << "Illegal search field index." << ModEndl;
		}
#endif
		return ILLEGAL;
	}

	; _SYDNEY_ASSERT(OpenOptionAnalyzer::isBinaryDataField(FileParam_, searchFieldIndex) == false);

	// 検索条件（フィールドデータと比較するデータ）を取得する
	LogicalFile::TreeNodeInterface::Type	operandType = Condition_->getType();

	ModUnicodeString	searchData =
		OpenOptionAnalyzer::getOperandValue(Condition_, LogicalFile::TreeNodeInterface::ConstantValue);

	switch (operandType) {
	case LogicalFile::TreeNodeInterface::LessThan:
	case LogicalFile::TreeNodeInterface::LessThanEquals:
		// 終了データを設定する
		ret = OpenOptionAnalyzer::checkAndSetNewCondition( operandType ,searchData
		                                        ,(Conditions_ + searchFieldIndex)->m_iStopOpe ,(Conditions_ + searchFieldIndex)->m_cstrStop );
#ifdef DEBUG
		if ( ret == ILLEGAL )
		{
			// e.g.) f1 < 100 && f1 <= 200 (終了データが重複)
//					もし可能ならば、内容も比較したいが、割愛
			SydDebugMessage << "Illegal search condition." << ModEndl;
		}
#endif
		break;
	case LogicalFile::TreeNodeInterface::GreaterThan:
	case LogicalFile::TreeNodeInterface::GreaterThanEquals:
	case LogicalFile::TreeNodeInterface::EqualsToNull:
	case LogicalFile::TreeNodeInterface::Equals:
	default:
		// 開始データを設定する
		ret = OpenOptionAnalyzer::checkAndSetNewCondition( operandType ,searchData
			                                    ,(Conditions_ + searchFieldIndex)->m_iStartOpe ,(Conditions_ + searchFieldIndex)->m_cstrStart );
#ifdef DEBUG
		if ( ret == ILLEGAL )
		{
			// e.g.) f1 > 100 && f1 >= 200 (開始データが重複)
			// もし可能ならば、内容も比較したいが、割愛
			SydDebugMessage << "Illegal search condition." << ModEndl;
		}
#endif
		break;
	}
	return ret;
}

//
//	FUNCTION private
//	Btree::OpenOptionAnalyzer::checkAndSetNewCondition --
//		TreeNode の条件を検査し、設定する。
//
//	NOTES
//		TreeNode の条件が既に登録済みかどうかを検査し、
//		新規登録（あるいは、同一条件で登録済み）のときは設定する。
//		異なる条件で登録済みの場合は設定しない。
//
//	ARGUMENTS
//	const LogicalFile::TreeNodeInterface::Type& operandType_
//		[in] TreeNode のオペランドタイプ
//	const ModUnicodeString& searchData_
//		[in] TreeNode の条件文字列
//	int iOperator_
//		[out] Conditions のオペレータコード
//	ModUnicodeString& cString_
//		[out] Conditions の条件文字列
//
//	RETURN
//	SearchResult
//		指定した検索条件でのオブジェクトの検索可能性判定
//			REGULAR: 与えられた TreeNode は受け付けられた。
//			ILLEGAL: 重複した条件が存在しているので拒否された。
//			NULLOUT: 矛盾した条件（検索結果が 'φ’（空集合）になる）
//
//	EXCEPTIONS
//	なし
//
//static
OpenOptionAnalyzer::SearchResult
OpenOptionAnalyzer::checkAndSetNewCondition(
		const LogicalFile::TreeNodeInterface::Type& operandType_
		,const ModUnicodeString& searchData_
		,int& iOperator_
		,ModUnicodeString& cString_)
{
	OpenOptionAnalyzer::SearchResult ret = REGULAR;

	// 同一インデックスに検索条件が設定されていないかチェックする
	if (cString_.getLength() == 0)
	{	// 新規条件
		cString_ = searchData_;
		iOperator_ = OpenOptionAnalyzer::getOperator(operandType_);
		//ret = REGULAR;
	}
	else if ( cString_ == searchData_ )
	{
		ret = OpenOptionAnalyzer::isContradict(operandType_, iOperator_);
	}
	else
	{	// cString_ != searchData_
		if ((iOperator_ == OpenOption::Ope::Equals) || (operandType_ == LogicalFile::TreeNodeInterface::Equals)) {
			// オペランドが等しくない限り、Hit しない
			ret = NULLOUT;// 検索結果は‘φ'
		} else {
			// 重複した異なる条件（‘φ'かは不明）
			ret = ILLEGAL;
		}
	}

#ifdef DEBUG
	if ( ret == NULLOUT ) {
		SydDebugMessage << "Void: Illegal search condition." << ModEndl;
	}
#endif
	return ret;
}

//
//	FUNCTION private
//	Btree::OpenOptionAnalyzer::isContradict --
//		2つのオペレータが矛盾するか検査
//
//	NOTES
//		同じオペランドが与えられたと仮定して、
//		2つのオペレータが同時に条件指定されたとき
//		矛盾するかどうか検査する。
//
//	ARGUMENTS
//	const LogicalFile::TreeNodeInterface::Type& operandType_
//		[in] TreeNode のオペランドタイプ
//	int iOperator_
//		[out] Conditions のオペレータコード
//
//	RETURN
//	SearchResult
//			REGULAR: 与えられた オペレータ は受け付けられた。
//			ILLEGAL: 重複した条件が存在しているので拒否された。
//			NULLOUT: 矛盾した条件（検索結果が 'φ’（空集合）になる）
//
//	EXCEPTIONS
//	なし
//
//static
OpenOptionAnalyzer::SearchResult
OpenOptionAnalyzer::isContradict(const LogicalFile::TreeNodeInterface::Type& OperandType_, int& iOperator_)
{
#ifdef DEBUG
	_SYDNEY_ASSERT(OpenOption::Ope::Equals == 0);
	_SYDNEY_ASSERT(OpenOption::Ope::GreaterThan == 1);
	_SYDNEY_ASSERT(OpenOption::Ope::GreaterThanEquals == 2);
	_SYDNEY_ASSERT(OpenOption::Ope::LessThan == 3);
	_SYDNEY_ASSERT(OpenOption::Ope::LessThanEquals == 4);
	_SYDNEY_ASSERT(OpenOption::Ope::EqualsToNull == 5);
#endif

	int i;
	OpenOptionAnalyzer::SearchResult ret = REGULAR;
	switch (OperandType_) {
	default:	return ILLEGAL;
	case LogicalFile::TreeNodeInterface::Equals:			i = 0; break;
	case LogicalFile::TreeNodeInterface::GreaterThan:		i = 1; break;
	case LogicalFile::TreeNodeInterface::GreaterThanEquals:	i = 2; break;
	case LogicalFile::TreeNodeInterface::LessThan:			i = 3; break;
	case LogicalFile::TreeNodeInterface::LessThanEquals:	i = 4; break;
	case LogicalFile::TreeNodeInterface::EqualsToNull:		i = 5; break;
	}

	static const enum DecisionType {
		 ILL //Illegal		評価不能
		,NLL //NullOut		‘φ'（空集合）
		,EQU //Equal		Equal 評価に書き換え可能
		,OVR //Overwrite	OperandType_に置き換え可能
		,KEP //Keep			現状のまま
	} DT[][6] = {//decision table
	// Ope::Eq ,GT  ,GE  ,LT  ,LE  ,EN
		 { KEP ,NLL ,OVR ,NLL ,OVR ,ILL } //TreeNodeInterface::Equals
		,{ NLL ,KEP ,OVR ,NLL ,NLL ,ILL } //TreeNodeInterface::GreaterThan
		,{ EQU ,KEP ,KEP ,NLL ,EQU ,ILL } //TreeNodeInterface::GreaterThanEquals
		,{ NLL ,NLL ,NLL ,KEP ,OVR ,ILL } //TreeNodeInterface::LessThan
		,{ EQU ,NLL ,EQU ,KEP ,KEP ,ILL } //TreeNodeInterface::LessThanEquals
		,{ ILL ,ILL ,ILL ,ILL ,ILL ,KEP } //TreeNodeInterface::EqualsToNull
	};

	switch ( DT[i][iOperator_] ) {
	default:
	case ILL:
		return ILLEGAL;
	case NLL:
		return NULLOUT;
	case EQU:
		iOperator_ = OpenOption::Ope::Equals;
		return REGULAR;
	case OVR:
		iOperator_ = OpenOptionAnalyzer::getOperator(OperandType_);
		//return REGULAR;
	case KEP:
		return REGULAR;
	}
}


//
//	FUNCTION private
//	Btree::OpenOptionAnalyzer::setMultiSearchParameter_setOption --
//		setMultiSearchParameter の内部構成メソッド
//
//	NOTES
//		複合検索条件によるSearchモードオープンパラメータを設定する。
//		Conditions_ から OpenOption の設定
//
//	ARGUMENTS
//	LogicalFile::OpenOption&				OpenOption_
//		Ｂ＋木ファイルオープンオプションオブジェクトへの参照
//	const Btree::FileParameter&				FileParam_
//		Ｂ＋木ファイルパラメータオブジェクトへの参照
//	Btree::MultiSearchConditionItem*		Conditions_
//		複合検索条件解析データ配列へのポインタ
//	const int								TargetFieldNum_
//		検索対象フィールド数
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
//static
void
OpenOptionAnalyzer::setMultiSearchParameter_setOption(
		LogicalFile::OpenOption&		OpenOption_,
		const FileParameter&			FileParam_,
		MultiSearchConditionItem*		Conditions_,
		int								TargetFieldNum_)
{
	// 条件付のすべてのキーフィールドについて、その条件を OpenOption に設定

	//
	// パラメータ設定
	//

	// "OpenMode" パラメータ
	OpenOption_.setInteger(_SYDNEY_OPEN_PARAMETER_KEY(FileCommon::OpenOption::OpenMode::Key), FileCommon::OpenOption::OpenMode::Search);

	// 条件付のすべてのキーフィールドについて、その条件を OpenOption に設定
	int	arrayIndex = 0;
	for (int targetFieldIndex = 0; targetFieldIndex < TargetFieldNum_; ++targetFieldIndex)
	{
		bool succeeded = false;

		if ((Conditions_ + targetFieldIndex)->m_iStartOpe >= 0)
		{
			// "SearchFieldIndex[arrayIndex]" パラメータ
			OpenOption_.setInteger(_SYDNEY_BTREE_OPEN_PARAMETER_FORMAT_KEY(Btree::OpenOption::SearchFieldIndex::Key, arrayIndex), targetFieldIndex);

			//
			// "SearchStart[arrayIndex]" パラメータ
			//

			//
			// ※ "EqualsToNull" の場合には、検索データが存在しないので
			//    チェックが必要
			//
			if ((Conditions_ + targetFieldIndex)->m_iStartOpe == OpenOption::Ope::EqualsToNull)
			{
				OpenOption_.setString( _SYDNEY_BTREE_OPEN_PARAMETER_FORMAT_KEY(Btree::OpenOption::SearchStart::Key, arrayIndex), "0");
			}
			else
			{
				OpenOption_.setString( _SYDNEY_BTREE_OPEN_PARAMETER_FORMAT_KEY(Btree::OpenOption::SearchStart::Key, arrayIndex), (Conditions_ + targetFieldIndex)->m_cstrStart);
			}

			//
			// "SearchStartOpe[arrayIndex]" パラメータ
			//
			OpenOption_.setInteger( _SYDNEY_BTREE_OPEN_PARAMETER_FORMAT_KEY(Btree::OpenOption::Ope::StartKey, arrayIndex), (Conditions_ + targetFieldIndex)->m_iStartOpe);

			succeeded = true;
		}

		if ((Conditions_ + targetFieldIndex)->m_iStopOpe >= 0)
		{
			//
			// "SearchStop[arrayIndex]" パラメータ
			//
			OpenOption_.setString( _SYDNEY_BTREE_OPEN_PARAMETER_FORMAT_KEY(Btree::OpenOption::SearchStop::Key, arrayIndex), (Conditions_ + targetFieldIndex)->m_cstrStop);

			//
			// "SearchStopOpe[arrayIndex]" パラメータ
			//
			OpenOption_.setInteger( _SYDNEY_BTREE_OPEN_PARAMETER_FORMAT_KEY(Btree::OpenOption::Ope::StopKey, arrayIndex), (Conditions_ + targetFieldIndex)->m_iStopOpe);

			succeeded = true;
		}

		if (succeeded) arrayIndex++;// 設定したので index を進める

	} // end for targetFieldIndex

	// "SearchFieldNumber" パラメータ
	OpenOption_.setInteger(_SYDNEY_BTREE_OPEN_PARAMETER_KEY(Btree::OpenOption::SearchFieldNumber::Key), arrayIndex);
}


//
//	FUNCTION private
//	Btree::OpenOptionAnalyzer::setMultiSearchParameter_setOptionNULL --
//		setMultiSearchParameter の内部構成メソッド
//
//	NOTES
//		複合検索条件によるSearchモードオープンパラメータを設定する。
//		ただし、検索結果は‘φ'（空集合）になること
//
//	ARGUMENTS
//	LogicalFile::OpenOption&				OpenOption_
//		Ｂ＋木ファイルオープンオプションオブジェクトへの参照
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
//static
void
OpenOptionAnalyzer::setMultiSearchParameter_setOptionNULL(LogicalFile::OpenOption& OpenOption_)
{
	// 条件付のすべてのキーフィールドについて、その条件を OpenOption に設定

	//
	// パラメータ設定
	//

	// "OpenMode" パラメータ
	OpenOption_.setInteger(_SYDNEY_OPEN_PARAMETER_KEY(FileCommon::OpenOption::OpenMode::Key), FileCommon::OpenOption::OpenMode::Search);

	// "SearchFieldNumber" パラメータが０
	OpenOption_.setInteger(_SYDNEY_BTREE_OPEN_PARAMETER_KEY(Btree::OpenOption::SearchFieldNumber::Key), 0);
}


//
//	FUNCTION private
//	Btree::OpenOptionAnalyzer::getOperandValue --
//		オペランド値の文字列表現を返す
//
//	NOTES
//	引数 pCondition からたどれる全てのオペランドから、
//	指定オペランド型のオペランドを検索し、
//	そのオペランドがもつ値の文字列表現を返す。
//
//	ARGUMENTS
//	const LogicalFile::TreeNodeInterface*		Condition_
//		検索条件オブジェクトへのポインタ
//	const LogicalFile::TreeNodeInterface::Type	OperandType_
//		[YET!]
//
//	RETURN
//	ModUnicodeString
//		オペランド値の文字列表現
//
//	EXCEPTIONS
//	なし
//
// static
ModUnicodeString
OpenOptionAnalyzer::getOperandValue(
	const LogicalFile::TreeNodeInterface*		Condition_,
	const LogicalFile::TreeNodeInterface::Type	OperandType_)
{
	int	operandSize = Condition_->getOperandSize();

	for (int i = 0; i < operandSize; ++i)
	{
		const LogicalFile::TreeNodeInterface*	operand = Condition_->getOperandAt(i);

		//
		// ※ LogicalFile::TreeNodeInterface::ConstantValue
		// 　 で指定されている場合は、
		//    LogicalFile::TreeNodeInterface::Variableでも
		// 　 一致しているものとして返す。
		//

		LogicalFile::TreeNodeInterface::Type	operandType = operand->getType();

		if (OperandType_ == LogicalFile::TreeNodeInterface::ConstantValue)
		{
			if (operandType == LogicalFile::TreeNodeInterface::ConstantValue ||
				operandType == LogicalFile::TreeNodeInterface::Variable)
			{
				return operand->getValue();
			}
		}
		else if (operandType == OperandType_)
		{
				return operand->getValue();
		}

	} // end for i

	return _TRMEISTER_U_STRING("-1");
}

//
//	FUNCTION private
//	Btree::OpenOptionAnalyzer::getOperator --
//		比較演算子を示す数値を返す
//
//	NOTES
//	比較演算子を示す数値を返す。
//
//	ARGUMENTS
//	const LogicalFile::TreeNodeInterface::Type	OperandType_
//		比較演算子を示す列挙子
//		 LogicalFile::TreeNodeInterface::NotEquals はない
//
//	RETURN
//	ModString
//		比較演算子を示す数値
//
//	EXCEPTIONS
//	なし
//
// static
int
OpenOptionAnalyzer::getOperator(
	const LogicalFile::TreeNodeInterface::Type	OperandType_)
{
	switch (OperandType_) {
	default:
		; _SYDNEY_ASSERT(false);
		throw Exception::Unexpected(moduleName, srcFile, __LINE__);
	case LogicalFile::TreeNodeInterface::Equals:				return OpenOption::Ope::Equals;
	case LogicalFile::TreeNodeInterface::GreaterThan:			return OpenOption::Ope::GreaterThan;
	case LogicalFile::TreeNodeInterface::GreaterThanEquals:		return OpenOption::Ope::GreaterThanEquals;
	case LogicalFile::TreeNodeInterface::LessThan:				return OpenOption::Ope::LessThan;
	case LogicalFile::TreeNodeInterface::LessThanEquals:		return OpenOption::Ope::LessThanEquals;
	case LogicalFile::TreeNodeInterface::EqualsToNull:			return OpenOption::Ope::EqualsToNull;
	}
}

//
//	FUNCTION private
//	Btree::OpenOptionAnalyzer::checkMultiSearchCondition --
//		複合検索条件をチェックする
//
//	NOTES
//	複合検索条件をチェックする。
//
//	ARGUMENTS
//	Btree::MultiSearchConditionItem*	Conditions_
//		複合検索条件解析データ配列へのポインタ
//	const int							TargetFieldNum_
//		複合検索条件解析データの最大要素数
//	const LogicalFile::OpenOption&		OpenOption_
//		Ｂ＋木ファイルオープンオプションオブジェクトへの参照
//	const Btree::FileParameter&			FileParam_
//		Ｂ＋木ファイルパラメータオブジェクトへの参照
//
//	RETURN
//	SearchResult
//		指定した検索条件でのオブジェクトの検索可能性判定
//			REGULAR: 指定した検索条件でオブジェクトの高速検索が可能
//			ILLEGAL: 指定した検索条件ではオブジェクトの高速検索が不可能
//			NULLOUT: 検索結果が 'φ’（空集合）になる
//
//	EXCEPTIONS
//	なし
//
// static
OpenOptionAnalyzer::SearchResult
OpenOptionAnalyzer::checkMultiSearchCondition(
	MultiSearchConditionItem*		Conditions_,
	const int						TargetFieldNum_,
	const LogicalFile::OpenOption&	OpenOption_,
	const FileParameter&			FileParam_)
{
	//
	// File::getSearchParameter()よりも先に
	// File::getSortParameter()が呼ばれているのであれば、
	// ソートキーと検索対象フィールドの整合性のチェックを行なう。
	//

	bool	establishedSortKey = false;
	bool	sortKeyIsObjectID = false;

	OpenOptionAnalyzer::checkSortKey(OpenOption_,
									 establishedSortKey,
									 sortKeyIsObjectID);

	if (sortKeyIsObjectID)
	{
		//
		// “オブジェクトIDでSearch”はできない。
		//

#ifdef DEBUG

		SydDebugMessage << "Can't search by object ID." << ModEndl;

#endif

		return ILLEGAL;
	}

	//
	// キーフィールドに対する複合条件は、先頭キーフィールドから
	// 設定する必要がある。
	// m_cstrStartではなく、m_iStartOpeを参照しているのは、
	// "EqualsToNull"の場合には検索データが設定されていないからである。
	//

	if ((Conditions_ + 1/*先頭キーfield*/)->m_iStartOpe < 0 &&
		(Conditions_ + 1/*先頭キーfield*/)->m_iStopOpe < 0)
	{
		//
		// 先頭キーフィールドには必ず検索条件をつける
		//

#ifdef DEBUG

		SydDebugMessage << "Illegal search condition." << ModEndl;

#endif

		return ILLEGAL;
	}

	bool	exceptEQ = false;

	// (OID 以外の)すべてのキーfieldに対して
	for (int i = 1 ; i < TargetFieldNum_; ++i)
	{
		if ((Conditions_ + i)->m_iStartOpe < 0)
		{
			if ((Conditions_ + i)->m_iStopOpe < 0)
			{
				continue;//条件指定なしのfield（先頭キーfield以外の場合）
			}

			//
			// "LessThan", "LessThanEquals" だけの条件だと
			// m_cstrStop 側にしか検索条件が設定されていないので
			// m_cstrStart 側に移動
			//
			(Conditions_ + i)->m_cstrStart = (Conditions_ + i)->m_cstrStop;
			(Conditions_ + i)->m_iStartOpe = (Conditions_ + i)->m_iStopOpe;

			(Conditions_ + i)->m_cstrStop = "";
			(Conditions_ + i)->m_iStopOpe = -1;
		}

		//
		// 最終検索条件以外で"Equals"の条件が存在するのは無効（∵高速検索できない）
		//
		if (exceptEQ)
		{
			// Equals 以外の条件以後にも次の条件が存在した。
#ifdef DEBUG
			SydDebugMessage << "Can't search." << ModEndl;
#endif
			return ILLEGAL;
		}

		if ((Conditions_ + i)->m_iStartOpe != OpenOption::Ope::Equals)
		{
			exceptEQ = true;// Equals 以外の条件が見つかった→この条件は最終であるべき
		}

		if ((Conditions_ + i)->m_cstrStart.getLength() > 0 &&
			(Conditions_ + i)->m_cstrStop.getLength() > 0)
		{
			//
			// 範囲指定（開始データ/終了データあり）
			//

			// 範囲開始値 ＞ 範囲終了値のチェック
			if (OpenOptionAnalyzer::checkSpanSearchCondition(Conditions_,
															 i,
															 FileParam_)
				== false)
			{
				// 範囲指定に不正があった…（検索しても一致しない：無効である）
#ifdef DEBUG
				SydDebugMessage << "Void: Illegal search condition (span)." << ModEndl;
#endif
				return NULLOUT;// 検索結果は‘φ'
			}

			if ((Conditions_ + i)->m_cstrStart == (Conditions_ + i)->m_cstrStop)
			{
				// 範囲指定なのに、開始データも終了データも同じ…

				if ( ((Conditions_ + i)->m_iStartOpe == OpenOption::Ope::GreaterThanEquals || (Conditions_ + i)->m_iStartOpe == OpenOption::Ope::Equals)
				   && (Conditions_ + i)->m_iStopOpe  == OpenOption::Ope::LessThanEquals )
				{
					//
					// f  = 100 and f <= 100
					// f >= 100 and f <= 100
					// のような検索条件は
					// f = 100
					// に変換できる。
					//

					(Conditions_ + i)->m_cstrStop = "";
					(Conditions_ + i)->m_iStopOpe = -1;
					(Conditions_ + i)->m_iStartOpe = OpenOption::Ope::Equals;
				}
				else
				{
					//
					// f >  100 and f <  100
					// f >= 100 and f <  100
					// f >  100 and f <= 100
					// のような検索条件はあり得ない
					//（一致しない）ので無効
					//
#ifdef DEBUG
					SydDebugMessage << "Void: Illegal search condition." << ModEndl;
#endif
					return NULLOUT;// 検索結果は‘φ'
				}
			}
			else
			{
				// 範囲開始値 < 範囲終了値
				if ((Conditions_ + i)->m_iStartOpe != OpenOption::Ope::GreaterThan &&
					(Conditions_ + i)->m_iStartOpe != OpenOption::Ope::GreaterThanEquals)
				{
					//
					// f = 100 and f < 200 のような検索条件は
					// 範囲指定ではない（高速検索できない）ので無効
					//
#ifdef DEBUG
					SydDebugMessage << "Illegal search condition." << ModEndl;
#endif
					return ILLEGAL;
				}
			}
		}

	} // end for i

	// すべて条件が整った
	return REGULAR;
}

//
//	FUNCTION private
//	Btree::OpenOptionAnalyzer::checkSpanSearchCondition --
//		範囲指定開始／終了値をチェックする
//
//	NOTES
//	範囲指定開始／終了値をチェックする。
//
//	ARGUMENTS
//	Btree::MultiSearchConditionItem*	Conditions_
//		複合検索条件解析データ配列へのポインタ
//	const int							ConditionIndex_
//		検索対象フィールドインデックス
//	const Btree::FileParameter&			FileParam_
//		Ｂ＋木ファイルパラメータオブジェクトへの参照
//
//	RETURN
//	bool
//		範囲指定に不正がないかどうか
//			true  : 不正がない
//			false : 不正
//
//	EXCEPTIONS
//	なし
//
// static
bool
OpenOptionAnalyzer::checkSpanSearchCondition(
	MultiSearchConditionItem*	Conditions_,
	const int					ConditionIndex_,
	const FileParameter&		FileParam_)
{
	; _SYDNEY_ASSERT(ConditionIndex_ < FileParam_.m_FieldNum);

	Common::DataType::Type	fieldType =
		*(FileParam_.m_FieldTypeArray + ConditionIndex_);

	//
	// 検索条件で指定された範囲のチェックを行なう。
	//

	ModAutoPointer<Common::Data> startValue =
		FileCommon::DataManager::createCommonData(fieldType, (Conditions_ + ConditionIndex_)->m_cstrStart);

	ModAutoPointer<Common::Data> stopValue =
		FileCommon::DataManager::createCommonData( fieldType, (Conditions_ + ConditionIndex_)->m_cstrStop);

	bool	searchable = true;
	if (startValue->compareTo( stopValue.get() ) > 0)
	{
		searchable = false;
	}

	return searchable;
}

//
//	Copyright (c) 2000, 2001, 2002, 2004, 2006, 2009, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
