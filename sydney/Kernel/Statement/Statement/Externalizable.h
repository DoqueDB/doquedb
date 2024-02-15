// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Externalizable.h -- Serialize関連のクラス
// 
// Copyright (c) 2012, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_STATEMENT_EXTERNALIZABLE_H
#define __SYDNEY_STATEMENT_EXTERNALIZABLE_H

#include "Statement/Module.h"
#include "Common/Externalizable.h"

_SYDNEY_BEGIN
_SYDNEY_STATEMENT_BEGIN

//
//	CLASS
//		Externalizable -- 
//
//	NOTES
//
class Externalizable
{
public:
	struct ClassID
	{
		enum Value
		{
			Base = Common::Externalizable::StatementClasses,
			ValueExpression = Base + 400,
			ContainsOperand = Base + 500
		};
	};

protected:
private:
	// never constructed
	Externalizable();
	~Externalizable();
};

//
//	FUNCTION global
//	Statement::getClassInstance -- クラスのインスタンスを確保する
//
//	NOTES
//	シリアル化可能クラスのクラスIDからそのクラスのインスタンスを確保する。
//
//	ARGUMENTS
//	int iClassID_
//		クラスID
//
//	RETURN
//	Common::Externalizable*
//		シリアル化可能クラスのインスタンス。
//		存在しないクラスIDの場合は0を返す。
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
Common::Externalizable* getClassInstance(int iClassID_);

_SYDNEY_STATEMENT_END
_SYDNEY_END

#endif // __SYDNEY_STATEMENT_EXTERNALIZABLE_H

//
// Copyright (c) 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
