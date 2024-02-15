// -*-Mode: Java; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// PartMatch.java -- 
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

import org.hibernate.Criteria;
import org.hibernate.HibernateException;
import org.hibernate.criterion.CriteriaQuery;

/**
 * "contains"の部分一致(前方一致、後方一致)をあらわすクラスです
 */
public class PartMatch implements ContainsValue
{
	/** HEAD */
	public static final String HEAD = "head";
	/** TAIL */
	public static final String TAIL = "tail";
	
	/** 条件 */
	private final ContainsValue condition;
	/** 部分一致の種別 */
	private final String partMatch;

	/**
	 * コンストラクタです。ContainsExpression からのみ生成可能です。
	 */
	protected PartMatch(ContainsValue condition, String partMatch)
	{
		this.condition = condition;
		this.partMatch = partMatch;
	}

	/**
	 * SQL文を作成します
	 */
	public String toSqlString(Criteria criteria, CriteriaQuery criteriaQuery)
		throws HibernateException
	{
		return partMatch + "("
			+ condition.toSqlString(criteria, criteriaQuery) + ")";
	}

	/**
	 * 文字列表現を取得します
	 */
	public String toString()
	{
		return partMatch + "(" + condition + ")";
	}
	
}

//
// Copyright (c) 2007, 2010, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
