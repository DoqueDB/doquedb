// -*-Mode: Java; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ContainsValueJunction.java
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

import java.util.ArrayList;
import java.util.Collection;
import java.util.Iterator;

import org.hibernate.Criteria;
import org.hibernate.HibernateException;
import org.hibernate.criterion.CriteriaQuery;

/**
 * "contains"検索条件の論理演算をあらわすクラス
 */
public class ContainsValueJunction implements ContainsValue
{
	/** スコア合成器 */
	/** 算術和 */
	public static final String SUM = "Sum";
	/** 代数和 */
	public static final String ASUM = "ASum";
	/** 算術積 */
	public static final String PROD = "Prod";
	/** 最大値 */
	public static final String MAX = "Max";
	/** 最小値 */
	public static final String MIN = "Min";
	
	/** 検索条件 */
	private final ArrayList values = new ArrayList();
	/** 論理演算子 */
	private final String op;
	/** スコア合成器 */
	private String combiner;
	
	/**
	 * コンストラクタです。ContainsExpression からのみ生成可能です。
	 */
	protected ContainsValueJunction(String op)
	{
		this.op = op;
	}

	/**
	 * 条件を加える
	 */
	public ContainsValueJunction add(ContainsValue v)
	{
		values.add(v);
		return this;
	}

	/**
	 * 条件を加える
	 */
	public ContainsValueJunction add(Collection list)
	{
		Iterator i = list.iterator();
		while (i.hasNext())
			values.add((ContainsValue)i.next());
		return this;
	}

	/**
	 * 条件を加える
	 */
	public ContainsValueJunction add(String v)
	{
		values.add(new SimpleContainsValue(v));
		return this;
	}

	/**
	 * スコア合成器を設定する
	 */
	public ContainsValueJunction combiner(String combiner)
	{
		this.combiner = combiner;
		return this;
	}

	/**
	 * SQL文を作成します
	 */
	public String toSqlString(Criteria criteria, CriteriaQuery criteriaQuery)
		throws HibernateException
	{
		StringBuilder buf = new StringBuilder().append("(");
		Iterator i = values.iterator();
		while (i.hasNext())
		{
			buf.append(
				((ContainsValue)i.next()).toSqlString(criteria, criteriaQuery));
			if (i.hasNext())
				buf.append(" ").append(op).append(" ");
		}
		if (combiner != null)
			buf.append(" combiner ").append(StringHelper.toSqlString(combiner));
		buf.append(")");

		return buf.toString();
	}

	/**
	 * 文字列表現を取得します
	 */
	public String toString()
	{
		StringBuilder buf = new StringBuilder().append("(");
		Iterator i = values.iterator();
		while (i.hasNext())
		{
			buf.append(i.next().toString());
			if (i.hasNext())
				buf.append(" ").append(op).append(" ");
		}
		if (combiner != null)
			buf.append(" combiner ").append(combiner);
		buf.append(")");

		return buf.toString();
	}
	
}

//
// Copyright (c) 2007, 2010, 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
