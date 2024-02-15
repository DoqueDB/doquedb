// -*-Mode: Java; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// WordList.java -- 
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
 * "contains"のワードリストをあらわすクラスです
 */
public class WordList implements ContainsValue
{
	/** ワードリスト */
	private final ArrayList words = new ArrayList();

	/**
	 * コンストラクタです。ContainsExpression からのみ生成可能です。
	 */
	protected WordList()
	{
	}

	/** ワードを追加します */
	public WordList add(WordTerm word)
	{
		words.add(word);
		return this;
	}

	/** ワードの配列を追加します */
	public WordList add(WordTerm[] words)
	{
		for (int i = 0; i < words.length; ++i)
			this.words.add(words[i]);
		return this;
	}

	/** ワードのコレクションを追加します */
	public WordList add(Collection words)
	{
		Iterator i = words.iterator();
		while (i.hasNext())
			this.words.add(i.next());
		return this;
	}

	/**
	 * SQL文を作成します
	 */
	public String toSqlString(Criteria criteria, CriteriaQuery criteriaQuery)
		throws HibernateException
	{
		StringBuilder buf = new StringBuilder();
		buf.append("wordlist(");
		Iterator i = words.iterator();
		while (i.hasNext())
		{
			buf.append(
				((WordTerm)i.next()).toSqlString(criteria, criteriaQuery));
			if (i.hasNext())
				buf.append(", ");
		}
		buf.append(")");
		return buf.toString();
	}

	/**
	 * 文字列表現を取得します
	 */
	public String toString()
	{
		StringBuilder buf = new StringBuilder();
		buf.append("wordlist(");
		Iterator i = words.iterator();
		while (i.hasNext())
		{
			buf.append(i.next());
			if (i.hasNext())
				buf.append(", ");
		}
		buf.append(")");
		return buf.toString();
	}
	
}

//
// Copyright (c) 2007, 2010, 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
