' -*-Mode: C++; tab-width: 4; c-basic-offset:4;-*-
'
' MakeException.vbs -- 例外クラスを自動生成するVBScript
'
' Copyright (c) 2000, 2001 Ricoh Company, Ltd.
' All rights reserved.
'
'		$Author$
'		$Date$
'		$Revision$
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

Dim msdevDir			'MSDEVのあるパス名
If arguments.Count > 1 Then
	msdevDir = arguments(1)
Else
	msdevDir = "."
End If

Dim Languages
Languages = Array("Japanese", "English")
Const maxLanguage = 1

Dim MessageFormat(1)
Dim MessageArgument
Dim AllNumberFiles
Dim AllExceptionFiles
Dim ThrowClassInstance

Dim IgnoredError
IgnoredError = -2147220728

'--------------------------------------------------------
'作業ディレクトリの取得とヘッダを置くディレクトリーの作成
'--------------------------------------------------------
Dim fileSystem
Set fileSystem = createObject("Scripting.FileSystemObject")
Dim topDir				'.cppと.hを作るディレクトリのトップ
topDir = fileSystem.GetAbsolutePathName(".")
Dim moduleName
moduleName = fileSystem.GetFileName(topDir)
Dim folderName
folderName = topDir & "/" & moduleName
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

'------------------------------------------
'ファイルを追加するべきワークスペースの検査
'------------------------------------------
Dim projectFileIsChanged		'プロジェクトファイルに変更があったか
projectFileIsChanged = False
Dim projectFileIsReadOnly		'プロジェクトファイルが読み込み専用か
Dim projectFileName
projectFileName = topDir & "/" & moduleName & ".dsp"
Dim projectFile
Set projectFile = fileSystem.GetFile(projectFileName)
If projectFile.Attributes and 1 Then
	'メッセージダイアログを出すとうざいのでコメントアウト
	'MsgBox moduleName & "のプロジェクトファイルが読み込み専用です。" & vbNewLine & "新しい例外を追加していたらまずプロジェクトファイルをチェックアウトしてください。" & vbNewLine & "例外を追加していなければそのままで結構です。", vbExclamation, "注意"
	projectFileIsReadOnly = True
Else
	projectFileIsReadOnly = False
End If

