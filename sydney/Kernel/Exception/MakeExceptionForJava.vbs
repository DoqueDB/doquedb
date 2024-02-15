' -*-Mode: C++; tab-width: 4; c-basic-offset:4;-*-
'
' MakeExceptionForJava.vbs -- Java用のエラーコードを自動生成するVBScript
'

Option Explicit

'------------------------
'コマンドライン引数の解析
'------------------------

Dim definitionFile		'エラー定義ファイルのファイル名
Dim arguments
Set arguments = WScript.Arguments
If arguments.Count > 0 Then
	definitionFile = arguments(0)
Else
	definitionFile = "ErrorDefinition.xml"
End If

Dim Languages
Languages = Array("Japanese", "English")
Const maxLanguage = 1

Dim MessageFormat(1)
Dim ErrorCodeFormat
Dim ThrowFormat

Dim IgnoredError
IgnoredError = -2147220728

'--------------------------------------------------------
'作業ディレクトリの取得とヘッダを置くディレクトリーの作成
'--------------------------------------------------------
Dim fileSystem
Set fileSystem = createObject("Scripting.FileSystemObject")
Dim topDir				'.cppと.hを作るディレクトリのトップ
topDir = fileSystem.GetAbsolutePathName(".")
Dim folderName
folderName = topDir & "/Java" 
If fileSystem.FolderExists(folderName) = False Then
	fileSystem.CreateFolder(folderName)
End If

'---------------------------------
'スクリプトとXMLファイルの時刻比較
'---------------------------------
Dim ForceToProduce
ForceToProduce = 0
Dim scriptFile
Dim xmlFile
Set scriptFile = fileSystem.GetFile("MakeException.vbs")
Set xmlFile = fileSystem.GetFile(definitionFile)
If scriptFile.DateLastModified > xmlFile.DateLastModified Then
	'XMLファイルに変更がなくてもスクリプトが新しければ再作成する
	ForceToProduce = 1
End If

