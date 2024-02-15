// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// XA_RecoverStatement.cpp --
// 
// Copyright (c) 2006, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Statement";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyDynamicCast.h"

#include "Statement/XA_RecoverStatement.h"

#if 0
#include "Analysis/XA_RecoverStatement.h"
#endif

_SYDNEY_USING
_SYDNEY_STATEMENT_USING

namespace
{
	// メンバの m_vecpElements 内でのインデックス

	enum
	{
		f__end_index
	};
}

//	FUNCTION public
//	Statement::XA_RecoverStatement::XA_RecoverStatement --
//		コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS

XA_RecoverStatement::XA_RecoverStatement()
	: Object(ObjectType::XA_RecoverStatement, f__end_index)
{}

#if 0
namespace
{
	Analysis::XA_RecoverStatement _analyzer;
}

// Analyzerを得る
// virtual
const Analysis::Analyzer*
XA_RecoverStatement::getAnalyzer() const
{
	return &_analyzer;
}
#endif

//
//	Copyright (c) 2006, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
