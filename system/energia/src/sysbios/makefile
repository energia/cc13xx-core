vpath % /db/vtree/library/trees/mcpi/mcpi-3.10.00.10/exports/coresdk_cc13xx_cc26xx_3_10_00_10/kernel/tirtos/packages/ti/sysbios/
vpath %.c /db/rtree/library/trees/xdcprod/xdcprod-x10/product/Linux/xdctools_3_50_00_10/packages/

CCOPTS    = -gdwarf-3 -gstrict-dwarf -fno-exceptions -I/home/xlibrary/trees/mcpi/mcpi-3.10.00.10/exports/coresdk_cc13xx_cc26xx_3_10_00_10/source/ti/devices/cc13x0 -Wunused -Wunknown-pragmas -ffunction-sections -fdata-sections  -mcpu=cortex-m3 -mthumb -mabi=aapcs -g  -Os  -I/db/vtree/library/trees/mcpi/mcpi-3.10.00.10/exports/coresdk_cc13xx_cc26xx_3_10_00_10/kernel/tirtos/packages/gnu/targets/arm//libs/install-native/$(GCCTARG)/include -Dfar= -D__DYNAMIC_REENT__   -Dti_sysbios_utils_Load_hwiEnabled__D=FALSE -I/db/vtree/library/trees/mcpi/mcpi-3.10.00.10/exports/coresdk_cc13xx_cc26xx_3_10_00_10/source/ti/devices/cc13x0 -Dti_sysbios_family_arm_cc26xx_Boot_driverlibVersion=2 -Dti_sysbios_knl_Task_minimizeLatency__D=FALSE -Dti_sysbios_rts_gnu_ReentSupport_enableReentSupport__D=TRUE -Dti_sysbios_knl_Clock_stopCheckNext__D=TRUE -Dti_sysbios_family_arm_m3_Hwi_enableException__D=FALSE -Dti_sysbios_family_arm_m3_Hwi_disablePriority__D=32U -Dti_sysbios_family_arm_m3_Hwi_numSparseInterrupts__D=0U

XDC_ROOT  = /db/rtree/library/trees/xdcprod/xdcprod-x10/product/Linux/xdctools_3_50_00_10/packages/

BIOS_ROOT = /db/vtree/library/trees/mcpi/mcpi-3.10.00.10/exports/coresdk_cc13xx_cc26xx_3_10_00_10/kernel/tirtos/packages/ti/sysbios/