'--------------------------------
'例外定義のエントリーを表すクラス
'--------------------------------
Class ErrorDefinition

	Public name
	Public desc
	Public varName
	Public argument()
	Public maxArgument

	Sub Class_Initialize()
	End Sub

	Function parse(node_)
		Dim nameNode
		Set nameNode = node_.selectSingleNode("Name")

		name = nameNode.text

		Dim currNode
		Set currNode = node_.selectSingleNode("Number")

		Dim number
		number = currNode.text

		Dim description
		Dim childNode
		Set currNode = node_.selectSingleNode("Description")
		If Not currNode Is Nothing Then
			Set childNode = currNode.selectSingleNode("Japanese")
			description = childNode.text
		End If

		'------------------------------------
		'Nameからエラー番号の変数名を作成する
		'------------------------------------
		varName = ""
		If name = "SQLSyntaxError" Then
			varName = "SQL_SYNTAX_ERROR"
		Else
			Dim length
			length = Len(name)
			Dim j
			For j = 0 To length - 1
				Dim c
				c = Asc(Right(name, length - j))
				If c >= 65 And c <= 90 Then
					If Len(varName) <> 0 Then
						varName = varName & "_"
					End If
				Else
					c = c - 32
				End If
				varName = varName & Chr(c)
			Next
		End If

		'-----------------------------------------
		'Descriptionから表示できない文字を削除する
		'-----------------------------------------
		desc = ""
		length = Len(description)
		For j = 0 To length - 1
			c = Asc(Right(description, length - j))
			If Not (c >= 0 And c < 32) Then
				desc = desc & Chr(c)
			End If
		Next


		'---------------------
		'ErrorCodeFormatを作る
		'---------------------
		ErrorCodeFormat = ErrorCodeFormat & vbTab & "/** " & desc & " */" & vbNewLine
		ErrorCodeFormat = ErrorCodeFormat & vbTab & "public final static int " & varName & " = " & number & ";" & vbNewLine

		Set currNode = node_.selectSingleNode("Message")
		If Not currNode Is Nothing Then
			Dim l
			For l = 0 To maxLanguage
				Set childNode = currNode.selectSingleNode(Languages(l))
				If Not childNode Is Nothing Then
					'-------------------
					'MessageFormatを作る
					'-------------------
					if Len(MessageFormat(l)) <> 0 Then
						MessageFormat(l) = MessageFormat(l) & "," & vbNewLine
					End If
					MessageFormat(l) = MessageFormat(l) & vbTab & vbTab & "new MessageEntry(ErrorCode." & varName & "," & vbNewLine
					MessageFormat(l) = MessageFormat(l) & vbTab & vbTab & """" & childNode.text & """)"
				End If
			Next
		End If

		Dim nodeList
		Set nodeList = node_.selectNodes("MessageArgument")
		'-------------------
		'MessageArgumentも作る
		'-------------------
		If nodeList.length = 0 Then
			maxArgument = 0
			Redim argument(0)
			argument(0) = ""
		Else
			maxArgument = nodeList.length - 1
			Redim argument(maxArgument)
			Dim i
			For i = 0 To maxArgument
				Set childNode = nodeList.nextNode
				argument(i) = childNode.text
			Next
		End If

		'--------------------
		'Javaの本体ファイルを作成する
		'--------------------
		Dim cppFile
		Set cppFile = fileSystem.OpenTextFile(folderName & "/" & name & ".java", 2, True, 0)
		writeJavaFunction(cppFile)
		cppFile.Close

		'---------------------
		'throwする部分を作成する
		'---------------------
		ThrowFormat = ThrowFormat & "		case ErrorCode." & varName & ":" & vbNewLine
		ThrowFormat = ThrowFormat & "			throw new " & name & "(e_);" & vbNewLine

		parse = True
	End Function

	Sub writeJavaFunction(file_)
		file_.WriteLine("// -*-Mode: Java; tab-width: 4; c-basic-offset: 4;-*-")
		file_.WriteLine("// vi:set ts=4 sw=4:")
		file_.WriteLine("//")
		file_.WriteLine("// !!!This file is AUTOMATICALY GENERATED. DO NOT EDIT!!!")
		file_.WriteLine("//")
		file_.WriteLine("// " & name & ".java -- 例外クラス")
		file_.WriteLine("//")
		file_.WriteLine("// Copyright (c) 2002, 2003 Ricoh Company, Ltd.")
		file_.WriteLine("// All rights reserved.")
		file_.WriteLine("//")
		file_.WriteLine("//	$Author$")
		file_.WriteLine("//	$Date$")
		file_.WriteLine("//	$Revision$")
		file_.WriteLine("//")
		file_.WriteLine("")
		file_.WriteLine("package jp.co.ricoh.sydney.exception;")
		file_.WriteLine("")
		file_.WriteLine("import jp.co.ricoh.sydney.common.*;")
		file_.WriteLine("")
		file_.WriteLine("/**")
		file_.WriteLine(" * " & desc)
		file_.WriteLine(" */")
		file_.WriteLine("public class " & name & " extends java.sql.SQLException")
		file_.WriteLine("{")
		Dim extraArgument
		Dim extraArgument2
		If name <> "ModError" Then
			file_.WriteLine("	/**")
			file_.WriteLine("	 * 新たに例外オブジェクトを作成する。")
			file_.WriteLine("	 */")
			Dim i
			If argument(0) <> "" Then
				For i = 0 To maxArgument
					If i <> 0 Then
						extraArgument = extraArgument & ", "
						extraArgument2 = extraArgument2 & ", "
					End If
					extraArgument = extraArgument & createVariable(argument(i), i)
					extraArgument2 = extraArgument2 & "arg" & i & "_"
				Next
			End If
			file_.WriteLine("	public " & name & "(" & extraArgument & ")")
			file_.WriteLine("	{")
			file_.WriteLine("		super(makeErrorMessage(" & extraArgument2 & "));")
			file_.WriteLine("	}")
			file_.WriteLine("")
		End If
		file_.WriteLine("	/**")
		file_.WriteLine("	 * 新たに例外オブジェクトを作成する。")
		file_.WriteLine("	 */")
		file_.WriteLine("	public " & name & "(ExceptionData e_)")
		file_.WriteLine("	{")
		file_.WriteLine("		super(e_.getErrorMessage());")
		file_.WriteLine("	}")
		file_.WriteLine("")
		If name <> "ModError" Then
			file_.WriteLine("	/**")
			file_.WriteLine("	 * エラーメッセージを作成する。")
			file_.WriteLine("	 */")
			file_.WriteLine("	private static String makeErrorMessage(" & extraArgument & ")")
			file_.WriteLine("	{")
			If argument(0) <> "" Then
				file_.WriteLine("		java.util.Vector arguments = new java.util.Vector();")
				For i = 0 To maxArgument
					file_.WriteLine("		arguments.add(" & createConvert(argument(i), i) & ");")
				Next
				file_.WriteLine("		return ErrorMessage.makeErrorMessage(")
				file_.WriteLine("				ErrorCode." & varName & ", arguments);")
			Else
				file_.WriteLine("		return ErrorMessage.makeErrorMessage(")
				file_.WriteLine("				ErrorCode." & varName & ", null);")
			End If
			file_.WriteLine("	}")
			file_.WriteLine("")
		End If
		file_.WriteLine("}")
		file_.WriteLine("")
		file_.WriteLine("//")
		file_.WriteLine("// Copyright (c) 2002, 2003 Ricoh Company, Ltd.")
		file_.WriteLine("// All rights reserved.")
		file_.WriteLine("//")
	End Sub

	Function createVariable(argument_, index_)
		If name = "ModError" and index_ = 0 Then
			createVariable = "ModException exception0_"
			Exit Function
		End If

		Dim result
		Dim digit
		digit = Right(argument_, 1)

		If digit = "s" Then
			result = "String arg" & index_ & "_"
		Else
			If digit = "d" or digit = "i" or digit = "u" or digit = "o" or digit = "x" or digit = "X" Then
				result = "int arg" & index_ & "_"
			Else
				WScript.Echo definitionFile & " : error: Unknown format: " & argument_
				createVariable = ""
				Exit Function
			End If
		End If
		createVariable = result
	End Function

	Function createConvert(argument_, index_)
		If name = "ModError" and index_ = 0 Then
			createConvert = "exception0_.toString()"
			Exit Function
		End If

		Dim result
		Dim digit
		digit = Right(argument_, 1)

		If digit = "s" Then
			result = "arg" & index_ & "_"
		Else
			If digit = "d" or digit = "i" or digit = "u" Then
				result = "Integer.toString(arg" & index_ & "_" & ")"
			ElseIf digit = "o" Then
				result = "Integer.toOctalString(arg" & index_ & "_" & ")"
			ElseIf digit = "x" or digit = "X" Then
				result = "Integer.toHexString(arg" & index_ & "_" & ")"
			Else
				WScript.Echo definitionFile & " : error: Unknown format: " & argument_
				createConvert = ""
				Exit Function
			End If
		End If
		createConvert = result
	End Function

