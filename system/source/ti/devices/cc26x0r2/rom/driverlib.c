#include "driverlib/aon_batmon.h"
#include "driverlib/aon_batmon.h"
#include "driverlib/aon_batmon.h"
#include "driverlib/aon_event.h"
#include "driverlib/aon_ioc.h"
#include "driverlib/aon_rtc.h"
#include "driverlib/aon_rtc.h"
#include "driverlib/aon_wuc.h"
#include "driverlib/aon_wuc.h"
#include "driverlib/aux_ctrl.h"
#include "driverlib/aux_tdc.h"
#include "driverlib/aux_timer.h"
#include "driverlib/aux_wuc.h"
#include "driverlib/aux_wuc.h"
#include "driverlib/chipinfo.h"
#include "driverlib/chipinfo.h"
#include "driverlib/cpu.h"
#include "driverlib/cpu.h"
#include "driverlib/crypto.h"
#include "driverlib/ddi.h"
#include "driverlib/ddi.h"
#include "driverlib/flash.h"
#include "driverlib/i2c.h"
#include "driverlib/interrupt.h"
#include "driverlib/ioc.h"
#include "driverlib/ioc.h"
#include "driverlib/osc.h"
#include "driverlib/osc.h"
#include "driverlib/prcm.h"
#include "driverlib/setup.h"
#include "driverlib/setup_rom.h"
#include "driverlib/setup_rom.h"
#include "driverlib/setup_rom.h"
#include "driverlib/smph.h"
#include "driverlib/ssi.h"
#include "driverlib/sys_ctrl.h"
#include "driverlib/sys_ctrl.h"
#include "driverlib/timer.h"
#include "driverlib/trng.h"
#include "driverlib/uart.h"
#include "driverlib/udma.h"
#include "driverlib/vims.h"
#include "inc/hw_adi.h"
#include "inc/hw_adi.h"
#include "inc/hw_adi_2_refsys.h"
#include "inc/hw_adi_2_refsys.h"
#include "inc/hw_adi_3_refsys.h"
#include "inc/hw_adi_3_refsys.h"
#include "inc/hw_adi_4_aux.h"
#include "inc/hw_aon_batmon.h"
#include "inc/hw_aon_ioc.h"
#include "inc/hw_aon_sysctl.h"
#include "inc/hw_aon_sysctl.h"
#include "inc/hw_aon_wuc.h"
#include "inc/hw_aux_wuc.h"
#include "inc/hw_ccfg.h"
#include "inc/hw_ccfg.h"
#include "inc/hw_ccfg.h"
#include "inc/hw_ccfg.h"
#include "inc/hw_ccfg.h"
#include "inc/hw_ddi_0_osc.h"
#include "inc/hw_fcfg1.h"
#include "inc/hw_fcfg1.h"
#include "inc/hw_fcfg1.h"
#include "inc/hw_fcfg1.h"
#include "inc/hw_flash.h"
#include "inc/hw_memmap.h"
#include "inc/hw_memmap.h"
#include "inc/hw_prcm.h"
#include "inc/hw_types.h"
#include "inc/hw_types.h"
#include "inc/hw_types.h"
#include "inc/hw_types.h"
#include "inc/hw_types.h"
#include "inc/hw_vims.h"

//*****************************************************************************
//
//! Disable all external interrupts
//
//*****************************************************************************
#if defined(__IAR_SYSTEMS_ICC__)
uint32_t
CPUcpsid(void)
{
    //
    // Read PRIMASK and disable interrupts.
    //
    __asm("    mrs     r0, PRIMASK\n"
          "    cpsid   i\n");

    //
    // "Warning[Pe940]: missing return statement at end of non-void function"
    // is suppressed here to avoid putting a "bx lr" in the inline assembly
    // above and a superfluous return statement here.
    //
#pragma diag_suppress=Pe940
}
#pragma diag_default=Pe940
#elif defined(__CC_ARM) || defined(__ARMCC_VERSION)
__asm uint32_t
CPUcpsid(void)
{
    //
    // Read PRIMASK and disable interrupts.
    //
    mrs     r0, PRIMASK;
    cpsid   i;
    bx      lr
}
#elif defined(__TI_COMPILER_VERSION__) || defined(DOXYGEN)
uint32_t
CPUcpsid(void)
{
    //
    // Read PRIMASK and disable interrupts.
    //
    __asm("    mrs     r0, PRIMASK\n"
          "    cpsid   i\n"
          "    bx      lr\n");

    //
    // The following keeps the compiler happy, because it wants to see a
    // return value from this function.  It will generate code to return
    // a zero.  However, the real return is the "bx lr" above, so the
    // return(0) is never executed and the function returns with the value
    // you expect in R0.
    //
    return(0);
}
#else
uint32_t __attribute__((naked))
CPUcpsid(void)
{
    uint32_t ui32Ret;

    //
    // Read PRIMASK and disable interrupts
    //
    __asm("    mrs     r0, PRIMASK\n"
          "    cpsid   i\n"
          "    bx      lr\n"
      : "=r"(ui32Ret));

    //
    // The return is handled in the inline assembly, but the compiler will
    // still complain if there is not an explicit return here (despite the fact
    // that this does not result in any code being produced because of the
    // naked attribute).
    //
    return(ui32Ret);
}
#endif
//*****************************************************************************
//
//! Enable all external interrupts
//
//*****************************************************************************
#if defined(__IAR_SYSTEMS_ICC__)
uint32_t
CPUcpsie(void)
{
    //
    // Read PRIMASK and enable interrupts.
    //
    __asm("    mrs     r0, PRIMASK\n"
          "    cpsie   i\n");

    //
    // "Warning[Pe940]: missing return statement at end of non-void function"
    // is suppressed here to avoid putting a "bx lr" in the inline assembly
    // above and a superfluous return statement here.
    //
#pragma diag_suppress=Pe940
}
#pragma diag_default=Pe940
#elif defined(__CC_ARM) || defined(__ARMCC_VERSION)
__asm uint32_t
CPUcpsie(void)
{
    //
    // Read PRIMASK and enable interrupts.
    //
    mrs     r0, PRIMASK;
    cpsie   i;
    bx      lr
}
#elif defined(__TI_COMPILER_VERSION__) || defined(DOXYGEN)
uint32_t
CPUcpsie(void)
{
    //
    // Read PRIMASK and enable interrupts.
    //
    __asm("    mrs     r0, PRIMASK\n"
          "    cpsie   i\n"
          "    bx      lr\n");

    //
    // The following keeps the compiler happy, because it wants to see a
    // return value from this function.  It will generate code to return
    // a zero.  However, the real return is the "bx lr" above, so the
    // return(0) is never executed and the function returns with the value
    // you expect in R0.
    //
    return(0);
}
#else
uint32_t __attribute__((naked))
CPUcpsie(void)
{
    uint32_t ui32Ret;

    //
    // Read PRIMASK and enable interrupts.
    //
    __asm("    mrs     r0, PRIMASK\n"
          "    cpsie   i\n"
          "    bx      lr\n"
      : "=r"(ui32Ret));

    //
    // The return is handled in the inline assembly, but the compiler will
    // still complain if there is not an explicit return here (despite the fact
    // that this does not result in any code being produced because of the
    // naked attribute).
    //
    return(ui32Ret);
}
#endif
//*****************************************************************************
//
//! Provide a small delay
//
//*****************************************************************************
#if defined(__IAR_SYSTEMS_ICC__)
void
CPUdelay(uint32_t ui32Count)
{
    //
    // Delay the specified number of times (3 cycles pr. loop)
    //
    __asm("CPUdelay:\n"
          "    subs    r0, #1\n"
          "    bne.n   CPUdelay\n"
          "    bx      lr");
#pragma diag_suppress=Pe940
}
#pragma diag_default=Pe940
#elif defined(__CC_ARM) || defined(__ARMCC_VERSION)
__asm void
CPUdelay(uint32_t ui32Count)
{
    //
    // Delay the specified number of times (3 cycles pr. loop)
    //
CPUdel
    subs    r0, #1;
    bne     CPUdel;
    bx      lr;
}
#elif defined(__TI_COMPILER_VERSION__) || defined(DOXYGEN)
//
// For CCS implement this function in pure assembly. This prevents the TI
// compiler from doing funny things with the optimizer.
//
    //
    // Delay the specified number of times (3 cycles pr. loop)
    //
__asm("    .sect \".text:CPUdelay\"\n"
      "    .clink\n"
      "    .thumbfunc CPUdelay\n"
      "    .thumb\n"
      "    .global CPUdelay\n"
      "CPUdelay:\n"
      "    subs r0, #1\n"
      "    bne.n CPUdelay\n"
      "    bx lr\n");
#else
void __attribute__((naked))
CPUdelay(uint32_t ui32Count)
{
    //
    // Delay the specified number of times (3 cycles pr. loop)
    //
    __asm("    subs    r0, #1\n"
          "    bne     CPUdelay\n"
          "    bx      lr");
}
#endif

void AONEventMcuWakeUpSet(uint32_t ui32MCUWUEvent, uint32_t ui32EventSrc) {
    uint32_t ui32Ctrl;

    //
    // Check the arguments.
    //
    ASSERT((ui32MCUWUEvent == AON_EVENT_MCU_WU0) ||
           (ui32MCUWUEvent == AON_EVENT_MCU_WU1) ||
           (ui32MCUWUEvent == AON_EVENT_MCU_WU2) ||
           (ui32MCUWUEvent == AON_EVENT_MCU_WU3));
    ASSERT(ui32EventSrc <= AON_EVENT_NONE);

    ui32Ctrl = HWREG(AON_EVENT_BASE + AON_EVENT_O_MCUWUSEL);

    if(ui32MCUWUEvent == AON_EVENT_MCU_WU0)
    {
        ui32Ctrl &= ~(AON_EVENT_MCUWUSEL_WU0_EV_M);
        ui32Ctrl |= (ui32EventSrc & 0x3f) << AON_EVENT_MCUWUSEL_WU0_EV_S;
    }
    else if(ui32MCUWUEvent == AON_EVENT_MCU_WU1)
    {
        ui32Ctrl &= ~(AON_EVENT_MCUWUSEL_WU1_EV_M);
        ui32Ctrl |= (ui32EventSrc & 0x3f) << AON_EVENT_MCUWUSEL_WU1_EV_S;
    }
    else if(ui32MCUWUEvent == AON_EVENT_MCU_WU2)
    {
        ui32Ctrl &= ~(AON_EVENT_MCUWUSEL_WU2_EV_M);
        ui32Ctrl |= (ui32EventSrc & 0x3f) << AON_EVENT_MCUWUSEL_WU2_EV_S;
    }
    else if(ui32MCUWUEvent == AON_EVENT_MCU_WU3)
    {
        ui32Ctrl &= ~(AON_EVENT_MCUWUSEL_WU3_EV_M);
        ui32Ctrl |= (ui32EventSrc & 0x3f) << AON_EVENT_MCUWUSEL_WU3_EV_S;
    }

    HWREG(AON_EVENT_BASE + AON_EVENT_O_MCUWUSEL) = ui32Ctrl;
}

uint32_t AONRTCCurrentCompareValueGet( void ) {
    uint32_t   ui32CurrentSec    ;
    uint32_t   ui32CurrentSubSec ;
    uint32_t   ui32SecondSecRead ;

    //
    // Reading SEC both before and after SUBSEC in order to detect if SEC incremented while reading SUBSEC
    // If SEC incremented, we can't be sure which SEC the SUBSEC belongs to, so repeating the sequence then.
    //
    do {
        ui32CurrentSec    = HWREG( AON_RTC_BASE + AON_RTC_O_SEC    );
        ui32CurrentSubSec = HWREG( AON_RTC_BASE + AON_RTC_O_SUBSEC );
        ui32SecondSecRead = HWREG( AON_RTC_BASE + AON_RTC_O_SEC    );
    } while ( ui32CurrentSec != ui32SecondSecRead );

    return (( ui32CurrentSec << 16 ) | ( ui32CurrentSubSec >> 16 ));
}

uint64_t AONRTCCurrent64BitValueGet( void ) {
    union {
        uint64_t  returnValue       ;
        uint32_t  secAndSubSec[ 2 ] ;
    } currentRtc                    ;
    uint32_t      ui32SecondSecRead ;

    //
    // Reading SEC both before and after SUBSEC in order to detect if SEC incremented while reading SUBSEC
    // If SEC incremented, we can't be sure which SEC the SUBSEC belongs to, so repeating the sequence then.
    //
    do {
        currentRtc.secAndSubSec[ 1 ] = HWREG( AON_RTC_BASE + AON_RTC_O_SEC    );
        currentRtc.secAndSubSec[ 0 ] = HWREG( AON_RTC_BASE + AON_RTC_O_SUBSEC );
        ui32SecondSecRead            = HWREG( AON_RTC_BASE + AON_RTC_O_SEC    );
    } while ( currentRtc.secAndSubSec[ 1 ] != ui32SecondSecRead );

    return ( currentRtc.returnValue );
}

void AUXWUCClockEnable(uint32_t ui32Clocks) {
    //
    // Check the arguments.
    //
    ASSERT((ui32Clocks & AUX_WUC_ADI_CLOCK) ||
           (ui32Clocks & AUX_WUC_OSCCTRL_CLOCK) ||
           (ui32Clocks & AUX_WUC_TDCIF_CLOCK) ||
           (ui32Clocks & AUX_WUC_ANAIF_CLOCK) ||
           (ui32Clocks & AUX_WUC_TIMER_CLOCK) ||
           (ui32Clocks & AUX_WUC_AIODIO0_CLOCK) ||
           (ui32Clocks & AUX_WUC_AIODIO1_CLOCK) ||
           (ui32Clocks & AUX_WUC_SMPH_CLOCK) ||
           (ui32Clocks & AUX_WUC_TDC_CLOCK) ||
           (ui32Clocks & AUX_WUC_ADC_CLOCK) ||
           (ui32Clocks & AUX_WUC_REF_CLOCK));

    //
    // Enable some of the clocks in the clock register.
    //
    HWREG(AUX_WUC_BASE + AUX_WUC_O_MODCLKEN0) |= (ui32Clocks &
                                                AUX_WUC_MODCLK_MASK);

    //
    // Check the rest.
    //
    if(ui32Clocks & AUX_WUC_ADC_CLOCK)
    {
        HWREG(AUX_WUC_BASE + AUX_WUC_O_ADCCLKCTL) =
            AUX_WUC_ADCCLKCTL_REQ;
    }
    if(ui32Clocks & AUX_WUC_TDC_CLOCK)
    {
        HWREG(AUX_WUC_BASE + AUX_WUC_O_TDCCLKCTL) =
            AUX_WUC_TDCCLKCTL_REQ;
    }
    if(ui32Clocks & AUX_WUC_REF_CLOCK)
    {
        HWREG(AUX_WUC_BASE + AUX_WUC_O_REFCLKCTL) =
            AUX_WUC_REFCLKCTL_REQ;
    }
}

void AUXWUCPowerCtrl(uint32_t ui32PowerMode) {
    //
    // Check the arguments.
    //
    ASSERT((ui32PowerMode == AUX_WUC_POWER_OFF) ||
           (ui32PowerMode == AUX_WUC_POWER_DOWN) ||
           (ui32PowerMode == AUX_WUC_POWER_ACTIVE));

    //
    // Power on/off.
    //
    if(ui32PowerMode == AUX_WUC_POWER_OFF)
    {
        HWREG(AUX_WUC_BASE + AUX_WUC_O_PWROFFREQ) = AUX_WUC_PWROFFREQ_REQ;
        HWREG(AUX_WUC_BASE + AUX_WUC_O_MCUBUSCTL) = AUX_WUC_MCUBUSCTL_DISCONNECT_REQ;
        return;
    }
    else
    {
        HWREG(AUX_WUC_BASE + AUX_WUC_O_PWROFFREQ) = 0x0;
    }

    //
    // Power down/active.
    //
    if(ui32PowerMode == AUX_WUC_POWER_DOWN)
    {
        HWREG(AUX_WUC_BASE + AUX_WUC_O_PWRDWNREQ) =
            AUX_WUC_PWRDWNREQ_REQ;
        HWREG(AUX_WUC_BASE + AUX_WUC_O_MCUBUSCTL) = AUX_WUC_MCUBUSCTL_DISCONNECT_REQ;
    }
    else
    {
        HWREG(AUX_WUC_BASE + AUX_WUC_O_PWRDWNREQ) = 0x0;
    }
}

void DDI32RegWrite(uint32_t ui32Base, uint32_t ui32Reg, uint32_t ui32Val) {
    //
    // Check the arguments.
    //
    ASSERT(DDIBaseValid(ui32Base));
    ASSERT(ui32Reg < DDI_SLAVE_REGS);

    //
    // Write the value to the register.
    //
    AuxAdiDdiSafeWrite(ui32Base + ui32Reg, ui32Val, 4);
}

void DDI16BitfieldWrite(uint32_t ui32Base, uint32_t ui32Reg, uint32_t ui32Mask, uint32_t ui32Shift, uint16_t ui32Data) {
    uint32_t ui32RegAddr;
    uint32_t ui32WrData;

    //
    // Check the arguments.
    //
    ASSERT(DDIBaseValid(ui32Base));

    //
    // 16-bit target is on 32-bit boundary so double offset.
    //
    ui32RegAddr = ui32Base + (ui32Reg << 1) + DDI_O_MASK16B;

    //
    // Adjust for target bit in high half of the word.
    //
    if(ui32Shift >= 16)
    {
        ui32Shift = ui32Shift - 16;
        ui32RegAddr += 4;
        ui32Mask = ui32Mask >> 16;
    }

    //
    // Shift data in to position.
    //
    ui32WrData = ui32Data << ui32Shift;

    //
    // Write data.
    //
    AuxAdiDdiSafeWrite(ui32RegAddr, (ui32Mask << 16) | ui32WrData, 4);
}

uint16_t DDI16BitRead(uint32_t ui32Base, uint32_t ui32Reg, uint32_t ui32Mask) {
    uint32_t ui32RegAddr;
    uint16_t ui16Data;

    //
    // Check the arguments.
    //
    ASSERT(DDIBaseValid(ui32Base));

    //
    // Calculate the address of the register.
    //
    ui32RegAddr = ui32Base + ui32Reg + DDI_O_DIR;

    //
    // Adjust for target bit in high half of the word.
    //
    if(ui32Mask & 0xFFFF0000)
    {
        ui32RegAddr += 2;
        ui32Mask = ui32Mask >> 16;
    }

    //
    // Read a halfword on the DDI interface.
    //
    ui16Data = AuxAdiDdiSafeRead(ui32RegAddr, 2);

    //
    // Mask data.
    //
    ui16Data = ui16Data & ui32Mask;

    //
    // Return masked data.
    //
    return(ui16Data);
}

uint16_t DDI16BitfieldRead(uint32_t ui32Base, uint32_t ui32Reg, uint32_t ui32Mask, uint32_t ui32Shift) {
    uint32_t ui32RegAddr;
    uint16_t ui16Data;

    //
    // Check the arguments.
    //
    ASSERT(DDIBaseValid(ui32Base));

    //
    // Calculate the register address.
    //
    ui32RegAddr = ui32Base + ui32Reg + DDI_O_DIR;

    //
    // Adjust for target bit in high half of the word.
    //
    if(ui32Shift >= 16)
    {
        ui32Shift = ui32Shift - 16;
        ui32RegAddr += 2;
        ui32Mask = ui32Mask >> 16;
    }

    //
    // Read the register.
    //
    ui16Data = AuxAdiDdiSafeRead(ui32RegAddr, 2);

    //
    // Mask data and shift into place.
    //
    ui16Data &= ui32Mask;
    ui16Data >>= ui32Shift;

    //
    // Return data.
    //
    return(ui16Data);
}

//*****************************************************************************
//
// Defines for accesses to the security control in the customer configuration
// area in flash top sector.
//
//*****************************************************************************
#define CCFG_OFFSET_SECURITY   CCFG_O_BL_CONFIG
#define CCFG_OFFSET_SECT_PROT  CCFG_O_CCFG_PROT_31_0
#define CCFG_SIZE_SECURITY     0x00000014
#define CCFG_SIZE_SECT_PROT    0x00000004

//*****************************************************************************
//
// Default values for security control in customer configuration area in flash
// top sector.
//
//*****************************************************************************
const uint8_t g_pui8CcfgDefaultSec[] = {0xFF, 0xFF, 0xFF, 0xC5,
                                        0xFF, 0xFF, 0xFF, 0xFF,
                                        0xC5, 0xFF, 0xFF, 0xFF,
                                        0xC5, 0xC5, 0xC5, 0xFF,
                                        0xC5, 0xC5, 0xC5, 0xFF
                                       };

//*****************************************************************************
//
// Function prototypes for static functions
//
//*****************************************************************************
static void IssueFsmCommand(tFlashStateCommandsType eCommand);
static void EnableSectorsForWrite(void);
static uint32_t ScaleCycleValues(uint32_t ui32SpecifiedTiming,
                                 uint32_t ui32ScaleValue);
static void SetWriteMode(void);
static void TrimForWrite(void);
static void SetReadMode(void);

void FlashPowerModeSet(uint32_t ui32PowerMode, uint32_t ui32BankGracePeriode, uint32_t ui32PumpGracePeriode) {
    //
    // Check the arguments.
    //
    ASSERT(ui32PowerMode == FLASH_PWR_ACTIVE_MODE ||
           ui32PowerMode == FLASH_PWR_OFF_MODE    ||
           ui32PowerMode == FLASH_PWR_DEEP_STDBY_MODE);
    ASSERT(ui32BankGracePeriode <= 0xFF);
    ASSERT(ui32PumpGracePeriode <= 0xFFFF);

    switch(ui32PowerMode)
    {
    case FLASH_PWR_ACTIVE_MODE:
        //
        // Set bank power mode to ACTIVE.
        //
        HWREG(FLASH_BASE + FLASH_O_FBFALLBACK) =
            (HWREG(FLASH_BASE + FLASH_O_FBFALLBACK) &
             ~FLASH_FBFALLBACK_BANKPWR0_M) | FBFALLBACK_ACTIVE;

        //
        // Set charge pump power mode to ACTIVE mode.
        //
        HWREG(FLASH_BASE + FLASH_O_FPAC1) =
            (HWREG(FLASH_BASE + FLASH_O_FPAC1) & ~FLASH_FPAC1_PUMPPWR_M) | (1 << FLASH_FPAC1_PUMPPWR_S);
        break;

    case FLASH_PWR_OFF_MODE:
        //
        // Set bank grace periode.
        //
        HWREG(FLASH_BASE + FLASH_O_FBAC) =
            (HWREG(FLASH_BASE + FLASH_O_FBAC) & (~FLASH_FBAC_BAGP_M)) |
            ((ui32BankGracePeriode << FLASH_FBAC_BAGP_S) & FLASH_FBAC_BAGP_M);

        //
        // Set pump grace periode.
        //
        HWREG(FLASH_BASE + FLASH_O_FPAC2) =
            (HWREG(FLASH_BASE + FLASH_O_FPAC2) & (~FLASH_FPAC2_PAGP_M)) |
            ((ui32PumpGracePeriode << FLASH_FPAC2_PAGP_S) & FLASH_FPAC2_PAGP_M);

        //
        // Set bank power mode to SLEEP.
        //
        HWREG(FLASH_BASE + FLASH_O_FBFALLBACK) &= ~FLASH_FBFALLBACK_BANKPWR0_M;

        //
        // Set charge pump power mode to SLEEP mode.
        //
        HWREG(FLASH_BASE + FLASH_O_FPAC1) &= ~FLASH_FPAC1_PUMPPWR_M;
        break;

    case FLASH_PWR_DEEP_STDBY_MODE:
        //
        // Set bank grace periode.
        //
        HWREG(FLASH_BASE + FLASH_O_FBAC) =
            (HWREG(FLASH_BASE + FLASH_O_FBAC) & (~FLASH_FBAC_BAGP_M)) |
            ((ui32BankGracePeriode << FLASH_FBAC_BAGP_S) & FLASH_FBAC_BAGP_M);

        //
        // Set pump grace periode.
        //
        HWREG(FLASH_BASE + FLASH_O_FPAC2) =
            (HWREG(FLASH_BASE + FLASH_O_FPAC2) & (~FLASH_FPAC2_PAGP_M)) |
            ((ui32PumpGracePeriode << FLASH_FPAC2_PAGP_S) & FLASH_FPAC2_PAGP_M);

        //
        // Set bank power mode to DEEP STANDBY mode.
        //
        HWREG(FLASH_BASE + FLASH_O_FBFALLBACK) =
            (HWREG(FLASH_BASE + FLASH_O_FBFALLBACK) &
             ~FLASH_FBFALLBACK_BANKPWR0_M) | FBFALLBACK_DEEP_STDBY;

        //
        // Set charge pump power mode to STANDBY mode.
        //
        HWREG(FLASH_BASE + FLASH_O_FPAC1) |= FLASH_FPAC1_PUMPPWR_M;
        break;
    }
}

uint32_t FlashPowerModeGet(void) {
    uint32_t ui32PowerMode;
    uint32_t ui32BankPwrMode;

    ui32BankPwrMode = HWREG(FLASH_BASE + FLASH_O_FBFALLBACK) &
                      FLASH_FBFALLBACK_BANKPWR0_M;

    if(ui32BankPwrMode == FBFALLBACK_SLEEP)
    {
        ui32PowerMode = FLASH_PWR_OFF_MODE;
    }
    else if(ui32BankPwrMode == FBFALLBACK_DEEP_STDBY)
    {
        ui32PowerMode = FLASH_PWR_DEEP_STDBY_MODE;
    }
    else
    {
        ui32PowerMode = FLASH_PWR_ACTIVE_MODE;
    }

    //
    // Return power mode.
    //
    return(ui32PowerMode);
}

void FlashProtectionSet(uint32_t ui32SectorAddress, uint32_t ui32ProtectMode) {
    uint32_t ui32SectorNumber;

    //
    // Check the arguments.
    //
    ASSERT(ui32SectorAddress <= (FLASHMEM_BASE + FlashSizeGet() -
                                 FlashSectorSizeGet()));
    ASSERT((ui32SectorAddress & (FlashSectorSizeGet() - 1)) == 00);

    if(ui32ProtectMode == FLASH_WRITE_PROTECT)
    {
        ui32SectorNumber = (ui32SectorAddress - FLASHMEM_BASE) /
                           FlashSectorSizeGet();
        HWREG(FLASH_BASE + FLASH_O_FSM_WR_ENA) = FSM_REG_WRT_ENABLE;

        if(ui32SectorNumber <= 31)
        {
            HWREG(FLASH_BASE + FLASH_O_FSM_BSLE0) |= (1 << ui32SectorNumber);
            HWREG(FLASH_BASE + FLASH_O_FSM_BSLP0) |= (1 << ui32SectorNumber);
        }
        else if(ui32SectorNumber <= 63)
        {
            HWREG(FLASH_BASE + FLASH_O_FSM_BSLE1) |=
                (1 << (ui32SectorNumber & 0x1F));
            HWREG(FLASH_BASE + FLASH_O_FSM_BSLP1) |=
                (1 << (ui32SectorNumber & 0x1F));
        }

        HWREG(FLASH_BASE + FLASH_O_FSM_WR_ENA) = FSM_REG_WRT_DISABLE;
    }
}

uint32_t FlashProtectionGet(uint32_t ui32SectorAddress) {
    uint32_t ui32SectorProtect;
    uint32_t ui32SectorNumber;

    //
    // Check the arguments.
    //
    ASSERT(ui32SectorAddress <= (FLASHMEM_BASE + FlashSizeGet() -
                                 FlashSectorSizeGet()));
    ASSERT((ui32SectorAddress & (FlashSectorSizeGet() - 1)) == 00);

    ui32SectorProtect = FLASH_NO_PROTECT;
    ui32SectorNumber = (ui32SectorAddress - FLASHMEM_BASE) / FlashSectorSizeGet();

    if(ui32SectorNumber <= 31)
    {
        if((HWREG(FLASH_BASE + FLASH_O_FSM_BSLE0) & (1 << ui32SectorNumber)) &&
                (HWREG(FLASH_BASE + FLASH_O_FSM_BSLP0) & (1 << ui32SectorNumber)))
        {
            ui32SectorProtect = FLASH_WRITE_PROTECT;
        }
    }
    else if(ui32SectorNumber <= 63)
    {
        if((HWREG(FLASH_BASE + FLASH_O_FSM_BSLE1) &
                (1 << (ui32SectorNumber & 0x1F))) &&
                (HWREG(FLASH_BASE + FLASH_O_FSM_BSLP1) &
                 (1 << (ui32SectorNumber & 0x1F))))
        {
            ui32SectorProtect = FLASH_WRITE_PROTECT;
        }
    }

    return(ui32SectorProtect);
}

