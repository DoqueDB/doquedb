// -*-Mode: Java; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// TROrderMulti.java -- 
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
import org.hibernate.criterion.Order;

/**
 * Doquedb用の Order をあらわすクラス
 */
public class TROrderMulti extends Order
{
	/** ADD */
	public static final int ADD = 1;

	/** プロパティ名 */
	private final String[] propertyNames;
	/** タイプ */
	private final int type;
	/** 降順/昇順 */
	private final boolean ascending;
	
	/** コンストラクタ。TROrderクラスからしか生成できない */
	protected TROrderMulti(String[] propertyNames, boolean ascending, int type)
	{
		super(propertyNames[0], ascending);	// 必要ないけどしょうがない...
		this.propertyNames = propertyNames;
		this.ascending = ascending;
		this.type = type;
	}

	/**
	 * SQL文を取得します
	 */
	public String toSqlString(Criteria criteria, CriteriaQuery criteriaQuery)
		throws HibernateException
	{
		StringBuilder buf = new StringBuilder();

		for (int i = 0; i < propertyNames.length; ++i)
		{
			String p = propertyNames[i];
			
			String[] columns
				= criteriaQuery.getColumnsUsingProjection(criteria, p);
			if (columns.length != 1)
				throw new HibernateException("not supported");

			if (i != 0)
				buf.append("+ ");	// 今は足し算だけ
			buf.append("score(").append(columns[0]).append(") ");
		}
		buf.append(ascending ? "asc" : "desc");

		return buf.toString();
	}

	/**
	 * 文字列表現を取得します
	 */
	public String toString()
	{
		StringBuilder buf = new StringBuilder();
		for (int i = 0; i < propertyNames.length; ++i)
		{
			String p = propertyNames[i];
			if (i != 0) buf.append("+ ");
			buf.append("score(").append(p).append(") ");
		}
		buf.append(ascending ? "asc" : "desc");
		return buf.toString();
	}

	/** スコアの昇順でソートするクラスを取得します */
	public static TROrderMulti scoreAddAsc(String[] propertyNames)
	{
		return new TROrderMulti(propertyNames, true, ADD);
	}
	/** スコアの降順でソートするクラスを取得します */
	public static TROrderMulti scoreAddDesc(String[] propertyNames)
	{
		return new TROrderMulti(propertyNames, false, ADD);
	}

}

//
// Copyright (c) 2007, 2010, 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
