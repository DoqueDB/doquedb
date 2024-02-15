#!/bin/sh
#
# Copyright (c) 2001, 2023, 2024 Ricoh Company, Ltd.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

cvr_dst=$sydtop/Test/SydTest/test/coverage

case $analysis_mode in
    inspxer_mode)
        exeprefix="inspxe-cl"
        intel_option='-collect mi2 -knob detect-invalid-accesses=true -knob detect-leaks-on-exit=true -knob enable-memory-growth-detection=true -knob enable-on-demand-leak-detection=true -knob still-allocated-memory=true -knob stack-depth=8 -module-filter-mode=include -q'
        if [ $isLinux -eq 1 ] ; then
            . /opt/intel/inspector/env/vars.sh
        fi
        ;;

    hotspots_mode)
        exeprefix="/opt/intel/vtune_profiler/bin64/vtune"
        intel_option="-collect hotspots"
        . /opt/intel/vtune_profiler/env/vars.sh
        ;;

    advisor_mode)
        exeprefix="/opt/intel/advisor/bin64/advixe-cl"
        intel_option="-collect=survey"
        . /opt/intel/advisor/env/vars.sh
        ;;
    
    purify_mode)
        RationalPurifyPlus="$PROGRAMFILESDIR/IBM/RationalPurifyPlus"
        prefix="$RationalPurifyPlus/Purify.exe"

        if [ "$arc" = "x64" ] ; then
            purifyd_sydtest="$RationalPurifyPlus/x64/cache/SydTest@Purify_C_Program Files_ricoh_TRMeister.exe"
        elif [ "$arc" = "x86" ] ; then
            purifyd_sydtest="$RationalPurifyPlus/x86/cache/SydTest\$Purify_C_Program Files_ricoh_TRMeister.exe"
        fi

        if [ "$PROCESSOR_ARCHITECTURE" = "AMD64" -o "$PROCESSOR_ARCHITEW6432" = "AMD64" ]; then
            PTF=result.pfy64
        else
            PTF=result.pfy
        fi

        if [ ! -e "$purifyd_sydtest" ] ; then
            "$purify" /run=no "$sydtest"
        fi

        cov_option="/LeaksAtExit=yes /SaveData=$pur_dst/$dir/$testdir/$PTF /SaveTextData=$pur_dst/$dir/$testdir/result.txt"

        if [ -z $purify_sh_mode ] ; then
            purify_sh_mode=0      # purify.sh から実行は1
            max_call_stack="/error-call-stack-length=50 /alloc-call-stack-length=50 /max-detail-chain /max-detail-error-call-stack-length=50 /max-detail-alloc-call-stack-length=50"
            cov_option="$cov_option $max_call_stack"
        fi
        ;;

    coverage_mode)
        RationalPurifyPlus="$PROGRAMFILESDIR/IBM/RationalPurifyPlus"
        prefix="$RationalPurifyPlus/Coverage.exe"
        cov_sydtest="$RationalPurifyPlus/cache/SydTest@Coverage_C_${progamdir}_ricoh_TRMeister.exe"

        if [ ! -e "$cov_sydtest" ] ; then
            "$coverage" /run=no "$sydtest"
        fi
        ;;

    bc_mode)
        # DevPartner Studio
        if [ -z $bc_sh_mode ] ; then
            bc_sh_mode=0
        fi
        if [ -z $purify_sh_mode ] ; then
            purify_sh_mode=0
        fi
        if [ "$arc" = "x64" ] ; then
            DevPartnerStudio="C:/Program Files (x86)/Micro Focus/DevPartner Studio"
        elif [ "$arc" = "x86" ] ; then
            DevPartnerStudio="C:/Program Files/Micro Focus/DevPartner Studio"
        fi
        exeprefix="$DevPartnerStudio/BoundsChecker/BC.exe"
        cov_option="/MC /B result.DPbcl /X result.xml"
        ;;

    *)
        echo "analysis_mode is unexpected."
        exit 1;;
        
esac

#
# Copyright (c) 2001, 2023, 2024 Ricoh Company, Ltd.
# All rights reserved.
#