uint32_t FlashProtectionSave(uint32_t ui32SectorAddress) {
    uint32_t ui32ErrorReturn;
    uint32_t ui32SectorNumber;
    uint32_t ui32CcfgSectorAddr;
    uint32_t ui32ProgBuf;

    ui32ErrorReturn = FAPI_STATUS_SUCCESS;

    //
    // Check the arguments.
    //
    ASSERT(ui32SectorAddress <= (FLASHMEM_BASE + FlashSizeGet() -
                                 FlashSectorSizeGet()));
    ASSERT((ui32SectorAddress & (FlashSectorSizeGet() - 1)) == 00);

    if(FlashProtectionGet(ui32SectorAddress) == FLASH_WRITE_PROTECT)
    {
        //
        // Find sector number for specified sector.
        //
        ui32SectorNumber = (ui32SectorAddress - FLASHMEM_BASE) / FlashSectorSizeGet();
        ui32CcfgSectorAddr = FLASHMEM_BASE + FlashSizeGet() - FlashSectorSizeGet();

        //
        // Adjust CCFG address to the 32-bit CCFG word holding the
        // protect-bit for the specified sector.
        //
        ui32CcfgSectorAddr += (((ui32SectorNumber >> 5) * 4) + CCFG_OFFSET_SECT_PROT);

        //
        // Find value to program by setting the protect-bit which
        // corresponds to specified sector number, to 0.
        // Leave other protect-bits unchanged.
        //
        ui32ProgBuf = (~(1 << (ui32SectorNumber & 0x1F))) &
                                   *(uint32_t *)ui32CcfgSectorAddr;

        ui32ErrorReturn = FlashProgram((uint8_t*)&ui32ProgBuf, ui32CcfgSectorAddr,
                                       CCFG_SIZE_SECT_PROT);
    }

    //
    // Return status.
    //
    return(ui32ErrorReturn);
}

uint32_t FlashSectorErase(uint32_t ui32SectorAddress) {
    uint32_t ui32ErrorReturn;
    uint32_t ui32Error;
    uint32_t ui32SectorBit;
    uint32_t ui32SectorNumber;

    //
    // Check the arguments.
    //
    ASSERT(ui32SectorAddress <= (FLASHMEM_BASE + FlashSizeGet() -
                                 FlashSectorSizeGet()));
    ASSERT((ui32SectorAddress & (FlashSectorSizeGet() - 1)) == 00);

    //
    // Enable all sectors for erase.
    //
    EnableSectorsForWrite();

    //
    // Check the arguments.
    //
    if((ui32SectorAddress >
        (FLASHMEM_BASE + FlashSizeGet() - FlashSectorSizeGet())) ||
       ((ui32SectorAddress & (FlashSectorSizeGet() - 1)) != 00))
    {
        //
        // Invalid arguments. Exit function!
        //
        FlashDisableSectorsForWrite();
        return (FAPI_STATUS_INCORRECT_DATABUFFER_LENGTH);
    }

    //
    // Clear the Status register.
    //
    IssueFsmCommand(FAPI_CLEAR_STATUS);

    //
    // Unprotect sector to be erased.
    //
    ui32SectorNumber = (ui32SectorAddress - FLASHMEM_BASE) / FlashSectorSizeGet();
    ui32SectorBit = 1 << (ui32SectorNumber & 0x1F);
    HWREG(FLASH_BASE + FLASH_O_FSM_WR_ENA) = FSM_REG_WRT_ENABLE;
    if(ui32SectorNumber < 0x20)
    {
        HWREG(FLASH_BASE + FLASH_O_FSM_SECTOR1) = ~ui32SectorBit;
    }
    else
    {
        HWREG(FLASH_BASE + FLASH_O_FSM_SECTOR2) = ~ui32SectorBit;
    }
    HWREG(FLASH_BASE + FLASH_O_FSM_WR_ENA) = FSM_REG_WRT_DISABLE;

    //
    // Write the address to the FSM.
    //
    HWREG(FLASH_BASE + FLASH_O_FADDR) = ui32SectorAddress + ADDR_OFFSET;

    //
    // Issue the sector erase command to the FSM.
    //
    IssueFsmCommand(FAPI_ERASE_SECTOR);

    //
    // Wait for erase to finish.
    //
    while(FlashCheckFsmForReady() == FAPI_STATUS_FSM_BUSY)
    {
    }

    //
    // Update status.
    //
    ui32ErrorReturn = FlashCheckFsmForError();

    //
    // Disable sectors for erase.
    //
    FlashDisableSectorsForWrite();

    //
    // Check if flash top sector was erased.
    //
    if(ui32SectorAddress == (FLASHMEM_BASE + FlashSizeGet() -
                             FlashSectorSizeGet()))
    {
        //
        // Program security data to default values in the customer configuration
        // area within the flash top sector.
        //
        ui32Error = FlashProgram((uint8_t *)g_pui8CcfgDefaultSec,
                                 (ui32SectorAddress + CCFG_OFFSET_SECURITY),
                                 CCFG_SIZE_SECURITY);

        if((ui32Error != FAPI_STATUS_SUCCESS) &&
                (ui32ErrorReturn == FAPI_STATUS_SUCCESS))
        {
            ui32ErrorReturn = ui32Error;
        }
}

    //
    // Return status of operation.
    //
    return(ui32ErrorReturn);
}

uint32_t FlashBankErase(bool bForcePrecondition) {
    uint32_t ui32ErrorReturn;
    uint32_t ui32Error;
    uint32_t ui32SectorAddress;
    uint32_t ui32RegVal;

    //
    // Enable all sectors for erase.
    //
    EnableSectorsForWrite();

    //
    // Clear the Status register.
    //
    IssueFsmCommand(FAPI_CLEAR_STATUS);

    //
    // Enable erase of all sectors and enable precondition if required.
    //
    ui32RegVal = HWREG(FLASH_BASE + FLASH_O_FSM_ST_MACHINE);
    HWREG(FLASH_BASE + FLASH_O_FSM_WR_ENA) = FSM_REG_WRT_ENABLE;
    HWREG(FLASH_BASE + FLASH_O_FSM_SECTOR1) = 0x00000000;
    HWREG(FLASH_BASE + FLASH_O_FSM_SECTOR2) = 0x00000000;
    if(bForcePrecondition)
    {
        HWREG(FLASH_BASE + FLASH_O_FSM_ST_MACHINE) |=
            FLASH_FSM_ST_MACHINE_DO_PRECOND;
    }
    HWREG(FLASH_BASE + FLASH_O_FSM_WR_ENA) = FSM_REG_WRT_DISABLE;

    //
    // Issue the bank erase command to the FSM.
    //
    IssueFsmCommand(FAPI_ERASE_BANK);

    //
    // Wait for erase to finish.
    //
    while(FlashCheckFsmForReady() == FAPI_STATUS_FSM_BUSY)
    {
    }

    //
    // Update status.
    //
    ui32ErrorReturn = FlashCheckFsmForError();

    //
    // Disable sectors for erase.
    //
    FlashDisableSectorsForWrite();

    //
    // Set configured precondition mode since it may have been forced on.
    //
    if(!(ui32RegVal & FLASH_FSM_ST_MACHINE_DO_PRECOND))
    {
        HWREG(FLASH_BASE + FLASH_O_FSM_WR_ENA) = FSM_REG_WRT_ENABLE;
        HWREG(FLASH_BASE + FLASH_O_FSM_ST_MACHINE) &=
            ~FLASH_FSM_ST_MACHINE_DO_PRECOND;
        HWREG(FLASH_BASE + FLASH_O_FSM_WR_ENA) = FSM_REG_WRT_DISABLE;
    }

    //
    // Program security data to default values in the customer configuration
    // area within the flash top sector.
    //
    ui32SectorAddress = FLASHMEM_BASE + FlashSizeGet() - FlashSectorSizeGet();
    ui32Error = FlashProgram((uint8_t *)g_pui8CcfgDefaultSec,
                             (ui32SectorAddress + CCFG_OFFSET_SECURITY),
                             CCFG_SIZE_SECURITY);

    if((ui32Error != FAPI_STATUS_SUCCESS) &&
            (ui32ErrorReturn == FAPI_STATUS_SUCCESS))
    {
        ui32ErrorReturn = ui32Error;
    }

    //
    // Return status of operation.
    //
    return(ui32ErrorReturn);
}

uint32_t FlashhOtpEngrErase(void) {
    uint32_t ui32ErrorReturn;
    uint32_t ui32RegVal;

    //
    // Enable all sectors for erase.
    //
    EnableSectorsForWrite();

    //
    // Clear the Status register.
    //
    IssueFsmCommand(FAPI_CLEAR_STATUS);

    //
    // Disable OTP protection.
    //
    HWREG(FLASH_BASE + FLASH_O_FBPROT) = FLASH_FBPROT_PROTL1DIS;
    HWREG(FLASH_BASE + FLASH_O_FBAC) |= FLASH_FBAC_OTPPROTDIS;
    HWREG(FLASH_BASE + FLASH_O_FBPROT) = 0;

    //
    // Enable test commands.
    //
    HWREG(FLASH_BASE + FLASH_O_FLOCK) = 0xAAAA;
    HWREG(FLASH_BASE + FLASH_O_FTCTL) |= FLASH_FTCTL_TEST_EN;
    HWREG(FLASH_BASE + FLASH_O_FLOCK) = 0x55AA;

    //
    // Set address to OTP.
    //
    HWREG(FLASH_BASE + FLASH_O_FADDR) = 0xF0000000;

    //
    // Enable for FSM test commands and erase precondition.
    //
    ui32RegVal = HWREG(FLASH_BASE + FLASH_O_FSM_ST_MACHINE);
    HWREG(FLASH_BASE + FLASH_O_FSM_WR_ENA) = FSM_REG_WRT_ENABLE;
    HWREG(FLASH_BASE + FLASH_O_FSM_ST_MACHINE) |=
        (FLASH_FSM_ST_MACHINE_CMD_EN | FLASH_FSM_ST_MACHINE_DO_PRECOND);
    HWREG(FLASH_BASE + FLASH_O_FSM_WR_ENA) = FSM_REG_WRT_DISABLE;

    //
    // Issue the erase command to the FSM.
    //
    IssueFsmCommand(FAPI_ERASE_OTP);

    //
    // Wait for erase to finish.
    //
    while(FlashCheckFsmForReady() == FAPI_STATUS_FSM_BUSY)
    {
    }

    //
    // Update status.
    //
    ui32ErrorReturn = FlashCheckFsmForError();

    //
    // Disable sectors for erase.
    //
    FlashDisableSectorsForWrite();

    //
    // Disable test commands.
    //
    HWREG(FLASH_BASE + FLASH_O_FLOCK) = 0xAAAA;
    HWREG(FLASH_BASE + FLASH_O_FTCTL) &= ~FLASH_FTCTL_TEST_EN;
    HWREG(FLASH_BASE + FLASH_O_FLOCK) = 0x55AA;

    //
    // Renable OTP protection.
    //
    HWREG(FLASH_BASE + FLASH_O_FBPROT) = FLASH_FBPROT_PROTL1DIS;
    HWREG(FLASH_BASE + FLASH_O_FBAC) &= ~FLASH_FBAC_OTPPROTDIS;
    HWREG(FLASH_BASE + FLASH_O_FBPROT) = 0;

    //
    // Disable FSM test command mode.
    //
    HWREG(FLASH_BASE + FLASH_O_FSM_WR_ENA) = FSM_REG_WRT_ENABLE;
    HWREG(FLASH_BASE + FLASH_O_FSM_ST_MACHINE) &= ~FLASH_FSM_ST_MACHINE_CMD_EN;

    //
    // Set configured precondition mode since it may have been changed.
    //
    if(!(ui32RegVal & FLASH_FSM_ST_MACHINE_DO_PRECOND))
    {
        HWREG(FLASH_BASE + FLASH_O_FSM_ST_MACHINE) &=
            ~FLASH_FSM_ST_MACHINE_DO_PRECOND;
    }
    HWREG(FLASH_BASE + FLASH_O_FSM_WR_ENA) = FSM_REG_WRT_DISABLE;

    //
    // Return status of operation.
    //
    return(ui32ErrorReturn);
}

uint32_t FlashProgram(uint8_t *pui8DataBuffer, uint32_t ui32Address, uint32_t ui32Count) {
    uint32_t ui32StartIndex;
    uint32_t ui32StopIndex;
    uint32_t ui32Index;
    uint8_t ui8BankWidth;
    uint8_t ui8NoOfBytes;
    tFwpWriteByte *oFwpWriteByte;
    uint32_t ui32ErrorReturn;

    //
    // Check the arguments.
    //
    ASSERT((ui32Address + ui32Count) <= (FLASHMEM_BASE + FlashSizeGet()));

    //
    // Enable sectors for programming.
    //
    EnableSectorsForWrite();
    oFwpWriteByte = FWPWRITE_BYTE_ADDRESS;

    //
    // Check the arguments.
    //
    if((ui32Address + ui32Count) > (FLASHMEM_BASE + FlashSizeGet()))
    {
        //
        // Invalid arguments. Exit function!
        //
        FlashDisableSectorsForWrite();
        return (FAPI_STATUS_INCORRECT_DATABUFFER_LENGTH);
    }

    //
    // Set the status to indicate success.
    //
    ui32ErrorReturn = FAPI_STATUS_SUCCESS;

    //
    // Find flash bank width in number of bytes.
    //
    ui8BankWidth =
        (uint8_t)(((HWREG(FLASH_BASE + FLASH_O_FCFG_BANK) &
                    FLASH_FCFG_BANK_MAIN_BANK_WIDTH_M) >>
                   FLASH_FCFG_BANK_MAIN_BANK_WIDTH_S) >> 3);

    //
    // Loop over the bytes to be programmed.
    //
    while(ui32Count)
    {
        //
        // Setup the start position within the write data registers.
        //
        ui32StartIndex = ui32Address & (uint32_t)(ui8BankWidth - 1);

        //
        // Setup number of bytes to program.
        //
        ui8NoOfBytes = ui8BankWidth - ui32StartIndex;
        if(ui8NoOfBytes > ui32Count)
        {
            ui8NoOfBytes = ui32Count;
        }

        //
        // Clear the Status register.
        //
        IssueFsmCommand(FAPI_CLEAR_STATUS);

        //
        // Write address to FADDR register.
        //
        HWREG(FLASH_BASE + FLASH_O_FADDR) = ui32Address + ADDR_OFFSET;

        //
        // Setup the stop position within the write data registers.
        //
        ui32StopIndex = ui32StartIndex + (uint32_t)(ui8NoOfBytes - 1);

        //
        // Write each byte to the FWPWrite registers.
        //
        for(ui32Index = ui32StartIndex; ui32Index <= ui32StopIndex; ui32Index++)
        {
            oFwpWriteByte[ui32Index] = *(pui8DataBuffer++);
        }

        //
        // Issue the Program command to the FSM.
        //
        IssueFsmCommand(FAPI_PROGRAM_DATA);

        //
        // Wait until the word has been programmed.
        //
        while(FlashCheckFsmForReady() == FAPI_STATUS_FSM_BUSY)
        {
        }

        //
        // Exit if an access violation occurred.
        //
        ui32ErrorReturn = FlashCheckFsmForError();
        if(ui32ErrorReturn != FAPI_STATUS_SUCCESS)
        {
            break;
        }

        //
        // Prepare for next data burst.
        //
        ui32Count   -= ((ui32StopIndex - ui32StartIndex) + 1);
        ui32Address += ((ui32StopIndex - ui32StartIndex) + 1);
    }

    //
    // Disable sectors for programming.
    //
    FlashDisableSectorsForWrite();

    //
    // Return status of operation.
    //
    return(ui32ErrorReturn);
}

uint32_t FlashProgramNowait(uint32_t ui32StartAddress, uint8_t *pui8DataBuffer, uint8_t ui8NoOfBytes) {
    uint32_t     ui32StartIndex;
    uint32_t     ui32StopIndex;
    uint32_t     ui32Index;
    uint32_t     ui32BankWidth;
    uint32_t     ui32ErrorReturn;
    tFwpWriteByte    *oFwpWriteByte;

    //
    // Check the arguments.
    //
    ASSERT((ui32StartAddress + ui8NoOfBytes) <= (FLASHMEM_BASE + FlashSizeGet()));

    //
    // Enable sectors for programming.
    //
    EnableSectorsForWrite();
    oFwpWriteByte = FWPWRITE_BYTE_ADDRESS;

    //
    // Check the arguments.
    //
    if((ui32StartAddress + ui8NoOfBytes) > (FLASHMEM_BASE + FlashSizeGet()))
    {
        //
        // Invalid arguments. Exit function!
        //
        FlashDisableSectorsForWrite();
        return (FAPI_STATUS_INCORRECT_DATABUFFER_LENGTH);
    }

    //
    // Set status to indicate success
    //
    ui32ErrorReturn  = FAPI_STATUS_SUCCESS;

    //
    // Find flash bank width in number of bytes.
    //
    ui32BankWidth = (((HWREG(FLASH_BASE + FLASH_O_FCFG_BANK) &
                       FLASH_FCFG_BANK_MAIN_BANK_WIDTH_M) >>
                      FLASH_FCFG_BANK_MAIN_BANK_WIDTH_S) >> 3);

    //
    // Setup the start position within the write data registers.
    //
    ui32StartIndex = ui32StartAddress & (ui32BankWidth - 1);

    //
    // Check to see if there is more data in the buffer than the register.
    // width.
    //
    if((ui8NoOfBytes == 0) || ((ui32StartIndex + ui8NoOfBytes) > ui32BankWidth))
    {
        ui32ErrorReturn = FAPI_STATUS_INCORRECT_DATABUFFER_LENGTH;
    }

    if(ui32ErrorReturn == FAPI_STATUS_SUCCESS)
    {
        //
        // Clear the Status register.
        //
        IssueFsmCommand(FAPI_CLEAR_STATUS);

        //
        // Write address to FADDR register.
        //
        HWREG(FLASH_BASE + FLASH_O_FADDR) = ui32StartAddress + ADDR_OFFSET;

        //
        // Setup the stop position within the write data registers.
        //
        ui32StopIndex  = ui32StartIndex + (uint32_t)(ui8NoOfBytes - 1);

        //
        // Write each byte to the FWPWrite registers.
        //
        for(ui32Index = ui32StartIndex; ui32Index <= ui32StopIndex; ui32Index++)
        {
            oFwpWriteByte[ui32Index] = *(pui8DataBuffer++);
}

        //
        // Issue the Program command to the FSM.
        //
        IssueFsmCommand(FAPI_PROGRAM_DATA);
    }

    //
    // Return the function status.
    //
    return(ui32ErrorReturn);
}

bool FlashEfuseReadRow(uint32_t *pui32EfuseData, uint32_t ui32RowAddress) {
    bool bStatus;

    //
    // Make sure the clock for the efuse is enabled
    //
    HWREG(FLASH_BASE + FLASH_O_CFG) &= ~FLASH_CFG_DIS_EFUSECLK;

    //
    // Set timing for EFUSE read operations.
    //
    HWREG(FLASH_BASE + FLASH_O_EFUSEREAD) |= ((5 << FLASH_EFUSEREAD_READCLOCK_S) &
            FLASH_EFUSEREAD_READCLOCK_M);

    //
    // Clear status register.
    //
    HWREG(FLASH_BASE + FLASH_O_EFUSEERROR) = 0;

    //
    // Select the FuseROM block 0.
    //
    HWREG(FLASH_BASE + FLASH_O_EFUSEADDR) = 0x00000000;

    //
    // Start the read operation.
    //
    HWREG(FLASH_BASE + FLASH_O_EFUSE) =
        (DUMPWORD_INSTR << FLASH_EFUSE_INSTRUCTION_S) |
        (ui32RowAddress & FLASH_EFUSE_DUMPWORD_M);

    //
    // Wait for operation to finish.
    //
    while(!(HWREG(FLASH_BASE + FLASH_O_EFUSEERROR) & FLASH_EFUSEERROR_DONE))
    {
    }

    //
    // Check if error reported.
    //
    if(HWREG(FLASH_BASE + FLASH_O_EFUSEERROR) & FLASH_EFUSEERROR_CODE_M)
    {
        //
        // Set error status.
        //
        bStatus = 1;

        //
        // Clear data.
        //
        *pui32EfuseData = 0;
    }
    else
    {
        //
        // Set ok status.
        //
        bStatus = 0;

        //
        // No error. Get data from data register.
        //
        *pui32EfuseData = HWREG(FLASH_BASE + FLASH_O_DATALOWER);
    }

    //
    // Disable the efuse clock to conserve power
    //
    HWREG(FLASH_BASE + FLASH_O_CFG) |= FLASH_CFG_DIS_EFUSECLK;

    //
    // Return the data.
    //
    return(bStatus);
}

uint32_t FlashProgramPattern(uint32_t ui32SectorAddress, uint32_t ui32DataPattern, bool bInvertData) {
    uint8_t ui8Index;
    uint8_t ui8BankWidth;
    tFwpWriteByte *oFwpWriteByte;
    uint32_t ui32ErrorReturn;

    //
    // Check the arguments.
    //
    ASSERT(ui32SectorAddress <= (FLASHMEM_BASE + FlashSizeGet() -
                                 FlashSectorSizeGet()));
    ASSERT((ui32SectorAddress & (FlashSectorSizeGet() - 1)) == 00);

    //
    // Enable sectors for programming.
    //
    EnableSectorsForWrite();
    oFwpWriteByte = FWPWRITE_BYTE_ADDRESS;

    //
    // Check the arguments.
    //
    if((ui32SectorAddress >
       (FLASHMEM_BASE + FlashSizeGet() - FlashSectorSizeGet())) ||
       ((ui32SectorAddress & (FlashSectorSizeGet() - 1)) != 00))
    {
        //
        // Invalid arguments. Exit function!
        //
        FlashDisableSectorsForWrite();
        return (FAPI_STATUS_INCORRECT_DATABUFFER_LENGTH);
    }

    //
    // Find flash bank width in number of bytes.
    //
    ui8BankWidth =
        (uint8_t)(((HWREG(FLASH_BASE + FLASH_O_FCFG_BANK) &
                    FLASH_FCFG_BANK_MAIN_BANK_WIDTH_M) >>
                   FLASH_FCFG_BANK_MAIN_BANK_WIDTH_S) >> 3);

    //
    // Clear the Status register.
    //
    IssueFsmCommand(FAPI_CLEAR_STATUS);

    //
    // Write address to FADDR register.
    //
    HWREG(FLASH_BASE + FLASH_O_FADDR) = ui32SectorAddress + ADDR_OFFSET;

    //
    // Write each byte of the pattern to the FWPWrite registers.
    //
    for(ui8Index = 0; ui8Index < ui8BankWidth; ui8Index++)
    {
        oFwpWriteByte[ui8Index] = ui32DataPattern >> ((ui8Index * 8) &
                                  (PATTERN_BITS - 1));
    }

    //
    // Enable for FSM test command and enable the Invert Data option if
    // required.
    //
    HWREG(FLASH_BASE + FLASH_O_FSM_WR_ENA) = FSM_REG_WRT_ENABLE;
    HWREG(FLASH_BASE + FLASH_O_FSM_ST_MACHINE) |= FLASH_FSM_ST_MACHINE_CMD_EN;
    if(bInvertData)
    {
        HWREG(FLASH_BASE + FLASH_O_FSM_ST_MACHINE) |= FLASH_FSM_ST_MACHINE_INV_DATA;
    }
    HWREG(FLASH_BASE + FLASH_O_FSM_WR_ENA) = FSM_REG_WRT_DISABLE;

    //
    // Issue the Program command to the FSM.
    //
    IssueFsmCommand(FAPI_PROGRAM_SECTOR);

    //
    // Wait until the sector has been programmed.
    //
    while(FlashCheckFsmForReady() == FAPI_STATUS_FSM_BUSY)
    {
    }

    //
    // Update status of the program operation.
    //
    ui32ErrorReturn = FlashCheckFsmForError();

    //
    // Disable sectors for programming.
    //
    FlashDisableSectorsForWrite();

    //
    // Disable FSM test command mode and the Invert Data option.
    //
    HWREG(FLASH_BASE + FLASH_O_FSM_WR_ENA) = FSM_REG_WRT_ENABLE;
    HWREG(FLASH_BASE + FLASH_O_FSM_ST_MACHINE) &= ~FLASH_FSM_ST_MACHINE_CMD_EN;
    HWREG(FLASH_BASE + FLASH_O_FSM_ST_MACHINE) &= ~FLASH_FSM_ST_MACHINE_INV_DATA;
    HWREG(FLASH_BASE + FLASH_O_FSM_WR_ENA) = FSM_REG_WRT_DISABLE;

    //
    // Return status of operation.
    //
    return(ui32ErrorReturn);
}

uint32_t FlashProgramEngr(uint8_t *pui8DataBuffer, uint32_t ui32AddressOffset, uint32_t ui32Count) {
    uint32_t ui32StartIndex;
    uint32_t ui32StopIndex;
    uint32_t ui32Index;
    uint8_t ui8BankWidth;
    uint8_t ui8NoOfBytes;
    tFwpWriteByte *oFwpWriteByte;
    uint32_t ui32ErrorReturn;

    //
    // Check the arguments.
    //
    ASSERT((ui32AddressOffset + ui32Count) <= 1024);

    //
    // Enable sectors for programming.
    //
    EnableSectorsForWrite();
    oFwpWriteByte = FWPWRITE_BYTE_ADDRESS;

    //
    // Check the arguments.
    //
    if((ui32AddressOffset + ui32Count) > 1024)
    {
        //
        // Invalid arguments. Exit function!
        //
        FlashDisableSectorsForWrite();
        return (FAPI_STATUS_INCORRECT_DATABUFFER_LENGTH);
    }

    //
    // Set the status to indicate success.
    //
    ui32ErrorReturn = FAPI_STATUS_SUCCESS;

    //
    // Find flash bank width in number of bytes.
    //
    ui8BankWidth =
        (uint8_t)(((HWREG(FLASH_BASE + FLASH_O_FCFG_BANK) &
                    FLASH_FCFG_BANK_MAIN_BANK_WIDTH_M) >>
                   FLASH_FCFG_BANK_MAIN_BANK_WIDTH_S) >> 3);

    //
    // Disable OTP protection.
    //
    //
    HWREG(FLASH_BASE + FLASH_O_FBPROT) = FLASH_FBPROT_PROTL1DIS;
    HWREG(FLASH_BASE + FLASH_O_FBAC) |= FLASH_FBAC_OTPPROTDIS;
    HWREG(FLASH_BASE + FLASH_O_FBPROT) = 0;

    //
    // Enable test commands.
    //
    HWREG(FLASH_BASE + FLASH_O_FLOCK) = 0xAAAA;
    HWREG(FLASH_BASE + FLASH_O_FTCTL) |= FLASH_FTCTL_TEST_EN;
    HWREG(FLASH_BASE + FLASH_O_FLOCK) = 0x55AA;

    //
    // Enable for FSM test command.
    //
    HWREG(FLASH_BASE + FLASH_O_FSM_WR_ENA) = FSM_REG_WRT_ENABLE;
    HWREG(FLASH_BASE + FLASH_O_FSM_ST_MACHINE) |= FLASH_FSM_ST_MACHINE_CMD_EN;
    HWREG(FLASH_BASE + FLASH_O_FSM_WR_ENA) = FSM_REG_WRT_DISABLE;

    //
    // Loop over the bytes to be programmed.
    //
    while(ui32Count)
    {
        //
        // Setup the start position within the write data registers.
        //
        ui32StartIndex = ui32AddressOffset & (uint32_t)(ui8BankWidth - 1);

        //
        // Setup number of bytes to program.
        //
        ui8NoOfBytes = ui8BankWidth - ui32StartIndex;
        if(ui8NoOfBytes > ui32Count)
        {
            ui8NoOfBytes = ui32Count;
        }

        //
        // Clear the Status register.
        //
        IssueFsmCommand(FAPI_CLEAR_STATUS);

        //
        // Write address to FADDR register.
        //
        HWREG(FLASH_BASE + FLASH_O_FADDR) = ui32AddressOffset + 0xF0080000;

        //
        // Setup the stop position within the write data registers.
        //
        ui32StopIndex = ui32StartIndex + (uint32_t)(ui8NoOfBytes - 1);

        //
        // Write each byte to the FWPWrite registers.
        //
        for(ui32Index = ui32StartIndex; ui32Index <= ui32StopIndex; ui32Index++)
        {
            oFwpWriteByte[ui32Index] = *(pui8DataBuffer++);
        }

        //
        // Issue programming command.
        //
        IssueFsmCommand(FAPI_PROGRAM_DATA);

        //
        // Wait until the word has been programmed.
        //
        while(FlashCheckFsmForReady() == FAPI_STATUS_FSM_BUSY)
        {
        }

        //
        // Update error status and exit if an error occurred.
        //
        ui32ErrorReturn = FlashCheckFsmForError();
        if(ui32ErrorReturn != FAPI_STATUS_SUCCESS)
        {
            break;
        }

        //
        // Prepare for next data burst.
        //
        ui32Count   -= ((ui32StopIndex - ui32StartIndex) + 1);
        ui32AddressOffset += ((ui32StopIndex - ui32StartIndex) + 1);
    }

    //
    // Disable sectors for programming.
    //
    FlashDisableSectorsForWrite();

    //
    // Reenable OTP protection.
    //
    HWREG(FLASH_BASE + FLASH_O_FBPROT) = FLASH_FBPROT_PROTL1DIS;
    HWREG(FLASH_BASE + FLASH_O_FBAC) &= ~FLASH_FBAC_OTPPROTDIS;
    HWREG(FLASH_BASE + FLASH_O_FBPROT) = 0;

    //
    // Disable test commands.
    //
    HWREG(FLASH_BASE + FLASH_O_FLOCK) = 0xAAAA;
    HWREG(FLASH_BASE + FLASH_O_FTCTL) &= ~FLASH_FTCTL_TEST_EN;
    HWREG(FLASH_BASE + FLASH_O_FLOCK) = 0x55AA;

    //
    // Disable FSM test command mode.
    //
    HWREG(FLASH_BASE + FLASH_O_FSM_WR_ENA) = FSM_REG_WRT_ENABLE;
    HWREG(FLASH_BASE + FLASH_O_FSM_ST_MACHINE) &= ~FLASH_FSM_ST_MACHINE_CMD_EN;
    HWREG(FLASH_BASE + FLASH_O_FSM_WR_ENA) = FSM_REG_WRT_DISABLE;

    //
    // Return status of operation.
    //
    return(ui32ErrorReturn);
}