End Class

Sub WriteMessageFile
	Dim file, fileName

	'----------------------------------------
	'ErrorCode.java
	'----------------------------------------
	fileName = folderName & "/ErrorCode.java"

	Set file = fileSystem.OpenTextFile(fileName, 2, True, 0)
	file.WriteLine("// -*-Mode: Java; tab-width: 4; c-basic-offset: 4;-*-")
	file.WriteLine("// vi:set ts=4 sw=4:")
	file.WriteLine("//")
	file.WriteLine("// !!!This file is AUTOMATICALY GENERATED. DO NOT EDIT!!!")
	file.WriteLine("//")
	file.WriteLine("// ErrorCode.java -- エラーコード")
	file.WriteLine("//")
	file.WriteLine("// Copyright (c) 2002, 2003 Ricoh Company, Ltd.")
	file.WriteLine("// All rights reserved.")
	file.WriteLine("//")
	file.WriteLine("//	$Author$")
	file.WriteLine("//	$Date$")
	file.WriteLine("//	$Revision$")
	file.WriteLine("//")
	file.WriteLine("")
	file.WriteLine("package jp.co.ricoh.sydney.exception;")
	file.WriteLine("")
	file.WriteLine("/**")
	file.WriteLine(" * エラーコード")
	file.WriteLine(" */")
	file.WriteLine("public final class ErrorCode")
	file.WriteLine("{")
	file.Write(ErrorCodeFormat)
	file.WriteLine("}")
	file.WriteLine("")
	file.WriteLine("//")
	file.WriteLine("// Copyright (c) 2002, 2003 Ricoh Company, Ltd.")
	file.WriteLine("// All rights reserved.")
	file.WriteLine("//")
	file.Close

	'----------------------------------------
	'MessageFormat.java
	'----------------------------------------
	Dim l
	For l = 0 To maxLanguage
		fileName = folderName & "/MessageFormat" & Languages(l) & ".java"

		Set file = fileSystem.OpenTextFile(fileName, 2, True, 0)
		file.WriteLine("// -*-Mode: Java; tab-width: 4; c-basic-offset: 4;-*-")
		file.WriteLine("// vi:set ts=4 sw=4:")
		file.WriteLine("//")
		file.WriteLine("// !!!This file is AUTOMATICALY GENERATED. DO NOT EDIT!!!")
		file.WriteLine("//")
		file.WriteLine("// MessageFormat" & Languages(l) & ".java -- エラーメッセージフォーマット")
		file.WriteLine("//")
		file.WriteLine("// Copyright (c) 2002, 2003 Ricoh Company, Ltd.")
		file.WriteLine("// All rights reserved.")
		file.WriteLine("//")
		file.WriteLine("//	$Author$")
		file.WriteLine("//	$Date$")
		file.WriteLine("//	$Revision$")
		file.WriteLine("//")
		file.WriteLine("")
		file.WriteLine("package jp.co.ricoh.sydney.exception;")
		file.WriteLine("")
		file.WriteLine("/**")
		file.WriteLine(" * エラーメッセージフォーマット")
		file.WriteLine(" */")
		file.WriteLine("public final class MessageFormat" & Languages(l))
		file.WriteLine("{")
		file.WriteLine("	/** フォーマット */")
		file.WriteLine("	public final static MessageEntry _table[] =")
		file.WriteLine("	{")
		file.WriteLine(MessageFormat(l))
		file.WriteLine("	};")
		file.WriteLine("}")
		file.WriteLine("")
		file.WriteLine("//")
		file.WriteLine("// Copyright (c) 2002, 2003 Ricoh Company, Ltd.")
		file.WriteLine("// All rights reserved.")
		file.WriteLine("//")
		file.Close
	Next

	'----------------------------------------
	'ThrowInstance.java
	'----------------------------------------
	fileName = folderName & "/ThrowClassInstance.java"

	Set file = fileSystem.OpenTextFile(fileName, 2, True, 0)
	file.WriteLine("// -*-Mode: Java; tab-width: 4; c-basic-offset: 4;-*-")
	file.WriteLine("// vi:set ts=4 sw=4:")
	file.WriteLine("//")
	file.WriteLine("// !!!This file is AUTOMATICALY GENERATED. DO NOT EDIT!!!")
	file.WriteLine("//")
	file.WriteLine("// ThrowClassInstance.java -- 例外をスローする")
	file.WriteLine("//")
	file.WriteLine("// Copyright (c) 2002, 2003 Ricoh Company, Ltd.")
	file.WriteLine("// All rights reserved.")
	file.WriteLine("//")
	file.WriteLine("//	$Author$")
	file.WriteLine("//	$Date$")
	file.WriteLine("//	$Revision$")
	file.WriteLine("//")
	file.WriteLine("")
	file.WriteLine("package jp.co.ricoh.sydney.exception;")
	file.WriteLine("")
	file.WriteLine("import jp.co.ricoh.sydney.common.*;")
	file.WriteLine("")
	file.WriteLine("/**")
	file.WriteLine(" * 例外をスローする。")
	file.WriteLine(" */")
	file.WriteLine("public final class ThrowClassInstance")
	file.WriteLine("{")
	file.WriteLine("	/**")
	file.WriteLine("	 * 例外をスローする。")
	file.WriteLine("	 */")
	file.WriteLine("	public static void throwException(ExceptionData e_)")
	file.WriteLine("		throws java.sql.SQLException")
	file.WriteLine("	{")
	file.WriteLine("		switch (e_.getErrorNumber())")
	file.WriteLine("		{")
	file.Write(ThrowFormat)
	file.WriteLine("		default:")
	file.WriteLine("			throw new Unexpected();")
	file.WriteLine("		}")
	file.WriteLine("	}")
	file.WriteLine("}")
	file.WriteLine("")
	file.WriteLine("//")
	file.WriteLine("// Copyright (c) 2002, 2003 Ricoh Company, Ltd.")
	file.WriteLine("// All rights reserved.")
	file.WriteLine("//")
	file.Close

