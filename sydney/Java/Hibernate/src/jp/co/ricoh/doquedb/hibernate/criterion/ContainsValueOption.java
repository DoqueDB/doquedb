// -*-Mode: Java; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ContainsValueOption.java -- 
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
 * "contains"検索の検索値オプションをあわらすクラスです。
 */
public class ContainsValueOption implements ContainsValue
{
	/** EXACTWORD */
	public static final String EXACTWORD = "exactword";
	/** SIMPLEWORD */
	public static final String SIMPLEWORD = "simpleword";
	/** STRING */
	public static final String STRING = "string";
	/** WORDHEAD */
	public static final String WORDHEAD = "wordhead";
	/** WORDTAIL */
	public static final String WORDTAIL = "wordtail";
	
	/** 検索条件 */
	private final SimpleContainsValue value;
	/** オプション */
	private final String option;
	
	/**
	 * コンストラクタです。ContainsExpression からのみ生成可能です。
	 */
	protected ContainsValueOption(SimpleContainsValue value, String option)
	{
		this.value = value;
		this.option = option;
	}

	/**
	 * SQL文を作成します
	 */
	public String toSqlString(Criteria criteria, CriteriaQuery criteriaQuery)
		throws HibernateException
	{
		return option + "(" + value.toSqlString(criteria, criteriaQuery) + ")";
	}

	/**
	 * 文字列表現を取得します。
	 */
	public String toString()
	{
		return option + "(" + value +")";
	}
	
}

//
// Copyright (c) 2007, 2010, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
