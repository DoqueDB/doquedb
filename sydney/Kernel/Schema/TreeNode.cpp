// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// TreeNode.cpp --
// 
// Copyright (c) 2000, 2002, 2004, 2023 Ricoh Company, Ltd.
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

namespace {
const char srcFile[] = __FILE__;
const char moduleName[] = "Schema";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "Schema/TreeNode.h"

#include "Common/Data.h"

#include "ModUnicodeString.h"
#include "ModUnicodeOstrStream.h"

_SYDNEY_USING
_SYDNEY_SCHEMA_USING;

/////////////////////////////
// Schema::TreeNode::Field //
/////////////////////////////

//	FUNCTION public
//	Schema::TreeNode::Field::getValue --
//		フィールドを表すノードの内容を文字列で得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		フィールドのファイル上の位置を文字列にしたもの
//
//	EXCEPTIONS

ModUnicodeString
TreeNode::Field::
getValue() const
{
	ModUnicodeOstrStream cStream;
	cStream << m_iPosition;
	return cStream.getString();
}

/////////////////////////////
// Schema::TreeNode::Value //
/////////////////////////////

//	FUNCTION public
//	Schema::TreeNode::Value::getValue --
//		値を表すノードの内容を文字列で得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		値を文字列にしたもの
//
//	EXCEPTIONS

ModUnicodeString
TreeNode::Value::
getValue() const
{
	if (m_iID != Schema::Object::ID::Invalid
		&& m_cstrValue.getLength() == 0) {
		ModUnicodeOstrStream cStream;
		cStream << m_iID;
		m_cstrValue = cStream.getString();
	}
	return m_cstrValue;
}

////////////////////////////////
// Schema::TreeNode::Variable //
////////////////////////////////

//	FUNCTION public
//	Schema::TreeNode::Variable::getValue --
//		値を表すノードの内容を文字列で得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		値を文字列にしたもの
//
//	EXCEPTIONS

ModUnicodeString
TreeNode::Variable::
getValue() const
{
	return m_pData->getString();
}

// FUNCTION public
//	Schema::TreeNode::Variable::getData -- Common::Dataで値を得る
//
// NOTES
//
// ARGUMENTS
//	なし
//
// RETURN
//	const Common::Data*
//
// EXCEPTIONS

//virtual
const Common::Data*
TreeNode::Variable::
getData() const
{
	return m_pData.get();
}

//	FUNCTION public
//	Schema::TreeNode::Variable::setData --
//		値を表すノードを設定する
//
//	NOTES
//
//	ARGUMENTS
//		Common::Data::Pointer pData_
//			設定するデータ
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
TreeNode::Variable::
setData(Common::Data::Pointer pData_)
{
	m_pData = pData_;
}

#ifndef SYD_COVERAGE
////////////////////////////
// Schema::TreeNode::Pair //
////////////////////////////

//	FUNCTION public
//	Schema::TreeNode::Pair::toString --
//		値を表すノードの内容を文字列で得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		値を文字列にしたもの
//
//	EXCEPTIONS

ModUnicodeString
TreeNode::Pair::
toString() const
{
	ModUnicodeOstrStream cStream;
	cStream << "\nPair: [\n" << m_cNode1.toString() << "\n,\n" << m_cNode2.toString() << "\n] :riaP\n";
	return cStream.getString();
}

////////////////////////////
// Schema::TreeNode::List //
////////////////////////////

//	FUNCTION public
//	Schema::TreeNode::List::toString --
//		値を表すノードの内容を文字列で得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		値を文字列にしたもの
//
//	EXCEPTIONS

ModUnicodeString
TreeNode::List::
toString() const
{
	ModUnicodeOstrStream cStream;
	ModSize n = m_vecNodes.getSize();
	cStream << "\nList: [\n";
	for (ModSize i = 0; i < n; ++i) {
		if (i > 0) {
			cStream << ",\n";
		}
		if (m_vecNodes[i]) {
			cStream << m_vecNodes[i]->toString() << "\n";
		} else {
			cStream << "<null>\n";
		}
	}
	cStream << "] :tsiL\n";
	return cStream.getString();
}

////////////////////////////
// Schema::TreeNode::And //
////////////////////////////

//	FUNCTION public
//	Schema::TreeNode::And::toString --
//		値を表すノードの内容を文字列で得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		値を文字列にしたもの
//
//	EXCEPTIONS

ModUnicodeString
TreeNode::And::
toString() const
{
	ModUnicodeOstrStream cStream;
	ModSize n = m_vecNodes.getSize();
	cStream << "\nAnd: [\n";
	for (ModSize i = 0; i < n; ++i) {
		if (i > 0) {
			cStream << ",\n";
		}
		if (m_vecNodes[i]) {
			cStream << m_vecNodes[i]->toString() << "\n";
		} else {
			cStream << "???\n";
		}
	}
	cStream << "] :dnA\n";
	return cStream.getString();
}

////////////////////////////
// Schema::TreeNode::Fetch //
////////////////////////////

//	FUNCTION public
//	Schema::TreeNode::Fetch::toString --
//		値を表すノードの内容を文字列で得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		値を文字列にしたもの
//
//	EXCEPTIONS

ModUnicodeString
TreeNode::Fetch::
toString() const
{
	ModUnicodeOstrStream cStream;
	cStream << "\nFetch: [\n";
	cStream << m_cNode.toString();
	cStream << "\n] :hcteF\n";
	return cStream.getString();
}

////////////////////////////
// Schema::TreeNode::Equals //
////////////////////////////

//	FUNCTION public
//	Schema::TreeNode::Equals::toString --
//		値を表すノードの内容を文字列で得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		値を文字列にしたもの
//
//	EXCEPTIONS

ModUnicodeString
TreeNode::Equals::
toString() const
{
	ModUnicodeOstrStream cStream;
	cStream << "\nEquals: [";
	cStream << m_cOperand.toString();
	cStream << "] :slauqE\n";
	return cStream.getString();
}

////////////////////////////
// Schema::TreeNode::EqualsToNull //
////////////////////////////

//	FUNCTION public
//	Schema::TreeNode::EqualsToNull::toString --
//		値を表すノードの内容を文字列で得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		値を文字列にしたもの
//
//	EXCEPTIONS

ModUnicodeString
TreeNode::EqualsToNull::
toString() const
{
	ModUnicodeOstrStream cStream;
	cStream << "\nEqualsToNull: [";
	cStream << m_cOperand.toString();
	cStream << "] :lluNoTslauqE\n";
	return cStream.getString();
}

#endif // ifndef SYD_COVERAGE

//
//	Copyright (c) 2000, 2002, 2004, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
