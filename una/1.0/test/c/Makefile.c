/*
 * Makefile.c ---
 * 
 * Copyright (c) 2005, 2022, 2023 Ricoh Company, Ltd.
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

LOCALLDFLAGS= -luna -lenstem -lunajp
SUBDIRS =

/*
 * test target directory (ie object library location)
 */
TESTTOPDIR = $(UNATOP)/test
TESTLIBDIR = $(UNATOP)/c.CONF
UNAJPOBJDIR = $(UNATOP)/m.una/ModNlpUnaJp/c.CONF

TESTLIB = $(TESTLIBDIR)/libuna$(D)
TESTLIB1 = $(TESTLIBDIR)/libunajp$(D)
TESTLIB5 = $(TESTLIBDIR)/libenstem$(D)

HDRS = datapath.h
SRCS =
OBJS =

/*
 * the order is ... EXTRALOCALXXX system-default EXTRACFALGS
 */
/* test target header (ie include dirs) */
EXTRACFLAGS = \
	-I$(TESTTOPDIR)/../m.una/include \
	-DMOD_IMPORT_COMMON_DLL \
	-DUNA_DLL

/* test target library (ie obj lib path and name) */
EXTRALDFLAGS =
EXTRAPURIFYFLAGS =
EXTRAQUANTIFYFLAGS =
EXTRAPURECOVFLAGS =
EXTRALOCALCFLAGS =
EXTRALOCALLDFLAGS =

TEST0 = norm$(E)
OBJS0 = norm$(O)

TEST1 = stem$(E)
OBJS1 = stem$(O)

TEST2 = una$(E)
OBJS2 = una$(O)

TEST3 = unanorm$(E)
OBJS3 = unanorm$(O)

TEST4 = nlpnorm$(E)
OBJS4 = nlpnorm$(O)

TEST5 = nlpblock$(E)
OBJS5 = nlpblock$(O)

TEST6 = unacore$(E)
OBJS6 = unacore$(O)

TEST7 = notcallfunc$(E)
OBJS7 = notcallfunc$(O)

TEST8 = nlpthread$(E)
OBJS8 = nlpthread$(O)

TEST9 = nlpnull$(E)
OBJS9 = nlpnull$(O)

TEST10 = nlpconcept$(E)
OBJS10 = nlpconcept$(O)

TEST11 = nlptest$(E)
OBJS11 = nlptest$(O)

TEST12 = nlpexpstr$(E)
OBJS12 = nlpexpstr$(O)

TEST13 = unaversion$(E)
OBJS13 = unaversion$(O)

NORMTARGETS = $(TEST0)
STEMTARGETS = $(TEST1)
UNATARGETS = $(TEST2) $(TEST3) $(TEST6) $(TEST7) $(TEST9) $(TEST12) $(UNA2TARGETS)
UNA2TARGETS = $(TEST4) $(TEST5) $(TEST8) $(TEST10)
ALLTARGETS = $(NORMTARGETS) $(STEMTARGETS) $(UNATARGETS) $(UNA2TARGETS) $(TEST13)

/*
 * all
 */
AllTarget($(ALLTARGETS))

ProgramTarget($(TEST0), $(OBJS0) $(TESTLIB1))
ProgramTarget($(TEST1), $(OBJS1) $(TESTLIB1) $(TESTLIB5))
ProgramTarget($(TEST2), $(OBJS2) $(TESTLIB1))
ProgramTarget($(TEST3), $(OBJS3) $(TESTLIB1) $(TESTLIB5))
ProgramTarget($(TEST4), $(OBJS4) $(TESTLIB))
ProgramTarget($(TEST5), $(OBJS5) $(TESTLIB))
ProgramTarget($(TEST6), $(OBJS6) $(TESTLIB1))
ProgramTarget($(TEST7), $(OBJS7) $(TESTLIB1) $(TESTLIB5))
ProgramTarget($(TEST8), $(OBJS8) $(TESTLIB))
ProgramTarget($(TEST9), $(OBJS9) $(TESTLIB1))
ProgramTarget($(TEST10), $(OBJS10) $(TESTLIB))
ProgramTarget($(TEST11), $(OBJS11) $(TESTLIB))
ProgramTarget($(TEST12), $(OBJS12) $(TESTLIB))
ProgramTarget($(TEST13), $(OBJS13) $(TESTLIB))

