/*
 * Makefile.c --- 
 * 
 * Copyright (c) 2022, 2023 Ricoh Company, Ltd.
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

SUBDIRS =

/* install headers */
EXPORT_HDRS =

HDRS = \
	UnicodeCharacterType.h \
	UnicodeDataFile.h \
	UnicodeDataRow.h \
	UnicodeDataRowBlocks.h \
	UnicodeDataRowCreater.h \
	UnicodeDataRowTypes.h \
	UnicodeDataRowUnicodeData.h

SRCS = \
	UnicodeCharacterType.cpp \
	UnicodeDataFile.cpp \
	UnicodeDataRow.cpp \
	UnicodeDataRowBlocks.cpp \
	UnicodeDataRowCreater.cpp \
	UnicodeDataRowUnicodeData.cpp \
	maketable.cpp

OBJS = \
	UnicodeCharacterType$O \
	UnicodeDataFile$O \
	UnicodeDataRow$O \
	UnicodeDataRowBlocks$O \
	UnicodeDataRowCreater$O \
	UnicodeDataRowUnicodeData$O \
	maketable$O

EXTRACFLAGS = -I$(MODTOP)$(S)m.common$(S)include
EXTRALDFLAGS = 
EXTRAPURIFYFLAGS =
EXTRAQUANTIFYFLAGS =
EXTRAPURECOVFLAGS =
EXTRALOCALCFLAGS =
EXTRALOCALLDFLAGS = 

PROGRAMS = \
	$(PROGRAM01)

PROGRAM01 = maketable$E

ALLTARGETS = $(PROGRAMS)

ProgramTarget($(PROGRAM01), $(OBJS))

/*
 * all
 */
AllTarget($(ALLTARGETS))

/*
 * clean
 */
CleanTarget($(ALLTARGETS))
CleanTarget($(OBJS))

#include "Makefile.h"
