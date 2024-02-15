// -*-Mode: Java; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ContainsValueBinary.java
// 
// Copyright (c) 2011, 2023 Ricoh Company, Ltd.
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
 * "contains"検索条件の二項演算子をあらわすクラス
 */
public class ContainsValueBinary implements ContainsValue
{
	/** 左の条件 */
	private ContainsValue left;
	/** 右の条件 */
	private ContainsValue right;
	/** 論理演算子 */
	private final String op;
	
	/**
	 * コンストラクタです。ContainsExpression からのみ生成可能です。
	 */
	protected ContainsValueBinary(String op)
	{
		this.op = op;
		this.left = null;
		this.right = null;
	}

	/**
	 * 条件を加える
	 */
	public ContainsValueBinary left(String v)
	{
		this.left = new SimpleContainsValue(v);
		return this;
	}

	/**
	 * 条件を加える
	 */
	public ContainsValueBinary left(ContainsValue v)
	{
		this.left = v;
		return this;
	}

	/**
	 * 条件を加える
	 */
	public ContainsValueBinary right(String v)
	{
		this.right = new SimpleContainsValue(v);
		return this;
	}

	/**
	 * 条件を加える
	 */
	public ContainsValueBinary right(ContainsValue v)
	{
		this.right = v;
		return this;
	}

	/**
	 * SQL文を作成します
	 */
	public String toSqlString(Criteria criteria, CriteriaQuery criteriaQuery)
		throws HibernateException
	{
		StringBuilder buf = new StringBuilder();
		buf.append(left.toSqlString(criteria, criteriaQuery));
		buf.append(" ").append(op).append(" ");
		buf.append(right.toSqlString(criteria, criteriaQuery));
		
		return buf.toString();
	}

	/**
	 * 文字列表現を取得します
	 */
	public String toString()
	{
		StringBuilder buf = new StringBuilder();
		buf.append(left.toString())
			.append(" ").append(op).append(" ")
			.append(right.toString());

		return buf.toString();
	}
	
}

//
// Copyright (c) 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