void FlashOtpProgramEraseSetup(void) {
    //
    // Disable OTP protection.
    //
    HWREG(FLASH_BASE + FLASH_O_FBPROT) = FLASH_FBPROT_PROTL1DIS;
    HWREG(FLASH_BASE + FLASH_O_FBAC) |= FLASH_FBAC_OTPPROTDIS;
    HWREG(FLASH_BASE + FLASH_O_FBPROT) = 0;

    //
    // Enable test commands by performing the following steps:
    // - Enable SW Interface mode
    // - Enable for test commands
    //
    HWREG(FLASH_BASE + FLASH_O_FLOCK) = 0x0000AAAA;
    HWREG(FLASH_BASE + FLASH_O_FTCTL) |= FLASH_FTCTL_TEST_EN;
    HWREG(FLASH_BASE + FLASH_O_FLOCK) = 0x000055AA;

    //
    // Enable for FSM test commands.
    //
    HWREG(FLASH_BASE + FLASH_O_FSM_WR_ENA) = FSM_REG_WRT_ENABLE;
    HWREG(FLASH_BASE + FLASH_O_FSM_ST_MACHINE) |= FLASH_FSM_ST_MACHINE_CMD_EN;
    HWREG(FLASH_BASE + FLASH_O_FSM_WR_ENA) = FSM_REG_WRT_DISABLE;
}

void FlashOtpProgramEraseCleanup(void) {
    //
    // Reenable OTP protection.
    //
    HWREG(FLASH_BASE + FLASH_O_FBPROT) = FLASH_FBPROT_PROTL1DIS;
    HWREG(FLASH_BASE + FLASH_O_FBAC) &= ~FLASH_FBAC_OTPPROTDIS;
    HWREG(FLASH_BASE + FLASH_O_FBPROT) = 0;

    //
    // Disable test commands and turn off SW interface mode.
    //
    HWREG(FLASH_BASE + FLASH_O_FLOCK) = 0x0000AAAA;
    HWREG(FLASH_BASE + FLASH_O_FTCTL) &= ~FLASH_FTCTL_TEST_EN;
    HWREG(FLASH_BASE + FLASH_O_FLOCK) = 0x55AA;

    //
    // Disable FSM test command mode.
    //
    HWREG(FLASH_BASE + FLASH_O_FSM_WR_ENA) = FSM_REG_WRT_ENABLE;
    HWREG(FLASH_BASE + FLASH_O_FSM_ST_MACHINE) &= ~FLASH_FSM_ST_MACHINE_CMD_EN;
    HWREG(FLASH_BASE + FLASH_O_FSM_WR_ENA) = FSM_REG_WRT_DISABLE;
}

void FlashDisableSectorsForWrite(void) {
    //
    // Configure flash back to read mode
    //
    SetReadMode();

    //
    // Disable Level 1 Protection.
    //
    HWREG(FLASH_BASE + FLASH_O_FBPROT) = FLASH_FBPROT_PROTL1DIS;

    //
    // Disable all sectors for erase and programming.
    //
    HWREG(FLASH_BASE + FLASH_O_FBSE) = 0x0000;

    //
    // Enable Level 1 Protection.
    //
    HWREG(FLASH_BASE + FLASH_O_FBPROT) = 0;

    //
    // Protect sectors from sector erase.
    //
    HWREG(FLASH_BASE + FLASH_O_FSM_WR_ENA) = FSM_REG_WRT_ENABLE;
    HWREG(FLASH_BASE + FLASH_O_FSM_SECTOR1) = 0xFFFFFFFF;
    HWREG(FLASH_BASE + FLASH_O_FSM_SECTOR2) = 0xFFFFFFFF;
    HWREG(FLASH_BASE + FLASH_O_FSM_WR_ENA) = FSM_REG_WRT_DISABLE;
}
//*****************************************************************************
//
//! \internal
//! Issues a command to the Flash State Machine.
//!
//! \param eCommand specifies the FSM command.
//!
//! Issues a command to the Flash State Machine.
//!
//! \return None
//
//*****************************************************************************
static void
IssueFsmCommand(tFlashStateCommandsType eCommand)
{
    //
    // Check the arguments.
    //
    ASSERT(
        eCommand == FAPI_ERASE_SECTOR    || eCommand == FAPI_ERASE_BANK ||
        eCommand == FAPI_VALIDATE_SECTOR || eCommand == FAPI_CLEAR_STATUS ||
        eCommand == FAPI_PROGRAM_RESUME  || eCommand == FAPI_ERASE_RESUME ||
        eCommand == FAPI_CLEAR_MORE      || eCommand == FAPI_PROGRAM_SECTOR ||
        eCommand == FAPI_PROGRAM_DATA    || eCommand == FAPI_ERASE_OTP);

    //
    // Enable write to FSM register.
    //
    HWREG(FLASH_BASE + FLASH_O_FSM_WR_ENA) = FSM_REG_WRT_ENABLE;

    //
    // Issue FSM command.
    //
    HWREG(FLASH_BASE + FLASH_O_FSM_CMD) = eCommand;

    //
    // Start command execute.
    //
    HWREG(FLASH_BASE + FLASH_O_FSM_EXECUTE) = FLASH_CMD_EXEC;

    //
    // Disable write to FSM register.
    //
    HWREG(FLASH_BASE + FLASH_O_FSM_WR_ENA) = FSM_REG_WRT_DISABLE;
}

//*****************************************************************************
//
//! \internal
//! Enables all sectors for erase and programming on the active bank.
//!
//! This function disables the idle reading power reduction mode, selects the
//! flash bank and enables all sectors for erase and programming on the active
//! bank.
//! Sectores may be protected from programming depending on the value of the
//! FLASH_O_FSM_BSLPx registers.
//! Sectores may be protected from erase depending on the value of the
//! FLASH_O_FSM_BSLEx registers. Additional sector erase protection is set by
//! the FLASH_O_FSM_SECTOR1 register.
//!
//! \return None
//
//*****************************************************************************
static void
EnableSectorsForWrite(void)
{
    //
    // Trim flash module for program/erase operation.
    //
    TrimForWrite();

    //
    // Configure flash to write mode
    //
    SetWriteMode();

    //
    // Select flash bank.
    //
    HWREG(FLASH_BASE + FLASH_O_FMAC) = 0x00;

    //
    // Disable Level 1 Protection.
    //
    HWREG(FLASH_BASE + FLASH_O_FBPROT) = FLASH_FBPROT_PROTL1DIS;

    //
    // Enable all sectors for erase and programming.
    //
    HWREG(FLASH_BASE + FLASH_O_FBSE) = 0xFFFF;

    //
    // Enable Level 1 Protection
    //
    HWREG(FLASH_BASE + FLASH_O_FBPROT) = 0;
}

//*****************************************************************************
//
//! \internal
//! Trims the Flash Bank and Flash Pump for program/erase functionality
//!
//! This trimming will make it possible to perform erase and program operations
//! of the flash. Trim values are loaded from factory configuration area
//! (referred to as FCGF1). The trimming done by this function is valid until
//! reset of the flash module.
//!
//! Some registers shall be written with a value that is a number of FCLK
//! cycles. The trim values controlling these registers have a value of
//! number of half us. FCLK = SysClk / ((RWAIT+1) x 2).
//! In order to calculate the register value for these registers the
//! following calculation must be done:
//!
//!                                    OtpValue                   SysClkMHz
//!                                    -------- us      OtpValue x ---------
//!                                       2                       (RWAIT+1)
//! RegValue_in_no_of_clk_cycles = ----------------- = ---------------------
//!                                       1                     4
//!                                 --------------
//!                                   SysClkMHz
//!                                  ------------
//!                                  (RWAIT+1)x 2
//!
//! This is equevivalent to:
//!
//!                                           16 x SysClkMHz
//!                                OtpValue x ---------------
//!                                              (RWAIT+1)
//! RegValue_in_no_of_clk_cycles = ----------------------------
//!                                           64
//!
//!                                                 16 x SysClkMHz
//! A scaling factor is set equal to: ui32FclkScale = --------------
//!                                                    (RWAIT+1)
//!
//! which gives:
//!                                 OtpValue x ui32FclkScale
//! RegValue_in_no_of_clk_cycles = ------------------------
//!                                          64
//!
//! \return None.
//
//*****************************************************************************
static void
TrimForWrite(void)
{
    uint32_t ui32Value;
    uint32_t ui32TempVal;
    uint32_t ui32FclkScale;
    uint32_t ui32RWait;

    //
    // Return if flash is already trimmed for program/erase operations.
    //
    if(HWREG(FLASH_BASE + FLASH_O_FWFLAG) & FW_WRT_TRIMMED)
    {
        return;
    }

    //***********************************************************************//
    //                                                                       //
    //                 Configure the FSM registers                           //
    //                                                                       //
    //***********************************************************************//

    //
    // Enable access to the FSM registers.
    //
    HWREG(FLASH_BASE + FLASH_O_FSM_WR_ENA) = FSM_REG_WRT_ENABLE;

    //
    // Determine the scaling value to be used on timing related trim values.
    // The scaling value is based on the flash module clock frequency and RWAIT
    //
    ui32RWait     = (HWREG(FLASH_BASE + FLASH_O_FRDCTL) &
                     FLASH_FRDCTL_RWAIT_M) >> FLASH_FRDCTL_RWAIT_S;
    ui32FclkScale = (16 * FLASH_MODULE_CLK_FREQ) / (ui32RWait + 1);

    //
    // Configure Program puls width bits 15:0.
    // (FCFG1 offset 0x188 bits 15:0).
    //
    ui32Value =
        (HWREG(FLASH_CFG_BASE + FCFG1_OFFSET + FCFG1_O_FLASH_PROG_EP) &
         FCFG1_FLASH_PROG_EP_PROGRAM_PW_M) >>
         FCFG1_FLASH_PROG_EP_PROGRAM_PW_S;

    ui32Value = ScaleCycleValues(ui32Value, ui32FclkScale);

    HWREG(FLASH_BASE + FLASH_O_FSM_PRG_PW) =
        (HWREG(FLASH_BASE + FLASH_O_FSM_PRG_PW) &
         ~FLASH_FSM_PRG_PW_PROG_PUL_WIDTH_M) |
        ((ui32Value << FLASH_FSM_PRG_PW_PROG_PUL_WIDTH_S) &
         FLASH_FSM_PRG_PW_PROG_PUL_WIDTH_M);

    //
    // Configure Erase puls width bits 31:0.
    // (FCFG1 offset 0x18C bits 31:0).
    //
    ui32Value =
        (HWREG(FLASH_CFG_BASE + FCFG1_OFFSET + FCFG1_O_FLASH_ERA_PW) &
         FCFG1_FLASH_ERA_PW_ERASE_PW_M) >>
         FCFG1_FLASH_ERA_PW_ERASE_PW_S;

    ui32Value = ScaleCycleValues(ui32Value, ui32FclkScale);

    HWREG(FLASH_BASE + FLASH_O_FSM_ERA_PW) =
        (HWREG(FLASH_BASE + FLASH_O_FSM_ERA_PW) &
         ~FLASH_FSM_ERA_PW_FSM_ERA_PW_M) |
        ((ui32Value << FLASH_FSM_ERA_PW_FSM_ERA_PW_S) &
         FLASH_FSM_ERA_PW_FSM_ERA_PW_M);


    //
    // Configure no of flash clock cycles from EXECUTEZ going low to the the
    // verify data can be read in the program verify mode bits 7:0.
    // (FCFG1 offset 0x174 bits 23:16).
    //
    ui32Value =
        (HWREG(FLASH_CFG_BASE + FCFG1_OFFSET + FCFG1_O_FLASH_C_E_P_R) &
         FCFG1_FLASH_C_E_P_R_PV_ACCESS_M) >>
         FCFG1_FLASH_C_E_P_R_PV_ACCESS_S;

    ui32Value = ScaleCycleValues(ui32Value, ui32FclkScale);

    HWREG(FLASH_BASE + FLASH_O_FSM_EX_VAL) =
        (HWREG(FLASH_BASE + FLASH_O_FSM_EX_VAL) &
         ~FLASH_FSM_EX_VAL_EXE_VALD_M) |
        ((ui32Value << FLASH_FSM_EX_VAL_EXE_VALD_S) &
         FLASH_FSM_EX_VAL_EXE_VALD_M);

    //
    // Configure the number of flash clocks from the start of the Read mode at
    // the end of the operations until the FSM clears the BUSY bit in FMSTAT.
    // (FCFG1 offset 0x178 bits 23:16).
    //
    ui32Value =
        (HWREG(FLASH_CFG_BASE + FCFG1_OFFSET + FCFG1_O_FLASH_P_R_PV) &
         FCFG1_FLASH_P_R_PV_RH_M) >>
         FCFG1_FLASH_P_R_PV_RH_S;

    HWREG(FLASH_BASE + FLASH_O_FSM_RD_H) =
        (HWREG(FLASH_BASE + FLASH_O_FSM_RD_H) &
         ~FLASH_FSM_RD_H_RD_H_M) |
        ((ui32Value << FLASH_FSM_RD_H_RD_H_S) &
         FLASH_FSM_RD_H_RD_H_M);

    //
    // Configure Program hold time
    // (FCFG1 offset 0x178 bits 31:24).
    //
    ui32Value =
        (HWREG(FLASH_CFG_BASE + FCFG1_OFFSET + FCFG1_O_FLASH_P_R_PV) &
         FCFG1_FLASH_P_R_PV_PH_M) >>
         FCFG1_FLASH_P_R_PV_PH_S;

    ui32Value = ScaleCycleValues(ui32Value, ui32FclkScale);

    HWREG(FLASH_BASE + FLASH_O_FSM_P_OH) =
        (HWREG(FLASH_BASE + FLASH_O_FSM_P_OH) &
         ~FLASH_FSM_P_OH_PGM_OH_M) |
        ((ui32Value << FLASH_FSM_P_OH_PGM_OH_S) &
         FLASH_FSM_P_OH_PGM_OH_M);

    //
    // Configure Erase hold time
    // (FCFG1 offset 0x17C bits 31:24).
    //
    ui32Value =
        (HWREG(FLASH_CFG_BASE + FCFG1_OFFSET + FCFG1_O_FLASH_EH_SEQ) &
         FCFG1_FLASH_EH_SEQ_EH_M) >>
         FCFG1_FLASH_EH_SEQ_EH_S;

    ui32Value = ScaleCycleValues(ui32Value, ui32FclkScale);

    HWREG(FLASH_BASE + FLASH_O_FSM_ERA_OH) =
        (HWREG(FLASH_BASE + FLASH_O_FSM_ERA_OH) &
         ~FLASH_FSM_ERA_OH_ERA_OH_M) |
        ((ui32Value << FLASH_FSM_ERA_OH_ERA_OH_S) &
         FLASH_FSM_ERA_OH_ERA_OH_M);

    //
    // Configure Program verify row switch time
    // (FCFG1 offset0x178 bits 15:8).
    //
    ui32Value =
        (HWREG(FLASH_CFG_BASE + FCFG1_OFFSET + FCFG1_O_FLASH_P_R_PV) &
         FCFG1_FLASH_P_R_PV_PVH_M) >>
         FCFG1_FLASH_P_R_PV_PVH_S;

    ui32Value = ScaleCycleValues(ui32Value, ui32FclkScale);

    HWREG(FLASH_BASE + FLASH_O_FSM_PE_VH) =
        (HWREG(FLASH_BASE + FLASH_O_FSM_PE_VH) &
         ~FLASH_FSM_PE_VH_PGM_VH_M) |
        ((ui32Value << FLASH_FSM_PE_VH_PGM_VH_S) &
         FLASH_FSM_PE_VH_PGM_VH_M);

    //
    // Configure Program Operation Setup time
    // (FCFG1 offset 0x170 bits 31:24).
    //
    ui32Value = (HWREG(FLASH_CFG_BASE + FCFG1_OFFSET + FCFG1_O_FLASH_E_P) &
                 FCFG1_FLASH_E_P_PSU_M) >>
                 FCFG1_FLASH_E_P_PSU_S;

    HWREG(FLASH_BASE + FLASH_O_FSM_PE_OSU) =
        (HWREG(FLASH_BASE + FLASH_O_FSM_PE_OSU) &
         ~FLASH_FSM_PE_OSU_PGM_OSU_M) |
        ((ui32Value << FLASH_FSM_PE_OSU_PGM_OSU_S) &
         FLASH_FSM_PE_OSU_PGM_OSU_M);

    //
    // Configure Erase Operation Setup time
    // (FCGF1 offset 0x170 bits 23:16).
    //
    ui32Value = (HWREG(FLASH_CFG_BASE + FCFG1_OFFSET + FCFG1_O_FLASH_E_P) &
                 FCFG1_FLASH_E_P_ESU_M) >>
                 FCFG1_FLASH_E_P_ESU_S;

    HWREG(FLASH_BASE + FLASH_O_FSM_PE_OSU) =
        (HWREG(FLASH_BASE + FLASH_O_FSM_PE_OSU) &
         ~FLASH_FSM_PE_OSU_ERA_OSU_M) |
        ((ui32Value << FLASH_FSM_PE_OSU_ERA_OSU_S) &
         FLASH_FSM_PE_OSU_ERA_OSU_M);

    //
    // Confgure Program Verify Setup time
    // (FCFG1 offset 0x170 bits 15:8).
    //
    ui32Value = (HWREG(FLASH_CFG_BASE + FCFG1_OFFSET + FCFG1_O_FLASH_E_P) &
                 FCFG1_FLASH_E_P_PVSU_M) >>
                 FCFG1_FLASH_E_P_PVSU_S;

    HWREG(FLASH_BASE + FLASH_O_FSM_PE_VSU) =
        (HWREG(FLASH_BASE + FLASH_O_FSM_PE_VSU) &
         ~FLASH_FSM_PE_VSU_PGM_VSU_M) |
        ((ui32Value << FLASH_FSM_PE_VSU_PGM_VSU_S) &
         FLASH_FSM_PE_VSU_PGM_VSU_M);

    //
    // Configure Erase Verify Setup time
    // (FCFG1 offset 0x170 bits 7:0).
    //
    ui32Value = (HWREG(FLASH_CFG_BASE + FCFG1_OFFSET + FCFG1_O_FLASH_E_P) &
                 FCFG1_FLASH_E_P_EVSU_M) >>
                 FCFG1_FLASH_E_P_EVSU_S;

    HWREG(FLASH_BASE + FLASH_O_FSM_PE_VSU) =
        (HWREG(FLASH_BASE + FLASH_O_FSM_PE_VSU) &
         ~FLASH_FSM_PE_VSU_ERA_VSU_M) |
        ((ui32Value << FLASH_FSM_PE_VSU_ERA_VSU_S) &
         FLASH_FSM_PE_VSU_ERA_VSU_M);

    //
    // Configure Addr to EXECUTEZ low setup time
    // (FCFG1 offset 0x174 bits 15:12).
    //
    ui32Value =
        (HWREG(FLASH_CFG_BASE + FCFG1_OFFSET + FCFG1_O_FLASH_C_E_P_R) &
         FCFG1_FLASH_C_E_P_R_A_EXEZ_SETUP_M) >>
         FCFG1_FLASH_C_E_P_R_A_EXEZ_SETUP_S;

    HWREG(FLASH_BASE + FLASH_O_FSM_CMP_VSU) =
        (HWREG(FLASH_BASE + FLASH_O_FSM_CMP_VSU) &
         ~FLASH_FSM_CMP_VSU_ADD_EXZ_M) |
        ((ui32Value << FLASH_FSM_CMP_VSU_ADD_EXZ_S) &
         FLASH_FSM_CMP_VSU_ADD_EXZ_M);

    //
    // Configure Voltage Status Count
    // (FCFG1 offset 0x17C bits 15:12).
    //
    ui32Value =
        (HWREG(FLASH_CFG_BASE + FCFG1_OFFSET + FCFG1_O_FLASH_EH_SEQ) &
         FCFG1_FLASH_EH_SEQ_VSTAT_M) >>
         FCFG1_FLASH_EH_SEQ_VSTAT_S;

    HWREG(FLASH_BASE + FLASH_O_FSM_VSTAT) =
        (HWREG(FLASH_BASE + FLASH_O_FSM_VSTAT) &
         ~FLASH_FSM_VSTAT_VSTAT_CNT_M) |
        ((ui32Value << FLASH_FSM_VSTAT_VSTAT_CNT_S) &
         FLASH_FSM_VSTAT_VSTAT_CNT_M);

    //
    // Configure Repeat Verify action setup
    // (FCFG1 offset 0x174 bits 31:24).
    //
    ui32Value =
        (HWREG(FLASH_CFG_BASE + FCFG1_OFFSET + FCFG1_O_FLASH_C_E_P_R) &
         FCFG1_FLASH_C_E_P_R_RVSU_M) >>
         FCFG1_FLASH_C_E_P_R_RVSU_S;

    HWREG(FLASH_BASE + FLASH_O_FSM_EX_VAL) =
        (HWREG(FLASH_BASE + FLASH_O_FSM_EX_VAL) &
         ~FLASH_FSM_EX_VAL_REP_VSU_M) |
        ((ui32Value << FLASH_FSM_EX_VAL_REP_VSU_S) &
         FLASH_FSM_EX_VAL_REP_VSU_M);

    //
    // Configure Maximum Programming Pulses
    // (FCFG1 offset 0x184 bits 15:0).
    //
    ui32Value = (HWREG(FLASH_CFG_BASE + FCFG1_OFFSET + FCFG1_O_FLASH_PP) &
                 FCFG1_FLASH_PP_MAX_PP_M) >>
                 FCFG1_FLASH_PP_MAX_PP_S;

    HWREG(FLASH_BASE + FLASH_O_FSM_PRG_PUL) =
        (HWREG(FLASH_BASE + FLASH_O_FSM_PRG_PUL) &
         ~FLASH_FSM_PRG_PUL_MAX_PRG_PUL_M) |
        ((ui32Value << FLASH_FSM_PRG_PUL_MAX_PRG_PUL_S) &
         FLASH_FSM_PRG_PUL_MAX_PRG_PUL_M);

    //
    // Configure Beginning level for VHVCT used during erase modes
    // (FCFG1 offset 0x180 bits 31:16).
    //
    ui32Value =
        (HWREG(FLASH_CFG_BASE + FCFG1_OFFSET + FCFG1_O_FLASH_VHV_E) &
         FCFG1_FLASH_VHV_E_VHV_E_START_M) >>
         FCFG1_FLASH_VHV_E_VHV_E_START_S;

    HWREG(FLASH_BASE + FLASH_O_FSM_PRG_PUL) =
        (HWREG(FLASH_BASE + FLASH_O_FSM_PRG_PUL) &
         ~FLASH_FSM_PRG_PUL_BEG_EC_LEVEL_M) |
        ((ui32Value << FLASH_FSM_PRG_PUL_BEG_EC_LEVEL_S) &
         FLASH_FSM_PRG_PUL_BEG_EC_LEVEL_M);

    //
    // Configure Maximum EC Level
    // (FCFG1 offset 0x2B0 bits 21:18).
    //
    ui32Value =
        (HWREG(FLASH_CFG_BASE + FCFG1_OFFSET + FCFG1_O_FLASH_OTP_DATA3) &
         FCFG1_FLASH_OTP_DATA3_MAX_EC_LEVEL_M) >>
         FCFG1_FLASH_OTP_DATA3_MAX_EC_LEVEL_S;

    HWREG(FLASH_BASE + FLASH_O_FSM_ERA_PUL) =
        (HWREG(FLASH_BASE + FLASH_O_FSM_ERA_PUL) &
         ~FLASH_FSM_ERA_PUL_MAX_EC_LEVEL_M) |
        ((ui32Value << FLASH_FSM_ERA_PUL_MAX_EC_LEVEL_S) &
         FLASH_FSM_ERA_PUL_MAX_EC_LEVEL_M);

    //
    // Configure Maximum Erase Pulses
    // (FCFG1 offset 0x188 bits 31:16).
    //
    ui32Value =
        (HWREG(FLASH_CFG_BASE + FCFG1_OFFSET + FCFG1_O_FLASH_PROG_EP) &
         FCFG1_FLASH_PROG_EP_MAX_EP_M) >>
         FCFG1_FLASH_PROG_EP_MAX_EP_S;

    HWREG(FLASH_BASE + FLASH_O_FSM_ERA_PUL) =
        (HWREG(FLASH_BASE + FLASH_O_FSM_ERA_PUL) &
         ~FLASH_FSM_ERA_PUL_MAX_ERA_PUL_M) |
        ((ui32Value << FLASH_FSM_ERA_PUL_MAX_ERA_PUL_S) &
         FLASH_FSM_ERA_PUL_MAX_ERA_PUL_M);

    //
    // Configure the VHVCT Step Size. This is the number of erase pulses that
    // must be completed for each level before the FSM increments the
    // CUR_EC_LEVEL to the next higher level. Actual erase pulses per level
    // equals (EC_STEP_SIZE +1). The stepping is only needed for the VHVCT
    // voltage.
    // (FCFG1 offset 0x2B0 bits 31:23).
    //
    ui32Value =
        (HWREG(FLASH_CFG_BASE + FCFG1_OFFSET + FCFG1_O_FLASH_OTP_DATA3) &
         FCFG1_FLASH_OTP_DATA3_EC_STEP_SIZE_M) >>
         FCFG1_FLASH_OTP_DATA3_EC_STEP_SIZE_S;

    HWREG(FLASH_BASE + FLASH_O_FSM_STEP_SIZE) =
        (HWREG(FLASH_BASE + FLASH_O_FSM_STEP_SIZE) &
         ~FLASH_FSM_STEP_SIZE_EC_STEP_SIZE_M) |
        ((ui32Value << FLASH_FSM_STEP_SIZE_EC_STEP_SIZE_S) &
         FLASH_FSM_STEP_SIZE_EC_STEP_SIZE_M);

    //
    // Configure the hight of each EC step. This is the number of counts that
    // the CUR_EC_LEVEL will increment when going to a new level. Actual count
    // size equals (EC_STEP_HEIGHT + 1). The stepping applies only to the VHVCT
    // voltage.
    // The read trim value is decremented by 1 before written to the register
    // since actual counts equals (register value + 1).
    // (FCFG1 offset 0x180 bits 15:0).
    //
    ui32Value =
        (HWREG(FLASH_CFG_BASE + FCFG1_OFFSET + FCFG1_O_FLASH_VHV_E) &
         FCFG1_FLASH_VHV_E_VHV_E_STEP_HIGHT_M) >>
         FCFG1_FLASH_VHV_E_VHV_E_STEP_HIGHT_S;

    HWREG(FLASH_BASE + FLASH_O_FSM_EC_STEP_HEIGHT) = ((ui32Value - 1) &
            FLASH_FSM_EC_STEP_HEIGHT_EC_STEP_HEIGHT_M);

    //
    // Configure Precondition used in erase operations
    // (FCFG1 offset 0x2B0 bit 22).
    //
    ui32Value =
        (HWREG(FLASH_CFG_BASE + FCFG1_OFFSET + FCFG1_O_FLASH_OTP_DATA3) &
         FCFG1_FLASH_OTP_DATA3_DO_PRECOND_M) >>
         FCFG1_FLASH_OTP_DATA3_DO_PRECOND_S;

    HWREG(FLASH_BASE + FLASH_O_FSM_ST_MACHINE) =
        (HWREG(FLASH_BASE + FLASH_O_FSM_ST_MACHINE) &
         ~FLASH_FSM_ST_MACHINE_DO_PRECOND_M) |
        ((ui32Value << FLASH_FSM_ST_MACHINE_DO_PRECOND_S) &
         FLASH_FSM_ST_MACHINE_DO_PRECOND_M);

    //
    // Enable the recommended Good Time function.
    //
    HWREG(FLASH_BASE + FLASH_O_FSM_ST_MACHINE) |=
        FLASH_FSM_ST_MACHINE_ONE_TIME_GOOD;

    //
    // Disable write access to FSM registers.
    //
    HWREG(FLASH_BASE + FLASH_O_FSM_WR_ENA) = FSM_REG_WRT_DISABLE;


    //***********************************************************************//
    //                                                                       //
    //                 Configure the voltage registers                       //
    //                                                                       //
    //***********************************************************************//

    //
    // Unlock voltage registers (0x2080 - 0x2098).
    //
    HWREG(FLASH_BASE + FLASH_O_FLOCK) = 0xAAAA;

    //
    // Configure voltage level for the specified pump voltage of high
    // voltage supply input during erase operation VHVCT_E and the TRIM13_E
    // (FCFG1 offset 0x190 bits[3:0] and bits[11:8]).
    //
    ui32TempVal = HWREG(FLASH_CFG_BASE + FCFG1_OFFSET + FCFG1_O_FLASH_VHV);

    ui32Value  = ((ui32TempVal & FCFG1_FLASH_VHV_TRIM13_E_M)>>
                  FCFG1_FLASH_VHV_TRIM13_E_S) << FLASH_FVHVCT1_TRIM13_E_S;
    ui32Value |= ((ui32TempVal & FCFG1_FLASH_VHV_VHV_E_M)>>
                  FCFG1_FLASH_VHV_VHV_E_S) << FLASH_FVHVCT1_VHVCT_E_S;

    HWREG(FLASH_BASE + FLASH_O_FVHVCT1) = (HWREG(FLASH_BASE + FLASH_O_FVHVCT1) &
         ~(FLASH_FVHVCT1_TRIM13_E_M | FLASH_FVHVCT1_VHVCT_E_M)) | ui32Value;

    //
    // Configure voltage level for the specified pump voltage of high voltage
    // supply input during program verify operation VHVCT_PV and the TRIM13_PV
    // (OTP offset 0x194 bits[19:16] and bits[27:24]).
    //
    ui32TempVal =
             HWREG(FLASH_CFG_BASE + FCFG1_OFFSET + FCFG1_O_FLASH_VHV_PV);

    ui32Value  = ((ui32TempVal & FCFG1_FLASH_VHV_PV_TRIM13_PV_M)>>
                  FCFG1_FLASH_VHV_PV_TRIM13_PV_S) <<
                                                     FLASH_FVHVCT1_TRIM13_PV_S;
    ui32Value |= ((ui32TempVal & FCFG1_FLASH_VHV_PV_VHV_PV_M)>>
                  FCFG1_FLASH_VHV_PV_VHV_PV_S) <<
                                                     FLASH_FVHVCT1_VHVCT_PV_S;

    HWREG(FLASH_BASE + FLASH_O_FVHVCT1) = (HWREG(FLASH_BASE + FLASH_O_FVHVCT1) &
         ~(FLASH_FVHVCT1_TRIM13_PV_M | FLASH_FVHVCT1_VHVCT_PV_M)) | ui32Value;

    //
    // Configure voltage level for the specified pump voltage of high voltage
    // supply input during program operation VHVCT_P and TRIM13_P
    // (FCFG1 offset 0x190 bits[19:16] and bits[27:24]).
    //
    ui32TempVal =
                HWREG(FLASH_CFG_BASE + FCFG1_OFFSET + FCFG1_O_FLASH_VHV);

    ui32Value = ((ui32TempVal & FCFG1_FLASH_VHV_TRIM13_P_M)>>
                  FCFG1_FLASH_VHV_TRIM13_P_S) << FLASH_FVHVCT2_TRIM13_P_S;
    ui32Value |= ((ui32TempVal & FCFG1_FLASH_VHV_VHV_P_M)>>
                  FCFG1_FLASH_VHV_VHV_P_S) << FLASH_FVHVCT2_VHVCT_P_S;

    HWREG(FLASH_BASE + FLASH_O_FVHVCT2) =
        (HWREG(FLASH_BASE + FLASH_O_FVHVCT2) &
         ~(FLASH_FVHVCT2_TRIM13_P_M | FLASH_FVHVCT2_VHVCT_P_M)) | ui32Value;

    //
    // Configure voltage level for the specified pump voltage of wordline power
    // supply for read mode
    // (FCFG1 offset 0x198 Bits 15:8).
    //
    ui32Value = (HWREG(FLASH_CFG_BASE + FCFG1_OFFSET + FCFG1_O_FLASH_V) &
                 FCFG1_FLASH_V_V_READ_M) >>
                 FCFG1_FLASH_V_V_READ_S;

    HWREG(FLASH_BASE + FLASH_O_FVREADCT) =
        (HWREG(FLASH_BASE + FLASH_O_FVREADCT) &
         ~FLASH_FVREADCT_VREADCT_M) |
        ((ui32Value << FLASH_FVREADCT_VREADCT_S) &
         FLASH_FVREADCT_VREADCT_M);

    //
    // Configure the voltage level for the VCG 2.5 CT pump voltage
    // (FCFG1 offset 0x194 bits 15:8).
    //
    ui32Value =
        (HWREG(FLASH_CFG_BASE + FCFG1_OFFSET + FCFG1_O_FLASH_VHV_PV) &
         FCFG1_FLASH_VHV_PV_VCG2P5_M) >>
         FCFG1_FLASH_VHV_PV_VCG2P5_S;

    HWREG(FLASH_BASE + FLASH_O_FVNVCT) =
        (HWREG(FLASH_BASE + FLASH_O_FVNVCT) &
         ~FLASH_FVNVCT_VCG2P5CT_M) |
        ((ui32Value << FLASH_FVNVCT_VCG2P5CT_S) &
         FLASH_FVNVCT_VCG2P5CT_M);

    //
    // Configure the voltage level for the specified pump voltage of high
    // current power input during program operation
    // (FCFG1 offset 0x198 bits 31:24).
    //
    ui32Value = (HWREG(FLASH_CFG_BASE + FCFG1_OFFSET + FCFG1_O_FLASH_V) &
                 FCFG1_FLASH_V_VSL_P_M) >>
                 FCFG1_FLASH_V_VSL_P_S;

    HWREG(FLASH_BASE + FLASH_O_FVSLP) =
        (HWREG(FLASH_BASE + FLASH_O_FVSLP) &
         ~FLASH_FVSLP_VSL_P_M) |
        ((ui32Value << FLASH_FVSLP_VSL_P_S) &
         FLASH_FVSLP_VSL_P_M);

    //
    // Configure the voltage level for the specified pump voltage of wordline
    // power supply during programming operations
    // (OTP offset 0x198 bits 23:16).
    //
    ui32Value = (HWREG(FLASH_CFG_BASE + FCFG1_OFFSET + FCFG1_O_FLASH_V) &
                 FCFG1_FLASH_V_VWL_P_M) >>
                 FCFG1_FLASH_V_VWL_P_S;

    HWREG(FLASH_BASE + FLASH_O_FVWLCT) =
        (HWREG(FLASH_BASE + FLASH_O_FVWLCT) &
         ~FLASH_FVWLCT_VWLCT_P_M) |
        ((ui32Value << FLASH_FVWLCT_VWLCT_P_S) &
         FLASH_FVWLCT_VWLCT_P_M);

    //
    // Configure the pump's TRIM_1P7 port pins.
    // (FCFG1 offset 0x2B0 bits 17:16).
    //
    ui32Value =
        (HWREG(FLASH_CFG_BASE + FCFG1_OFFSET + FCFG1_O_FLASH_OTP_DATA3) &
         FCFG1_FLASH_OTP_DATA3_TRIM_1P7_M) >>
         FCFG1_FLASH_OTP_DATA3_TRIM_1P7_S;

    HWREG(FLASH_BASE + FLASH_O_FSEQPMP) =
        (HWREG(FLASH_BASE + FLASH_O_FSEQPMP) &
         ~FLASH_FSEQPMP_TRIM_1P7_M) |
        ((ui32Value << FLASH_FSEQPMP_TRIM_1P7_S) &
         FLASH_FSEQPMP_TRIM_1P7_M);

    //
    // Lock the voltage registers.
    //
    HWREG(FLASH_BASE + FLASH_O_FLOCK) = 0x55AA;

    //
    // Set trimmed flag.
    //
    HWREG(FLASH_BASE + FLASH_O_FWLOCK) = 5;
    HWREG(FLASH_BASE + FLASH_O_FWFLAG) |= FW_WRT_TRIMMED;
    HWREG(FLASH_BASE + FLASH_O_FWLOCK) = 0;
}

