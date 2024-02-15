// -*-Mode: Java; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// SimpleContainsValue.java
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
 * Doquedbの全文検索用演算子である"contains"を表現するためのクラスです。
 */
public class SimpleContainsValue implements ContainsValue
{
	/** 検索条件 */
	private final String value;
	/** 言語指定 */
	private String language;
	
	/**
	 * コンストラクタです。ContainsExpression からのみ生成可能です。
	 */
	protected SimpleContainsValue(String value)
	{
		this.value = value;
		this.language = null;
	}

	/**
	 * 言語を指定します。
	 */
	public SimpleContainsValue language(String language)
	{
		this.language = language;
		return this;
	}
	
	/**
	 * SQL文を作成します
	 */
	public String toSqlString(Criteria criteria, CriteriaQuery criteriaQuery)
		throws HibernateException
	{
		StringBuilder buf = new StringBuilder(StringHelper.toSqlString(value));
		if (language != null)
			buf.append(" language ").append(StringHelper.toSqlString(language));
		return buf.toString();
	}

	/**
	 * 文字列表現を取得します
	 */
	public String toString()
	{
		if (language == null)
			return value;
		else
			return value + " language " + language;
	}
}

//
// Copyright (c) 2007, 2010, 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
