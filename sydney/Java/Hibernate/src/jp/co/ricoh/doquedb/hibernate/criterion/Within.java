// -*-Mode: Java; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Within.java --
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

import org.hibernate.Criteria;
import org.hibernate.HibernateException;
import org.hibernate.criterion.CriteriaQuery;

/**
 * "contains"のWithinをあらわすクラスです
 */
public class Within implements ContainsValue
{
	/** SYMMETRIC */
	public static final String SYMMETRIC = "symmetric";
	/** ASYMMETRIC */
	public static final String ASYMMETRIC = "asymmetric";
	
	/** 検索条件 */
	private final ContainsValue[] values;
	/** 順序 */
	private String order;
	/** 下限 */
	private Integer lower;
	/** 上限 */
	private final Integer upper;

	/**
	 * コンストラクタです。ContainsExpression からのみ生成可能です。
	 */
	protected Within(ContainsValue[] values, int upper)
	{
		this.values = values;
		this.upper = new Integer(upper);
	}

	/**
	 * 下限を設定します。
	 */
	public Within lower(int lower)
	{
		this.lower = new Integer(lower);
		return this;
	}

	/**
	 * 順序指定を設定します
	 */
	public Within order(String order)
	{
		this.order = order;
		return this;
	}
	
	/**
	 * SQL文を作成します
	 */
	public String toSqlString(Criteria criteria, CriteriaQuery criteriaQuery)
		throws HibernateException
	{
		StringBuilder buf = new StringBuilder();
		buf.append("within(");
		for (int i = 0; i < values.length; ++i)
		{
			buf.append(values[i].toSqlString(criteria, criteriaQuery));
			buf.append(" ");
		}
		if (order != null)
			buf.append(order).append(" ");
		if (lower != null)
			buf.append("lower ").append(lower).append(" ");
		buf.append("upper ").append(upper).append(")");

		return buf.toString();
	}

	/**
	 * 文字列表現を取得します
	 */
	public String toString()
	{
		StringBuilder buf = new StringBuilder();
		buf.append("within(");
		for (int i = 0; i < values.length; ++i)
		{
			buf.append(values[i]);
			buf.append(" ");
		}
		if (order != null)
			buf.append(order).append(" ");
		if (lower != null)
			buf.append("lower ").append(lower).append(" ");
		buf.append("upper ").append(upper).append(")");

		return buf.toString();
	}
	
}

//
// Copyright (c) 2007, 2010, 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