//*****************************************************************************
//
//! \internal
//! Used to scale the TI OTP values based on the FClk scaling value.
//!
//! \param ui32SpecifiedTiming
//! \param ui32ScaleValue
//!
//! Used to scale the TI OTP values based on the FClk scaling value.
//!
//! \return Returns the scaled value
//
//*****************************************************************************
static uint32_t
ScaleCycleValues(uint32_t ui32SpecifiedTiming, uint32_t ui32ScaleValue)
{
    return((ui32SpecifiedTiming * ui32ScaleValue) >> 6);
}

//*****************************************************************************
//
//! \internal
//! Used to set flash in read mode.
//!
//! Flash is configured with values loaded from OTP dependent on the current
//! regulator mode.
//!
//! \return None.
//
//*****************************************************************************
static void
SetReadMode(void)
{
    uint32_t ui32TrimValue;
    uint32_t ui32Value;

    //
    // Configure the STANDBY_MODE_SEL, STANDBY_PW_SEL, DIS_STANDBY, DIS_IDLE,
    // VIN_AT_X and VIN_BY_PASS for read mode
    //
    if(HWREG(AON_SYSCTL_BASE + AON_SYSCTL_O_PWRCTL) &
       AON_SYSCTL_PWRCTL_EXT_REG_MODE)
    {
        // Select trim values for external regulator mode:
        // Configure STANDBY_MODE_SEL (OTP offset 0x308 bit 7)
        // COnfigure STANDBY_PW_SEL   (OTP offset 0x308 bit 6:5)
        // Must be done while the register bit field CONFIG.DIS_STANDBY = 1
        HWREG(FLASH_BASE + FLASH_O_CFG) |= FLASH_CFG_DIS_STANDBY;

        ui32TrimValue =
           HWREG(FLASH_CFG_BASE + FCFG1_OFFSET + FCFG1_O_FLASH_OTP_DATA4);

        ui32Value = ((ui32TrimValue &
                      FCFG1_FLASH_OTP_DATA4_STANDBY_MODE_SEL_EXT_RD_M) >>
                      FCFG1_FLASH_OTP_DATA4_STANDBY_MODE_SEL_EXT_RD_S) <<
                    FLASH_CFG_STANDBY_MODE_SEL_S;

        ui32Value |= ((ui32TrimValue &
                       FCFG1_FLASH_OTP_DATA4_STANDBY_PW_SEL_EXT_RD_M) >>
                       FCFG1_FLASH_OTP_DATA4_STANDBY_PW_SEL_EXT_RD_S) <<
                     FLASH_CFG_STANDBY_PW_SEL_S;

        // Configure DIS_STANDBY (OTP offset 0x308 bit 4).
        // Configure DIS_IDLE    (OTP offset 0x308 bit 3).
        ui32Value |= ((ui32TrimValue &
                       (FCFG1_FLASH_OTP_DATA4_DIS_STANDBY_EXT_RD_M |
                        FCFG1_FLASH_OTP_DATA4_DIS_IDLE_EXT_RD_M)) >>
                        FCFG1_FLASH_OTP_DATA4_DIS_IDLE_EXT_RD_S) <<
                        FLASH_CFG_DIS_IDLE_S;

        HWREG(FLASH_BASE + FLASH_O_CFG) = (HWREG(FLASH_BASE + FLASH_O_CFG) &
                                           ~(FLASH_CFG_STANDBY_MODE_SEL_M |
                                             FLASH_CFG_STANDBY_PW_SEL_M   |
                                             FLASH_CFG_DIS_STANDBY_M      |
                                             FLASH_CFG_DIS_IDLE_M)) | ui32Value;

        // Check if sample and hold functionality is disabled.
        if(HWREG(FLASH_BASE + FLASH_O_CFG) & FLASH_CFG_DIS_IDLE)
        {
            //
            // Wait for disabled sample and hold functionality to be stable.
            //
            while(!(HWREG(FLASH_BASE + FLASH_O_STAT) & FLASH_STAT_SAMHOLD_DIS))
            {
            }
        }

        // Configure VIN_AT_X (OTP offset 0x308 bits 2:0)
        ui32Value = ((ui32TrimValue &
                      FCFG1_FLASH_OTP_DATA4_VIN_AT_X_EXT_RD_M) >>
                      FCFG1_FLASH_OTP_DATA4_VIN_AT_X_EXT_RD_S) <<
                    FLASH_FSEQPMP_VIN_AT_X_S;

        // Configure VIN_BY_PASS which is dependent on the VIN_AT_X value.
        // If VIN_AT_X = 7 then VIN_BY_PASS should be 0 otherwise
        // VIN_BY_PASS should be 1
        if(((ui32Value & FLASH_FSEQPMP_VIN_AT_X_M) >>
            FLASH_FSEQPMP_VIN_AT_X_S) != 0x7)
        {
            ui32Value |= FLASH_FSEQPMP_VIN_BY_PASS;
        }

        HWREG(FLASH_BASE + FLASH_O_FLOCK) = 0xAAAA;
        HWREG(FLASH_BASE + FLASH_O_FSEQPMP) =
                                    (HWREG(FLASH_BASE + FLASH_O_FSEQPMP) &
                                     ~(FLASH_FSEQPMP_VIN_BY_PASS_M |
                                       FLASH_FSEQPMP_VIN_AT_X_M))  | ui32Value;
        HWREG(FLASH_BASE + FLASH_O_FLOCK) = 0x55AA;
    }
    else
    {
        // Select trim values for internal regulator mode:
        // Configure STANDBY_MODE_SEL (OTP offset 0x308 bit 15)
        // COnfigure STANDBY_PW_SEL   (OTP offset 0x308 bit 14:13)
        // Must be done while the register bit field CONFIG.DIS_STANDBY = 1
        HWREG(FLASH_BASE + FLASH_O_CFG) |= FLASH_CFG_DIS_STANDBY;

        ui32TrimValue =
           HWREG(FLASH_CFG_BASE + FCFG1_OFFSET + FCFG1_O_FLASH_OTP_DATA4);

        ui32Value = ((ui32TrimValue &
                      FCFG1_FLASH_OTP_DATA4_STANDBY_MODE_SEL_INT_RD_M) >>
                      FCFG1_FLASH_OTP_DATA4_STANDBY_MODE_SEL_INT_RD_S) <<
                    FLASH_CFG_STANDBY_MODE_SEL_S;

        ui32Value |= ((ui32TrimValue &
                       FCFG1_FLASH_OTP_DATA4_STANDBY_PW_SEL_INT_RD_M) >>
                       FCFG1_FLASH_OTP_DATA4_STANDBY_PW_SEL_INT_RD_S) <<
                     FLASH_CFG_STANDBY_PW_SEL_S;

        // Configure DIS_STANDBY (OTP offset 0x308 bit 12).
        // Configure DIS_IDLE    (OTP offset 0x308 bit 11).
        ui32Value |= ((ui32TrimValue &
                       (FCFG1_FLASH_OTP_DATA4_DIS_STANDBY_INT_RD_M |
                        FCFG1_FLASH_OTP_DATA4_DIS_IDLE_INT_RD_M)) >>
                        FCFG1_FLASH_OTP_DATA4_DIS_IDLE_INT_RD_S) <<
                        FLASH_CFG_DIS_IDLE_S;

        HWREG(FLASH_BASE + FLASH_O_CFG) = (HWREG(FLASH_BASE + FLASH_O_CFG) &
                                           ~(FLASH_CFG_STANDBY_MODE_SEL_M |
                                             FLASH_CFG_STANDBY_PW_SEL_M   |
                                             FLASH_CFG_DIS_STANDBY_M      |
                                             FLASH_CFG_DIS_IDLE_M)) | ui32Value;

        // Check if sample and hold functionality is disabled.
        if(HWREG(FLASH_BASE + FLASH_O_CFG) & FLASH_CFG_DIS_IDLE)
        {
            //
            // Wait for disabled sample and hold functionality to be stable.
            //
            while(!(HWREG(FLASH_BASE + FLASH_O_STAT) & FLASH_STAT_SAMHOLD_DIS))
            {
            }
        }

        // Configure VIN_AT_X (OTP offset 0x308 bits 10:8)
        ui32Value = (((ui32TrimValue &
                       FCFG1_FLASH_OTP_DATA4_VIN_AT_X_INT_RD_M) >>
                       FCFG1_FLASH_OTP_DATA4_VIN_AT_X_INT_RD_S) <<
                       FLASH_FSEQPMP_VIN_AT_X_S);

        // Configure VIN_BY_PASS which is dependent on the VIN_AT_X value.
        // If VIN_AT_X = 7 then VIN_BY_PASS should be 0 otherwise
        // VIN_BY_PASS should be 1
        if(((ui32Value & FLASH_FSEQPMP_VIN_AT_X_M) >>
            FLASH_FSEQPMP_VIN_AT_X_S) != 0x7)
        {
            ui32Value |= FLASH_FSEQPMP_VIN_BY_PASS;
        }

        HWREG(FLASH_BASE + FLASH_O_FLOCK) = 0xAAAA;
        HWREG(FLASH_BASE + FLASH_O_FSEQPMP) =
                                    (HWREG(FLASH_BASE + FLASH_O_FSEQPMP) &
                                     ~(FLASH_FSEQPMP_VIN_BY_PASS_M |
                                       FLASH_FSEQPMP_VIN_AT_X_M)) | ui32Value;
        HWREG(FLASH_BASE + FLASH_O_FLOCK) = 0x55AA;
    }
}

//*****************************************************************************
//
//! \internal
//! Used to set flash in write mode.
//!
//! Flash is configured with values loaded from OTP dependent on the current
//! regulator mode.
//!
//! \return None.
//
//*****************************************************************************
static void
SetWriteMode(void)
{
    uint32_t ui32TrimValue;
    uint32_t ui32Value;

    //
    // Configure the STANDBY_MODE_SEL, STANDBY_PW_SEL, DIS_STANDBY, DIS_IDLE,
    // VIN_AT_X and VIN_BY_PASS for program/erase mode
    //
    if(HWREG(AON_SYSCTL_BASE + AON_SYSCTL_O_PWRCTL) &
       AON_SYSCTL_PWRCTL_EXT_REG_MODE)
    {
        // Select trim values for external regulator mode:
        // Configure STANDBY_MODE_SEL (OTP offset 0x308 bit 23)
        // COnfigure STANDBY_PW_SEL   (OTP offset 0x308 bit 22:21)
        // Must be done while the register bit field CONFIG.DIS_STANDBY = 1
        HWREG(FLASH_BASE + FLASH_O_CFG) |= FLASH_CFG_DIS_STANDBY;

        ui32TrimValue =
           HWREG(FLASH_CFG_BASE + FCFG1_OFFSET + FCFG1_O_FLASH_OTP_DATA4);

        ui32Value = ((ui32TrimValue &
                      FCFG1_FLASH_OTP_DATA4_STANDBY_MODE_SEL_EXT_WRT_M) >>
                      FCFG1_FLASH_OTP_DATA4_STANDBY_MODE_SEL_EXT_WRT_S) <<
                      FLASH_CFG_STANDBY_MODE_SEL_S;

        ui32Value |= ((ui32TrimValue &
                       FCFG1_FLASH_OTP_DATA4_STANDBY_PW_SEL_EXT_WRT_M) >>
                       FCFG1_FLASH_OTP_DATA4_STANDBY_PW_SEL_EXT_WRT_S) <<
                       FLASH_CFG_STANDBY_PW_SEL_S;

        // Configure DIS_STANDBY (OTP offset 0x308 bit 20).
        // Configure DIS_IDLE    (OTP offset 0x308 bit 19).
        ui32Value |= ((ui32TrimValue &
                       (FCFG1_FLASH_OTP_DATA4_DIS_STANDBY_EXT_WRT_M |
                        FCFG1_FLASH_OTP_DATA4_DIS_IDLE_EXT_WRT_M)) >>
                        FCFG1_FLASH_OTP_DATA4_DIS_IDLE_EXT_WRT_S) <<
                        FLASH_CFG_DIS_IDLE_S;

        HWREG(FLASH_BASE + FLASH_O_CFG) = (HWREG(FLASH_BASE + FLASH_O_CFG) &
                                           ~(FLASH_CFG_STANDBY_MODE_SEL_M |
                                             FLASH_CFG_STANDBY_PW_SEL_M   |
                                             FLASH_CFG_DIS_STANDBY_M      |
                                             FLASH_CFG_DIS_IDLE_M)) | ui32Value;

        // Check if sample and hold functionality is disabled.
        if(HWREG(FLASH_BASE + FLASH_O_CFG) & FLASH_CFG_DIS_IDLE)
        {
            //
            // Wait for disabled sample and hold functionality to be stable.
            //
            while(!(HWREG(FLASH_BASE + FLASH_O_STAT) & FLASH_STAT_SAMHOLD_DIS))
            {
            }
        }

        // Configure VIN_AT_X (OTP offset 0x308 bits 18:16)
        ui32Value = ((ui32TrimValue &
                      FCFG1_FLASH_OTP_DATA4_VIN_AT_X_EXT_WRT_M) >>
                      FCFG1_FLASH_OTP_DATA4_VIN_AT_X_EXT_WRT_S) <<
                      FLASH_FSEQPMP_VIN_AT_X_S;

        // Configure VIN_BY_PASS which is dependent on the VIN_AT_X value.
        // If VIN_AT_X = 7 then VIN_BY_PASS should be 0 otherwise
        // VIN_BY_PASS should be 1
        if(((ui32Value & FLASH_FSEQPMP_VIN_AT_X_M) >>
            FLASH_FSEQPMP_VIN_AT_X_S) != 0x7)
        {
            ui32Value |= FLASH_FSEQPMP_VIN_BY_PASS;
        }

        HWREG(FLASH_BASE + FLASH_O_FLOCK) = 0xAAAA;
        HWREG(FLASH_BASE + FLASH_O_FSEQPMP) =
                                    (HWREG(FLASH_BASE + FLASH_O_FSEQPMP) &
                                     ~(FLASH_FSEQPMP_VIN_BY_PASS_M |
                                       FLASH_FSEQPMP_VIN_AT_X_M))  | ui32Value;
        HWREG(FLASH_BASE + FLASH_O_FLOCK) = 0x55AA;
    }
    else
    {
        // Select trim values for internal regulator mode:
        // Configure STANDBY_MODE_SEL (OTP offset 0x308 bit 31)
        // COnfigure STANDBY_PW_SEL   (OTP offset 0x308 bit 30:29)
        // Must be done while the register bit field CONFIG.DIS_STANDBY = 1
        HWREG(FLASH_BASE + FLASH_O_CFG) |= FLASH_CFG_DIS_STANDBY;

        ui32TrimValue =
           HWREG(FLASH_CFG_BASE + FCFG1_OFFSET + FCFG1_O_FLASH_OTP_DATA4);

        ui32Value = ((ui32TrimValue &
                      FCFG1_FLASH_OTP_DATA4_STANDBY_MODE_SEL_INT_WRT_M) >>
                      FCFG1_FLASH_OTP_DATA4_STANDBY_MODE_SEL_INT_WRT_S) <<
                      FLASH_CFG_STANDBY_MODE_SEL_S;

        ui32Value |= ((ui32TrimValue &
                       FCFG1_FLASH_OTP_DATA4_STANDBY_PW_SEL_INT_WRT_M) >>
                       FCFG1_FLASH_OTP_DATA4_STANDBY_PW_SEL_INT_WRT_S) <<
                       FLASH_CFG_STANDBY_PW_SEL_S;

        // Configure DIS_STANDBY (OTP offset 0x308 bit 28).
        // Configure DIS_IDLE    (OTP offset 0x308 bit 27).
        ui32Value |= ((ui32TrimValue &
                      (FCFG1_FLASH_OTP_DATA4_DIS_STANDBY_INT_WRT_M |
                       FCFG1_FLASH_OTP_DATA4_DIS_IDLE_INT_WRT_M)) >>
                       FCFG1_FLASH_OTP_DATA4_DIS_IDLE_INT_WRT_S) <<
                       FLASH_CFG_DIS_IDLE_S;


        HWREG(FLASH_BASE + FLASH_O_CFG) = (HWREG(FLASH_BASE + FLASH_O_CFG) &
                                           ~(FLASH_CFG_STANDBY_MODE_SEL_M |
                                             FLASH_CFG_STANDBY_PW_SEL_M   |
                                             FLASH_CFG_DIS_STANDBY_M      |
                                             FLASH_CFG_DIS_IDLE_M)) | ui32Value;

        // Check if sample and hold functionality is disabled.
        if(HWREG(FLASH_BASE + FLASH_O_CFG) & FLASH_CFG_DIS_IDLE)
        {
            //
            // Wait for disabled sample and hold functionality to be stable.
            //
            while(!(HWREG(FLASH_BASE + FLASH_O_STAT) & FLASH_STAT_SAMHOLD_DIS))
            {
            }
        }

        // Configure VIN_AT_X (OTP offset 0x308 bits 26:24)
        ui32Value = ((ui32TrimValue &
                      FCFG1_FLASH_OTP_DATA4_VIN_AT_X_INT_WRT_M) >>
                      FCFG1_FLASH_OTP_DATA4_VIN_AT_X_INT_WRT_S) <<
                      FLASH_FSEQPMP_VIN_AT_X_S;

        // Configure VIN_BY_PASS which is dependent on the VIN_AT_X value.
        // If VIN_AT_X = 7 then VIN_BY_PASS should be 0 otherwise
        // VIN_BY_PASS should be 1
        if(((ui32Value & FLASH_FSEQPMP_VIN_AT_X_M) >>
            FLASH_FSEQPMP_VIN_AT_X_S) != 0x7)
        {
            ui32Value |= FLASH_FSEQPMP_VIN_BY_PASS;
        }

        HWREG(FLASH_BASE + FLASH_O_FLOCK) = 0xAAAA;
        HWREG(FLASH_BASE + FLASH_O_FSEQPMP) =
                                    (HWREG(FLASH_BASE + FLASH_O_FSEQPMP) &
                                     ~(FLASH_FSEQPMP_VIN_BY_PASS_M |
                                       FLASH_FSEQPMP_VIN_AT_X_M)) | ui32Value;
        HWREG(FLASH_BASE + FLASH_O_FLOCK) = 0x55AA;
    }
}

