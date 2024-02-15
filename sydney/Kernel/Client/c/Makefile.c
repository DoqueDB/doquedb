/*
 * Makefile.c --- Kernel/Client/c
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

MODULE = Client
TARGET_BASE = SyClient
SRCDIR = ..
HDRDIR = ../$(MODULE)
MESSAGE_DEFINITION =
MESSAGE_TARGET =

LOCAL_EXPORT_HDRDIR = ../../include/$(MODULE)
TOP_EXPORT_HDRDIR = ../../../include/$(MODULE)
TOP_INSTALL_DIR = ../../../c.CONF
PACKAGE_HDRDIR = $(TOP_INSTALL_DIR)/$(PACKAGE_NAME)/include/$(MODULE)

/* above variables MUST be defined      */
/****************************************/

/* headers created for message */
MESSAGE_HDRS =

/* headers installed in tape image */
PACKAGE_HDRS = \
	$(HDRDIR)/DataSource.h \
	$(HDRDIR)/Module.h \
	$(HDRDIR)/Object.h \
	$(HDRDIR)/PrepareStatement.h \
	$(HDRDIR)/ResultSet.h \
	$(HDRDIR)/Session.h \
	$(HDRDIR)/Singleton.h

/* headers installed in Kernel */
LOCAL_EXPORT_HDRS =

/* headers installed in TOP */
TOP_EXPORT_HDRS = \
	$(PACKAGE_HDRS) \
	$(MESSAGE_HDRS)

HDRS = \
	$(TOP_EXPORT_HDRS) \
	$(LOCAL_EXPORT_HDRS) \
	$(HDRDIR)/Connection.h \
	$(HDRDIR)/Parameter.h \
	$(HDRDIR)/Port.h

MESSAGE_OBJS =

OBJS = \
	$(MESSAGE_OBJS) \
	Client$O \
	Connection$O \
	DataSource$O \
	Object$O \
	Port$O \
	PrepareStatement$O \
	ResultSet$O \
	Session$O \
	Singleton$O

#ifdef SYD_DLL
EXPORTFLAGS = \
	-DSYD_EXPORT_FUNCTION \
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

TARGET = $(MODULE).ol

ALLTARGETS = \
	$(MESSAGE_TARGET) \
	$(TARGET)

/*
 * all
 */
AllTarget($(ALLTARGETS))
clientall:: all	

ObjectListTarget($(TARGET), $(OBJS), $(TOP_INSTALL_DIR))
clientobjlist:: objlist

/*
 * message
 */
/* MessageTarget($(MESSAGE_TARGET), $(MESSAGE_DEFINITION), $(BUILD_JAR)) */

/*
 * install library and header
 */
InstallHeaderTarget($(TOP_EXPORT_HDRS), $(TOP_EXPORT_HDRDIR))
/* InstallHeaderTarget($(LOCAL_EXPORT_HDRS), $(LOCAL_EXPORT_HDRDIR)) */

/*
 * install header for package
 */
TapeHeaderTarget($(PACKAGE_HDRS), $(PACKAGE_HDRDIR))

/*
 * clean
 */
CleanTarget($(ALLTARGETS))
CleanTarget($(OBJS))

#include "Makefile.h"

/*
  Copyright (c) 1999, 2023 Ricoh Company, Ltd.
  All rights reserved.
*/