'--------------------------------
'例外定義のエントリーを表すクラス
'--------------------------------
Class ErrorDefinition

	Public name
	Public number
	Public argument()
	Public maxArgument
	Public message()
	Public description()
	Public solution()

	Sub Class_Initialize()
		ReDim message(maxLanguage)
		ReDim description(maxLanguage)
		ReDim solution(maxLanguage)
	End Sub
	Function parse(node_, bakList_)
		Dim nameNode
		Set nameNode = node_.selectSingleNode("Name")
		name = nameNode.text

		Dim currNode
		Set currNode = node_.selectSingleNode("Number")
		If Not currNode Is Nothing Then
			number = currNode.text
		End If

		'------------------------------
		'すべてのNumberファイルを並べる
		'------------------------------
		AllNumberFiles = AllNumberFiles & "#include ""Number" & name & ".h""" & vbNewLine

		'------------------------------------
		'すべての例外クラスをインクルードする
		'------------------------------------
		AllExceptionFiles = AllExceptionFiles & "#include """ & moduleName & "/" & name & ".h""" & vbNewLine

		'----------------------------------
		'throwClassInstanceのcaseの中身を作る
		'----------------------------------
		ThrowClassInstance = ThrowClassInstance & "	case ErrorNumber::" & name & ":"& vbNewLine
		ThrowClassInstance = ThrowClassInstance & "		throw " & moduleName & "::" & name & "(cObject_);" & vbNewLine

		Dim prefix
		prefix = vbTab & "{" & moduleName & "::ErrorNumber::" & name & "," & vbNewLine

		Set currNode = node_.selectSingleNode("Message")
		If Not currNode Is Nothing Then
			Dim childNode
			Dim l
			For l = 0 To maxLanguage
				Set childNode = currNode.selectSingleNode(Languages(l))
				If Not childNode Is Nothing Then
					message(l) = childNode.text
					'-------------------
					'MessageFormatも作る
					'-------------------
					MessageFormat(l) = MessageFormat(l) & prefix & vbTab & """" & message(l) & """}," & vbNewLine
				End If
			Next
		End If

		Dim nodeList
		Set nodeList = node_.selectNodes("MessageArgument")
		'-------------------
		'MessageArgumentも作る
		'-------------------
		MessageArgument = MessageArgument & prefix & vbTab & """"
		If nodeList.length = 0 Then
			maxArgument = 0
			Redim argument(0)
			argument(0) = ""
			MessageArgument = MessageArgument & "\000"
		Else
			maxArgument = nodeList.length - 1
			Redim argument(maxArgument)
			Dim i
			For i = 0 To maxArgument
				Set childNode = nodeList.nextNode
				argument(i) = childNode.text
				MessageArgument = MessageArgument & argument(i) & "\000"
			Next
		End If
		MessageArgument = MessageArgument & """}," & vbNewLine

		'------------------------
		'ここでチェックして抜ける
		'------------------------
		If ForceToProduce = 0 and checkNode(nameNode, bakList_) = False Then
			parse = False
			Exit Function
		End If

		Set currNode = node_.selectSingleNode("Description")
		If Not currNode Is Nothing Then
			For l = 0 To maxLanguage
				Set childNode = currNode.selectSingleNode(Languages(l))
				If Not childNode Is Nothing Then
					description(l) = childNode.text
				End If
			Next
		End If

		Set currNode = node_.selectSingleNode("Solution")
		If Not currNode Is Nothing Then
			For l = 0 To maxLanguage
				Set childNode = currNode.selectSingleNode(Languages(l))
				If Not childNode Is Nothing Then
					solution(l) = childNode.text
				End If
			Next
		End If

		parse = True
	End Function

	Function checkNode(node_, bakList_)
		Dim node
		Dim bakNode
		If IsEmpty(bakList_) = False Then
			Dim i
			For i = 0 To bakList_.length - 1
				Set bakNode = bakList_.nextNode
				If bakNode.text = node_.text Then
					If bakNode.parentNode.text = node_.parentNode.text Then
						Set node = node_
					End If
				End If
			Next
			bakList_.reset()
		End If

		CheckNode = IsEmpty(node)
	End Function

	Sub write(project_)
		'--------------------------
		'例外クラスの.hと.cppを作る
		'--------------------------
		createNumberFile project_, folderName & "/Number" & name & ".h"
		createHeaderFile project_, folderName & "/" & name & ".h"
		createCppFile project_, topDir & "/" & name & ".cpp"

	End Sub

	Sub createNumberFile(project_, fileName_)
		'プロジェクトにファイルを加える
		AddHeaderFile project_, fileName_

		Dim headerFile
		Set headerFile = fileSystem.OpenTextFile(fileName_, 2, True, 0)

		Dim notes
		If IsNull(description(0)) = False Then
			notes = Replace(description(0), vbLf, vbLf & "//")
		Else
			notes = ""
		End If

		headerFile.WriteLine("// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-")
		headerFile.WriteLine("// vi:set ts=4 sw=4:")
		headerFile.WriteLine("//")
		headerFile.WriteLine("// !!!This file is AUTOMATICALY GENERATED. DO NOT EDIT!!!")
		headerFile.WriteLine("//")
		headerFile.WriteLine("// Number" & name & ".h -- " & notes)
		headerFile.WriteLine("//")
		headerFile.WriteLine("// Copyright (c) 2000, 2001 Ricoh Company, Ltd.")
		headerFile.WriteLine("// All rights reserved.")
		headerFile.WriteLine("//")
		headerFile.WriteLine("")
		headerFile.WriteLine("#ifndef __SYDNEY_" & UCase(moduleName) & "_NUMBER" & UCase(name) & "_H")
		headerFile.WriteLine("#define __SYDNEY_" & UCase(moduleName) & "_NUMBER" & UCase(name) & "_H")
		headerFile.WriteLine("")
		headerFile.WriteLine("#include """ & moduleName & "/Module.h""")
		headerFile.WriteLine("#include """ & moduleName & "/ErrorNumber.h""")
		headerFile.WriteLine("")
		headerFile.WriteLine("_SYDNEY_BEGIN")
		headerFile.WriteLine("_SYDNEY_" & UCase(moduleName) & "_BEGIN")
		headerFile.WriteLine("")
		headerFile.WriteLine("namespace ErrorNumber {")
		headerFile.WriteLine("//")
		headerFile.WriteLine("//	CONST")
		headerFile.WriteLine("//	" & moduleName & "::ErrorNumber" & name & "-- " & notes)
		headerFile.WriteLine("//")
		headerFile.WriteLine("//	NOTES")
		headerFile.WriteLine("")
		headerFile.WriteLine("const Type " & name & " = " & number & ";")
		headerFile.WriteLine("}")
		headerFile.WriteLine("")
		headerFile.WriteLine("_SYDNEY_" & Ucase(moduleName) & "_END")
		headerFile.WriteLine("_SYDNEY_END")
		headerFile.WriteLine("")
		headerFile.WriteLine("#endif //__SYDNEY_" & Ucase(moduleName) & "_NUMBER" & UCase(name) & "_H")
		headerFile.WriteLine("//")
		headerFile.WriteLine("//	Copyright (c) 2000, 2001 Ricoh Company, Ltd.")
		headerFile.WriteLine("//	All rights reserved.")
		headerFile.WriteLine("//")
		headerFile.Close()
	End Sub

	Sub createHeaderFile(project_, fileName_)
		'プロジェクトにファイルを加える
		AddHeaderFile project_, fileName_

		Dim headerFile
		Set headerFile = fileSystem.OpenTextFile(fileName_, 2, True, 0)

		Dim notes
		If IsNull(description(0)) = False Then
			notes = Replace(description(0), vbLf, vbLf & "//")
		Else
			notes = ""
		End If

		headerFile.WriteLine("// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-")
		headerFile.WriteLine("// vi:set ts=4 sw=4:")
		headerFile.WriteLine("//")
		headerFile.WriteLine("// !!!This file is AUTOMATICALY GENERATED. DO NOT EDIT!!!")
		headerFile.WriteLine("//")
		headerFile.WriteLine("// " & name & ".h -- " & notes)
		headerFile.WriteLine("//")
		headerFile.WriteLine("// Copyright (c) 2000, 2001 Ricoh Company, Ltd.")
		headerFile.WriteLine("// All rights reserved.")
		headerFile.WriteLine("//")
		headerFile.WriteLine("")
		headerFile.WriteLine("#ifndef __SYDNEY_" & Ucase(moduleName) & "_" & UCase(name) & "_H")
		headerFile.WriteLine("#define __SYDNEY_" & Ucase(moduleName) & "_" & UCase(name) & "_H")
		headerFile.WriteLine("")
		headerFile.WriteLine("#include """ & moduleName & "/Module.h""")
		headerFile.WriteLine("#include """ & moduleName & "/Object.h""")
		If name = "ModError" Then
			headerFile.WriteLine("#include ""ModException.h""")
		End If
		headerFile.WriteLine("")
		headerFile.WriteLine("_SYDNEY_BEGIN")
		headerFile.WriteLine("_SYDNEY_" & Ucase(moduleName) & "_BEGIN")
		headerFile.WriteLine("")
		headerFile.WriteLine("//")
		headerFile.WriteLine("//	CLASS")
		headerFile.WriteLine("//	" & moduleName & "::" & name & "-- " & notes)
		headerFile.WriteLine("//")
		headerFile.WriteLine("//	NOTES")
		headerFile.WriteLine("")
		headerFile.WriteLine("class SYD_EXCEPTION_FUNCTION " & name & " : public Object")
		headerFile.WriteLine("{")
		headerFile.WriteLine("public:")
		'writeHeaderFunction headerFile, ""
		writeHeaderFunction headerFile, "Object"
		writeHeaderFunction headerFile, "char"
		'writeHeaderFunction headerFile, "ModUnicodeChar"
		headerFile.WriteLine("};")
		headerFile.WriteLine("")
		headerFile.WriteLine("_SYDNEY_" & Ucase(moduleName) & "_END")
		headerFile.WriteLine("_SYDNEY_END")
		headerFile.WriteLine("")
		headerFile.WriteLine("#endif //__SYDNEY_" & Ucase(moduleName) & "_" & UCase(name) & "_H")
		headerFile.WriteLine("//")
		headerFile.WriteLine("//	Copyright (c) 2000, 2001 Ricoh Company, Ltd.")
		headerFile.WriteLine("//	All rights reserved.")
		headerFile.WriteLine("//")
		headerFile.Close()
	End Sub

	Sub createCppFile(project_, fileName_)
		'プロジェクトにファイルを加える
		AddCppFile project_, fileName_

		Dim cppFile
		Set cppFile = fileSystem.OpenTextFile(fileName_, 2, True, 0)

		Dim notes
		If IsNull(description(0)) = False Then
			notes = Replace(description(0), vbLf, vbLf & "//")
		Else
			notes = ""
		End If

		cppFile.WriteLine("// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-")
		cppFile.WriteLine("// vi:set ts=4 sw=4:")
		cppFile.WriteLine("//")
		cppFile.WriteLine("// !!!This file is AUTOMATICALY GENERATED. DO NOT EDIT!!!")
		cppFile.WriteLine("//")
		cppFile.WriteLine("// " & name & ".cpp -- " & notes)
		cppFile.WriteLine("//")
		cppFile.WriteLine("// Copyright (c) 2000, 2001 Ricoh Company, Ltd.")
		cppFile.WriteLine("// All rights reserved.")
		cppFile.WriteLine("//")
		cppFile.WriteLine("//	$Author$")
		cppFile.WriteLine("//	$Date$")
		cppFile.WriteLine("//	$Revision$")
		cppFile.WriteLine("//")
		cppFile.WriteLine("")
		cppFile.WriteLine("namespace {")
		cppFile.WriteLine("const char rcsid[] = ""$Header$"";")
		cppFile.WriteLine("}")
		cppFile.WriteLine("")
		cppFile.WriteLine("#include ""SyDefault.h""")
		cppFile.WriteLine("#include """ & moduleName & "/Module.h""")
		cppFile.WriteLine("#include """ & moduleName & "/" & name & ".h""")
		cppFile.WriteLine("#include """ & moduleName & "/Number" & name & ".h""")
		cppFile.WriteLine("#include """ & moduleName & "/ErrorMessage.h""")
		If name = "ModError" Then
			cppFile.WriteLine("#include ""ModKanjiCode.h""")
			cppFile.WriteLine("#include ""ModUnicodeChar.h""")
			cppFile.WriteLine("#include ""ModUnicodeString.h""")
			cppFile.WriteLine("namespace {")
			cppFile.WriteLine("#ifdef	OS_WINDOWSNT4_0")
			cppFile.WriteLine("const ModKanjiCode::KanjiCodeType	_eLiteralCode = ModKanjiCode::shiftJis;")
			cppFile.WriteLine("#else")
			cppFile.WriteLine("const ModKanjiCode::KanjiCodeType	_eLiteralCode = ModKanjiCode::euc;")
			cppFile.WriteLine("#endif")
			cppFile.WriteLine("}")
		End If
		cppFile.WriteLine("")
		cppFile.WriteLine("_SYDNEY_USING")
		cppFile.WriteLine("_SYDNEY_" & Ucase(moduleName) & "_USING")
		cppFile.WriteLine("")
		'writeCppFunction cppFile, ""
		writeCppFunction cppFile, "Object"
		writeCppFunction cppFile, "char"
		'writeCppFunction cppFile, "ModUnicodeChar"
		cppFile.WriteLine("//")
		cppFile.WriteLine("//	Copyright (c) 2000, 2001 Ricoh Company, Ltd.")
		cppFile.WriteLine("//	All rights reserved.")
		cppFile.WriteLine("//")
	End Sub

	Sub writeHeaderFunction(file_, stringType_)
		file_.WriteLine("	//コンストラクタ")
		file_.Write("	" & name & "(")
		If stringType_ = "Object" Then
			file_.Write("const Object& cObject_")
		ElseIf stringType_ <> "" Then
			file_.WriteLine("const " & stringType_ & "* pszModuleName_,")
			file_.WriteLine("						   const " & stringType_ & "* pszFileName_,")
			file_.Write("						   int iLineNumber_")
			If argument(0) <> "" Then
				Dim i
				For i = 0 To maxArgument
					file_.WriteLine(",")
					Dim extraArgument
					extraArgument = createType(argument(i), i) & " " & createVariable(argument(i), i)
					file_.Write("						   " & extraArgument)
				Next
			End If
		End If
		file_.WriteLine(");")
	End Sub

	Sub writeCppFunction(file_, stringType_)
		file_.WriteLine("//")
		file_.WriteLine("//	FUNCTION public")
		file_.WriteLine("//	" & moduleName & "::" & name & "::" & name & "--")
		file_.WriteLine("//		コンストラクタ")
		file_.WriteLine("//")
		file_.WriteLine("//	NOTES")
		file_.WriteLine("//")
		file_.WriteLine("//	ARGUMENTS")
		If stringType_ = "" Then
			file_.WriteLine("//	なし")
		ElseIf stringType_ = "Object" Then
			file_.WriteLine("// const " & moduleName & "::Object& cObject_")
			file_.WriteLine("//		内容をコピーする基底クラス")
		Else
			file_.WriteLine("//	const " & stringType_ & "* pszModuleName_")
			file_.WriteLine("//		例外が発生したモジュール名")
			file_.WriteLine("//	const " & stringType_ & "* pszFileName_")
			file_.WriteLine("//		例外が発生したファイル名")
			file_.WriteLine("//	int iLineNumber_")
			file_.WriteLine("//		例外が発生した行番号")
			file_.WriteLine("//	(この例外固有の引数)")
		End If
		file_.WriteLine("//")
		file_.WriteLine("//	RETURN")
		file_.WriteLine("//	なし")
		file_.WriteLine("//")
		file_.WriteLine("//	EXCEPTIONS")
		file_.WriteLine("//	なし")
		file_.WriteLine("//")
		file_.WriteLine(name & "::")
		file_.Write(name & "(")
		If stringType_ = "Object" Then
			file_.Write("const Object& cObject_")
		ElseIf stringType_ <> "" Then
			file_.WriteLine("")
			file_.WriteLine("	const " & stringType_ & "* pszModuleName_,")
			file_.WriteLine("	const " & stringType_ & "* pszFileName_,")
			file_.Write("	int iLineNumber_")
			If argument(0) <> "" Then
				Dim i
				For i = 0 To maxArgument
					file_.WriteLine(",")
					Dim extraArgument
					extraArgument = createType(argument(i), i) & " " & createVariable(argument(i), i)
					file_.Write("	" & extraArgument)
				Next
			End If
		End If
		file_.WriteLine(")")
		file_.Write(": Object(")
		If stringType_ = "" Then
			file_.Write("ErrorNumber::" & name)
		ElseIf stringType_ = "Object" Then
			file_.Write("cObject_")
		ElseIf stringType_ <> "" Then
			file_.WriteLine("ErrorNumber::" & name & ",")
			file_.WriteLine("			pszModuleName_,")
			file_.WriteLine("			pszFileName_,")
			file_.Write("			iLineNumber_")
		End If
		file_.WriteLine(")")
		file_.WriteLine("{")
		If stringType_ <> "" and stringType_ <> "Object" Then
			file_.WriteLine("	//エラーメッセージ引数を作成する")
			If name = "ModError" Then
				file_.WriteLine("	ModException& cException = const_cast<ModException&>(" & createVariable(argument(0), 0) & ");")
				file_.WriteLine("	ModUnicodeString strMsg(cException.setMessage(),")
				file_.WriteLine("			0, _eLiteralCode);")
			End If
			file_.WriteLine("	ErrorMessage::makeMessageArgument(getErrorMessageArgument(),")
			file_.Write("									  getErrorNumber()")
			If name = "ModError" Then
				file_.WriteLine(",")
				file_.Write("									  (const ModUnicodeChar*)strMsg")
			ElseIf argument(0) <> "" Then
				For i = 0 To maxArgument
					file_.WriteLine(",")
					Dim extraVariable
					extraVariable = createVariable(argument(i), i)
					file_.Write("									  " & extraVariable)
				Next
			End If
			file_.WriteLine(");")
			file_.WriteLine("")
		End If
		file_.WriteLine("}")
		file_.WriteLine("")
	End Sub

	Function createType(argument_, index_)
		If name = "ModError" and index_ = 0 Then
			createType = "const ModException&"
			Exit Function
		End If

		Dim result
		Dim digit, digit2, digit3
		digit = Right(argument_, 1)
		digit2 = Left(Right(argument_, 2), 1)
		digit3 = Left(Right(argument_, 3), 1)

		If digit = "s" Then
			result = "const ModUnicodeChar* "
		Else
			If digit2 = "h" Then
				If digit = "d" or digit = "i" Then
					result = "short "
				ElseIf digit = "u" or digit = "o" or digit = "x" or digit = "X" Then
					result = "unsigned short "
				Else
					WScript.Echo definitionFile & " : error: Unknown format: " & argument_
					createType = ""
					Exit Function
				End If
			ElseIf digit2 = "l" Then
				If digit3 = "l" Then
					If digit = "d" or digit = "i" Then
						result = "ModInt64 "
					ElseIf digit = "u" or digit = "o" or digit = "x" or digit = "X" Then
						result = "ModInt64 "
					Else
						WScript.Echo definitionFile & " : error: Unknown format: " & argument_
						createType = ""
						Exit Function
					End If
				Else
					If digit = "d" or digit = "i" Then
						result = "long "
					ElseIf digit = "u" or digit = "o" or digit = "x" or digit = "X" Then
						result = "unsigned long "
					Else
						WScript.Echo definitionFile & " : error: Unknown format: " & argument_
						createType = ""
						Exit Function
					End If
				End If
			Else
				If digit = "d" or digit = "i" Then
					result = "int "
				ElseIf digit = "u" or digit = "o" or digit = "x" or digit = "X" Then
					result = "unsigned int "
				Else
					WScript.Echo definitionFile & " : error: Unknown format: " & argument_
					createType = ""
					Exit Function
				End If
   			End If
		End If
		createType = result
	End Function

	Function createVariable(argument_, index_)
		If name = "ModError" and index_ = 0 Then
			createVariable = "cException0_"
			Exit Function
		End If

		Dim result
		Dim digit
		digit = Right(argument_, 1)

		If digit = "s" Then
			result = "pszStrArg" & index_ & "_"
		Else
			If digit = "d" or digit = "i" Then
				result = "iIntArg" & index_ & "_"
			ElseIf digit = "u" or digit = "o" or digit = "x" or digit = "X" Then
				result = "uIntArg" & index_ & "_"
			Else
				WScript.Echo definitionFile & " : error: Unknown format: " & argument_
				createVariable = ""
				Exit Function
			End If
		End If
		createVariable = result
	End Function