void IntEnable(uint32_t ui32Interrupt) {
    //
    // Check the arguments.
    //
    ASSERT(ui32Interrupt < NUM_INTERRUPTS);

    //
    // Determine the interrupt to enable.
    //
    if(ui32Interrupt == INT_MEMMANAGE_FAULT)
    {
        //
        // Enable the MemManage interrupt.
        //
        HWREG(NVIC_SYS_HND_CTRL) |= NVIC_SYS_HND_CTRL_MEM;
    }
    else if(ui32Interrupt == INT_BUS_FAULT)
    {
        //
        // Enable the bus fault interrupt.
        //
        HWREG(NVIC_SYS_HND_CTRL) |= NVIC_SYS_HND_CTRL_BUS;
    }
    else if(ui32Interrupt == INT_USAGE_FAULT)
    {
        //
        // Enable the usage fault interrupt.
        //
        HWREG(NVIC_SYS_HND_CTRL) |= NVIC_SYS_HND_CTRL_USAGE;
    }
    else if(ui32Interrupt == INT_SYSTICK)
    {
        //
        // Enable the System Tick interrupt.
        //
        HWREG(NVIC_ST_CTRL) |= NVIC_ST_CTRL_INTEN;
    }
    else if((ui32Interrupt >= 16) && (ui32Interrupt <= 47))
    {
        //
        // Enable the general interrupt.
        //
        HWREG(NVIC_EN0) = 1 << (ui32Interrupt - 16);
    }
    else if(ui32Interrupt >= 48)
    {
        //
        // Enable the general interrupt.
        //
        HWREG(NVIC_EN1) = 1 << (ui32Interrupt - 48);
    }
}

void IntDisable(uint32_t ui32Interrupt) {
    //
    // Check the arguments.
    //
    ASSERT(ui32Interrupt < NUM_INTERRUPTS);

    //
    // Determine the interrupt to disable.
    //
    if(ui32Interrupt == INT_MEMMANAGE_FAULT)
    {
        //
        // Disable the MemManage interrupt.
        //
        HWREG(NVIC_SYS_HND_CTRL) &= ~(NVIC_SYS_HND_CTRL_MEM);
    }
    else if(ui32Interrupt == INT_BUS_FAULT)
    {
        //
        // Disable the bus fault interrupt.
        //
        HWREG(NVIC_SYS_HND_CTRL) &= ~(NVIC_SYS_HND_CTRL_BUS);
    }
    else if(ui32Interrupt == INT_USAGE_FAULT)
    {
        //
        // Disable the usage fault interrupt.
        //
        HWREG(NVIC_SYS_HND_CTRL) &= ~(NVIC_SYS_HND_CTRL_USAGE);
    }
    else if(ui32Interrupt == INT_SYSTICK)
    {
        //
        // Disable the System Tick interrupt.
        //
        HWREG(NVIC_ST_CTRL) &= ~(NVIC_ST_CTRL_INTEN);
    }
    else if((ui32Interrupt >= 16) && (ui32Interrupt <= 47))
    {
        //
        // Disable the general interrupt.
        //
        HWREG(NVIC_DIS0) = 1 << (ui32Interrupt - 16);
    }
    else if(ui32Interrupt >= 48)
    {
        //
        // Disable the general interrupt.
        //
        HWREG(NVIC_DIS1) = 1 << (ui32Interrupt - 48);
    }
}

void IntPendClear(uint32_t ui32Interrupt) {
    //
    // Check the arguments.
    //
    ASSERT(ui32Interrupt < NUM_INTERRUPTS);

    //
    // Determine the interrupt to unpend.
    //
    if(ui32Interrupt == INT_PENDSV)
    {
        //
        // Unpend the PendSV interrupt.
        //
        HWREG(NVIC_INT_CTRL) |= NVIC_INT_CTRL_UNPEND_SV;
    }
    else if(ui32Interrupt == INT_SYSTICK)
    {
        //
        // Unpend the SysTick interrupt.
        //
        HWREG(NVIC_INT_CTRL) |= NVIC_INT_CTRL_PENDSTCLR;
    }
    else if((ui32Interrupt >= 16) && (ui32Interrupt <= 47))
    {
        //
        // Unpend the general interrupt.
        //
        HWREG(NVIC_UNPEND0) = 1 << (ui32Interrupt - 16);
    }
    else if(ui32Interrupt >= 48)
    {
        //
        // Unpend the general interrupt.
        //
        HWREG(NVIC_UNPEND1) = 1 << (ui32Interrupt - 48);
    }
}

void IOCPortConfigureSet(uint32_t ui32IOId, uint32_t ui32PortId, uint32_t ui32IOConfig) {
    uint32_t ui32Reg;

    //
    // Check the arguments.
    //
    ASSERT(ui32IOId <= IOID_31);
    ASSERT(ui32PortId <= IOC_PORT_RFC_GPI1);

    //
    // Get the register address.
    //
    ui32Reg = IOC_BASE + ( ui32IOId << 2 );

    //
    // Configure the port.
    //
    HWREG(ui32Reg) = ui32IOConfig | ui32PortId;
}

//*****************************************************************************
//
// Arrays that maps the "peripheral set" number (which is stored in the
// third nibble of the PRCM_PERIPH_* defines) to the PRCM register that
// contains the relevant bit for that peripheral.
//
//*****************************************************************************

// Run mode registers
static const uint32_t g_pui32RCGCRegs[] =
{
    PRCM_O_GPTCLKGR,
    PRCM_O_SSICLKGR,
    PRCM_O_UARTCLKGR,
    PRCM_O_I2CCLKGR,
    PRCM_O_SECDMACLKGR,
    PRCM_O_GPIOCLKGR,
    PRCM_O_I2SCLKGR
};

// Sleep mode registers
static const uint32_t g_pui32SCGCRegs[] =
{
    PRCM_O_GPTCLKGS,
    PRCM_O_SSICLKGS,
    PRCM_O_UARTCLKGS,
    PRCM_O_I2CCLKGS,
    PRCM_O_SECDMACLKGS,
    PRCM_O_GPIOCLKGS,
    PRCM_O_I2SCLKGS
};

// Deep sleep mode registers
static const uint32_t g_pui32DCGCRegs[] =
{
    PRCM_O_GPTCLKGDS,
    PRCM_O_SSICLKGDS,
    PRCM_O_UARTCLKGDS,
    PRCM_O_I2CCLKGDS,
    PRCM_O_SECDMACLKGDS,
    PRCM_O_GPIOCLKGDS,
    PRCM_O_I2SCLKGDS
};

//*****************************************************************************
//
// This macro extracts the array index out of the peripheral number
//
//*****************************************************************************
#define PRCM_PERIPH_INDEX(a)  (((a) >> 8) & 0xf)

//*****************************************************************************
//
// This macro extracts the peripheral instance number and generates bit mask
//
//*****************************************************************************
#define PRCM_PERIPH_MASKBIT(a) (0x00000001 << ((a) & 0xf))


void PRCMPowerDomainOn(uint32_t ui32Domains) {
    //
    // Check the arguments.
    //
    ASSERT((ui32Domains & PRCM_DOMAIN_RFCORE) ||
           (ui32Domains & PRCM_DOMAIN_SERIAL) ||
           (ui32Domains & PRCM_DOMAIN_PERIPH) ||
           (ui32Domains & PRCM_DOMAIN_CPU) ||
           (ui32Domains & PRCM_DOMAIN_VIMS));

    //
    // Assert the request to power on the right domains.
    //
    if(ui32Domains & PRCM_DOMAIN_RFCORE)
    {
        HWREG(PRCM_BASE + PRCM_O_PDCTL0RFC   ) = 1;
        HWREG(PRCM_BASE + PRCM_O_PDCTL1RFC   ) = 1;
    }
    if(ui32Domains & PRCM_DOMAIN_SERIAL)
    {
        HWREG(PRCM_BASE + PRCM_O_PDCTL0SERIAL) = 1;
    }
    if(ui32Domains & PRCM_DOMAIN_PERIPH)
    {
        HWREG(PRCM_BASE + PRCM_O_PDCTL0PERIPH) = 1;
    }
    if(ui32Domains & PRCM_DOMAIN_VIMS)
    {
        HWREG(PRCM_BASE + PRCM_O_PDCTL1VIMS  ) = 1;
    }
    if(ui32Domains & PRCM_DOMAIN_CPU)
    {
        HWREG(PRCM_BASE + PRCM_O_PDCTL1CPU   ) = 1;
    }
}

void PRCMPowerDomainOff(uint32_t ui32Domains) {
    //
    // Check the arguments.
    //
    ASSERT((ui32Domains & PRCM_DOMAIN_RFCORE) ||
           (ui32Domains & PRCM_DOMAIN_SERIAL) ||
           (ui32Domains & PRCM_DOMAIN_PERIPH) ||
           (ui32Domains & PRCM_DOMAIN_CPU) ||
           (ui32Domains & PRCM_DOMAIN_VIMS));

    //
    // Assert the request to power off the right domains.
    //
    if(ui32Domains & PRCM_DOMAIN_RFCORE)
    {
        HWREG(PRCM_BASE + PRCM_O_PDCTL0RFC   ) = 0;
        HWREG(PRCM_BASE + PRCM_O_PDCTL1RFC   ) = 0;
    }
    if(ui32Domains & PRCM_DOMAIN_SERIAL)
    {
        HWREG(PRCM_BASE + PRCM_O_PDCTL0SERIAL) = 0;
    }
    if(ui32Domains & PRCM_DOMAIN_PERIPH)
    {
        HWREG(PRCM_BASE + PRCM_O_PDCTL0PERIPH) = 0;
    }
    if(ui32Domains & PRCM_DOMAIN_VIMS)
    {
        HWREG(PRCM_BASE + PRCM_O_PDCTL1VIMS  ) = 0;
    }
    if(ui32Domains & PRCM_DOMAIN_CPU)
    {
        HWREG(PRCM_BASE + PRCM_O_PDCTL1CPU   ) = 0;
    }
}

void PRCMPeripheralRunEnable(uint32_t ui32Peripheral) {
    //
    // Check the arguments.
    //
    ASSERT(PRCMPeripheralValid(ui32Peripheral));

    //
    // Enable module in Run Mode.
    //
    HWREG(PRCM_BASE + g_pui32RCGCRegs[PRCM_PERIPH_INDEX(ui32Peripheral)]) |=
        PRCM_PERIPH_MASKBIT(ui32Peripheral);
}

void PRCMPeripheralRunDisable(uint32_t ui32Peripheral) {
    //
    // Check the arguments.
    //
    ASSERT(PRCMPeripheralValid(ui32Peripheral));

    //
    // Disable module in Run Mode.
    //
    HWREG(PRCM_BASE + g_pui32RCGCRegs[PRCM_PERIPH_INDEX(ui32Peripheral)]) &=
        ~PRCM_PERIPH_MASKBIT(ui32Peripheral);
}

void PRCMPeripheralSleepEnable(uint32_t ui32Peripheral) {
    //
    // Check the arguments.
    //
    ASSERT(PRCMPeripheralValid(ui32Peripheral));

    //
    // Enable this peripheral in sleep mode.
    //
    HWREG(PRCM_BASE + g_pui32SCGCRegs[PRCM_PERIPH_INDEX(ui32Peripheral)]) |=
        PRCM_PERIPH_MASKBIT(ui32Peripheral);
}

void PRCMPeripheralSleepDisable(uint32_t ui32Peripheral) {
    //
    // Check the arguments.
    //
    ASSERT(PRCMPeripheralValid(ui32Peripheral));

    //
    // Disable this peripheral in sleep mode
    //
    HWREG(PRCM_BASE + g_pui32SCGCRegs[PRCM_PERIPH_INDEX(ui32Peripheral)]) &=
        ~PRCM_PERIPH_MASKBIT(ui32Peripheral);
}

void PRCMPeripheralDeepSleepEnable(uint32_t ui32Peripheral) {
    //
    // Check the arguments.
    //
    ASSERT(PRCMPeripheralValid(ui32Peripheral));

    //
    // Enable this peripheral in deep-sleep mode.
    //
    HWREG(PRCM_BASE + g_pui32DCGCRegs[PRCM_PERIPH_INDEX(ui32Peripheral)]) |=
        PRCM_PERIPH_MASKBIT(ui32Peripheral);
}

void PRCMPeripheralDeepSleepDisable(uint32_t ui32Peripheral) {
    //
    // Check the arguments.
    //
    ASSERT(PRCMPeripheralValid(ui32Peripheral));

    //
    // Disable this peripheral in Deep Sleep mode.
    //
    HWREG(PRCM_BASE + g_pui32DCGCRegs[PRCM_PERIPH_INDEX(ui32Peripheral)]) &=
        ~PRCM_PERIPH_MASKBIT(ui32Peripheral);
}

uint32_t PRCMPowerDomainStatus(uint32_t ui32Domains) {
    bool bStatus;
    uint32_t ui32StatusRegister0;
    uint32_t ui32StatusRegister1;

    //
    // Check the arguments.
    //
    ASSERT((ui32Domains & (PRCM_DOMAIN_RFCORE |
                           PRCM_DOMAIN_SERIAL |
                           PRCM_DOMAIN_PERIPH)));

    bStatus = true;
    ui32StatusRegister0 = HWREG(PRCM_BASE + PRCM_O_PDSTAT0);
    ui32StatusRegister1 = HWREG(PRCM_BASE + PRCM_O_PDSTAT1);

    //
    // Return the correct power status.
    //
    if(ui32Domains & PRCM_DOMAIN_RFCORE)
    {
       bStatus = bStatus &&
                 ((ui32StatusRegister0 & PRCM_PDSTAT0_RFC_ON) ||
                  (ui32StatusRegister1 & PRCM_PDSTAT1_RFC_ON));
    }
    if(ui32Domains & PRCM_DOMAIN_SERIAL)
    {
        bStatus = bStatus && (ui32StatusRegister0 & PRCM_PDSTAT0_SERIAL_ON);
    }
    if(ui32Domains & PRCM_DOMAIN_PERIPH)
    {
        bStatus = bStatus && (ui32StatusRegister0 & PRCM_PDSTAT0_PERIPH_ON);
    }

    //
    // Return the status.
    //
    return (bStatus ? PRCM_DOMAIN_POWER_ON : PRCM_DOMAIN_POWER_OFF);
}

void PRCMDeepSleep(void) {
    //
    // Enable deep-sleep.
    //
    HWREG(NVIC_SYS_CTRL) |= NVIC_SYS_CTRL_SLEEPDEEP;

    //
    // Wait for an interrupt.
    //
    CPUwfi();

    //
    // Disable deep-sleep so that a future sleep will work correctly.
    //
    HWREG(NVIC_SYS_CTRL) &= ~(NVIC_SYS_CTRL_SLEEPDEEP);
}

void SMPHAcquire(uint32_t ui32Semaphore) {
    //
    // Check the arguments.
    //
    ASSERT((ui32Semaphore == SMPH_0) ||
           (ui32Semaphore == SMPH_1) ||
           (ui32Semaphore == SMPH_2) ||
           (ui32Semaphore == SMPH_3) ||
           (ui32Semaphore == SMPH_4) ||
           (ui32Semaphore == SMPH_5) ||
           (ui32Semaphore == SMPH_6) ||
           (ui32Semaphore == SMPH_7) ||
           (ui32Semaphore == SMPH_8) ||
           (ui32Semaphore == SMPH_9) ||
           (ui32Semaphore == SMPH_10) ||
           (ui32Semaphore == SMPH_11) ||
           (ui32Semaphore == SMPH_12) ||
           (ui32Semaphore == SMPH_13) ||
           (ui32Semaphore == SMPH_14) ||
           (ui32Semaphore == SMPH_15) ||
           (ui32Semaphore == SMPH_16) ||
           (ui32Semaphore == SMPH_17) ||
           (ui32Semaphore == SMPH_18) ||
           (ui32Semaphore == SMPH_19) ||
           (ui32Semaphore == SMPH_20) ||
           (ui32Semaphore == SMPH_21) ||
           (ui32Semaphore == SMPH_22) ||
           (ui32Semaphore == SMPH_23) ||
           (ui32Semaphore == SMPH_24) ||
           (ui32Semaphore == SMPH_25) ||
           (ui32Semaphore == SMPH_26) ||
           (ui32Semaphore == SMPH_27) ||
           (ui32Semaphore == SMPH_28) ||
           (ui32Semaphore == SMPH_29) ||
           (ui32Semaphore == SMPH_30) ||
           (ui32Semaphore == SMPH_31));

    //
    // Wait for semaphore to be release such that it can be claimed
    // Semaphore register reads 1 when lock was acquired otherwise 0
    // (i.e. SMPH_CLAIMED).
    //
    while(HWREG(SMPH_BASE + SMPH_O_SMPH0 + 4 * ui32Semaphore) ==
            SMPH_CLAIMED)
    {
    }
}

void TRNGConfigure(uint32_t ui32MinSamplesPerCycle, uint32_t ui32MaxSamplesPerCycle, uint32_t ui32ClocksPerSample) {
    uint32_t ui32Val;

    //
    // Make sure the TRNG is disabled.
    //
    ui32Val = HWREG(TRNG_BASE + TRNG_O_CTL) & ~TRNG_CTL_TRNG_EN;
    HWREG(TRNG_BASE + TRNG_O_CTL) = ui32Val;

    //
    // Configure the startup number of samples.
    //
    ui32Val &= ~TRNG_CTL_STARTUP_CYCLES_M;
    ui32Val |= ((( ui32MaxSamplesPerCycle >> 8 ) << TRNG_CTL_STARTUP_CYCLES_S ) & TRNG_CTL_STARTUP_CYCLES_M );
    HWREG(TRNG_BASE + TRNG_O_CTL) = ui32Val;

    //
    // Configure the minimum and maximum number of samples pr generated number
    // and the number of clocks per sample.
    //
    HWREG(TRNG_BASE + TRNG_O_CFG0) = (
        ((( ui32MaxSamplesPerCycle >> 8 ) << TRNG_CFG0_MAX_REFILL_CYCLES_S ) & TRNG_CFG0_MAX_REFILL_CYCLES_M ) |
        ((( ui32ClocksPerSample         ) << TRNG_CFG0_SMPL_DIV_S          ) & TRNG_CFG0_SMPL_DIV_M          ) |
        ((( ui32MinSamplesPerCycle >> 6 ) << TRNG_CFG0_MIN_REFILL_CYCLES_S ) & TRNG_CFG0_MIN_REFILL_CYCLES_M )   );
}

uint32_t TRNGNumberGet(uint32_t ui32Word) {
    uint32_t ui32RandomNumber;

    //
    // Check the arguments.
    //
    ASSERT((ui32Word == TRNG_HI_WORD) ||
           (ui32Word == TRNG_LOW_WORD));

    //
    // Return the right requested part of the generated number.
    //
    if(ui32Word == TRNG_HI_WORD)
    {
        ui32RandomNumber = HWREG(TRNG_BASE + TRNG_O_OUT1);
    }
    else
    {
        ui32RandomNumber = HWREG(TRNG_BASE + TRNG_O_OUT0);
    }

    //
    // Initiate generation of new number.
    //
    HWREG(TRNG_BASE + TRNG_O_IRQFLAGCLR) = 0x1;

    //
    // Return the random number.
    //
    return ui32RandomNumber;
}

void VIMSModeSet(uint32_t ui32Base, uint32_t ui32Mode) {
    uint32_t ui32Reg;

    //
    // Check the arguments.
    //
    ASSERT(VIMSBaseValid(ui32Base));

    ASSERT((ui32Mode == VIMS_MODE_DISABLED)   ||
           (ui32Mode == VIMS_MODE_ENABLED)    ||
           (ui32Mode == VIMS_MODE_OFF));

    //
    // Set the mode.
    //
    ui32Reg = HWREG(ui32Base + VIMS_O_CTL);
    ui32Reg &= ~VIMS_CTL_MODE_M;
    ui32Reg |= (ui32Mode & VIMS_CTL_MODE_M);

    HWREG(ui32Base + VIMS_O_CTL) = ui32Reg;
}

uint32_t VIMSModeGet(uint32_t ui32Base) {
    uint32_t ui32Reg;

    //
    // Check the arguments.
    //
    ASSERT(VIMSBaseValid(ui32Base));

    ui32Reg = HWREG(ui32Base + VIMS_O_STAT);
    if(ui32Reg & VIMS_STAT_MODE_CHANGING)
    {
        return (VIMS_MODE_CHANGING);
    }
    else
    {
        return (ui32Reg & VIMS_STAT_MODE_M);
    }
}

uint32_t CRYPTOAesLoadKey(uint32_t *pui32AesKey, uint32_t ui32KeyLocation) {
    //
    // Check the arguments.
    //
    ASSERT((ui32KeyLocation == CRYPTO_KEY_AREA_0) |
           (ui32KeyLocation == CRYPTO_KEY_AREA_1) |
           (ui32KeyLocation == CRYPTO_KEY_AREA_2) |
           (ui32KeyLocation == CRYPTO_KEY_AREA_3) |
           (ui32KeyLocation == CRYPTO_KEY_AREA_4) |
           (ui32KeyLocation == CRYPTO_KEY_AREA_5) |
           (ui32KeyLocation == CRYPTO_KEY_AREA_6) |
           (ui32KeyLocation == CRYPTO_KEY_AREA_7));

    //
    // Disable the external interrupt to stop the interrupt form propagating
    // from the module to the System CPU.
    //
    IntDisable(INT_CRYPTO_RESULT_AVAIL_IRQ);

    //
    // Enable internal interrupts.
    //
    HWREG(CRYPTO_BASE + CRYPTO_O_IRQTYPE) = CRYPTO_IRQTYPE_LEVEL;
    HWREG(CRYPTO_BASE + CRYPTO_O_IRQEN) = CRYPTO_IRQEN_DMA_IN_DONE |
                                          CRYPTO_IRQEN_RESULT_AVAIL;

    //
    // Configure master control module.
    //
    HWREGBITW(CRYPTO_BASE + CRYPTO_O_ALGSEL, CRYPTO_ALGSEL_KEY_STORE_BITN) = 1;

    //
    // Clear any outstanding events.
    //
    HWREG(CRYPTO_BASE + CRYPTO_O_IRQCLR) = (CRYPTO_IRQCLR_DMA_IN_DONE |
                                            CRYPTO_IRQCLR_RESULT_AVAIL);

    //
    // Configure key store module for 128 bit operation.
    //
    HWREG(CRYPTO_BASE + CRYPTO_O_KEYSIZE) &= ~CRYPTO_KEYSIZE_SIZE_M;
    HWREG(CRYPTO_BASE + CRYPTO_O_KEYSIZE) |= KEY_STORE_SIZE_128;

    //
    // Enable keys to write (e.g. Key 0).
    //
    HWREG(CRYPTO_BASE + CRYPTO_O_KEYWRITEAREA) = (0x00000001 << ui32KeyLocation);

    //
    // Enable Crypto DMA channel 0.
    //
    HWREGBITW(CRYPTO_BASE + CRYPTO_O_DMACH0CTL, CRYPTO_DMACH0CTL_EN_BITN) = 1;

    //
    // Base address of the key in ext. memory.
    //
    HWREG(CRYPTO_BASE + CRYPTO_O_DMACH0EXTADDR) = (uint32_t)pui32AesKey;

    //
    // Total key length in bytes (e.g. 16 for 1 x 128-bit key).
    // Writing the length of the key enables the DMA operation.
    //
    HWREG(CRYPTO_BASE + CRYPTO_O_DMACH0LEN) = KEY_BLENGTH;

    //
    // Wait for the DMA operation to complete.
    //
    do
    {
        CPUdelay(1);
    }
    while(!(HWREG(CRYPTO_BASE + CRYPTO_O_IRQSTAT) & 0x00000001));

    //
    // Check for errors in DMA and key store.
    //
    if((HWREG(CRYPTO_BASE + CRYPTO_O_IRQSTAT) &
            (CRYPTO_IRQSTAT_DMA_BUS_ERR |
             CRYPTO_IRQSTAT_KEY_ST_WR_ERR)) == 0)
    {
        //
        // Acknowledge/clear the interrupt and disable the master control.
        //
        HWREG(CRYPTO_BASE + CRYPTO_O_IRQCLR) = (CRYPTO_IRQCLR_DMA_IN_DONE |
                                                CRYPTO_IRQCLR_RESULT_AVAIL);
        HWREG(CRYPTO_BASE + CRYPTO_O_ALGSEL) = 0x00000000;

        //
        // Check status, if error return error code.
        //
        if(HWREG(CRYPTO_BASE + CRYPTO_O_KEYWRITTENAREA) != (0x00000001 << ui32KeyLocation))
        {
            return (AES_KEYSTORE_READ_ERROR);
        }
    }

    //
    // Return success.
    //
    return (AES_SUCCESS);
}

uint32_t CRYPTOAesCbc(uint32_t *pui32MsgIn, uint32_t *pui32MsgOut, uint32_t ui32MsgLength, uint32_t *pui32Nonce, uint32_t ui32KeyLocation, bool bEncrypt, bool bIntEnable) {
    uint32_t ui32CtrlVal;

    //
    // Enable internal interrupts.
    //
    HWREG(CRYPTO_BASE + CRYPTO_O_IRQTYPE) = CRYPTO_IRQTYPE_LEVEL;
    HWREG(CRYPTO_BASE + CRYPTO_O_IRQEN) = CRYPTO_IRQEN_RESULT_AVAIL;

    //
    // Clear any outstanding interrupts.
    //
    HWREG(CRYPTO_BASE + CRYPTO_O_IRQCLR) = (CRYPTO_IRQCLR_DMA_IN_DONE |
                                            CRYPTO_IRQCLR_RESULT_AVAIL);

    //
    // Wait for interrupt lines from module to be cleared
    //
    while(HWREG(CRYPTO_BASE + CRYPTO_O_IRQSTAT) & (CRYPTO_IRQSTAT_DMA_IN_DONE | CRYPTO_IRQSTAT_RESULT_AVAIL));

    //
    // If using interrupts clear any pending interrupts and enable interrupts
    // for the Crypto module.
    //
    if(bIntEnable)
    {
        IntPendClear(INT_CRYPTO_RESULT_AVAIL_IRQ);
        IntEnable(INT_CRYPTO_RESULT_AVAIL_IRQ);
    }

    //
    // Configure Master Control module.
    //
    HWREG(CRYPTO_BASE + CRYPTO_O_ALGSEL) = CRYPTO_ALGSEL_AES;

    //
    //
    //
    HWREG(CRYPTO_BASE + CRYPTO_O_KEYREADAREA) = ui32KeyLocation;

    //
    //Wait until key is loaded to the AES module.
    //
    do
    {
        CPUdelay(1);
    }
    while((HWREG(CRYPTO_BASE + CRYPTO_O_KEYREADAREA) & CRYPTO_KEYREADAREA_BUSY));

    //
    // Check for Key store Read error.
    //
    if((HWREG(CRYPTO_BASE + CRYPTO_O_IRQSTAT)& CRYPTO_KEY_ST_RD_ERR))
    {
        return (AES_KEYSTORE_READ_ERROR);
    }

    //
    // Write initialization vector.
    //
    HWREG(CRYPTO_BASE + CRYPTO_O_AESIV0) = pui32Nonce[0];
    HWREG(CRYPTO_BASE + CRYPTO_O_AESIV1) = pui32Nonce[1];
    HWREG(CRYPTO_BASE + CRYPTO_O_AESIV2) = pui32Nonce[2];
    HWREG(CRYPTO_BASE + CRYPTO_O_AESIV3) = pui32Nonce[3];

    //
    // Configure AES engine for AES-CBC with 128-bit key size.
    //
    ui32CtrlVal  = (CRYPTO_AESCTL_SAVE_CONTEXT | CRYPTO_AESCTL_CBC);
    if(bEncrypt)
    {
        ui32CtrlVal |= CRYPTO_AES128_ENCRYPT;
    }
    else
    {
        ui32CtrlVal |= CRYPTO_AES128_DECRYPT;
    }
    HWREG(CRYPTO_BASE + CRYPTO_O_AESCTL) = ui32CtrlVal;

    //
    // Write the length of the crypto block (plain text).
    // Low and high part (high part is assumed to be always 0).
    //
    HWREG(CRYPTO_BASE + CRYPTO_O_AESDATALEN0) = ui32MsgLength;
    HWREG(CRYPTO_BASE + CRYPTO_O_AESDATALEN1) = 0;
    HWREG(CRYPTO_BASE + CRYPTO_O_AESAUTHLEN)  = 0;

    //
    // Enable Crypto DMA channel 0.
    //
    HWREGBITW(CRYPTO_BASE + CRYPTO_O_DMACH0CTL, CRYPTO_DMACH0CTL_EN_BITN) = 1;

    //
    // Base address of the input data in ext. memory.
    //
    HWREG(CRYPTO_BASE + CRYPTO_O_DMACH0EXTADDR) = (uint32_t)pui32MsgIn;

    //
    // Input data length in bytes, equal to the message.
    //
    HWREG(CRYPTO_BASE + CRYPTO_O_DMACH0LEN) = ui32MsgLength;

    //
    // Enable Crypto DMA channel 1.
    //
    HWREGBITW(CRYPTO_BASE + CRYPTO_O_DMACH1CTL, CRYPTO_DMACH1CTL_EN_BITN) = 1;

    //
    // Set up the address and length of the output data.
    //
    HWREG(CRYPTO_BASE + CRYPTO_O_DMACH1EXTADDR) = (uint32_t)pui32MsgOut;
    HWREG(CRYPTO_BASE + CRYPTO_O_DMACH1LEN) = ui32MsgLength;

    //
    // Return success
    //
    return AES_SUCCESS;
}