BIOS_DEFS =  -Dti_sysbios_BIOS_swiEnabled__D=TRUE -Dti_sysbios_BIOS_taskEnabled__D=TRUE -Dti_sysbios_BIOS_clockEnabled__D=TRUE -Dti_sysbios_BIOS_runtimeCreatesEnabled__D=TRUE -Dti_sysbios_knl_Task_moduleStateCheckFlag__D=FALSE -Dti_sysbios_knl_Task_objectCheckFlag__D=FALSE -Dti_sysbios_hal_Hwi_DISABLE_ALL_HOOKS -Dti_sysbios_knl_Swi_DISABLE_ALL_HOOKS -Dti_sysbios_BIOS_smpEnabled__D=FALSE -Dti_sysbios_Build_useHwiMacros -Dti_sysbios_Build_useIndirectReferences=FALSE -Dti_sysbios_knl_Swi_numPriorities__D=16 -Dti_sysbios_knl_Task_deleteTerminatedTasks__D=FALSE -Dti_sysbios_knl_Task_numPriorities__D=4 -Dti_sysbios_knl_Task_checkStackFlag__D=FALSE -Dti_sysbios_knl_Task_initStackFlag__D=TRUE -Dti_sysbios_knl_Clock_TICK_SOURCE=ti_sysbios_knl_Clock_TickSource_TIMER -Dti_sysbios_knl_Clock_TICK_MODE=ti_sysbios_knl_Clock_TickMode_DYNAMIC -Dti_sysbios_hal_Core_delegate_getId=ti_sysbios_hal_CoreNull_getId__E -Dti_sysbios_hal_Core_delegate_interruptCore=ti_sysbios_hal_CoreNull_interruptCore__E -Dti_sysbios_hal_Core_delegate_lock=ti_sysbios_hal_CoreNull_lock__E -Dti_sysbios_hal_Core_delegate_unlock=ti_sysbios_hal_CoreNull_unlock__E -Dti_sysbios_hal_Core_numCores__D=1 -Dti_sysbios_hal_CoreNull_numCores__D=1 -Dti_sysbios_utils_Load_taskEnabled__D=TRUE -Dti_sysbios_utils_Load_swiEnabled__D=FALSE -Dti_sysbios_utils_Load_hwiEnabled__D=FALSE -Dti_sysbios_family_arm_m3_Hwi_dispatcherSwiSupport__D=TRUE -Dti_sysbios_family_arm_m3_Hwi_dispatcherTaskSupport__D=TRUE -Dti_sysbios_family_arm_m3_Hwi_dispatcherAutoNestingSupport__D=TRUE -Dti_sysbios_family_arm_m3_Hwi_dispatcherIrpTrackingSupport__D=TRUE -Dti_sysbios_knl_Semaphore_supportsEvents__D=FALSE -Dti_sysbios_knl_Semaphore_supportsPriority__D=TRUE -Dxdc_runtime_Assert_DISABLE_ALL -Dxdc_runtime_Log_DISABLE_ALL 

BIOS_INC  = -I"/db/vtree/library/trees/mcpi/mcpi-3.10.00.10/exports/coresdk_cc13xx_cc26xx_3_10_00_10/kernel/tirtos/packages/"

INCS =  $(BIOS_INC) -I"/db/vtree/library/trees/mcpi/mcpi-3.10.00.10/exports/coresdk_cc13xx_cc26xx_3_10_00_10/kernel/tirtos/packages/" -I"/db/vtree/library/trees/mcpi/mcpi-3.10.00.10/exports/coresdk_cc13xx_cc26xx_3_10_00_10/source/" -I"/db/ztree/library/trees/emt/emt-d09/src/" -I"/db/rtree/library/trees/xdcprod/xdcprod-x10/product/Linux/xdctools_3_50_00_10/packages/" -I"../" -I"/packages//" 

GCCTARG = arm-none-eabi

CC = /db/toolsrc/library/tools/vendors/linaro/gcc-arm-none-eabi-4_9-2015q3/bin/arm-none-eabi-gcc -c -MD -MF $@.dep $(CCOPTS)
ASM = /db/toolsrc/library/tools/vendors/linaro/gcc-arm-none-eabi-4_9-2015q3/bin/arm-none-eabi-gcc -c -x assembler-with-cpp $(CCOPTS)
AR = /db/toolsrc/library/tools/vendors/linaro/gcc-arm-none-eabi-4_9-2015q3/bin/arm-none-eabi-ar  cr

DEL = rm -f
CP = cp -f

define RM
	$(if $(wildcard $1),$(DEL) $1,:)
endef

define ASSEMBLE
	@echo asmm3g $< ...
	@$(ASM) $(BIOS_DEFS) -I "/db/rtree/library/trees/xdcprod/xdcprod-x10/product/Linux/xdctools_3_50_00_10/packages/" -Dxdc_target_types__=/db/vtree/library/trees/mcpi/mcpi-3.10.00.10/exports/coresdk_cc13xx_cc26xx_3_10_00_10/kernel/tirtos/packages/gnu/targets/arm/std.h -Dxdc_target_name__=M3 $(INCS) $<
endef

