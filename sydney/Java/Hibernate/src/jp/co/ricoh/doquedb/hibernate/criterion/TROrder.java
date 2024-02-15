// -*-Mode: Java; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// TROrder.java -- 
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
public class TROrder extends Order
{
	/** SCORE */
	public static final int SCORE = 1;
	/** WORD DF */
	public static final int WORD_DF = 2;
	/** WORD_SCALE */
	public static final int WORD_SCALE = 3;

	/** プロパティ名 */
	private final String propertyName;
	/** タイプ */
	private final int type;
	/** 降順/昇順 */
	private final boolean ascending;
	
	/** コンストラクタ。TROrderクラスからしか生成できない */
	protected TROrder(String propertyName, boolean ascending, int type)
	{
		super(propertyName, ascending);	// 必要ないけどしょうがない...
		this.propertyName = propertyName;
		this.ascending = ascending;
		this.type = type;
	}

	/**
	 * SQL文を取得します
	 */
	public String toSqlString(Criteria criteria, CriteriaQuery criteriaQuery)
		throws HibernateException
	{
		String[] columns
			= criteriaQuery.getColumnsUsingProjection(criteria, propertyName);
		if (columns.length != 1)
			throw new HibernateException("not supported");

		StringBuilder buf = new StringBuilder();
		switch (type)
		{
		case SCORE:
			buf.append("score(").append(columns[0]).append(") ");
			break;
		case WORD_DF:
			buf.append("word(").append(columns[0]).append(").df ");
			break;
		case WORD_SCALE:
			buf.append("word(").append(columns[0]).append(").scale ");
			break;
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
		switch (type)
		{
		case SCORE:
			buf.append("score(").append(propertyName).append(") ");
			break;
		case WORD_DF:
			buf.append("word(").append(propertyName).append(").df ");
			break;
		case WORD_SCALE:
			buf.append("word(").append(propertyName).append(").scale ");
			break;
		}
		buf.append(ascending ? "asc" : "desc");
		return buf.toString();
	}

	/** スコアの昇順でソートするクラスを取得します */
	public static TROrder scoreAsc(String propertyName)
	{
		return new TROrder(propertyName, true, SCORE);
	}
	/** スコアの降順でソートするクラスを取得します */
	public static TROrder scoreDesc(String propertyName)
	{
		return new TROrder(propertyName, false, SCORE);
	}

	/** DFの昇順でソートするクラスを取得します */
	public static TROrder wordDfAsc(String propertyName)
	{
		return new TROrder(propertyName, true, WORD_DF);
	}
	/** DFの降順でソートするクラスを取得します */
	public static TROrder wordDfDesc(String propertyName)
	{
		return new TROrder(propertyName, false, WORD_DF);
	}

	/** SCALEの昇順でソートするクラスを取得します */
	public static TROrder wordScaleAsc(String propertyName)
	{
		return new TROrder(propertyName, true, WORD_SCALE);
	}
	/** SCALEの降順でソートするクラスを取得します */
	public static TROrder wordScaleDesc(String propertyName)
	{
		return new TROrder(propertyName, false, WORD_SCALE);
	}
	
}

//
// Copyright (c) 2007, 2010, 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