/*
 * clean
 */
CleanTarget($(ALLTARGETS) $(OBJS) *$O *Test.* *.a *.so)

/************/

BINSH = /bin/bash
DATE = /bin/date

SCRIPT_DIR=../src
ALL=*.*

test-setup: testlib-setup testdata-setup

testlib-setup:
	-$(CP) $(TESTLIBDIR)/lib*$(D) ./
	-$(CP) $(MOD_LIBDIR)/lib*$(D) ./

testdata-setup:
	-$(MKDIR) ../unadic-n/
	-$(CP) ../unadic/norm/$(ALL) ../unadic-n/
	-$(CP) ../unadic/stem/$(ALL) ../unadic-n/
	-$(MKDIR) ../errdic.norm1/norm/
	-$(CP) ../unadic/norm/$(ALL) ../errdic.norm1/norm/
	-$(MV) ../errdic.norm1/norm/combiMap.dat ../errdic.norm1/norm/combiMap.bak
	-$(CP) ../unadic/unaparam.dat ../errdic.norm1/
	-$(MKDIR) ../errdic.norm2/norm/
	-$(CP) ../unadic/norm/$(ALL) ../errdic.norm2/norm/
	-$(MV) ../errdic.norm2/norm/connect.tbl ../errdic.norm2/norm/connect.bak
	-$(CP) ../unadic/unaparam.dat ../errdic.norm2/
	-$(MKDIR) ../errdic.norm3/norm/
	-$(CP) ../unadic/norm/$(ALL) ../errdic.norm3/norm/
	-$(MV) ../errdic.norm3/norm/expApp.dic ../errdic.norm3/norm/expApp.bak
	-$(CP) ../unadic/unaparam.dat ../errdic.norm3/
	-$(MKDIR) ../errdic.norm4/norm/
	-$(CP) ../unadic/norm/$(ALL) ../errdic.norm4/norm/
	-$(MV) ../errdic.norm4/norm/expWrd.dic ../errdic.norm4/norm/expWrd.bak
	-$(CP) ../unadic/unaparam.dat ../errdic.norm4/
	-$(MKDIR) ../errdic.norm5/norm/
	-$(CP) ../unadic/norm/$(ALL) ../errdic.norm5/norm/
	-$(MV) ../errdic.norm5/norm/postMap.dat ../errdic.norm5/norm/postMap.bak
	-$(CP) ../unadic/unaparam.dat ../errdic.norm5/
	-$(MKDIR) ../errdic.norm6/norm/
	-$(CP) ../unadic/norm/$(ALL) ../errdic.norm6/norm/
	-$(MV) ../errdic.norm6/norm/preMap.dat ../errdic.norm6/norm/preMap.bak
	-$(CP) ../unadic/unaparam.dat ../errdic.norm6/
	-$(MKDIR) ../errdic.norm7/norm/
	-$(CP) ../unadic/norm/$(ALL) ../errdic.norm7/norm/
	-$(MV) ../errdic.norm7/norm/ruleApp.dic ../errdic.norm7/norm/ruleApp.bak
	-$(CP) ../unadic/unaparam.dat ../errdic.norm7/
	-$(MKDIR) ../errdic.norm8/norm/
	-$(CP) ../unadic/norm/$(ALL) ../errdic.norm8/norm/
	-$(MV) ../errdic.norm8/norm/ruleWrd.dic ../errdic.norm8/norm/ruleWrd.bak
	-$(CP) ../unadic/unaparam.dat ../errdic.norm8/
	-$(MKDIR) ../errdic.norm9/norm/
	-$(CP) ../unadic/norm/$(ALL) ../errdic.norm9/norm/
	-$(MV) ../errdic.norm9/norm/unkcost.tbl ../errdic.norm9/norm/unkcost.bak
	-$(CP) ../unadic/unaparam.dat ../errdic.norm9/
	-$(MKDIR) ../errdic.norm10/norm/
	-$(CP) ../unadic/norm/$(ALL) ../errdic.norm10/norm/
	-$(MV) ../errdic.norm10/norm/unkmk.tbl ../errdic.norm10/norm/unkmk.bak
	-$(CP) ../unadic/unaparam.dat ../errdic.norm10/
	-$(MKDIR) ../errdic.una1/una/
	-$(CP) ../unadic/una/$(ALL) ../errdic.una1/una/
	-$(MV) ../errdic.una1/una/connect.tbl ../errdic.una1/una/connect.bak
	-$(MKDIR) ../errdic.una1/norm/
	-$(CP) ../unadic/norm/$(ALL) ../errdic.una1/norm/
	-$(CP) ../unadic/unaparam.dat ../errdic.una1/
	-$(MKDIR) ../errdic.una2/una/
	-$(CP) ../unadic/una/$(ALL) ../errdic.una2/una/
	-$(MV) ../errdic.una2/una/engmk.tbl ../errdic.una2/una/engmk.bak
	-$(MKDIR) ../errdic.una2/norm/
	-$(CP) ../unadic/norm/$(ALL) ../errdic.una2/norm/
	-$(CP) ../unadic/unaparam.dat ../errdic.una2/
	-$(MKDIR) ../errdic.una3/una/
	-$(CP) ../unadic/una/$(ALL) ../errdic.una3/una/
	-$(MV) ../errdic.una3/una/unastd.tbl ../errdic.una3/una/unastd.bak
	-$(MKDIR) ../errdic.una3/norm/
	-$(CP) ../unadic/norm/$(ALL) ../errdic.una3/norm/
	-$(CP) ../unadic/unaparam.dat ../errdic.una3/
	-$(MKDIR) ../errdic.una4/una/
	-$(CP) ../unadic/una/$(ALL) ../errdic.una4/una/
	-$(MV) ../errdic.una4/una/unawrd.dic ../errdic.una4/una/unawrd.bak
	-$(MKDIR) ../errdic.una4/norm/
	-$(CP) ../unadic/norm/$(ALL) ../errdic.una4/norm/
	-$(CP) ../unadic/unaparam.dat ../errdic.una4/
	-$(MKDIR) ../errdic.una5/una/
	-$(CP) ../unadic/una/$(ALL) ../errdic.una5/una/
	-$(MV) ../errdic.una5/una/unkcost.tbl ../errdic.una5/una/unkcost.bak
	-$(MKDIR) ../errdic.una5/norm/
	-$(CP) ../unadic/norm/$(ALL) ../errdic.una5/norm/
	-$(CP) ../unadic/unaparam.dat ../errdic.una5/
	-$(MKDIR) ../errdic.una6/una/
	-$(CP) ../unadic/una/$(ALL) ../errdic.una6/una/
	-$(MV) ../errdic.una6/una/unkmk.tbl ../errdic.una6/una/unkmk.bak
	-$(MKDIR) ../errdic.una6/norm/
	-$(CP) ../unadic/norm/$(ALL) ../errdic.una6/norm/
	-$(CP) ../unadic/unaparam.dat ../errdic.una6/

