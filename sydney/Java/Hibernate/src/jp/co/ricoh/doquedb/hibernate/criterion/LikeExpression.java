// -*-Mode: Java; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// LikeExpression.java --
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
import org.hibernate.criterion.Criterion;
import org.hibernate.engine.spi.SessionFactoryImplementor;
import org.hibernate.engine.spi.TypedValue;
import org.hibernate.type.Type;

/**
 * Doquedb用のlike条件です。
 */
public class LikeExpression implements Criterion
{
	/** プロパティ名 */
	private final String propertyName;
	/** 検索条件 */
	private final Object value;
	/** エスケープ文字 */
	private String escapeChar;
	/** 言語情報 */
	private String lang;

	/**
	 * コンストラクタです。Restrictions からのみ生成可能です。
	 */
	protected LikeExpression(String propertyName, Object value)
	{
		this.propertyName = propertyName;
		this.value = value;
		this.escapeChar = null;
		this.lang = null;
	}

	/**
	 * エスケープ文字を設定します。
	 */
	public LikeExpression escape(String escapeChar)
	{
		this.escapeChar = escapeChar;
		return this;
	}

	/**
	 * 言語情報を設定します。
	 */
	public LikeExpression language(String lang)
	{
		this.lang = lang;
		return this;
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
		// SQL文を格納するバッファ
		StringBuilder buf = new StringBuilder();

		// カラムが複数あったら、and で結合させるので、括弧を加える
		if (columns.length > 1) buf.append('(');
		for (int i = 0; i < columns.length; ++i)
		{
			buf.append(columns[i]).append(" like ")
				.append(StringHelper.toSqlString(value.toString()));
			if (escapeChar != null)
			{
				// エスケープ文字を設定
				buf.append(" escape ").
					append(StringHelper.toSqlString(escapeChar));
			}
			if (lang != null)
			{
				// 言語指定を設定
				buf.append(" language ").
					append(StringHelper.toSqlString(lang));
			}
			if (i < columns.length - 1)
				buf.append(" and ");
		}
		if (columns.length > 1) buf.append(')');
		return buf.toString();
	}

	/**
	 * プレースホルダー '?' に対応するオブジェクトを返します。
	 */
	public TypedValue[] getTypedValues(Criteria criteria,
									   CriteriaQuery criteriaQuery)
		throws HibernateException
	{
		return new TypedValue[0];
	}

	/**
	 * 文字列表現を取り出します
	 */
	public String toString()
	{
		StringBuilder buff = new StringBuilder(propertyName);
		buff.append(" like ").append(value.toString());
		if (escapeChar != null)
			buff.append("escape ").append(escapeChar);
		if (lang != null)
			buff.append("language ").append(lang);
		return buff.toString();
	}

}

//
// Copyright (c) 2007, 2010, 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
