// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// StartBackupStatement.h --
// 
// Copyright (c) 2000, 2002, 2012, 2013, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_STATEMENT_STARTBACKUPSTATEMENT_H
#define __SYDNEY_STATEMENT_STARTBACKUPSTATEMENT_H

#include "Statement/ObjectConnection.h"
#include "Statement/Identifier.h"

_SYDNEY_BEGIN

namespace Statement
{

	class Identifier;

//
//	CLASS
//	Statement::StartBackupStatement --
//
//	NOTES
//
//
class SYD_STATEMENT_FUNCTION StartBackupStatement  : public Statement::ObjectConnection
{
private:
	// メンバのm_vecpElements内でのindex
	enum { f_BackupType, f_VersionType, f__end_index };
public:
	//constructor
	StartBackupStatement()
		: ObjectConnection(ObjectType::StartBackupStatement)
	{}
	//コンストラクタ(2)
	StartBackupStatement(int iType_, int iVersion_);

	//デストラクタ
	virtual ~StartBackupStatement();

	// アクセサ
	enum BackupType {
		Unknown = 0,
		Full,
		Master,
		LogicalLog
	};

	// Type を得る
	int	getType() const
		{ return ObjectConnection::getScaler(f_BackupType); }
	// Type を設定する
	void setType(int iValue_)
		{ ObjectConnection::setScaler(f_BackupType ,iValue_); }

    // 版指定を表す識別子
    enum VersionType {
        UnknownVersion = 0,
		DiscardSnapshot
    };

    // 版指定を得る
    int getVersion() const
		{ return ObjectConnection::getScaler(f_VersionType); }
	bool isRecovery() const
		{ return ( getVersion() == DiscardSnapshot ); }
	
    // 版指定を設定する
    void setVersion(int iVersion_)
		{ ObjectConnection::setScaler(f_VersionType, iVersion_); }
    
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
	StartBackupStatement& operator=(const StartBackupStatement& cOther_);

};

}

_SYDNEY_END

#endif //__SYDNEY_STATEMENT_STARTBACKUPSTATEMENT_H

//
//	Copyright (c) 2000, 2002, 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
