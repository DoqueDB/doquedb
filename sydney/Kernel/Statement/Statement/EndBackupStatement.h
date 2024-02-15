// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// EndBackupStatement.h --
// 
// Copyright (c) 2000, 2002, 2009, 2012, 2013, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_STATEMENT_ENDBACKUPSTATEMENT_H
#define __SYDNEY_STATEMENT_ENDBACKUPSTATEMENT_H

#include "Common/Common.h"
#include "Statement/ObjectConnection.h"
#include "Statement/Identifier.h"
#include "Statement/LogicalLogOption.h"

_SYDNEY_BEGIN

namespace Statement
{

	class Identifier;

//
//	CLASS
//	Statement::EndBackupStatement --
//
//	NOTES
//
//
class SYD_STATEMENT_FUNCTION EndBackupStatement  : public Statement::ObjectConnection
{
private:
	// メンバのm_vecpElements内でのindex
	enum { f_LogicalLogOption, f__end_index };
public:
	//コンストラクタ(1)
	EndBackupStatement(int iLogicalLogOption_ = LogicalLogOption::None);
	//デストラクタ
	virtual ~EndBackupStatement();

	// LogicalLogOptiopn を得る
	int	getLogicalLogOption() const
		{ return ObjectConnection::getScaler(f_LogicalLogOption); }
	// Type を設定する
	void setLogicalLogOption(int iValue_)
		{ ObjectConnection::setScaler(f_LogicalLogOption ,iValue_); }

	//自身をコピーする
	Object* copy() const;

	// SQL文で値を得る
	ModUnicodeString toSQLStatement(bool bForCascade_ = false) const;
	
#if 0
	// Analyzerを得る
	virtual const Analysis::Analyzer* getAnalyzer() const;
#endif

protected:
private:
	// 代入オペレーターは使わない
	EndBackupStatement& operator=(const EndBackupStatement& cOther_);

};

}

_SYDNEY_END

#endif //__SYDNEY_STATEMENT_ENDBACKUPSTATEMENT_H

//
//	Copyright (c) 2000, 2002, 2009, 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
