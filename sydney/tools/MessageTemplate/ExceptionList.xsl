<?xml version="1.0" encoding="Shift_JIS"?>

<xsl:stylesheet xmlns:xsl="http://www.w3.org/TR/WD-xsl">

<xsl:template match="/">

	<!--
		リストの表示
	-->

	<table border="1">
		<th>番号</th><th>名前</th><th>対応</th><th>メッセージ</th>
		<xsl:apply-templates select="//Error"/>
	</table>

</xsl:template>

	<!--
		 template 宣言 
	 -->

	<xsl:template match="Error">
		<tr>
		<td><xsl:value-of select="Number"/></td>
		<td><xsl:value-of select="Name"/></td>
		<td><xsl:value-of select="Solution"/></td>
		<td><xsl:value-of select="Message/English"/></td>
		</tr>
	</xsl:template>

</xsl:stylesheet>