uint32_t CRYPTOAesCbcStatus(void) {
    return(CRYPTOAesEcbStatus());
}

uint32_t CRYPTOAesEcb(uint32_t *pui32MsgIn, uint32_t *pui32MsgOut, uint32_t ui32KeyLocation, bool bEncrypt, bool bIntEnable) {
    //
    // Enable internal interrupts.
    //
    HWREG(CRYPTO_BASE + CRYPTO_O_IRQTYPE) = CRYPTO_IRQTYPE_LEVEL;
    HWREG(CRYPTO_BASE + CRYPTO_O_IRQEN) = CRYPTO_IRQEN_RESULT_AVAIL;

    //
    // Clear any outstanding interrupts.
    //
    HWREG(CRYPTO_BASE + CRYPTO_O_IRQCLR) = (CRYPTO_IRQCLR_DMA_IN_DONE |
                                            CRYPTO_IRQCLR_RESULT_AVAIL);

    //
    // Wait for interrupt lines from module to be cleared
    //
    while(HWREG(CRYPTO_BASE + CRYPTO_O_IRQSTAT) & (CRYPTO_IRQSTAT_DMA_IN_DONE | CRYPTO_IRQSTAT_RESULT_AVAIL));

    //
    // If using interrupts clear any pending interrupts and enable interrupts
    // for the Crypto module.
    //
    if(bIntEnable)
    {
        IntPendClear(INT_CRYPTO_RESULT_AVAIL_IRQ);
        IntEnable(INT_CRYPTO_RESULT_AVAIL_IRQ);
    }

    //
    // Configure Master Control module.
    //
    HWREG(CRYPTO_BASE + CRYPTO_O_ALGSEL) = CRYPTO_ALGSEL_AES;

    //
    //
    //
    HWREG(CRYPTO_BASE + CRYPTO_O_KEYREADAREA) = ui32KeyLocation;

    //
    //Wait until key is loaded to the AES module.
    //
    do
    {
        CPUdelay(1);
    }
    while((HWREG(CRYPTO_BASE + CRYPTO_O_KEYREADAREA) & CRYPTO_KEYREADAREA_BUSY));

    //
    // Check for Key store Read error.
    //
    if((HWREG(CRYPTO_BASE + CRYPTO_O_IRQSTAT)& CRYPTO_KEY_ST_RD_ERR))
    {
        return (AES_KEYSTORE_READ_ERROR);
    }

    //
    // Configure AES engine (program AES-ECB-128 encryption and no
    // initialization vector - IV).
    //
    if(bEncrypt)
    {
        HWREG(CRYPTO_BASE + CRYPTO_O_AESCTL) = CRYPTO_AES128_ENCRYPT;
    }
    else
    {
        HWREG(CRYPTO_BASE + CRYPTO_O_AESCTL) = CRYPTO_AES128_DECRYPT;
    }

    //
    // Write the length of the data.
    //
    HWREG(CRYPTO_BASE + CRYPTO_O_AESDATALEN0) = AES_ECB_LENGTH;
    HWREG(CRYPTO_BASE + CRYPTO_O_AESDATALEN1) = 0;

    //
    // Enable Crypto DMA channel 0.
    //
    HWREGBITW(CRYPTO_BASE + CRYPTO_O_DMACH0CTL, CRYPTO_DMACH0CTL_EN_BITN) = 1;

    //
    // Base address of the input data in ext. memory.
    //
    HWREG(CRYPTO_BASE + CRYPTO_O_DMACH0EXTADDR) = (uint32_t)pui32MsgIn;

    //
    // Input data length in bytes, equal to the message.
    //
    HWREG(CRYPTO_BASE + CRYPTO_O_DMACH0LEN) = AES_ECB_LENGTH;

    //
    // Enable Crypto DMA channel 1.
    //
    HWREGBITW(CRYPTO_BASE + CRYPTO_O_DMACH1CTL, CRYPTO_DMACH1CTL_EN_BITN) = 1;

    //
    // Set up the address and length of the output data.
    //
    HWREG(CRYPTO_BASE + CRYPTO_O_DMACH1EXTADDR) = (uint32_t)pui32MsgOut;
    HWREG(CRYPTO_BASE + CRYPTO_O_DMACH1LEN) = AES_ECB_LENGTH;

    //
    // Return success
    //
    return AES_SUCCESS;
}

uint32_t CRYPTOAesEcbStatus(void) {
    uint32_t ui32Status;

    //
    // Get the current DMA status.
    //
    ui32Status = HWREG(CRYPTO_BASE + CRYPTO_O_DMASTAT);

    //
    // Check if DMA is still busy.
    //
    if(ui32Status & CRYPTO_DMA_BSY)
    {
        return (AES_DMA_BSY);
    }

    //
    // Check the status of the DMA operation - return error if not success.
    //
    if(ui32Status & CRYPTO_DMA_BUS_ERROR)
    {
        return (AES_DMA_BUS_ERROR);
    }

    //
    // Operation successful - disable interrupt and return success.
    //
    IntDisable(INT_CRYPTO_RESULT_AVAIL_IRQ);
    return (AES_SUCCESS);
}

uint32_t CRYPTOCcmAuthEncrypt(bool bEncrypt, uint32_t ui32AuthLength , uint32_t *pui32Nonce, uint32_t *pui32PlainText, uint32_t ui32PlainTextLength, uint32_t *pui32Header, uint32_t ui32HeaderLength, uint32_t ui32KeyLocation, uint32_t ui32FieldLength, bool bIntEnable) {
    uint32_t ui32CtrlVal;
    uint32_t i;
    uint32_t *pui32CipherText;
    union {
        uint32_t w[4];
        uint8_t  b[16];
    } ui8InitVec;

    //
    // Input address for the encryption engine is the same as the output.
    //
    pui32CipherText = pui32PlainText;

    //
    // Disable global interrupt, enable local interrupt and clear any pending
    // interrupts.
    //
    IntDisable(INT_CRYPTO_RESULT_AVAIL_IRQ);
    HWREG(CRYPTO_BASE + CRYPTO_O_IRQCLR) = (CRYPTO_IRQCLR_DMA_IN_DONE |
                                            CRYPTO_IRQCLR_RESULT_AVAIL);

    //
    // Enable internal interrupts.
    //
    HWREG(CRYPTO_BASE + CRYPTO_O_IRQTYPE) = CRYPTO_IRQTYPE_LEVEL;
    HWREG(CRYPTO_BASE + CRYPTO_O_IRQEN) = CRYPTO_IRQEN_DMA_IN_DONE |
                                          CRYPTO_IRQEN_RESULT_AVAIL;

    //
    // Configure master control module for AES operation.
    //
    HWREG(CRYPTO_BASE + CRYPTO_O_ALGSEL) = CRYPTO_ALGSEL_AES;

    //
    // Enable keys to read (e.g. Key 0).
    //
    HWREG(CRYPTO_BASE + CRYPTO_O_KEYREADAREA) = ui32KeyLocation;

    //
    // Wait until key is loaded to the AES module.
    //
    do
    {
        CPUdelay(1);
    }
    while((HWREG(CRYPTO_BASE + CRYPTO_O_KEYREADAREA) & CRYPTO_KEYREADAREA_BUSY));

    //
    // Check for Key store Read error.
    //
    if((HWREG(CRYPTO_BASE + CRYPTO_O_IRQSTAT)& CRYPTO_KEY_ST_RD_ERR))
    {
        return (AES_KEYSTORE_READ_ERROR);
    }

    //
    // Prepare the initialization vector (IV),
    // Length of Nonce l(n) = 15 - ui32FieldLength.
    //
    ui8InitVec.b[0] = ui32FieldLength - 1;
    for(i = 0; i < 12; i++)
    {
        ui8InitVec.b[i + 1] = ((uint8_t*)pui32Nonce)[i];
    }
    if(ui32FieldLength == 2)
    {
        ui8InitVec.b[13] = ((uint8_t*)pui32Nonce)[12];
    }
    else
    {
        ui8InitVec.b[13] = 0;
    }
    ui8InitVec.b[14] = 0;
    ui8InitVec.b[15] = 0;

    //
    // Write initialization vector.
    //
    HWREG(CRYPTO_BASE + CRYPTO_O_AESIV0) = ui8InitVec.w[0];
    HWREG(CRYPTO_BASE + CRYPTO_O_AESIV1) = ui8InitVec.w[1];
    HWREG(CRYPTO_BASE + CRYPTO_O_AESIV2) = ui8InitVec.w[2];
    HWREG(CRYPTO_BASE + CRYPTO_O_AESIV3) = ui8InitVec.w[3];

    //
    // Configure AES engine.
    //
    ui32CtrlVal = ((ui32FieldLength - 1) << CRYPTO_AESCTL_CCM_L_S);
    if ( ui32AuthLength >= 2 ) {
        ui32CtrlVal |= ((( ui32AuthLength - 2 ) >> 1 ) << CRYPTO_AESCTL_CCM_M_S );
    }
    ui32CtrlVal |= CRYPTO_AESCTL_CCM;
    ui32CtrlVal |= CRYPTO_AESCTL_CTR;
    ui32CtrlVal |= CRYPTO_AESCTL_SAVE_CONTEXT;
    ui32CtrlVal |= (KEY_STORE_SIZE_128 << CRYPTO_AESCTL_KEY_SIZE_S);
    ui32CtrlVal |= (1 << CRYPTO_AESCTL_DIR_S);
    ui32CtrlVal |= (CRYPTO_AES_CTR_128 << CRYPTO_AESCTL_CTR_WIDTH_S);

    //
    // Write the configuration for 128 bit AES-CCM.
    //
    HWREG(CRYPTO_BASE + CRYPTO_O_AESCTL) = ui32CtrlVal;

    //
    // Write the length of the crypto block (plain text).
    // Low and high part (high part is assumed to be always 0).
    //
    HWREG(CRYPTO_BASE + CRYPTO_O_AESDATALEN0) = ui32PlainTextLength;
    HWREG(CRYPTO_BASE + CRYPTO_O_AESDATALEN1)  =  0;

    //
    // Write the length of the header field.
    // Also called AAD - Additional Authentication Data.
    //
    HWREG(CRYPTO_BASE + CRYPTO_O_AESAUTHLEN) = ui32HeaderLength;

    //
    // Check if any header information (AAD).
    // If so configure the DMA controller to fetch the header.
    //
    if(ui32HeaderLength != 0)
    {
        //
        // Enable DMA channel 0.
        //
        HWREGBITW(CRYPTO_BASE + CRYPTO_O_DMACH0CTL, CRYPTO_DMACH0CTL_EN_BITN) = 1;

        //
        // Register the base address of the header (AAD).
        //
        HWREG(CRYPTO_BASE + CRYPTO_O_DMACH0EXTADDR) = (uint32_t)pui32Header;

        //
        // Header length in bytes (may be non-block size aligned).
        //
        HWREG(CRYPTO_BASE + CRYPTO_O_DMACH0LEN) = ui32HeaderLength;

        //
        // Wait for completion of the header data transfer, DMA_IN_DONE.
        //
        do
        {
            CPUdelay(1);
        }
        while(!(HWREG(CRYPTO_BASE + CRYPTO_O_IRQSTAT) & CRYPTO_IRQSTAT_DMA_IN_DONE));

        //
        // Check for DMA errors.
        //
        if(HWREG(CRYPTO_BASE + CRYPTO_O_IRQSTAT) & CRYPTO_DMA_BUS_ERR)
        {
            return AES_DMA_BUS_ERROR;
        }
    }

    //
    // Clear interrupt status.
    //
    HWREG(CRYPTO_BASE + CRYPTO_O_IRQCLR) = (CRYPTO_IRQCLR_DMA_IN_DONE |
                                            CRYPTO_IRQCLR_RESULT_AVAIL);

    //
    // Wait for interrupt lines from module to be cleared
    //
    while(HWREG(CRYPTO_BASE + CRYPTO_O_IRQSTAT) & (CRYPTO_IRQSTAT_DMA_IN_DONE | CRYPTO_IRQSTAT_RESULT_AVAIL));

    //
    // Disable CRYPTO_IRQEN_DMA_IN_DONE interrupt as we only
    // want interrupt to trigger once RESULT_AVAIL occurs.
    //
    HWREG(CRYPTO_BASE + CRYPTO_O_IRQEN) &= ~CRYPTO_IRQEN_DMA_IN_DONE;


    //
    // Is using interrupts enable globally.
    //
    if(bIntEnable)
    {
        IntPendClear(INT_CRYPTO_RESULT_AVAIL_IRQ);
        IntEnable(INT_CRYPTO_RESULT_AVAIL_IRQ);
    }

    //
    // Enable interrupts locally.
    //
    HWREG(CRYPTO_BASE + CRYPTO_O_IRQEN) = CRYPTO_IRQEN_RESULT_AVAIL;

    //
    // Perform encryption if requested.
    //
    if(bEncrypt)
    {
        //
        // Enable DMA channel 0
        //
        HWREGBITW(CRYPTO_BASE + CRYPTO_O_DMACH0CTL, CRYPTO_DMACH0CTL_EN_BITN) = 1;

        //
        // base address of the payload data in ext. memory.
        //
        HWREG(CRYPTO_BASE + CRYPTO_O_DMACH0EXTADDR) =
            (uint32_t)pui32PlainText;

        //
        // Enable DMA channel 1
        //
        HWREGBITW(CRYPTO_BASE + CRYPTO_O_DMACH1CTL, CRYPTO_DMACH1CTL_EN_BITN) = 1;

        //
        // Base address of the output data buffer.
        //
        HWREG(CRYPTO_BASE + CRYPTO_O_DMACH1EXTADDR) =
            (uint32_t)pui32CipherText;

        //
        // Payload data length in bytes, equal to the plaintext length.
        //
        HWREG(CRYPTO_BASE + CRYPTO_O_DMACH0LEN) = ui32PlainTextLength;
        //
        // Output data length in bytes, equal to the plaintext length.
        //
        HWREG(CRYPTO_BASE + CRYPTO_O_DMACH1LEN) = ui32PlainTextLength;
    }

    return AES_SUCCESS;
}

uint32_t CRYPTOCcmAuthEncryptStatus(void) {
    uint32_t ui32Status;

    //
    // Get the current DMA status.
    //
    ui32Status = HWREG(CRYPTO_BASE + CRYPTO_O_DMASTAT);

    //
    // Check if DMA is still busy.
    //
    if(ui32Status & CRYPTO_DMA_BSY)
    {
        return (AES_DMA_BSY);
    }

    //
    // Check the status of the DMA operation - return error if not success.
    //
    if(ui32Status & CRYPTO_DMA_BUS_ERROR)
    {
        return (AES_DMA_BUS_ERROR);
    }

    //
    // Operation successful - disable interrupt and return success.
    //
    IntDisable(INT_CRYPTO_RESULT_AVAIL_IRQ);
    return (AES_SUCCESS);
}

uint32_t CRYPTOCcmAuthEncryptResultGet(uint32_t ui32TagLength, uint32_t *pui32CcmTag) {
    uint32_t volatile ui32Tag[4];
    uint32_t ui32Idx;

    //
    // Result has already been copied to the output buffer by DMA
    // Disable master control.
    //
    HWREG(CRYPTO_BASE + CRYPTO_O_ALGSEL) = 0x00000000;

    //
    // Read tag - wait for the context ready bit.
    //
    do
    {
        CPUdelay(1);
    }
    while(!(HWREG(CRYPTO_BASE + CRYPTO_O_AESCTL) &
            CRYPTO_AESCTL_SAVED_CONTEXT_RDY));

    //
    // Read the Tag registers.
    //
    ui32Tag[0] = HWREG(CRYPTO_BASE + CRYPTO_O_AESTAGOUT0);
    ui32Tag[1] = HWREG(CRYPTO_BASE + CRYPTO_O_AESTAGOUT1);
    ui32Tag[2] = HWREG(CRYPTO_BASE + CRYPTO_O_AESTAGOUT2);
    ui32Tag[3] = HWREG(CRYPTO_BASE + CRYPTO_O_AESTAGOUT3);

    for(ui32Idx = 0; ui32Idx < ui32TagLength ; ui32Idx++)
    {
        *((uint8_t*)pui32CcmTag + ui32Idx) = *((uint8_t*)ui32Tag + ui32Idx);
    }

    //
    // Operation successful -  clear interrupt status.
    //
    HWREG(CRYPTO_BASE + CRYPTO_O_IRQCLR) = (CRYPTO_IRQCLR_DMA_IN_DONE |
                                            CRYPTO_IRQCLR_RESULT_AVAIL);
    return AES_SUCCESS;
}

uint32_t CRYPTOCcmInvAuthDecrypt(bool bDecrypt, uint32_t ui32AuthLength, uint32_t *pui32Nonce, uint32_t *pui32CipherText, uint32_t ui32CipherTextLength, uint32_t *pui32Header, uint32_t ui32HeaderLength, uint32_t ui32KeyLocation, uint32_t ui32FieldLength, bool bIntEnable) {
    uint32_t ui32CtrlVal;
    uint32_t i;
    uint32_t *pui32PlainText;
    uint32_t ui32CryptoBlockLength;
    union {
        uint32_t w[4];
        uint8_t  b[16];
    } ui8InitVec;

    //
    // Input address for the encryption engine is the same as the output.
    //
    pui32PlainText = pui32CipherText;

    //
    // Disable global interrupt, enable local interrupt and clear any pending.
    // interrupts.
    //
    IntDisable(INT_CRYPTO_RESULT_AVAIL_IRQ);
    HWREG(CRYPTO_BASE + CRYPTO_O_IRQCLR) = (CRYPTO_IRQCLR_DMA_IN_DONE |
                                            CRYPTO_IRQCLR_RESULT_AVAIL);
    //
    // Enable internal interrupts.
    //
    HWREG(CRYPTO_BASE + CRYPTO_O_IRQTYPE) = CRYPTO_IRQTYPE_LEVEL;
    HWREG(CRYPTO_BASE + CRYPTO_O_IRQEN) = CRYPTO_IRQEN_DMA_IN_DONE |
                                          CRYPTO_IRQEN_RESULT_AVAIL;

    //
    // Configure master control module for AES operation.
    //
    HWREG(CRYPTO_BASE + CRYPTO_O_ALGSEL) = CRYPTO_ALGSEL_AES;

    //
    // Enable keys to read (e.g. Key 0).
    //
    HWREG(CRYPTO_BASE + CRYPTO_O_KEYREADAREA) = ui32KeyLocation;

    //
    // Wait until key is loaded to the AES module.
    //
    do
    {
        CPUdelay(1);
    }
    while((HWREG(CRYPTO_BASE + CRYPTO_O_KEYREADAREA) & CRYPTO_KEYREADAREA_BUSY));

    //
    // Check for Key store Read error.
    //
    if((HWREG(CRYPTO_BASE + CRYPTO_O_IRQSTAT)& CRYPTO_KEY_ST_RD_ERR))
    {
        return (AES_KEYSTORE_READ_ERROR);
    }

    //
    // Prepare the initialization vector (IV),
    // Length of Nonce l(n) = 15 - ui32FieldLength.
    //
    ui8InitVec.b[0] = ui32FieldLength - 1;
    for(i = 0; i < 12; i++)
    {
        ui8InitVec.b[i + 1] = ((uint8_t*)pui32Nonce)[i];
    }
    if(ui32FieldLength == 2)
    {
        ui8InitVec.b[13] = ((uint8_t*)pui32Nonce)[12];
    }
    else
    {
        ui8InitVec.b[13] = 0;
    }
    ui8InitVec.b[14] = 0;
    ui8InitVec.b[15] = 0;

    //
    // Write initialization vector.
    //
    HWREG(CRYPTO_BASE + CRYPTO_O_AESIV0) = ui8InitVec.w[0];
    HWREG(CRYPTO_BASE + CRYPTO_O_AESIV1) = ui8InitVec.w[1];
    HWREG(CRYPTO_BASE + CRYPTO_O_AESIV2) = ui8InitVec.w[2];
    HWREG(CRYPTO_BASE + CRYPTO_O_AESIV3) = ui8InitVec.w[3];

    //
    // Configure AES engine
    //
    ui32CryptoBlockLength = ui32CipherTextLength - ui32AuthLength;
    ui32CtrlVal = ((ui32FieldLength - 1) << CRYPTO_AESCTL_CCM_L_S);
    if ( ui32AuthLength >= 2 ) {
        ui32CtrlVal |= ((( ui32AuthLength - 2 ) >> 1 ) << CRYPTO_AESCTL_CCM_M_S );
    }
    ui32CtrlVal |= CRYPTO_AESCTL_CCM;
    ui32CtrlVal |= CRYPTO_AESCTL_CTR;
    ui32CtrlVal |= CRYPTO_AESCTL_SAVE_CONTEXT;
    ui32CtrlVal |= (KEY_STORE_SIZE_128 << CRYPTO_AESCTL_KEY_SIZE_S);
    ui32CtrlVal |= (0 << CRYPTO_AESCTL_DIR_S);
    ui32CtrlVal |= (CRYPTO_AES_CTR_128 << CRYPTO_AESCTL_CTR_WIDTH_S);

    //
    // Write the configuration for 128 bit AES-CCM.
    //
    HWREG(CRYPTO_BASE + CRYPTO_O_AESCTL) = ui32CtrlVal;

    //
    // Write the length of the crypto block (plain text).
    // Low and high part (high part is assumed to be always 0).
    //
    HWREG(CRYPTO_BASE + CRYPTO_O_AESDATALEN0) = ui32CryptoBlockLength;
    HWREG(CRYPTO_BASE + CRYPTO_O_AESDATALEN1)  =  0;

    //
    // Write the length of the header field.
    // Also called AAD - Additional Authentication Data.
    //
    HWREG(CRYPTO_BASE + CRYPTO_O_AESAUTHLEN) = ui32HeaderLength;

    //
    // Check if any header information (AAD).
    // If so configure the DMA controller to fetch the header.
    //
    if(ui32HeaderLength != 0)
    {
        //
        // Enable DMA channel 0.
        //
        HWREGBITW(CRYPTO_BASE + CRYPTO_O_DMACH0CTL, CRYPTO_DMACH0CTL_EN_BITN) = 1;

        //
        // Register the base address of the header (AAD).
        //
        HWREG(CRYPTO_BASE + CRYPTO_O_DMACH0EXTADDR) = (uint32_t)pui32Header;

        //
        // Header length in bytes (may be non-block size aligned).
        //
        HWREG(CRYPTO_BASE + CRYPTO_O_DMACH0LEN) = ui32HeaderLength;

        //
        // Wait for completion of the header data transfer, DMA_IN_DONE.
        //
        do
        {
            CPUdelay(1);
        }
        while(!(HWREG(CRYPTO_BASE + CRYPTO_O_IRQSTAT) & CRYPTO_IRQSTAT_DMA_IN_DONE));

        //
        // Check for DMA errors.
        //
        if(HWREG(CRYPTO_BASE + CRYPTO_O_IRQSTAT) & CRYPTO_DMA_BUS_ERR)
        {
            return AES_DMA_BUS_ERROR;
        }
    }

    //
    // Clear interrupt status.
    //
    HWREG(CRYPTO_BASE + CRYPTO_O_IRQCLR) = (CRYPTO_IRQCLR_DMA_IN_DONE |
                                            CRYPTO_IRQCLR_RESULT_AVAIL);

    //
    // Wait for interrupt lines from module to be cleared
    //
    while(HWREG(CRYPTO_BASE + CRYPTO_O_IRQSTAT) & (CRYPTO_IRQSTAT_DMA_IN_DONE | CRYPTO_IRQSTAT_RESULT_AVAIL));

    //
    // Disable CRYPTO_IRQEN_DMA_IN_DONE interrupt as we only
    // want interrupt to trigger once RESULT_AVAIL occurs.
    //
    HWREG(CRYPTO_BASE + CRYPTO_O_IRQEN) &= ~CRYPTO_IRQEN_DMA_IN_DONE;

    //
    // Is using interrupts - clear and enable globally.
    //
    if(bIntEnable)
    {
        IntPendClear(INT_CRYPTO_RESULT_AVAIL_IRQ);
        IntEnable(INT_CRYPTO_RESULT_AVAIL_IRQ);
    }

    //
    // Enable internal interrupts.
    //
    HWREG(CRYPTO_BASE + CRYPTO_O_IRQTYPE) = CRYPTO_IRQTYPE_LEVEL;
    HWREG(CRYPTO_BASE + CRYPTO_O_IRQEN) = CRYPTO_IRQEN_RESULT_AVAIL;

    //
    // Perform decryption if requested.
    //
    if(bDecrypt)
    {
        //
        // Configure the DMA controller - enable both DMA channels.
        //
        HWREGBITW(CRYPTO_BASE + CRYPTO_O_DMACH0CTL, CRYPTO_DMACH0CTL_EN_BITN) = 1;

        //
        // Base address of the payload data in ext. memory.
        //
        HWREG(CRYPTO_BASE + CRYPTO_O_DMACH0EXTADDR) =
            (uint32_t)pui32CipherText;

        //
        // Payload data length in bytes, equal to the cipher text length.
        //
        HWREG(CRYPTO_BASE + CRYPTO_O_DMACH0LEN) = ui32CryptoBlockLength;

        //
        // Enable DMA channel 1.
        //
        HWREGBITW(CRYPTO_BASE + CRYPTO_O_DMACH1CTL, CRYPTO_DMACH1CTL_EN_BITN) = 1;

        //
        // Base address of the output data buffer.
        //
        HWREG(CRYPTO_BASE + CRYPTO_O_DMACH1EXTADDR) =
            (uint32_t)pui32PlainText;

        //
        // Output data length in bytes, equal to the cipher text length.
        //
        HWREG(CRYPTO_BASE + CRYPTO_O_DMACH1LEN) = ui32CryptoBlockLength;
    }

    return AES_SUCCESS;
}

uint32_t CRYPTOCcmInvAuthDecryptStatus(void) {
    uint32_t ui32Status;

    //
    // Get the current DMA status.
    //
    ui32Status = HWREG(CRYPTO_BASE + CRYPTO_O_DMASTAT);

    //
    // Check if DMA is still busy.
    //
    if(ui32Status & CRYPTO_DMA_BSY)
    {
        return (AES_DMA_BSY);
    }

    //
    // Check the status of the DMA operation - return error if not success.
    //
    if(ui32Status & CRYPTO_DMA_BUS_ERROR)
    {
        return (AES_DMA_BUS_ERROR);
    }

    //
    // Operation successful - disable interrupt and return success
    //
    IntDisable(INT_CRYPTO_RESULT_AVAIL_IRQ);
    return (AES_SUCCESS);
}

uint32_t CRYPTOCcmInvAuthDecryptResultGet(uint32_t ui32AuthLength, uint32_t *pui32CipherText, uint32_t ui32CipherTextLength, uint32_t *pui32CcmTag) {
    uint32_t volatile ui32Tag[4];
    uint32_t ui32TagIndex;
    uint32_t i;
    uint32_t ui32Idx;

    ui32TagIndex = ui32CipherTextLength - ui32AuthLength;

    //
    // Result has already been copied to the output buffer by DMA
    // Disable master control.
    //
    HWREG(CRYPTO_BASE + CRYPTO_O_ALGSEL) = 0x00000000;

    //
    // Read tag - wait for the context ready bit.
    //
    do
    {
        CPUdelay(1);
    }
    while(!(HWREG(CRYPTO_BASE + CRYPTO_O_AESCTL) &
            CRYPTO_AESCTL_SAVED_CONTEXT_RDY));

    //
    // Read the Tag registers.
    //
    ui32Tag[0] = HWREG(CRYPTO_BASE + CRYPTO_O_AESTAGOUT0);
    ui32Tag[1] = HWREG(CRYPTO_BASE + CRYPTO_O_AESTAGOUT1);
    ui32Tag[2] = HWREG(CRYPTO_BASE + CRYPTO_O_AESTAGOUT2);
    ui32Tag[3] = HWREG(CRYPTO_BASE + CRYPTO_O_AESTAGOUT3);

    for(ui32Idx = 0; ui32Idx < ui32AuthLength ; ui32Idx++)
    {
        *((uint8_t*)pui32CcmTag + ui32Idx) = *((uint8_t*)ui32Tag + ui32Idx);
    }

    //
    // Operation successful -  clear interrupt status.
    //
    HWREG(CRYPTO_BASE + CRYPTO_O_IRQCLR) = (CRYPTO_IRQCLR_DMA_IN_DONE |
                                            CRYPTO_IRQCLR_RESULT_AVAIL);

    //
    // Verify the Tag.
    //
    for(i = 0; i < ui32AuthLength; i++)
    {
        if(*((uint8_t *)pui32CcmTag + i) !=
            (*((uint8_t *)pui32CipherText + ui32TagIndex + i)))
        {
            return CCM_AUTHENTICATION_FAILED;
        }
    }

    return AES_SUCCESS;
}