End Class

Sub WriteMessageFile(project_)
	Dim file, fileName

	fileName = folderName & "/AllNumberFiles.h"

	'プロジェクトにファイルを加える
	AddHeaderFile project_, fileName

	Set file = fileSystem.OpenTextFile(fileName, 2, True, 0)
	file.WriteLine(AllNumberFiles)
	file.Close

	fileName = folderName & "/AllExceptionFiles.h"

	'プロジェクトにファイルを加える
	AddHeaderFile project_, fileName

	Set file = fileSystem.OpenTextFile(fileName, 2, True, 0)
	file.WriteLine(AllExceptionFiles)
	file.Close

	fileName = folderName & "/ThrowClassInstance.h"

	'プロジェクトにファイルを加える
	AddHeaderFile project_, fileName

	Set file = fileSystem.OpenTextFile(fileName, 2, True, 0)
	file.WriteLine(ThrowClassInstance)
	file.Close

	fileName = folderName & "/MessageArgument.h"

	'プロジェクトにファイルを加える
	AddHeaderFile project_, fileName

	Set file = fileSystem.OpenTextFile(folderName & "/MessageArgument.h", 2, True, 0)
	file.WriteLine(MessageArgument)
	file.Close

	Dim l
	For l = 0 To maxLanguage

		fileName = folderName & "/MessageFormat" & Languages(l) & ".h"

		'プロジェクトにファイルを加える
		AddHeaderFile project_, fileName

		Set file = fileSystem.OpenTextFile(fileName, 2, True, 0)
		file.WriteLine(MessageFormat(l))
		file.Close
	Next
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
	Dim application
	Set application = WScript.CreateObject("msdev.application")
	application.Documents.Open projectFileName
	Dim project
	Set project = application.Projects("" & moduleName)

	'-----------------------------------------------------
	'前回生成に使用したXMLファイルのバックアップを読み込む
	'-----------------------------------------------------
	Dim xmlBak
	Set xmlBak = CreateObject("microsoft.xmldom")
	xmlDoc.async = False
	Dim bakList
	If xmlBak.load(definitionFile & "_") Then
		Set bakList = xmlBak.selectNodes("ErrorDefine/Error/Name")
	End If

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

		If errorDef.parse(errorNode, bakList) = False Then
			noNeed = noNeed & " " & errorDef.name
		Else
			generated = generated & " " & errorDef.name
			errorDef.write(project)
		End If
	Next
	'------------------------------------------------------------------
	'すべて読み終わったらMessageFormatとMessageArgumentをファイルに書く
	'------------------------------------------------------------------
	WriteMessageFile(project)
	application.Documents.SaveAll
	application.Quit

	'メッセージダイアログを出すとうざいのでコメントアウト
	'If noNeed <> "" Then
		'MsgBox "以下のクラスは変更がありませんでした。" & vbNewLine & noNeed, vbInformation, "変更のなかったクラス"
	'End If
	If generated <> "" Then
		WScript.Echo "以下のクラスについてファイルを作成しました。" & vbNewLine & generated
	End If
	'タイムスタンプを新しくする
	xmlDoc.Save(definitionFile & "_")

	'アプリケーションをdisconnectする
	WScript.DisconnectObject application

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

