// -*-Mode: Java; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// TRSubqueries.java -- 
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

import org.hibernate.criterion.Criterion;
import org.hibernate.criterion.DetachedCriteria;

/**
 * Doquedb用のSubqueries
 */
public class TRSubqueries
{		
	/**
	 * 任意要素に対する、右辺にサブクエリが存在する "in" 条件を作成します。
	 *
	 * @param propertyName
	 *			プロパティー名
	 * @param dc
	 *			サブクエリ
	 * @return Criterion
	 */
	public static Criterion propertyInAnyElement(String propertyName,
												 DetachedCriteria dc)
	{
		return new AnyElementPropertySubqueryExpression(propertyName,
														" in ", dc);
	}
	
	/**
	 * 任意要素に対する、右辺にサブクエリが存在する "not in" 条件を作成します。
	 *
	 * @param propertyName
	 *			プロパティー名
	 * @param dc
	 *			サブクエリ
	 * @return Criterion
	 */
	public static Criterion propertyNotInAnyElement(String propertyName,
													DetachedCriteria dc)
	{
		return new AnyElementPropertySubqueryExpression(propertyName,
														" not in ", dc);
	}
	
	/**
	 * 任意要素に対する、右辺にサブクエリが存在する "equal" 条件を作成します。
	 *
	 * @param propertyName
	 *			プロパティー名
	 * @param dc
	 *			サブクエリ
	 * @return Criterion
	 */
	public static Criterion propertyEqAnyElement(String propertyName,
												 DetachedCriteria dc)
	{
		return new AnyElementPropertySubqueryExpression(propertyName,
														"=", dc);
	}
	
	/**
	 * 任意要素に対する、右辺にサブクエリが存在する "not equal" 条件を
	 * 作成します。
	 *
	 * @param propertyName
	 *			プロパティー名
	 * @param dc
	 *			サブクエリ
	 * @return Criterion
	 */
	public static Criterion propertyNeAnyElement(String propertyName,
												 DetachedCriteria dc)
	{
		return new AnyElementPropertySubqueryExpression(propertyName,
														"<>", dc);
	}
	
	/**
	 * 任意要素に対する、右辺にサブクエリが存在する "greater than" 条件を
	 * 作成します。
	 *
	 * @param propertyName
	 *			プロパティー名
	 * @param dc
	 *			サブクエリ
	 * @return Criterion
	 */
	public static Criterion propertyGtAnyElement(String propertyName,
												 DetachedCriteria dc)
	{
		return new AnyElementPropertySubqueryExpression(propertyName,
														">", dc);
	}
	
	/**
	 * 任意要素に対する、右辺にサブクエリが存在する "less than" 条件を
	 * 作成します。
	 *
	 * @param propertyName
	 *			プロパティー名
	 * @param dc
	 *			サブクエリ
	 * @return Criterion
	 */
	public static Criterion propertyLtAnyElement(String propertyName,
												 DetachedCriteria dc)
	{
		return new AnyElementPropertySubqueryExpression(propertyName,
														"<", dc);
	}
	
	/**
	 * 任意要素に対する、右辺にサブクエリが存在する "greater than or equal"
	 * 条件を作成します。
	 *
	 * @param propertyName
	 *			プロパティー名
	 * @param dc
	 *			サブクエリ
	 * @return Criterion
	 */
	public static Criterion propertyGeAnyElement(String propertyName,
												 DetachedCriteria dc)
	{
		return new AnyElementPropertySubqueryExpression(propertyName,
														">=", dc);
	}
	
	/**
	 * 任意要素に対する、右辺にサブクエリが存在する "less than or equal"
	 * 条件を作成します。
	 *
	 * @param propertyName
	 *			プロパティー名
	 * @param dc
	 *			サブクエリ
	 * @return Criterion
	 */
	public static Criterion propertyLeAnyElement(String propertyName,
												 DetachedCriteria dc)
	{
		return new AnyElementPropertySubqueryExpression(propertyName,
														"<=", dc);
	}
	
}
