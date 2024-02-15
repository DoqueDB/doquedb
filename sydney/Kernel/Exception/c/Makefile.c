/*
 * Makefile.c --- Kernel/Exception/c
 * 
 * Copyright (c) 1999, 2023 Ricoh Company, Ltd.
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
CLIENTSUBDIRS =

/****************************************/
/* following variables MUST be defined  */

MODULE = Exception
TARGET_BASE = SyExcept
SRCDIR = ..
HDRDIR = ../$(MODULE)
MESSAGE_DEFINITION = $(SRCDIR)/ErrorDefinition.xml
MESSAGE_TARGET = $(MESSAGE_DEFINITION)_

LOCAL_EXPORT_HDRDIR = ../../include/$(MODULE)
TOP_EXPORT_HDRDIR = ../../../include/$(MODULE)
TOP_INSTALL_DIR = ../../../c.CONF
PACKAGE_HDRDIR = $(TOP_INSTALL_DIR)/$(PACKAGE_NAME)/include/$(MODULE)

/* above variables MUST be defined      */
/****************************************/

/* headers created for message */
MESSAGE_HDRS =

/* headers installed in Kernel */
LOCAL_EXPORT_HDRS =

/* headers installed in TOP */
TOP_EXPORT_HDRS = \
	$(HDRDIR)/ErrorMessage.h \
	$(HDRDIR)/ErrorNumber.h \
	$(HDRDIR)/FakeError.h \
	$(HDRDIR)/Message.h \
	$(HDRDIR)/Module.h \
	$(HDRDIR)/Object.h \
	$(HDRDIR)/SystemLevel.h \
	$(HDRDIR)/UserLevel.h

/* headers installed in package */
PACKAGE_HDRS = \
	$(HDRDIR)/ErrorNumber.h \
	$(HDRDIR)/Message.h \
	$(HDRDIR)/Module.h \
	$(HDRDIR)/Object.h \
	$(HDRDIR)/SystemLevel.h \
	$(HDRDIR)/UserLevel.h

HDRS = \
	$(TOP_EXPORT_HDRS) \
	$(LOCAL_EXPORT_HDRS) \
	$(HDRDIR)/AutoCriticalSection.h

MESSAGE_OBJS =

OBJS = \
	$(MESSAGE_OBJS) \
	ErrorMessage$O \
	Exception$O \
	FakeError$O \
	Object$O \
	UserLevel$O \
	SystemLevel$O

#ifdef SYD_DLL
EXPORTFLAGS = \
	-DSYD_KERNEL_EXPORT_FUNCTION
#endif
EXTRACFLAGS = \
	$(EXPORTFLAGS)
EXTRALDFLAGS =
EXTRAPURIFYFLAGS =
EXTRAQUANTIFYFLAGS =
EXTRAPURECOVFLAGS =
EXTRALOCALCFLAGS =

/********************************************************/

#ifdef OS_RHLINUX6_0
.NOTPARALLEL:
#endif

TARGET = $(MODULE).ol

#ifndef SYD_COVERAGE
ALLTARGETS = \
	message installh \
	$(TARGET)
#else
ALLTARGETS = \
	$(TARGET)
#endif

objlist:: $(MESSAGE_TARGET)
clientobjlist:: objlist

/*
 * all
 */
AllTarget($(ALLTARGETS))
clientall:: all	

/*
 * ObjectListTarget
 */
#include "Makefile_object.h"

/*
 * message
 */
/* this line should be placed in this order */
message:: message-java message-perl
MessageTarget($(MESSAGE_TARGET), $(MESSAGE_DEFINITION), $(BUILD_JAR))
message:: $(CVSIGNORE)

/*
 * install library and header
 */
installh:: install-java install-perl
InstallHeaderTarget($(TOP_EXPORT_HDRS), $(TOP_EXPORT_HDRDIR))
#include "Makefile_header.h"
#include "Makefile_number.h"

/* InstallHeaderTarget($(LOCAL_EXPORT_HDRS), $(LOCAL_EXPORT_HDRDIR)) */

/*
 * install header for package
 */
TapeHeaderTarget($(PACKAGE_HDRS), $(PACKAGE_HDRDIR))

/*
 * clean
 */
CleanTarget($(TARGET))
CleanTarget($(MESSAGE_TARGET))
CleanTarget($(OBJS))

/*
 * message for java
 */
message-java:
	-@$(MKDIR) ../Java
	$(MKMESSAGE)ForJava $(MODULE) $(MESSAGE_DEFINITION) $(CHARSET)
JAVAFILES = *.java
install-java:
	@$(INSTALL) $(INSTALLINCLFLAGS) ../Java/$(JAVAFILES) $(SYDTOP)/Java/JDBC/src/jp/co/ricoh/doquedb/exception/

/*
 * message for perl
 */
PERLDIR = ../Perl
PERLFILE = $(PERLDIR)/ErrorDef.pl
message-perl: $(PERLFILE)
$(PERLFILE): $(MESSAGE_DEFINITION)
	-@$(MKDIR) $(PERLDIR)
	$(MKMESSAGE)ForPerl $(MODULE) $(MESSAGE_DEFINITION) $(CHARSET)
install-perl:
	@$(INSTALL) $(INSTALLINCLFLAGS) $(PERLFILE) $(SYDTOP)/Perl/Net/

#include "Makefile.h"

/*
  Copyright (c) 1999, 2023 Ricoh Company, Ltd.
  All rights reserved.
*/