Sub AddHeaderFile(project_, fileName_)
	On Error Resume Next
	project_.AddFile fileName_
	If Err.Number = 0 Then

		'ファイルが追加された

		If projectFileIsReadOnly Then

			'読み込み専用なのにファイルが追加されたらエラー

			On Error Goto 0
			ErrorCode = 17
			project_.Application.Quit
			Err.Raise ErrorCode, ": error : 新しい例外を追加したらまずプロジェクトファイルをチェックアウトしてください。"
		End If

		'---------------------------------------
		'Installの構成にカスタムビルドを設定する
		'---------------------------------------
		Dim configuration
		Set configuration = project_.Configurations(project_.Name & " - Win32 Install")
		configuration.AddCustomBuildStepToFile fileName_, "copy $(InputPath) ..\..\include\$(InputPath)", "..\..\include\$(InputPath)", "ヘッダーファイルのインストール中 - $(InputPath)"

		projectFileIsChanged = True

	ElseIf Err.Number <> -2147220728 Then	'すでにあることによる失敗(-2147220728)は無視する
		If Err.Number <> IgnoredError Then
			MsgBox "ファイルの追加に失敗しました(" & Err.Number & "): " & Err.Description & vbNewLine & "このエラーは無視します。 " & vbNewLine & "例外を追加したのにこのエラーが出たときは正しくビルドできません。", vbExclamation, "注意"
			IgnoredError = Err.Number
		End If
	End If
	Err.Clear
	On Error Goto 0
End Sub

Sub AddCppFile(project_, fileName_)
	On Error Resume Next
	project_.AddFile fileName_
	If Err.Number = 0 Then
		If projectFileIsReadOnly Then

			'読み込み専用なのにファイルが追加されたらエラー

			On Error Goto 0
			ErrorCode = 17
			project_.Application.Quit
			Err.Raise ErrorCode, ": error: 新しい例外を追加したらまずプロジェクトファイルをチェックアウトしてください。"
		End If

		projectFileIsChanged = True

	ElseIf Err.Number <> -2147220728 Then	'すでにあることによる失敗(-2147220728)は無視する
		If Err.Number <> IgnoredError Then
			MsgBox "ファイルの追加に失敗しました(" & Err.Number & "): " & Err.Description & vbNewLine & "このエラーは無視します。 " & vbNewLine & "例外を追加したのにこのエラーが出たときは正しくビルドできません。", vbExclamation, "注意"
			IgnoredError = Err.Number
		End If
	End If
	Err.Clear
End Sub
