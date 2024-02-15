// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Hint.h -- Ｂ＋木ファイル用ヒントパラメータのヘッダファイル
// 
// Copyright (c) 2001, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_BTREE_HINT_H
#define __SYDNEY_BTREE_HINT_H

#include "Common/Common.h"

_SYDNEY_BEGIN

namespace Btree
{

namespace Hint
{

//
//	NAMESPACE
//	Btree::Hint::NodeKeyDivideRate::Key -- ノード内のキー分割率の名前空間
//
//	NOTES
//	ノード内のキー分割率の名前空間。
//	ノード内のキー分割率は0[％]〜100[％]で指定する。
//	オブジェクト挿入時にノードの分割が必要となった際、
//	分割後の2つのノードにそれぞれどれくらいずつキーを
//	分割するかを指定する。
//	このヒントでは『元のノードにどれくらいのキーを残すか』を指定する。
//
namespace NodeKeyDivideRate
{

//
//	CONST
//	Btree::Hint::NodeKeyDivideRate::Key -- ノード内のキー分割率のキー
//
//	NOTES
//	ノード内のキー分割率のキー。
//
const char* const	Key = "NodeKeyDivideRate";

//
//	CONST
//	Btree::Hint::NodeKeyDivideRate::Default --
//		ノード内のキー分割率のデフォルト値
//
//	NOTES
//	ノード内のキー分割率のデフォルト値。 [%]
//
const int	Default = 50;

} // end of namespace Btree::Hint::NodeKeyDivideRate

//
//	NAMESPACE
//	Btree::Hint::NodeMergeCheckRate --
//		ノードマージチェックの閾値の名前空間
//
//	NOTES
//	ノードマージチェックの閾値の名前空間。
//	閾値は、次数を100％としたときの率で指定する。
//
namespace NodeMergeCheckRate
{

//
//	CONST
//	Btree::Hint::NodeMergeCheckRate::Key -- ノードマージチェックの閾値のキー
//
//	NOTES
//	ノードマージチェックの閾値のキー。
//
//
const char* const	Key = "NodeMergeCheckRate";

//
//	CONST
//	Btree::Hint::NodeMergeCheckRate::Default --
//		ノードマージチェックの閾値のデフォルト値
//
//	NOTES
//	ノードマージチェックの閾値のデフォルト値。 [%]
//
const int	Default = 25;

} // end of namespace Btree::Hint::NodeMergeCheckRate

//
//	NAMESPACE
//	Btree::Hint::NodeMergeExecuteRate --
//		ノードマージ実行の閾値の名前空間
//
//	NOTES
//	ノードマージ実行の閾値の名前空間
//
namespace NodeMergeExecuteRate
{

//
//	CONST
//	Btree::Hint::NodeMergeExecuteRate::Key -- ノードマージ実行の閾値のキー
//
//	NOTES
//	ノードマージ実行の閾値のキー。
//
const char* const	Key = "NodeMergeExecuteRate";

//
//	CONST
//	Btree::Hint::NodeMergeExecuteRate::Default --
//		ノードマージ実行の閾値のデフォルト値
//
//	NOTES
//	ノードマージ実行の閾値のデフォルト値。 [%]
//
const int	Default = 100;

} // end of namespace Btree::Hint::NodeMergeExecuteRate

} // end of namespace Btree::Hint

} // end of namespace Btree

_SYDNEY_END

#endif // __SYDNEY_BTREE_HINT_H

//
//	Copyright (c) 2001, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
