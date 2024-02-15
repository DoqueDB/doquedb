// -*-Mode: Java; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// WordTerm.java --
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
 * "contains"のワードをあらわすクラスです
 */
public class WordTerm implements ContainsValue
{
	/** カテゴリ */
	/** 必須 */
	public static final String ESSENTIAL = "Essential";
	/** 重要 */
	public static final String IMPORTANT = "Important";
	/** 有用 */
	public static final String HELPFUL = "Helpful";
	/** 必須関連語 */
	public static final String ESSENTIAL_RELATED = "EssentialRelated";
	/** 重要関連語 */
	public static final String IMPORTANT_RELATED = "ImportantRelated";
	/** 有用関連語 */
	public static final String HELPFUL_RELATED = "HelpfulRelated";
	
	/** 単語 */
	private final String word;
	/** カテゴリ */
	private String category;
	/** スケール */
	private Double scale;
	/** 言語 */
	private String language;
	/** 出現頻度 */
	private Integer df;

	/**
	 * コンストラクタです。ContainsExpression からのみ生成可能です。
	 */
	protected WordTerm(String word)
	{
		this.word = word;
		this.category = null;
		this.scale = null;
		this.language = null;
		this.df = null;
	}

	/** カテゴリを追加します */
	public WordTerm category(String category)
	{
		this.category = category;
		return this;
	}

	/** スケールを追加します */
	public WordTerm scale(double scale)
	{
		this.scale = new Double(scale);
		return this;
	}

	/** 言語を追加します */
	public WordTerm language(String language)
	{
		this.language = language;
		return this;
	}

	/** 出現頻度を追加します */
	public WordTerm df(int df)
	{
		this.df = new Integer(df);
		return this;
	}

	/**
	 * SQL文を作成します
	 */
	public String toSqlString(Criteria criteria, CriteriaQuery criteriaQuery)
		throws HibernateException
	{
		StringBuilder buf = new StringBuilder();
		buf.append(StringHelper.toSqlString(word));
		if (category != null)
			buf.append(" category ").append(StringHelper.toSqlString(category));
		if (scale != null)
			buf.append(" scale ").append(scale.toString());
		if (language != null)
			buf.append(" language ").append(StringHelper.toSqlString(language));
		if (df != null)
			buf.append(" df ").append(df.toString());
		return buf.toString();
	}

	/**
	 * 文字列表現を取得します
	 */
	public String toString()
	{
		StringBuilder buf = new StringBuilder();
		buf.append(word);
		if (category != null)
			buf.append(" category ").append(category);
		if (scale != null)
			buf.append(" scale ").append(scale);
		if (language != null)
			buf.append(" language ").append(language);
		if (df != null)
			buf.append(" df ").append(df);
		return buf.toString();
	}
	
}

//
// Copyright (c) 2007, 2010, 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