End Sub

'==================
'ここから処理の開始
'==================

Dim ErrorCode
ErrorCode = 0

'---------------------------------------
'例外を定義しているXMLファイルを読み込む
'---------------------------------------
Dim xmlDoc
Set xmlDoc = CreateObject("microsoft.xmldom")
xmlDoc.async = False

On Error Resume Next
xmlDoc.load(definitionFile)
If xmlDoc.parseError.errorCode <> 0 Then
	On Error Goto 0
	ErrorCode = 17
	Err.Raise ErrorCode, ": error: エラー定義ファイルに誤りがあります。" & vbNewLine & xmlDoc.parseError.reason
End If
Err.Clear

'-------------------------------------------------
'XMLファイルの内容をチェックして正しければ実行する
'-------------------------------------------------
If CheckDefinition(xmlDoc) = False Then
	On Error Goto 0
	ErrorCode = 17
	Err.Raise ErrorCode, ": error: エラー定義ファイルに誤りがあります。"
Else
	On Error Goto 0

	'---------------------------------
	'<Error>タグごとに例外の定義がある
	'---------------------------------
	Dim nodeList
	Set nodeList = xmlDoc.getElementsByTagName("Error")

	'------------------------------------------------------------------
	' generated: 生成した例外の名称を並べる
	' noNeed:    前回から変更がないので生成しなかった例外の名称を並べる
	'------------------------------------------------------------------
	Dim generated
	Dim noNeed
	Dim i
	For i = 0 To nodeList.length - 1
		Dim errorNode
		Set errorNode = nodeList.nextNode

		Dim errorDef
		Set errorDef = New ErrorDefinition

		errorDef.parse(errorNode)
	Next
	'------------------------------------------------------------------
	'すべて読み終わったらMessageFormatとErrorCodeをファイルに書く
	'------------------------------------------------------------------
	WriteMessageFile

	WScript.Quit ErrorCode