void OSCClockSourceSet(uint32_t ui32SrcClk, uint32_t ui32Osc) {
    //
    // Check the arguments.
    //
    ASSERT((ui32SrcClk & OSC_SRC_CLK_LF) ||
           (ui32SrcClk & OSC_SRC_CLK_MF) ||
           (ui32SrcClk & OSC_SRC_CLK_HF));
    ASSERT((ui32Osc == OSC_RCOSC_HF) ||
           (ui32Osc == OSC_RCOSC_LF) ||
           (ui32Osc == OSC_XOSC_HF) ||
           (ui32Osc == OSC_XOSC_LF));

    //
    // Request the high frequency source clock (using 24 MHz XTAL)
    //
    if(ui32SrcClk & OSC_SRC_CLK_HF)
    {
        //
        // Enable the HF XTAL as HF clock source
        //
        DDI16BitfieldWrite(AUX_DDI0_OSC_BASE, DDI_0_OSC_O_CTL0,
                           DDI_0_OSC_CTL0_SCLK_HF_SRC_SEL_M,
                           DDI_0_OSC_CTL0_SCLK_HF_SRC_SEL_S,
                           ui32Osc);
    }

    //
    // Configure the medium frequency source clock
    //
    if(ui32SrcClk & OSC_SRC_CLK_MF)
    {
        DDI16BitfieldWrite(AUX_DDI0_OSC_BASE, DDI_0_OSC_O_CTL0,
                           DDI_0_OSC_CTL0_SCLK_MF_SRC_SEL_M,
                           DDI_0_OSC_CTL0_SCLK_MF_SRC_SEL_S,
                           ui32Osc);
    }

    //
    // Configure the low frequency source clock.
    //
    if(ui32SrcClk & OSC_SRC_CLK_LF)
    {
        //
        // Change the clock source.
        //
        DDI16BitfieldWrite(AUX_DDI0_OSC_BASE, DDI_0_OSC_O_CTL0,
                           DDI_0_OSC_CTL0_SCLK_LF_SRC_SEL_M,
                           DDI_0_OSC_CTL0_SCLK_LF_SRC_SEL_S,
                           ui32Osc);
    }
}

uint32_t OSCClockSourceGet(uint32_t ui32SrcClk) {
    uint32_t ui32ClockSource;

    //
    // Check the arguments.
    //
    ASSERT((ui32SrcClk & OSC_SRC_CLK_LF) ||
           (ui32SrcClk & OSC_SRC_CLK_HF));

    //
    // Return the source for the selected clock.
    //
    if(ui32SrcClk == OSC_SRC_CLK_LF)
    {
        ui32ClockSource = DDI16BitfieldRead(AUX_DDI0_OSC_BASE, DDI_0_OSC_O_STAT0,
                                            DDI_0_OSC_STAT0_SCLK_LF_SRC_M,
                                            DDI_0_OSC_STAT0_SCLK_LF_SRC_S);
    }
    else
    {
        ui32ClockSource = DDI16BitfieldRead(AUX_DDI0_OSC_BASE, DDI_0_OSC_O_STAT0,
                                            DDI_0_OSC_STAT0_SCLK_HF_SRC_M,
                                            DDI_0_OSC_STAT0_SCLK_HF_SRC_S);
    }
    return (ui32ClockSource);
}
// We need intrinsic functions for IAR (if used in source code)
#ifdef __IAR_SYSTEMS_ICC__
#include <intrinsics.h>
#endif


//*****************************************************************************
//
// Defined CPU delay macro with microseconds as input
// Quick check shows: (To be further investigated)
// At 48 MHz RCOSC and VIMS.CONTROL.PREFETCH = 0, there is 5 cycles
// At 48 MHz RCOSC and VIMS.CONTROL.PREFETCH = 1, there is 4 cycles
// At 24 MHz RCOSC and VIMS.CONTROL.PREFETCH = 0, there is 3 cycles
//
//*****************************************************************************
#define CPU_DELAY_MICRO_SECONDS( x ) \
   CPUdelay(((uint32_t)((( x ) * 48.0 ) / 5.0 )) - 1 )


uint32_t SysCtrlResetSourceGet( void ) {
   if ( HWREG( AON_SYSCTL_BASE + AON_SYSCTL_O_RESETCTL ) & AON_SYSCTL_RESETCTL_WU_FROM_SD_M ) {
      return ( RSTSRC_WAKEUP_FROM_SHUTDOWN );
   } else {
      return (( HWREG( AON_SYSCTL_BASE + AON_SYSCTL_O_RESETCTL ) &
                AON_SYSCTL_RESETCTL_RESET_SRC_M ) >>
                AON_SYSCTL_RESETCTL_RESET_SRC_S ) ;
   }
}

int32_t AONBatMonTemperatureGetDegC( void ) {
   int32_t  signedTemp        ; // Signed extended temperature with 8 fractional bits
   int32_t  tempCorrection    ; // Voltage dependent temp correction with 8 fractional bits
   int8_t   voltageSlope      ; // Signed byte value representing the TEMP slope with battery voltage, in degrees C/V, with 4 fractional bits.

   //
   // Shift left then right to sign extend the BATMON_TEMP field
   //
   signedTemp = ((((int32_t)HWREG( AON_BATMON_BASE + AON_BATMON_O_TEMP ))
     << ( 32 - AON_BATMON_TEMP_INT_W - AON_BATMON_TEMP_INT_S ))
     >> ( 32 - AON_BATMON_TEMP_INT_W - AON_BATMON_TEMP_INT_S ));

   //
   // Typecasting voltageSlope to int8_t prior to assignment in order to make sure sign extension works properly
   // Using byte read (HWREGB) in order to make more efficient code since voltageSlope is assigned to bits[7:0] of FCFG1_O_MISC_TRIM
   //
   voltageSlope      = ((int8_t)HWREGB( FCFG1_BASE + FCFG1_O_MISC_TRIM ));
   tempCorrection    = (( voltageSlope * (((int32_t)HWREG( AON_BATMON_BASE + AON_BATMON_O_BAT )) - 0x300 )) >> 4 );

   return ((( signedTemp - tempCorrection ) + 0x80 ) >> 8 );
}
// We need intrinsic functions for IAR (if used in source code)
#ifdef __IAR_SYSTEMS_ICC__
#include <intrinsics.h>
#endif

void SetupAfterColdResetWakeupFromShutDownCfg1( uint32_t ccfg_ModeConfReg ) {
    int32_t    i32VddrSleepTrim;
    int32_t    i32VddrSleepDelta;

    {
        i32VddrSleepTrim = SetupSignExtendVddrTrimValue((
            HWREG( FCFG1_BASE + FCFG1_O_LDO_TRIM ) &
            FCFG1_LDO_TRIM_VDDR_TRIM_SLEEP_M ) >>
            FCFG1_LDO_TRIM_VDDR_TRIM_SLEEP_S ) ;
    }

    //
    // Adjust the VDDR_TRIM_SLEEP value with value adjustable by customer (CCFG_MODE_CONF_VDDR_TRIM_SLEEP_DELTA)
    // Read and sign extend VddrSleepDelta (in range -8 to +7)
    //
    i32VddrSleepDelta = ((((int32_t)ccfg_ModeConfReg )
        << ( 32 - CCFG_MODE_CONF_VDDR_TRIM_SLEEP_DELTA_W - CCFG_MODE_CONF_VDDR_TRIM_SLEEP_DELTA_S ))
        >> ( 32 - CCFG_MODE_CONF_VDDR_TRIM_SLEEP_DELTA_W ));
    // Calculate new VDDR sleep trim
    i32VddrSleepTrim = ( i32VddrSleepTrim + i32VddrSleepDelta + 1 );
    if ( i32VddrSleepTrim >  21 ) i32VddrSleepTrim =  21;
    if ( i32VddrSleepTrim < -10 ) i32VddrSleepTrim = -10;
    // Write adjusted value using MASKED write (MASK8)
    HWREGH( ADI3_BASE + ADI_O_MASK8B + ( ADI_3_REFSYS_O_DCDCCTL1 * 2 )) = (( ADI_3_REFSYS_DCDCCTL1_VDDR_TRIM_SLEEP_M << 8 ) |
        (( i32VddrSleepTrim << ADI_3_REFSYS_DCDCCTL1_VDDR_TRIM_SLEEP_S ) & ADI_3_REFSYS_DCDCCTL1_VDDR_TRIM_SLEEP_M ));

    //
    // 1.
    // Do not allow DCDC to be enabled if in external regulator mode.
    // Preventing this by setting both the RECHARGE and the ACTIVE bits bit in the CCFG_MODE_CONF copy register (ccfg_ModeConfReg).
    //
    // 2.
    // Adjusted battery monitor low limit in internal regulator mode.
    // This is done by setting AON_BATMON_FLASHPUMPP0_LOWLIM=0 in internal regulator mode.
    //
    if ( HWREG( AON_SYSCTL_BASE + AON_SYSCTL_O_PWRCTL ) & AON_SYSCTL_PWRCTL_EXT_REG_MODE ) {
        ccfg_ModeConfReg |= ( CCFG_MODE_CONF_DCDC_RECHARGE_M | CCFG_MODE_CONF_DCDC_ACTIVE_M );
    } else {
        HWREGBITW( AON_BATMON_BASE + AON_BATMON_O_FLASHPUMPP0, AON_BATMON_FLASHPUMPP0_LOWLIM_BITN ) = 0;
    }

    //
    // set the RECHARGE source based upon CCFG:MODE_CONF:DCDC_RECHARGE
    // Note: Inverse polarity
    //
    HWREGBITW( AON_SYSCTL_BASE + AON_SYSCTL_O_PWRCTL, AON_SYSCTL_PWRCTL_DCDC_EN_BITN ) =
        ((( ccfg_ModeConfReg >> CCFG_MODE_CONF_DCDC_RECHARGE_S ) & 1 ) ^ 1 );

    //
    // set the ACTIVE source based upon CCFG:MODE_CONF:DCDC_ACTIVE
    // Note: Inverse polarity
    //
    HWREGBITW( AON_SYSCTL_BASE + AON_SYSCTL_O_PWRCTL, AON_SYSCTL_PWRCTL_DCDC_ACTIVE_BITN ) =
        ((( ccfg_ModeConfReg >> CCFG_MODE_CONF_DCDC_ACTIVE_S ) & 1 ) ^ 1 );
}

void SetupAfterColdResetWakeupFromShutDownCfg2( uint32_t ui32Fcfg1Revision, uint32_t ccfg_ModeConfReg ) {
    uint32_t   ui32Trim;

    //
    // Following sequence is required for using XOSCHF, if not included
    // devices crashes when trying to switch to XOSCHF.
    //
    // Trim CAP settings. Get and set trim value for the ANABYPASS_VALUE1
    // register
    ui32Trim = SetupGetTrimForAnabypassValue1( ccfg_ModeConfReg );
    DDI32RegWrite(AUX_DDI0_OSC_BASE, DDI_0_OSC_O_ANABYPASSVAL1, ui32Trim);

    // Trim RCOSC_LF. Get and set trim values for the RCOSCLF_RTUNE_TRIM and
    // RCOSCLF_CTUNE_TRIM fields in the XOSCLF_RCOSCLF_CTRL register.
    ui32Trim = SetupGetTrimForRcOscLfRtuneCtuneTrim();
    DDI16BitfieldWrite(AUX_DDI0_OSC_BASE, DDI_0_OSC_O_LFOSCCTL,
                       (DDI_0_OSC_LFOSCCTL_RCOSCLF_CTUNE_TRIM_M |
                        DDI_0_OSC_LFOSCCTL_RCOSCLF_RTUNE_TRIM_M),
                       DDI_0_OSC_LFOSCCTL_RCOSCLF_CTUNE_TRIM_S,
                       ui32Trim);

    // Trim XOSCHF IBIAS THERM. Get and set trim value for the
    // XOSCHF IBIAS THERM bit field in the ANABYPASS_VALUE2 register. Other
    // register bit fields are set to 0.
    ui32Trim = SetupGetTrimForXoscHfIbiastherm();
    DDI32RegWrite(AUX_DDI0_OSC_BASE, DDI_0_OSC_O_ANABYPASSVAL2,
                  ui32Trim<<DDI_0_OSC_ANABYPASSVAL2_XOSC_HF_IBIASTHERM_S);

    // Trim AMPCOMP settings required before switch to XOSCHF
    ui32Trim = SetupGetTrimForAmpcompTh2();
    DDI32RegWrite(AUX_DDI0_OSC_BASE, DDI_0_OSC_O_AMPCOMPTH2, ui32Trim);
    ui32Trim = SetupGetTrimForAmpcompTh1();
    DDI32RegWrite(AUX_DDI0_OSC_BASE, DDI_0_OSC_O_AMPCOMPTH1, ui32Trim);
    ui32Trim = SetupGetTrimForAmpcompCtrl( ui32Fcfg1Revision );
    DDI32RegWrite(AUX_DDI0_OSC_BASE, DDI_0_OSC_O_AMPCOMPCTL, ui32Trim);

    //
    // Set trim for DDI_0_OSC_ADCDOUBLERNANOAMPCTL_ADC_SH_MODE_EN in accordance to FCFG1 setting
    // This is bit[5] in the DDI_0_OSC_O_ADCDOUBLERNANOAMPCTL register
    // Using MASK4 write + 1 => writing to bits[7:4]
    //
    ui32Trim = SetupGetTrimForAdcShModeEn( ui32Fcfg1Revision );
    HWREGB( AUX_DDI0_OSC_BASE + DDI_O_MASK4B + ( DDI_0_OSC_O_ADCDOUBLERNANOAMPCTL * 2 ) + 1 ) =
      ( 0x20 | ( ui32Trim << 1 ));

    //
    // Set trim for DDI_0_OSC_ADCDOUBLERNANOAMPCTL_ADC_SH_VBUF_EN in accordance to FCFG1 setting
    // This is bit[4] in the DDI_0_OSC_O_ADCDOUBLERNANOAMPCTL register
    // Using MASK4 write + 1 => writing to bits[7:4]
    //
    ui32Trim = SetupGetTrimForAdcShVbufEn( ui32Fcfg1Revision );
    HWREGB( AUX_DDI0_OSC_BASE + DDI_O_MASK4B + ( DDI_0_OSC_O_ADCDOUBLERNANOAMPCTL * 2 ) + 1 ) =
      ( 0x10 | ( ui32Trim ));

    //
    // Set trim for the PEAK_DET_ITRIM, HP_BUF_ITRIM and LP_BUF_ITRIM bit fields
    // in the DDI0_OSC_O_XOSCHFCTL register in accordance to FCFG1 setting.
    // Remaining register bit fields are set to their reset values of 0.
    //
    ui32Trim = SetupGetTrimForXoscHfCtl(ui32Fcfg1Revision);
    DDI32RegWrite(AUX_DDI0_OSC_BASE, DDI_0_OSC_O_XOSCHFCTL, ui32Trim);

    //
    // Set trim for DBLR_LOOP_FILTER_RESET_VOLTAGE in accordance to FCFG1 setting
    // (This is bits [18:17] in DDI_0_OSC_O_ADCDOUBLERNANOAMPCTL)
    // (Using MASK4 write + 4 => writing to bits[19:16] => (4*4))
    // (Assuming: DDI_0_OSC_ADCDOUBLERNANOAMPCTL_DBLR_LOOP_FILTER_RESET_VOLTAGE_S = 17 and
    //  that DDI_0_OSC_ADCDOUBLERNANOAMPCTL_DBLR_LOOP_FILTER_RESET_VOLTAGE_M = 0x00060000)
    //
    ui32Trim = SetupGetTrimForDblrLoopFilterResetVoltage( ui32Fcfg1Revision );
    HWREGB( AUX_DDI0_OSC_BASE + DDI_O_MASK4B + ( DDI_0_OSC_O_ADCDOUBLERNANOAMPCTL * 2 ) + 4 ) =
      ( 0x60 | ( ui32Trim << 1 ));

    //
    // Update DDI_0_OSC_ATESTCTL_ATESTLF_RCOSCLF_IBIAS_TRIM with data from
    // FCFG1_OSC_CONF_ATESTLF_RCOSCLF_IBIAS_TRIM
    // This is DDI_0_OSC_O_ATESTCTL bit[7]
    // ( DDI_0_OSC_O_ATESTCTL is currently hidden (but=0x00000020))
    // Using MASK4 write + 1 => writing to bits[7:4]
    //
    ui32Trim = SetupGetTrimForRcOscLfIBiasTrim( ui32Fcfg1Revision );
    HWREGB( AUX_DDI0_OSC_BASE + DDI_O_MASK4B + ( 0x00000020 * 2 ) + 1 ) =
      ( 0x80 | ( ui32Trim << 3 ));

    //
    // Update DDI_0_OSC_LFOSCCTL_XOSCLF_REGULATOR_TRIM and
    //        DDI_0_OSC_LFOSCCTL_XOSCLF_CMIRRWR_RATIO in one write
    // This can be simplified since the registers are packed together in the same
    // order both in FCFG1 and in the HW register.
    // This spans DDI_0_OSC_O_LFOSCCTL bits[23:18]
    // Using MASK8 write + 4 => writing to bits[23:16]
    //
    ui32Trim = SetupGetTrimForXoscLfRegulatorAndCmirrwrRatio( ui32Fcfg1Revision );
    HWREGH( AUX_DDI0_OSC_BASE + DDI_O_MASK8B + ( DDI_0_OSC_O_LFOSCCTL * 2 ) + 4 ) =
      ( 0xFC00 | ( ui32Trim << 2 ));

    //
    // Set trim the HPM_IBIAS_WAIT_CNT, LPM_IBIAS_WAIT_CNT and IDAC_STEP bit
    // fields in the DDI0_OSC_O_RADCEXTCFG register in accordance to FCFG1 setting.
    // Remaining register bit fields are set to their reset values of 0.
    //
    ui32Trim = SetupGetTrimForRadcExtCfg(ui32Fcfg1Revision);
    DDI32RegWrite(AUX_DDI0_OSC_BASE, DDI_0_OSC_O_RADCEXTCFG, ui32Trim);

    // Setting FORCE_KICKSTART_EN (ref. CC26_V1_BUG00261). Should also be done for PG2
    // (This is bit 22 in DDI_0_OSC_O_CTL0)
    HWREG( AUX_DDI0_OSC_BASE + DDI_O_SET + DDI_0_OSC_O_CTL0 ) = DDI_0_OSC_CTL0_FORCE_KICKSTART_EN;
}

void SetupAfterColdResetWakeupFromShutDownCfg3( uint32_t ccfg_ModeConfReg ) {
    uint32_t   fcfg1OscConf;
    uint32_t   ui32Trim;
    uint32_t   currentHfClock;
    uint32_t   ccfgExtLfClk;

    //
    // Examin the XOSC_FREQ field to select 0x1=HPOSC, 0x2=48MHz XOSC, 0x3=24MHz XOSC
    //
    switch (( ccfg_ModeConfReg & CCFG_MODE_CONF_XOSC_FREQ_M ) >> CCFG_MODE_CONF_XOSC_FREQ_S ) {
    case 2 :
        // XOSC source is a 48 MHz xtal
        // Do nothing (since this is the reset setting)
        break;
    case 1 :
        // XOSC source is HPOSC (trim the HPOSC if this is a chip with HPOSC, otherwise skip trimming and default to 24 MHz XOSC)

        fcfg1OscConf = HWREG( FCFG1_BASE + FCFG1_O_OSC_CONF );

        if (( fcfg1OscConf & FCFG1_OSC_CONF_HPOSC_OPTION ) == 0 ) {
            // This is a HPOSC chip, apply HPOSC settings
            // Set bit DDI_0_OSC_CTL0_HPOSC_MODE_EN (this is bit 14 in DDI_0_OSC_O_CTL0)
            HWREG( AUX_DDI0_OSC_BASE + DDI_O_SET + DDI_0_OSC_O_CTL0 ) = DDI_0_OSC_CTL0_HPOSC_MODE_EN;

            // ADI_2_REFSYS_HPOSCCTL2_BIAS_HOLD_MODE_EN = FCFG1_OSC_CONF_HPOSC_BIAS_HOLD_MODE_EN   (1 bit)
            // ADI_2_REFSYS_HPOSCCTL2_CURRMIRR_RATIO    = FCFG1_OSC_CONF_HPOSC_CURRMIRR_RATIO      (4 bits)
            // ADI_2_REFSYS_HPOSCCTL1_BIAS_RES_SET      = FCFG1_OSC_CONF_HPOSC_BIAS_RES_SET        (4 bits)
            // ADI_2_REFSYS_HPOSCCTL0_FILTER_EN         = FCFG1_OSC_CONF_HPOSC_FILTER_EN           (1 bit)
            // ADI_2_REFSYS_HPOSCCTL0_BIAS_RECHARGE_DLY = FCFG1_OSC_CONF_HPOSC_BIAS_RECHARGE_DELAY (2 bits)
            // ADI_2_REFSYS_HPOSCCTL0_SERIES_CAP        = FCFG1_OSC_CONF_HPOSC_SERIES_CAP          (2 bits)
            // ADI_2_REFSYS_HPOSCCTL0_DIV3_BYPASS       = FCFG1_OSC_CONF_HPOSC_DIV3_BYPASS         (1 bit)

            HWREG( ADI2_BASE + ADI_2_REFSYS_O_HPOSCCTL2 ) = (( HWREG( ADI2_BASE + ADI_2_REFSYS_O_HPOSCCTL2 ) &
                  ~( ADI_2_REFSYS_HPOSCCTL2_BIAS_HOLD_MODE_EN_M | ADI_2_REFSYS_HPOSCCTL2_CURRMIRR_RATIO_M  )                                                                       ) |
                   ((( fcfg1OscConf & FCFG1_OSC_CONF_HPOSC_BIAS_HOLD_MODE_EN_M   ) >> FCFG1_OSC_CONF_HPOSC_BIAS_HOLD_MODE_EN_S   ) << ADI_2_REFSYS_HPOSCCTL2_BIAS_HOLD_MODE_EN_S   ) |
                   ((( fcfg1OscConf & FCFG1_OSC_CONF_HPOSC_CURRMIRR_RATIO_M      ) >> FCFG1_OSC_CONF_HPOSC_CURRMIRR_RATIO_S      ) << ADI_2_REFSYS_HPOSCCTL2_CURRMIRR_RATIO_S      )   );
            HWREG( ADI2_BASE + ADI_2_REFSYS_O_HPOSCCTL1 ) = (( HWREG( ADI2_BASE + ADI_2_REFSYS_O_HPOSCCTL1 ) & ~( ADI_2_REFSYS_HPOSCCTL1_BIAS_RES_SET_M )                          ) |
                   ((( fcfg1OscConf & FCFG1_OSC_CONF_HPOSC_BIAS_RES_SET_M        ) >> FCFG1_OSC_CONF_HPOSC_BIAS_RES_SET_S        ) << ADI_2_REFSYS_HPOSCCTL1_BIAS_RES_SET_S        )   );
            HWREG( ADI2_BASE + ADI_2_REFSYS_O_HPOSCCTL0 ) = (( HWREG( ADI2_BASE + ADI_2_REFSYS_O_HPOSCCTL0 ) &
                  ~( ADI_2_REFSYS_HPOSCCTL0_FILTER_EN_M | ADI_2_REFSYS_HPOSCCTL0_BIAS_RECHARGE_DLY_M | ADI_2_REFSYS_HPOSCCTL0_SERIES_CAP_M | ADI_2_REFSYS_HPOSCCTL0_DIV3_BYPASS_M )) |
                   ((( fcfg1OscConf & FCFG1_OSC_CONF_HPOSC_FILTER_EN_M           ) >> FCFG1_OSC_CONF_HPOSC_FILTER_EN_S           ) << ADI_2_REFSYS_HPOSCCTL0_FILTER_EN_S           ) |
                   ((( fcfg1OscConf & FCFG1_OSC_CONF_HPOSC_BIAS_RECHARGE_DELAY_M ) >> FCFG1_OSC_CONF_HPOSC_BIAS_RECHARGE_DELAY_S ) << ADI_2_REFSYS_HPOSCCTL0_BIAS_RECHARGE_DLY_S   ) |
                   ((( fcfg1OscConf & FCFG1_OSC_CONF_HPOSC_SERIES_CAP_M          ) >> FCFG1_OSC_CONF_HPOSC_SERIES_CAP_S          ) << ADI_2_REFSYS_HPOSCCTL0_SERIES_CAP_S          ) |
                   ((( fcfg1OscConf & FCFG1_OSC_CONF_HPOSC_DIV3_BYPASS_M         ) >> FCFG1_OSC_CONF_HPOSC_DIV3_BYPASS_S         ) << ADI_2_REFSYS_HPOSCCTL0_DIV3_BYPASS_S         )   );
            break;
        }
        // Not a HPOSC chip - fall through to default
    default :
        // XOSC source is a 24 MHz xtal (default)
        // Set bit DDI_0_OSC_CTL0_XTAL_IS_24M (this is bit 31 in DDI_0_OSC_O_CTL0)
        HWREG( AUX_DDI0_OSC_BASE + DDI_O_SET + DDI_0_OSC_O_CTL0 ) = DDI_0_OSC_CTL0_XTAL_IS_24M;
        break;
    }

    //
    // Set XOSC_HF in bypass mode if CCFG is configured for external TCXO
    // Please note that it is up to the custommer to make sure that the external clock source is up and running before XOSC_HF can be used.
    //
    if (( HWREG( CCFG_BASE + CCFG_O_SIZE_AND_DIS_FLAGS ) & CCFG_SIZE_AND_DIS_FLAGS_DIS_TCXO ) == 0 ) {
        HWREG( AUX_DDI0_OSC_BASE + DDI_O_SET + DDI_0_OSC_O_XOSCHFCTL ) = DDI_0_OSC_XOSCHFCTL_BYPASS;
    }

    // Clear DDI_0_OSC_CTL0_CLK_LOSS_EN (ClockLossEventEnable()). This is bit 9 in DDI_0_OSC_O_CTL0.
    // This is typically already 0 except on Lizard where it is set in ROM-boot
    HWREG( AUX_DDI0_OSC_BASE + DDI_O_CLR + DDI_0_OSC_O_CTL0 ) = DDI_0_OSC_CTL0_CLK_LOSS_EN;

    // Setting DDI_0_OSC_CTL1_XOSC_HF_FAST_START according to value found in FCFG1
    ui32Trim = SetupGetTrimForXoscHfFastStart();
    HWREGB( AUX_DDI0_OSC_BASE + DDI_O_MASK4B + ( DDI_0_OSC_O_CTL1 * 2 )) = ( 0x30 | ui32Trim );

    //
    // setup the LF clock based upon CCFG:MODE_CONF:SCLK_LF_OPTION
    //
    switch (( ccfg_ModeConfReg & CCFG_MODE_CONF_SCLK_LF_OPTION_M ) >> CCFG_MODE_CONF_SCLK_LF_OPTION_S ) {
    case 0 : // XOSC_HF_DLF (XOSCHF/1536) -> SCLK_LF (=31250Hz)
        OSCClockSourceSet( OSC_SRC_CLK_LF, OSC_XOSC_HF );
        SetupSetAonRtcSubSecInc( 0x8637BD );
        break;
    case 1 : // EXTERNAL signal -> SCLK_LF (frequency=2^38/CCFG_EXT_LF_CLK_RTC_INCREMENT)
        // Set SCLK_LF to use the same source as SCLK_HF
        // Can be simplified a bit since possible return values for HF matches LF settings
        currentHfClock = OSCClockSourceGet( OSC_SRC_CLK_HF );
        OSCClockSourceSet( OSC_SRC_CLK_LF, currentHfClock );
        while( OSCClockSourceGet( OSC_SRC_CLK_LF ) != currentHfClock ) {
            // Wait until switched
        }
        ccfgExtLfClk = HWREG( CCFG_BASE + CCFG_O_EXT_LF_CLK );
        SetupSetAonRtcSubSecInc(( ccfgExtLfClk & CCFG_EXT_LF_CLK_RTC_INCREMENT_M ) >> CCFG_EXT_LF_CLK_RTC_INCREMENT_S );
        IOCPortConfigureSet(( ccfgExtLfClk & CCFG_EXT_LF_CLK_DIO_M ) >> CCFG_EXT_LF_CLK_DIO_S,
                              IOC_PORT_AON_CLK32K,
                              IOC_STD_INPUT | IOC_HYST_ENABLE );   // Route external clock to AON IOC w/hysteresis
                                                                   // Set XOSC_LF in bypass mode to allow external 32k clock
        HWREG( AUX_DDI0_OSC_BASE + DDI_O_SET + DDI_0_OSC_O_CTL0 ) = DDI_0_OSC_CTL0_XOSC_LF_DIG_BYPASS;
        // Fall through to set XOSC_LF as SCLK_LF source
    case 2 : // XOSC_LF -> SLCK_LF (32768 Hz)
        OSCClockSourceSet( OSC_SRC_CLK_LF, OSC_XOSC_LF );
        break;
    default : // (=3) RCOSC_LF
        OSCClockSourceSet( OSC_SRC_CLK_LF, OSC_RCOSC_LF );
        break;
    }

    //
    // Update ADI_4_AUX_ADCREF1_VTRIM with value from FCFG1
    //
    HWREGB( AUX_ADI4_BASE + ADI_4_AUX_O_ADCREF1 ) =
      ((( HWREG( FCFG1_BASE + FCFG1_O_SOC_ADC_REF_TRIM_AND_OFFSET_EXT ) >>
      FCFG1_SOC_ADC_REF_TRIM_AND_OFFSET_EXT_SOC_ADC_REF_VOLTAGE_TRIM_TEMP1_S ) <<
      ADI_4_AUX_ADCREF1_VTRIM_S ) &
      ADI_4_AUX_ADCREF1_VTRIM_M );

    //
    // Set ADI_4_AUX:ADC0.SMPL_CYCLE_EXP to it's default minimum value (=3)
    // (Note: Using MASK8B requires that the bits to be modified must be within the same
    //        byte boundary which is the case for the ADI_4_AUX_ADC0_SMPL_CYCLE_EXP field)
    //
    HWREGH( AUX_ADI4_BASE + ADI_O_MASK8B + ( ADI_4_AUX_O_ADC0 * 2 )) =
      ( ADI_4_AUX_ADC0_SMPL_CYCLE_EXP_M << 8 ) | ( 3 << ADI_4_AUX_ADC0_SMPL_CYCLE_EXP_S );

    //
    // Sync with AON
    //
    SysCtrlAonSync();
}