define COMPILE
	@echo clm3g $< ...
	@$(CC) $(BIOS_DEFS) -I "/db/rtree/library/trees/xdcprod/xdcprod-x10/product/Linux/xdctools_3_50_00_10/packages/" -Dxdc_target_types__=/db/vtree/library/trees/mcpi/mcpi-3.10.00.10/exports/coresdk_cc13xx_cc26xx_3_10_00_10/kernel/tirtos/packages/gnu/targets/arm/std.h -Dxdc_target_name__=M3 $(INCS) $<
endef


all: sysbios.am3g

m3_Hwi_asm_gnu.o: family/arm/m3/Hwi_asm_gnu.sv7M makefile
	@-$(call RM, $@)
	$(ASSEMBLE) -o m3_Hwi_asm_gnu.o

m3_Hwi_asm_switch_gnu.o: family/arm/m3/Hwi_asm_switch_gnu.sv7M makefile
	@-$(call RM, $@)
	$(ASSEMBLE) -o m3_Hwi_asm_switch_gnu.o

m3_IntrinsicsSupport_asm_gnu.o: family/arm/m3/IntrinsicsSupport_asm_gnu.sv7M makefile
	@-$(call RM, $@)
	$(ASSEMBLE) -o m3_IntrinsicsSupport_asm_gnu.o

m3_TaskSupport_asm_gnu.o: family/arm/m3/TaskSupport_asm_gnu.sv7M makefile
	@-$(call RM, $@)
	$(ASSEMBLE) -o m3_TaskSupport_asm_gnu.o


_BIOS.o: BIOS.c makefile
	@-$(call RM, _BIOS.o)
	$(COMPILE) -o _BIOS.o
knl_Clock.o: knl/Clock.c makefile
	@-$(call RM, knl_Clock.o)
	$(COMPILE) -o knl_Clock.o
knl_Idle.o: knl/Idle.c makefile
	@-$(call RM, knl_Idle.o)
	$(COMPILE) -o knl_Idle.o
knl_Intrinsics.o: knl/Intrinsics.c makefile
	@-$(call RM, knl_Intrinsics.o)
	$(COMPILE) -o knl_Intrinsics.o
knl_Event.o: knl/Event.c makefile
	@-$(call RM, knl_Event.o)
	$(COMPILE) -o knl_Event.o
knl_Mailbox.o: knl/Mailbox.c makefile
	@-$(call RM, knl_Mailbox.o)
	$(COMPILE) -o knl_Mailbox.o
knl_Queue.o: knl/Queue.c makefile
	@-$(call RM, knl_Queue.o)
	$(COMPILE) -o knl_Queue.o
knl_Semaphore.o: knl/Semaphore.c makefile
	@-$(call RM, knl_Semaphore.o)
	$(COMPILE) -o knl_Semaphore.o
knl_Swi.o: knl/Swi.c makefile
	@-$(call RM, knl_Swi.o)
	$(COMPILE) -o knl_Swi.o
knl_Swi_andn.o: knl/Swi_andn.c makefile
	@-$(call RM, knl_Swi_andn.o)
	$(COMPILE) -o knl_Swi_andn.o
knl_Task.o: knl/Task.c makefile
	@-$(call RM, knl_Task.o)
	$(COMPILE) -o knl_Task.o
hal_Hwi.o: hal/Hwi.c makefile
	@-$(call RM, hal_Hwi.o)
	$(COMPILE) -o hal_Hwi.o
hal_Hwi_stack.o: hal/Hwi_stack.c makefile
	@-$(call RM, hal_Hwi_stack.o)
	$(COMPILE) -o hal_Hwi_stack.o
hal_Hwi_startup.o: hal/Hwi_startup.c makefile
	@-$(call RM, hal_Hwi_startup.o)
	$(COMPILE) -o hal_Hwi_startup.o
hal_Seconds.o: hal/Seconds.c makefile
	@-$(call RM, hal_Seconds.o)
	$(COMPILE) -o hal_Seconds.o
hal_Timer.o: hal/Timer.c makefile
	@-$(call RM, hal_Timer.o)
	$(COMPILE) -o hal_Timer.o