End If

'-------------------------------
'XMLファイルの内容をチェックする
'-------------------------------
Function CheckDefinition(doc_)
	Dim nameList
	Set nameList = doc_.selectNodes("ErrorDefine/Error/Name")
	Dim numberList
	Set numberList = doc_.selectNodes("ErrorDefine/Error/Number")

	If nameList.length <> numberList.length Then
		WScript.Echo definitionFile & " : error: <Name>タグと<Number>タグの数が合いません"
		CheckDefinition = False
		Exit Function
	End If

	Dim i
	For i = 0 To nameList.length - 1
		Dim nameNode
		Set nameNode = nameList.item(i)
		Dim numberNode
		Set numberNode = numberList.item(i)

		Dim j
		For j = i + 1 To nameList.length - 1
			Dim otherName
			Set otherName = nameList.item(j)
			Dim otherNumber
			Set otherNumber = numberList.item(j)
			If nameNode.text = otherName.text Then
				WScript.Echo definitionFile & " : error: 同じ名前の例外が複数あります: " & nameNode.text
				CheckDefinition = False
				Exit Function
			End If
			If numberNode.text = otherNumber.text Then
				WScript.Echo definitionFile & " : error: 同じ番号の例外が複数あります: " & numberNode.text
				CheckDefinition = False
				Exit Function
			End If
		Next
	Next
	CheckDefinition = True
End Function

'
' Copyright (c) 2002, 2003, 2023 Ricoh Company, Ltd.
' All rights reserved.
'