testdata-clean:
	-$(RMALL) ../unadic-n/
	-$(RMALL) ../errdic.norm1/
	-$(RMALL) ../errdic.norm2/
	-$(RMALL) ../errdic.norm3/
	-$(RMALL) ../errdic.norm4/
	-$(RMALL) ../errdic.norm5/
	-$(RMALL) ../errdic.norm6/
	-$(RMALL) ../errdic.norm7/
	-$(RMALL) ../errdic.norm8/
	-$(RMALL) ../errdic.norm9/
	-$(RMALL) ../errdic.norm10/
	-$(RMALL) ../errdic.una1/
	-$(RMALL) ../errdic.una2/
	-$(RMALL) ../errdic.una3/
	-$(RMALL) ../errdic.una4/
	-$(RMALL) ../errdic.una5/
	-$(RMALL) ../errdic.una6/

NORMTESTS = norm_eucTest norm_uniTest norm_miscTest norm_errTest
STEMTESTS = stem_eucTest stem_uniTest stem_restemTest stem_errTest
UNATESTS = una_hyphenTest una_excTest una_kanaTest una_stdTest una_expTest una_modeTest \
           una_codeTest una_errTest una_originTest una_keywordTest una_threadTest \
           una_basicTest una_bugTest una_expStrTest una_multiTest una_errMultiTest