uint32_t SetupGetTrimForAnabypassValue1( uint32_t ccfg_ModeConfReg ) {
    uint32_t ui32Fcfg1Value            ;
    uint32_t ui32XoscHfRow             ;
    uint32_t ui32XoscHfCol             ;
    int32_t  i32CustomerDeltaAdjust    ;
    uint32_t ui32TrimValue             ;

    // Use device specific trim values located in factory configuration
    // area for the XOSC_HF_COLUMN_Q12 and XOSC_HF_ROW_Q12 bit fields in
    // the ANABYPASS_VALUE1 register. Value for the other bit fields
    // are set to 0.

    ui32Fcfg1Value = HWREG(FCFG1_BASE + FCFG1_O_CONFIG_OSC_TOP);
    ui32XoscHfRow = (( ui32Fcfg1Value &
        FCFG1_CONFIG_OSC_TOP_XOSC_HF_ROW_Q12_M ) >>
        FCFG1_CONFIG_OSC_TOP_XOSC_HF_ROW_Q12_S );
    ui32XoscHfCol = (( ui32Fcfg1Value &
        FCFG1_CONFIG_OSC_TOP_XOSC_HF_COLUMN_Q12_M ) >>
        FCFG1_CONFIG_OSC_TOP_XOSC_HF_COLUMN_Q12_S );

    i32CustomerDeltaAdjust = 0;
    if (( ccfg_ModeConfReg & CCFG_MODE_CONF_XOSC_CAP_MOD ) == 0 ) {
        // XOSC_CAP_MOD = 0 means: CAP_ARRAY_DELTA is in use -> Apply compensation
        // XOSC_CAPARRAY_DELTA is located in bit[15:8] of ccfg_ModeConfReg
        // Note: HW_REV_DEPENDENT_IMPLEMENTATION. Field width is not given by
        // a define and sign extension must therefore be hardcoded.
        // ( A small test program is created verifying the code lines below:
        //   Ref.: ..\test\small_standalone_test_programs\CapArrayDeltaAdjust_test.c)
        i32CustomerDeltaAdjust = ((int32_t)ccfg_ModeConfReg << 16 ) >> 24;

        while ( i32CustomerDeltaAdjust < 0 ) {
            ui32XoscHfCol >>= 1;                              // COL 1 step down
            if ( ui32XoscHfCol == 0 ) {                       // if COL below minimum
                ui32XoscHfCol = 0xFFFF;                       //   Set COL to maximum
                ui32XoscHfRow >>= 1;                          //   ROW 1 step down
                if ( ui32XoscHfRow == 0 ) {                   // if ROW below minimum
                   ui32XoscHfRow = 1;                         //   Set both ROW and COL
                   ui32XoscHfCol = 1;                         //   to minimum
                }
            }
            i32CustomerDeltaAdjust++;
        }
        while ( i32CustomerDeltaAdjust > 0 ) {
            ui32XoscHfCol = ( ui32XoscHfCol << 1 ) | 1;       // COL 1 step up
            if ( ui32XoscHfCol > 0xFFFF ) {                   // if COL above maximum
                ui32XoscHfCol = 1;                            //   Set COL to minimum
                ui32XoscHfRow = ( ui32XoscHfRow << 1 ) | 1;   //   ROW 1 step up
                if ( ui32XoscHfRow > 0xF ) {                  // if ROW above maximum
                   ui32XoscHfRow = 0xF;                       //   Set both ROW and COL
                   ui32XoscHfCol = 0xFFFF;                    //   to maximum
                }
            }
            i32CustomerDeltaAdjust--;
        }
    }

    ui32TrimValue = (( ui32XoscHfRow << DDI_0_OSC_ANABYPASSVAL1_XOSC_HF_ROW_Q12_S    ) |
                     ( ui32XoscHfCol << DDI_0_OSC_ANABYPASSVAL1_XOSC_HF_COLUMN_Q12_S )   );

    return (ui32TrimValue);
}

uint32_t SetupGetTrimForRcOscLfRtuneCtuneTrim( void ) {
    uint32_t ui32TrimValue;

    // Use device specific trim values located in factory configuration
    // area
    ui32TrimValue =
        ((HWREG(FCFG1_BASE + FCFG1_O_CONFIG_OSC_TOP) &
          FCFG1_CONFIG_OSC_TOP_RCOSCLF_CTUNE_TRIM_M)>>
          FCFG1_CONFIG_OSC_TOP_RCOSCLF_CTUNE_TRIM_S)<<
            DDI_0_OSC_LFOSCCTL_RCOSCLF_CTUNE_TRIM_S;

    ui32TrimValue |=
        ((HWREG(FCFG1_BASE + FCFG1_O_CONFIG_OSC_TOP) &
          FCFG1_CONFIG_OSC_TOP_RCOSCLF_RTUNE_TRIM_M)>>
          FCFG1_CONFIG_OSC_TOP_RCOSCLF_RTUNE_TRIM_S)<<
            DDI_0_OSC_LFOSCCTL_RCOSCLF_RTUNE_TRIM_S;

    return(ui32TrimValue);
}

uint32_t SetupGetTrimForXoscHfIbiastherm( void ) {
    uint32_t ui32TrimValue;

    // Use device specific trim value located in factory configuration
    // area
    ui32TrimValue =
        (HWREG(FCFG1_BASE + FCFG1_O_ANABYPASS_VALUE2) &
         FCFG1_ANABYPASS_VALUE2_XOSC_HF_IBIASTHERM_M)>>
         FCFG1_ANABYPASS_VALUE2_XOSC_HF_IBIASTHERM_S;

    return(ui32TrimValue);
}

uint32_t SetupGetTrimForAmpcompTh2( void ) {
    uint32_t ui32TrimValue;
    uint32_t ui32Fcfg1Value;

    // Use device specific trim value located in factory configuration
    // area. All defined register bit fields have corresponding trim
    // value in the factory configuration area
    ui32Fcfg1Value = HWREG(FCFG1_BASE + FCFG1_O_AMPCOMP_TH2);
    ui32TrimValue = ((ui32Fcfg1Value &
                      FCFG1_AMPCOMP_TH2_LPMUPDATE_LTH_M)>>
                      FCFG1_AMPCOMP_TH2_LPMUPDATE_LTH_S)<<
                   DDI_0_OSC_AMPCOMPTH2_LPMUPDATE_LTH_S;
    ui32TrimValue |= (((ui32Fcfg1Value &
                        FCFG1_AMPCOMP_TH2_LPMUPDATE_HTM_M)>>
                        FCFG1_AMPCOMP_TH2_LPMUPDATE_HTM_S)<<
                     DDI_0_OSC_AMPCOMPTH2_LPMUPDATE_HTH_S);
    ui32TrimValue |= (((ui32Fcfg1Value &
                        FCFG1_AMPCOMP_TH2_ADC_COMP_AMPTH_LPM_M)>>
                        FCFG1_AMPCOMP_TH2_ADC_COMP_AMPTH_LPM_S)<<
                     DDI_0_OSC_AMPCOMPTH2_ADC_COMP_AMPTH_LPM_S);
    ui32TrimValue |= (((ui32Fcfg1Value &
                        FCFG1_AMPCOMP_TH2_ADC_COMP_AMPTH_HPM_M)>>
                        FCFG1_AMPCOMP_TH2_ADC_COMP_AMPTH_HPM_S)<<
                     DDI_0_OSC_AMPCOMPTH2_ADC_COMP_AMPTH_HPM_S);

    return(ui32TrimValue);
}

uint32_t SetupGetTrimForAmpcompTh1( void ) {
    uint32_t ui32TrimValue;
    uint32_t ui32Fcfg1Value;

    // Use device specific trim values located in factory configuration
    // area. All defined register bit fields have a corresponding trim
    // value in the factory configuration area
    ui32Fcfg1Value = HWREG(FCFG1_BASE + FCFG1_O_AMPCOMP_TH1);
    ui32TrimValue = (((ui32Fcfg1Value &
                        FCFG1_AMPCOMP_TH1_HPMRAMP3_LTH_M)>>
                        FCFG1_AMPCOMP_TH1_HPMRAMP3_LTH_S)<<
                     DDI_0_OSC_AMPCOMPTH1_HPMRAMP3_LTH_S);
    ui32TrimValue |= (((ui32Fcfg1Value &
                        FCFG1_AMPCOMP_TH1_HPMRAMP3_HTH_M)>>
                        FCFG1_AMPCOMP_TH1_HPMRAMP3_HTH_S)<<
                     DDI_0_OSC_AMPCOMPTH1_HPMRAMP3_HTH_S);
    ui32TrimValue |= (((ui32Fcfg1Value &
                        FCFG1_AMPCOMP_TH1_IBIASCAP_LPTOHP_OL_CNT_M)>>
                        FCFG1_AMPCOMP_TH1_IBIASCAP_LPTOHP_OL_CNT_S)<<
                     DDI_0_OSC_AMPCOMPTH1_IBIASCAP_LPTOHP_OL_CNT_S);
    ui32TrimValue |= (((ui32Fcfg1Value &
                        FCFG1_AMPCOMP_TH1_HPMRAMP1_TH_M)>>
                        FCFG1_AMPCOMP_TH1_HPMRAMP1_TH_S)<<
                     DDI_0_OSC_AMPCOMPTH1_HPMRAMP1_TH_S);

    return(ui32TrimValue);
}

uint32_t SetupGetTrimForAmpcompCtrl( uint32_t ui32Fcfg1Revision ) {
    uint32_t ui32TrimValue    ;
    uint32_t ui32Fcfg1Value   ;
    uint32_t ibiasOffset      ;
    uint32_t ibiasInit        ;
    uint32_t modeConf1        ;
    int32_t  deltaAdjust      ;

    // Use device specific trim values located in factory configuration
    // area. Register bit fields without trim values in the factory
    // configuration area will be set to the value of 0.
    ui32Fcfg1Value = HWREG( FCFG1_BASE + FCFG1_O_AMPCOMP_CTRL1 );

    ibiasOffset    = ( ui32Fcfg1Value &
                       FCFG1_AMPCOMP_CTRL1_IBIAS_OFFSET_M ) >>
                       FCFG1_AMPCOMP_CTRL1_IBIAS_OFFSET_S ;
    ibiasInit      = ( ui32Fcfg1Value &
                       FCFG1_AMPCOMP_CTRL1_IBIAS_INIT_M ) >>
                       FCFG1_AMPCOMP_CTRL1_IBIAS_INIT_S ;

    if (( HWREG( CCFG_BASE + CCFG_O_SIZE_AND_DIS_FLAGS ) & CCFG_SIZE_AND_DIS_FLAGS_DIS_XOSC_OVR_M ) == 0 ) {
        // Adjust with DELTA_IBIAS_OFFSET and DELTA_IBIAS_INIT from CCFG
        modeConf1   = HWREG( CCFG_BASE + CCFG_O_MODE_CONF_1 );

        // Both fields are signed 4-bit values. This is an assumption when doing the sign extension.
        deltaAdjust = ((int32_t)modeConf1 << ( 32 - CCFG_MODE_CONF_1_DELTA_IBIAS_OFFSET_S - 4 )) >> 28;
        deltaAdjust += (int32_t)ibiasOffset;
        if ( deltaAdjust < 0 ) {
           deltaAdjust = 0;
        }
        if ( deltaAdjust > ( DDI_0_OSC_AMPCOMPCTL_IBIAS_OFFSET_M >> DDI_0_OSC_AMPCOMPCTL_IBIAS_OFFSET_S )) {
            deltaAdjust  = ( DDI_0_OSC_AMPCOMPCTL_IBIAS_OFFSET_M >> DDI_0_OSC_AMPCOMPCTL_IBIAS_OFFSET_S );
        }
        ibiasOffset = (uint32_t)deltaAdjust;

        deltaAdjust = ((int32_t)modeConf1 << ( 32 - CCFG_MODE_CONF_1_DELTA_IBIAS_INIT_S - 4 )) >> 28;
        deltaAdjust += (int32_t)ibiasInit;
        if ( deltaAdjust < 0 ) {
           deltaAdjust = 0;
        }
        if ( deltaAdjust > ( DDI_0_OSC_AMPCOMPCTL_IBIAS_INIT_M >> DDI_0_OSC_AMPCOMPCTL_IBIAS_INIT_S )) {
            deltaAdjust  = ( DDI_0_OSC_AMPCOMPCTL_IBIAS_INIT_M >> DDI_0_OSC_AMPCOMPCTL_IBIAS_INIT_S );
        }
        ibiasInit = (uint32_t)deltaAdjust;
    }
    ui32TrimValue = ( ibiasOffset << DDI_0_OSC_AMPCOMPCTL_IBIAS_OFFSET_S ) |
                    ( ibiasInit   << DDI_0_OSC_AMPCOMPCTL_IBIAS_INIT_S   ) ;

    ui32TrimValue |= (((ui32Fcfg1Value &
                        FCFG1_AMPCOMP_CTRL1_LPM_IBIAS_WAIT_CNT_FINAL_M)>>
                        FCFG1_AMPCOMP_CTRL1_LPM_IBIAS_WAIT_CNT_FINAL_S)<<
                       DDI_0_OSC_AMPCOMPCTL_LPM_IBIAS_WAIT_CNT_FINAL_S);
    ui32TrimValue |= (((ui32Fcfg1Value &
                        FCFG1_AMPCOMP_CTRL1_CAP_STEP_M)>>
                        FCFG1_AMPCOMP_CTRL1_CAP_STEP_S)<<
                       DDI_0_OSC_AMPCOMPCTL_CAP_STEP_S);
    ui32TrimValue |= (((ui32Fcfg1Value &
                        FCFG1_AMPCOMP_CTRL1_IBIASCAP_HPTOLP_OL_CNT_M)>>
                        FCFG1_AMPCOMP_CTRL1_IBIASCAP_HPTOLP_OL_CNT_S)<<
                       DDI_0_OSC_AMPCOMPCTL_IBIASCAP_HPTOLP_OL_CNT_S);

    if ( ui32Fcfg1Revision >= 0x00000022 ) {
        ui32TrimValue |= ((( ui32Fcfg1Value &
            FCFG1_AMPCOMP_CTRL1_AMPCOMP_REQ_MODE_M ) >>
            FCFG1_AMPCOMP_CTRL1_AMPCOMP_REQ_MODE_S ) <<
           DDI_0_OSC_AMPCOMPCTL_AMPCOMP_REQ_MODE_S );
    }

    return(ui32TrimValue);
}

uint32_t SetupGetTrimForDblrLoopFilterResetVoltage( uint32_t ui32Fcfg1Revision ) {
   uint32_t dblrLoopFilterResetVoltageValue = 0; // Reset value

   if ( ui32Fcfg1Revision >= 0x00000020 ) {
      dblrLoopFilterResetVoltageValue = ( HWREG( FCFG1_BASE + FCFG1_O_MISC_OTP_DATA_1 ) &
         FCFG1_MISC_OTP_DATA_1_DBLR_LOOP_FILTER_RESET_VOLTAGE_M ) >>
         FCFG1_MISC_OTP_DATA_1_DBLR_LOOP_FILTER_RESET_VOLTAGE_S;
   }

   return ( dblrLoopFilterResetVoltageValue );
}

uint32_t SetupGetTrimForAdcShModeEn( uint32_t ui32Fcfg1Revision ) {
   uint32_t getTrimForAdcShModeEnValue = 1; // Recommended default setting

   if ( ui32Fcfg1Revision >= 0x00000022 ) {
      getTrimForAdcShModeEnValue = ( HWREG( FCFG1_BASE + FCFG1_O_OSC_CONF ) &
         FCFG1_OSC_CONF_ADC_SH_MODE_EN_M ) >>
         FCFG1_OSC_CONF_ADC_SH_MODE_EN_S;
   }

   return ( getTrimForAdcShModeEnValue );
}

uint32_t SetupGetTrimForAdcShVbufEn( uint32_t ui32Fcfg1Revision ) {
   uint32_t getTrimForAdcShVbufEnValue = 1; // Recommended default setting

   if ( ui32Fcfg1Revision >= 0x00000022 ) {
      getTrimForAdcShVbufEnValue = ( HWREG( FCFG1_BASE + FCFG1_O_OSC_CONF ) &
         FCFG1_OSC_CONF_ADC_SH_VBUF_EN_M ) >>
         FCFG1_OSC_CONF_ADC_SH_VBUF_EN_S;
   }

   return ( getTrimForAdcShVbufEnValue );
}

uint32_t SetupGetTrimForXoscHfCtl( uint32_t ui32Fcfg1Revision ) {
   uint32_t getTrimForXoschfCtlValue = 0; // Recommended default setting
   uint32_t fcfg1Data;

   if ( ui32Fcfg1Revision >= 0x00000020 ) {
      fcfg1Data = HWREG( FCFG1_BASE + FCFG1_O_MISC_OTP_DATA_1 );
      getTrimForXoschfCtlValue =
         ( ( ( fcfg1Data & FCFG1_MISC_OTP_DATA_1_PEAK_DET_ITRIM_M ) >>
             FCFG1_MISC_OTP_DATA_1_PEAK_DET_ITRIM_S ) <<
           DDI_0_OSC_XOSCHFCTL_PEAK_DET_ITRIM_S);

      getTrimForXoschfCtlValue |=
         ( ( ( fcfg1Data & FCFG1_MISC_OTP_DATA_1_HP_BUF_ITRIM_M ) >>
             FCFG1_MISC_OTP_DATA_1_HP_BUF_ITRIM_S ) <<
           DDI_0_OSC_XOSCHFCTL_HP_BUF_ITRIM_S);

      getTrimForXoschfCtlValue |=
         ( ( ( fcfg1Data & FCFG1_MISC_OTP_DATA_1_LP_BUF_ITRIM_M ) >>
             FCFG1_MISC_OTP_DATA_1_LP_BUF_ITRIM_S ) <<
           DDI_0_OSC_XOSCHFCTL_LP_BUF_ITRIM_S);
   }

   return ( getTrimForXoschfCtlValue );
}

uint32_t SetupGetTrimForXoscHfFastStart( void ) {
   uint32_t ui32XoscHfFastStartValue   ;

   // Get value from FCFG1
   ui32XoscHfFastStartValue = ( HWREG( FCFG1_BASE + FCFG1_O_OSC_CONF ) &
      FCFG1_OSC_CONF_XOSC_HF_FAST_START_M ) >>
      FCFG1_OSC_CONF_XOSC_HF_FAST_START_S;

   return ( ui32XoscHfFastStartValue );
}

uint32_t SetupGetTrimForRadcExtCfg( uint32_t ui32Fcfg1Revision ) {
   uint32_t getTrimForRadcExtCfgValue = 0x403F8000; // Recommended default setting
   uint32_t fcfg1Data;

   if ( ui32Fcfg1Revision >= 0x00000020 ) {
      fcfg1Data = HWREG( FCFG1_BASE + FCFG1_O_MISC_OTP_DATA_1 );
      getTrimForRadcExtCfgValue =
         ( ( ( fcfg1Data & FCFG1_MISC_OTP_DATA_1_HPM_IBIAS_WAIT_CNT_M ) >>
             FCFG1_MISC_OTP_DATA_1_HPM_IBIAS_WAIT_CNT_S ) <<
           DDI_0_OSC_RADCEXTCFG_HPM_IBIAS_WAIT_CNT_S);

      getTrimForRadcExtCfgValue |=
         ( ( ( fcfg1Data & FCFG1_MISC_OTP_DATA_1_LPM_IBIAS_WAIT_CNT_M ) >>
             FCFG1_MISC_OTP_DATA_1_LPM_IBIAS_WAIT_CNT_S ) <<
           DDI_0_OSC_RADCEXTCFG_LPM_IBIAS_WAIT_CNT_S);

      getTrimForRadcExtCfgValue |=
         ( ( ( fcfg1Data & FCFG1_MISC_OTP_DATA_1_IDAC_STEP_M ) >>
             FCFG1_MISC_OTP_DATA_1_IDAC_STEP_S ) <<
           DDI_0_OSC_RADCEXTCFG_IDAC_STEP_S);
   }

   return ( getTrimForRadcExtCfgValue );
}

uint32_t SetupGetTrimForRcOscLfIBiasTrim( uint32_t ui32Fcfg1Revision ) {
   uint32_t trimForRcOscLfIBiasTrimValue = 0; // Default value

   if ( ui32Fcfg1Revision >= 0x00000022 ) {
      trimForRcOscLfIBiasTrimValue = ( HWREG( FCFG1_BASE + FCFG1_O_OSC_CONF ) &
         FCFG1_OSC_CONF_ATESTLF_RCOSCLF_IBIAS_TRIM_M ) >>
         FCFG1_OSC_CONF_ATESTLF_RCOSCLF_IBIAS_TRIM_S ;
   }

   return ( trimForRcOscLfIBiasTrimValue );
}

uint32_t SetupGetTrimForXoscLfRegulatorAndCmirrwrRatio( uint32_t ui32Fcfg1Revision ) {
   uint32_t trimForXoscLfRegulatorAndCmirrwrRatioValue = 0; // Default value for both fields

   if ( ui32Fcfg1Revision >= 0x00000022 ) {
      trimForXoscLfRegulatorAndCmirrwrRatioValue = ( HWREG( FCFG1_BASE + FCFG1_O_OSC_CONF ) &
         ( FCFG1_OSC_CONF_XOSCLF_REGULATOR_TRIM_M |
           FCFG1_OSC_CONF_XOSCLF_CMIRRWR_RATIO_M  )) >>
           FCFG1_OSC_CONF_XOSCLF_CMIRRWR_RATIO_S  ;
   }

   return ( trimForXoscLfRegulatorAndCmirrwrRatioValue );
}

void SetupSetCacheModeAccordingToCcfgSetting( void ) {
    //
    // - Make sure to enable aggressive VIMS clock gating for power optimization
    //   Only for PG2 devices.
    // - Enable cache prefetch enable as default setting
    //   (Slightly higher power consumption, but higher CPU performance)
    // - IF ( CCFG_..._DIS_GPRAM == 1 )
    //   then: Enable cache (set cache mode = 1), even if set by ROM boot code
    //         (This is done because it's not set by boot code when running inside
    //         a debugger supporting the Halt In Boot (HIB) functionality).
    //   else: Set MODE_GPRAM if not already set (see inline comments as well)
    //
    uint32_t vimsCtlMode0 ;

    while ( HWREGBITW( VIMS_BASE + VIMS_O_STAT, VIMS_STAT_MODE_CHANGING_BITN )) {
        // Do nothing - wait for an eventual ongoing mode change to complete.
        // (There should typically be no wait time here, but need to be sure)
    }

    //
    // Note that Mode=0 is equal to MODE_GPRAM
    //
    vimsCtlMode0 = (( HWREG( VIMS_BASE + VIMS_O_CTL ) & ~VIMS_CTL_MODE_M ) | VIMS_CTL_DYN_CG_EN_M | VIMS_CTL_PREF_EN_M );

#if defined( DO_NOT_ENABLE_CACHE_IN_TRIM_DEVICE )
    HWREG( VIMS_BASE + VIMS_O_CTL ) = vimsCtlMode0;
#else

    if ( HWREG( CCFG_BASE + CCFG_O_SIZE_AND_DIS_FLAGS ) & CCFG_SIZE_AND_DIS_FLAGS_DIS_GPRAM ) {
        // Enable cache (and hence disable GPRAM)
        HWREG( VIMS_BASE + VIMS_O_CTL ) = ( vimsCtlMode0 | VIMS_CTL_MODE_CACHE );
    } else if (( HWREG( VIMS_BASE + VIMS_O_STAT ) & VIMS_STAT_MODE_M ) != VIMS_STAT_MODE_GPRAM ) {
        //
        // GPRAM is enabled in CCFG but not selected
        // Note: It is recommended to go via MODE_OFF when switching to MODE_GPRAM
        //
        HWREG( VIMS_BASE + VIMS_O_CTL ) = ( vimsCtlMode0 | VIMS_CTL_MODE_OFF );
        while (( HWREG( VIMS_BASE + VIMS_O_STAT ) & VIMS_STAT_MODE_M ) != VIMS_STAT_MODE_OFF ) {
            // Do nothing - wait for an eventual mode change to complete (This goes fast).
        }
        HWREG( VIMS_BASE + VIMS_O_CTL ) = vimsCtlMode0;
    } else {
        // Correct mode, but make sure PREF_EN and DYN_CG_EN always are set
        HWREG( VIMS_BASE + VIMS_O_CTL ) = vimsCtlMode0;
    }
#endif
}

void SetupSetAonRtcSubSecInc( uint32_t subSecInc ) {
   //
   // Loading a new RTCSUBSECINC value is done in 5 steps:
   // 1. Write bit[15:0] of new SUBSECINC value to AUX_WUC_O_RTCSUBSECINC0
   // 2. Write bit[23:16] of new SUBSECINC value to AUX_WUC_O_RTCSUBSECINC1
   // 3. Set AUX_WUC_RTCSUBSECINCCTL_UPD_REQ
   // 4. Wait for AUX_WUC_RTCSUBSECINCCTL_UPD_ACK
   // 5. Clear AUX_WUC_RTCSUBSECINCCTL_UPD_REQ
   //
   HWREG( AUX_WUC_BASE + AUX_WUC_O_RTCSUBSECINC0 ) = (( subSecInc       ) & AUX_WUC_RTCSUBSECINC0_INC15_0_M  );
   HWREG( AUX_WUC_BASE + AUX_WUC_O_RTCSUBSECINC1 ) = (( subSecInc >> 16 ) & AUX_WUC_RTCSUBSECINC1_INC23_16_M );

   HWREG( AUX_WUC_BASE + AUX_WUC_O_RTCSUBSECINCCTL ) = AUX_WUC_RTCSUBSECINCCTL_UPD_REQ;
   while( ! ( HWREGBITW( AUX_WUC_BASE + AUX_WUC_O_RTCSUBSECINCCTL, AUX_WUC_RTCSUBSECINCCTL_UPD_ACK_BITN )));
   HWREG( AUX_WUC_BASE + AUX_WUC_O_RTCSUBSECINCCTL ) = 0;
}
