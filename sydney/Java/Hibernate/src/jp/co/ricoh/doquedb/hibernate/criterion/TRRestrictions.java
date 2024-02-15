// -*-Mode: Java; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// TRRestrictions.java -- 
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

import java.util.Collection;

import org.hibernate.criterion.Criterion;

/**
 * Doquedb専用のRestrictions
 */
public class TRRestrictions
{
	/**
	 * "like" 条件を作成します。
	 * このCriterionは、Hibernateのものから拡張されており、
	 * エスケープ文字と、言語情報の指定が可能となっています。
	 *
	 * @param propertyName
	 *			プロパティ名
	 * @param value
	 *			検索条件。前方一致の場合は<t>'hoge%'</t>、
	 *			部分一致の場合は<t>'%hoge%'</t>などとします。
	 * @return LikeExpression
	 */
	public static LikeExpression likeWithEscape(String propertyName,
												Object value)
	{
		return new LikeExpression(propertyName, value);
	}

	/**
	 * "similar to" 条件を作成します。
	 *
	 * @param propertyName
	 *			プロパティ名
	 * @param value
	 *			検索条件。正規表現を指定します。
	 * @return SimilarExpression
	 */
	public static SimilarExpression similar(String propertyName,
											Object value)
	{
		return new SimilarExpression(propertyName, value);
	}

	/**
	 * "contains" 条件を作成します。
	 *
	 * @param propertyName
	 *			プロパティ名
	 * @param value
	 *			部分一致させる文字列
	 * @return ContainsExpression
	 */
	public static ContainsExpression contains(String propertyName,
											  String value)
	{
		return new ContainsExpression(propertyName, value);
	}

	/**
	 * "contains" 条件を作成します。
	 *
	 * @param propetyName
	 *			プロパティ名
	 * @param value
	 *			<t>ContainsExpression</t>のstaticメソッドで作成したContainsValue
	 * @return ContainsExpression
	 */
	public static ContainsExpression contains(String propertyName,
											  ContainsValue value)
	{
		return new ContainsExpression(propertyName, value);
	}

	/**
	 * "contains" 条件を作成します。
	 *
	 * @param propetyName
	 *			プロパティ名
	 * @param freeText
	 *			自然文文字列
	 * @return ContainsExpression
	 */
	public static ContainsExpression containsFreeText(String propertyName,
													  String freeText)
	{
		return new ContainsExpression(propertyName,
									  ContainsExpression.freeText(freeText));
	}

	/**
	 * 任意要素に対する "equal" 条件を作成します。
	 *
	 * @param propertyName
	 *			プロパティ名
	 * @param value
	 *			検索条件
	 * @return Criterion
	 */
	public static Criterion eqAnyElement(String propertyName,
										 Object value)
	{
		return new AnyElementExpression(propertyName, value, "=");
	}
	
	/**
	 * 任意要素に対する "not equal" 条件を作成します。
	 *
	 * @param propertyName
	 *			プロパティ名
	 * @param value
	 *			検索条件
	 * @return Criterion
	 */
	public static Criterion neAnyElement(String propertyName,
										 Object value)
	{
		return new AnyElementExpression(propertyName, value, "<>");
	}
	
	/**
	 * 任意要素に対する "greater than" 条件を作成します。
	 *
	 * @param propertyName
	 *			プロパティ名
	 * @param value
	 *			検索条件
	 * @return Criterion
	 */
	public static Criterion gtAnyElement(String propertyName,
										 Object value)
	{
		return new AnyElementExpression(propertyName, value, ">");
	}
	
	/**
	 * 任意要素に対する "less than" 条件を作成します。
	 *
	 * @param propertyName
	 *			プロパティ名
	 * @param value
	 *			検索条件
	 * @return Criterion
	 */
	public static Criterion ltAnyElement(String propertyName,
										 Object value)
	{
		return new AnyElementExpression(propertyName, value, "<");
	}
	
	/**
	 * 任意要素に対する "greaer than or equal" 条件を作成します。
	 *
	 * @param propertyName
	 *			プロパティ名
	 * @param value
	 *			検索条件
	 * @return Criterion
	 */
	public static Criterion geAnyElement(String propertyName,
										 Object value)
	{
		return new AnyElementExpression(propertyName, value, ">=");
	}
	
	/**
	 * 任意要素に対する "less than or equal" 条件を作成します。
	 *
	 * @param propertyName
	 *			プロパティ名
	 * @param value
	 *			検索条件
	 * @return Criterion
	 */
	public static Criterion leAnyElement(String propertyName,
										 Object value)
	{
		return new AnyElementExpression(propertyName, value, "<=");
	}
	
	/**
	 * 任意要素に対する "between" 条件を作成します。
	 *
	 * @param propertyName
	 *			プロパティ名
	 * @param lo	下限
	 * @param hi	上限
	 * @return Criterion
	 */
	public static Criterion betweenAnyElement(String propertyName,
											  Object lo, Object hi)
	{
		return new AnyElementBetweenExpression(propertyName, lo, hi);
	}
	
	/**
	 * 任意要素に対する "in" 条件を作成します。
	 *
	 * @param propertyName
	 *			プロパティ名
	 * @param values
	 *			検索条件の配列
	 * @return Criterion
	 */
	public static Criterion inAnyElement(String propertyName, Object[] values)
	{
		return new AnyElementInExpression(propertyName, values);
	}
	
	/**
	 * 任意要素に対する "in" 条件を作成します。
	 *
	 * @param propertyName
	 *			プロパティ名
	 * @param values
	 *			検索条件のCollection
	 * @return Criterion
	 */
	public static Criterion inAnyElement(String propertyName,
										 Collection values)
	{
		return new AnyElementInExpression(propertyName, values.toArray());
	}
	
	/**
	 * 任意要素に対する "is null" 条件を作成します。
	 *
	 * @param propertyName
	 *			プロパティ名
	 * @return Criterion
	 */
	public static Criterion isNullAnyElement(String propertyName)
	{
		return new AnyElementNullExpression(propertyName);
	}
	
	/**
	 * 任意要素に対する "is not null" 条件を作成します。
	 *
	 * @param propertyName
	 *			プロパティ名
	 * @return Criterion
	 */
	public static Criterion isNotNullAnyElement(String propertyName)
	{
		return new AnyElementNotNullExpression(propertyName);
	}
	
}

//
// Copyright (c) 2007, 2010, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