ALLTESTS = $(NORMTESTS) $(STEMTESTS) $(UNATESTS)

normtest: $(NORMTARGETS) $(NORMTESTS)
stemtest: $(STEMTARGETS) $(STEMTESTS)
unatest: $(UNATARGETS) $(UNA2TARGETS) $(UNATESTS)

test: $(ALLTARGETS) $(ALLTESTS)

norm_eucTest norm_uniTest norm_miscTest norm_errTest: $(TEST0)
	$(DATE) >  $@.out
	$(BINSH) $(SCRIPT_DIR)/$@.sh >> $@.out
	$(DATE) >> $@.out

stem_eucTest stem_uniTest stem_restemTest stem_errTest: $(TEST1)
	$(DATE) >  $@.out
	$(BINSH) $(SCRIPT_DIR)/$@.sh >> $@.out
	$(DATE) >> $@.out

una_excTest una_kanaTest una_stdTest: $(TEST2)
	$(DATE) >  $@.out
	$(BINSH) $(SCRIPT_DIR)/$@.sh >> $@.out
	$(DATE) >> $@.out

una_expTest una_basicTest: $(TEST3) $(TEST4)
	$(DATE) >  $@.out
	$(BINSH) $(SCRIPT_DIR)/$@.sh >> $@.out
	$(DATE) >> $@.out

una_errTest una_keywordTest una_bugTest: $(TEST4)
	$(DATE) >  $@.out
	$(BINSH) $(SCRIPT_DIR)/$@.sh >> $@.out
	$(DATE) >> $@.out

una_modeTest: $(TEST3) $(TEST4) $(TEST5)
	$(DATE) >  $@.out
	$(BINSH) $(SCRIPT_DIR)/$@.sh >> $@.out
	$(DATE) >> $@.out

una_hyphenTest : $(TEST6)
	$(DATE) >  $@.out
	$(BINSH) $(SCRIPT_DIR)/$@.sh >> $@.out
	$(DATE) >> $@.out

una_codeTest: $(TEST1) $(TEST4) $(TEST7)
	$(DATE) >  $@.out
	$(BINSH) $(SCRIPT_DIR)/$@.sh >> $@.out
	$(DATE) >> $@.out

una_originTest: $(TEST4) $(TEST5)
	$(DATE) >  $@.out
	$(BINSH) $(SCRIPT_DIR)/$@.sh >> $@.out
	$(DATE) >> $@.out

una_threadTest: $(TEST8)
	$(DATE) >  $@.out
	$(BINSH) $(SCRIPT_DIR)/$@.sh >> $@.out
	$(DATE) >> $@.out

una_expStrTest: $(TEST4) $(TEST8) $(TEST12)
	$(DATE) >  $@.out
	$(BINSH) $(SCRIPT_DIR)/$@.sh >> $@.out
	$(DATE) >> $@.out

una_multiTest: $(TEST5)
	$(DATE) >  $@.out
	$(BINSH) $(SCRIPT_DIR)/$@.sh >> $@.out
	$(DATE) >> $@.out

una_errMultiTest: $(TEST5)
	$(DATE) >  $@.out
	$(BINSH) $(SCRIPT_DIR)/$@.sh >> $@.out
	$(DATE) >> $@.out

unastd$O:
	$(CP) $(UNAJPOBJDIR)/unastd$(O) ./
unamorph$O:
	$(CP) $(UNAJPOBJDIR)/unamorph$(O) ./
unamdtri$O:
	$(CP) $(UNAJPOBJDIR)/unamdtri$(O) ./
unamdunk$O:
	$(CP) $(UNAJPOBJDIR)/unamdunk$(O) ./
unamdeng$O:
	$(CP) $(UNAJPOBJDIR)/unamdeng$(O) ./
unaapinf$O:
	$(CP) $(UNAJPOBJDIR)/unaapinf$(O) ./
unabns$O:
	$(CP) $(UNAJPOBJDIR)/unabns$(O) ./
unakapi$O:
	$(CP) $(UNAJPOBJDIR)/unakapi$(O) ./

#include "Makefile.h"
