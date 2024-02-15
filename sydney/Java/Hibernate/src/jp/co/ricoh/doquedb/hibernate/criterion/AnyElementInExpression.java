// -*-Mode: Java; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// AnyElementInExpression.java --
// 
// Copyright (c) 2007, 2010, 2011, 2023 Ricoh Company, Ltd.
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

import jp.co.ricoh.doquedb.hibernate.usertype.CollectionType;

import java.sql.Types;

import org.hibernate.Criteria;
import org.hibernate.EntityMode;
import org.hibernate.HibernateException;
import org.hibernate.criterion.CriteriaQuery;
import org.hibernate.criterion.Criterion;
import org.hibernate.criterion.SimpleExpression;
import org.hibernate.engine.spi.SessionFactoryImplementor;
import org.hibernate.engine.spi.TypedValue;
import org.hibernate.type.CustomType;
import org.hibernate.type.Type;
import org.hibernate.usertype.UserType;
import org.hibernate.internal.util.StringHelper;

/**
 * 任意要素に対する検索条件をあらわすクラスです。
 * 検索対象のカラムが配列型ではない場合には、通常の検索になります。
 */
public class AnyElementInExpression implements Criterion
{
	/** プロパティー名 */
	private final String propertyName;
	/** 検索条件 */
	private final Object[] values;

	/**
	 * コンストラクタです。Restrictions からのみ生成可能です。
	 */
	protected AnyElementInExpression(String propertyName,
									 Object[] values)
	{
		this.propertyName = propertyName;
		this.values = values;
	}

	/**
	 * SQL文を生成します。
	 */
	public String toSqlString(Criteria criteria, CriteriaQuery criteriaQuery)
		throws HibernateException
	{
		// プロパティーに対応するカラムを取り出す
		String[] columns
			= criteriaQuery.getColumnsUsingProjection(criteria, propertyName);
		// プロパティーのタイプを取り出す
		Type type
			= criteriaQuery.getTypeUsingProjection(criteria, propertyName);
		// SessionFactoryを得る
		SessionFactoryImplementor factory = criteriaQuery.getFactory();
		// SQL型を取り出す(この値はUserType.sqlTypes)
		int[] sqlTypes = type.sqlTypes(factory);
		if (sqlTypes.length != 1)
			throw new HibernateException("not supported");

		// SQL文を格納するバッファ
		StringBuilder buf = new StringBuilder();

		buf.append(columns[0]);
		if (sqlTypes[0] == Types.ARRAY)
		{
			// 配列型なので、任意要素の検索になる
			buf.append("[]");
		}

		buf.append("in (")
			.append(StringHelper.repeat("?, ", values.length -1))
			.append("?)");

		return buf.toString();
	}

	/**
	 * プレースホルダー '?' に対応するオブジェクトを返します。
	 */
	public TypedValue[] getTypedValues(Criteria criteria,
									   CriteriaQuery criteriaQuery)
		throws HibernateException
	{
		Type type = criteriaQuery.getType(criteria, propertyName);
		if (!(type instanceof CustomType))
		{
			throw new HibernateException("unsupported type: " + type);
		}

		CustomType ct = (CustomType)type;
		UserType ut = ct.getUserType();
		if (!(ut instanceof CollectionType))
		{
			throw new HibernateException("unsupported user type: " + ut);
		}

		CollectionType col = (CollectionType)ut;
		TypedValue[] v = new TypedValue[values.length];
		for (int i = 0; i < values.length; ++i)
		{
			v[i] = new TypedValue(col.getElementType(),
								  values[i], EntityMode.POJO);
		}
		return v;
	}

	/**
	 * 文字列表現を取り出します
	 */
	public String toString()
	{
		return propertyName + "[] in (" + StringHelper.toString(values) + ")";
	}

}

//
// Copyright (c) 2007, 2010, 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