utils_Load.o: utils/Load.c makefile
	@-$(call RM, utils_Load.o)
	$(COMPILE) -o utils_Load.o
utils_Load_CPU.o: utils/Load_CPU.c makefile
	@-$(call RM, utils_Load_CPU.o)
	$(COMPILE) -o utils_Load_CPU.o
gates_GateHwi.o: gates/GateHwi.c makefile
	@-$(call RM, gates_GateHwi.o)
	$(COMPILE) -o gates_GateHwi.o
gates_GateMutex.o: gates/GateMutex.c makefile
	@-$(call RM, gates_GateMutex.o)
	$(COMPILE) -o gates_GateMutex.o
m3_Hwi.o: family/arm/m3/Hwi.c makefile
	@-$(call RM, m3_Hwi.o)
	$(COMPILE) -o m3_Hwi.o
m3_TaskSupport.o: family/arm/m3/TaskSupport.c makefile
	@-$(call RM, m3_TaskSupport.o)
	$(COMPILE) -o m3_TaskSupport.o
m3_Timer.o: family/arm/m3/Timer.c makefile
	@-$(call RM, m3_Timer.o)
	$(COMPILE) -o m3_Timer.o
m3_TimestampProvider.o: family/arm/m3/TimestampProvider.c makefile
	@-$(call RM, m3_TimestampProvider.o)
	$(COMPILE) -o m3_TimestampProvider.o
heaps_HeapMem.o: heaps/HeapMem.c makefile
	@-$(call RM, heaps_HeapMem.o)
	$(COMPILE) -o heaps_HeapMem.o
lm4_Timer.o: family/arm/lm4/Timer.c makefile
	@-$(call RM, lm4_Timer.o)
	$(COMPILE) -o lm4_Timer.o
cc26xx_Boot.o: family/arm/cc26xx/Boot.c makefile
	@-$(call RM, cc26xx_Boot.o)
	$(COMPILE) -o cc26xx_Boot.o
cc26xx_Seconds.o: family/arm/cc26xx/Seconds.c makefile
	@-$(call RM, cc26xx_Seconds.o)
	$(COMPILE) -o cc26xx_Seconds.o
cc26xx_Timer.o: family/arm/cc26xx/Timer.c makefile
	@-$(call RM, cc26xx_Timer.o)
	$(COMPILE) -o cc26xx_Timer.o
cc26xx_TimestampProvider.o: family/arm/cc26xx/TimestampProvider.c makefile
	@-$(call RM, cc26xx_TimestampProvider.o)
	$(COMPILE) -o cc26xx_TimestampProvider.o
gnu_ReentSupport.o: rts/gnu/ReentSupport.c makefile
	@-$(call RM, gnu_ReentSupport.o)
	$(COMPILE) -o gnu_ReentSupport.o

sysbios.am3g: m3_Hwi_asm_gnu.o m3_Hwi_asm_switch_gnu.o m3_IntrinsicsSupport_asm_gnu.o m3_TaskSupport_asm_gnu.o _BIOS.o knl_Clock.o knl_Idle.o knl_Intrinsics.o knl_Event.o knl_Mailbox.o knl_Queue.o knl_Semaphore.o knl_Swi.o knl_Swi_andn.o knl_Task.o hal_Hwi.o hal_Hwi_stack.o hal_Hwi_startup.o hal_Seconds.o hal_Timer.o utils_Load.o utils_Load_CPU.o gates_GateHwi.o gates_GateMutex.o m3_Hwi.o m3_TaskSupport.o m3_Timer.o m3_TimestampProvider.o heaps_HeapMem.o lm4_Timer.o cc26xx_Boot.o cc26xx_Seconds.o cc26xx_Timer.o cc26xx_TimestampProvider.o gnu_ReentSupport.o
	@echo arm3g $^ ...
	@-$(call RM, $@)
	@$(AR) $@ $^


clean:
	@$(DEL) ../makefile.libs
	@-$(call RM, *)

