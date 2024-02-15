// -*-Mode: Java; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// AnyElementPropertySubqueryExpression.java --
// 
// Copyright (c) 2007, 2010, 2023 Ricoh Company, Ltd.
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

package jp.co.ricoh.doquedb.hibernate.criterion;

import java.sql.Types;

import org.hibernate.Criteria;
import org.hibernate.criterion.CriteriaQuery;
import org.hibernate.criterion.DetachedCriteria;
import org.hibernate.criterion.SubqueryExpression;
import org.hibernate.engine.spi.SessionFactoryImplementor;
import org.hibernate.engine.spi.TypedValue;
import org.hibernate.type.Type;

public class AnyElementPropertySubqueryExpression extends SubqueryExpression
{
	/** プロパティ名 */
	private String propertyName;

	/** コンストラクタ */
	protected AnyElementPropertySubqueryExpression(String propertyName,
												   String op,
												   DetachedCriteria dc)
	{
		super(op, null, dc);
		this.propertyName = propertyName;
	}

	/** SQL文の左側を作成します */
	protected String toLeftSqlString(Criteria criteria,
									 CriteriaQuery criteriaQuery)
	{
		// プロパティーに対応するカラムを取り出す
		String column = criteriaQuery.getColumn(criteria, propertyName);
		// プロパティーのタイプを取り出す
		Type type
			= criteriaQuery.getTypeUsingProjection(criteria, propertyName);
		// SessionFactoryを得る
		SessionFactoryImplementor factory = criteriaQuery.getFactory();
		// SQL型を取り出す(この値はUserType.sqlTypes)
		int[] sqlTypes = type.sqlTypes(factory);

		if (sqlTypes[0] == Types.ARRAY)
			// 配列なので
			column += "[]";

		return column;
	}

}

//
// Copyright (c) 2007, 2010, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
