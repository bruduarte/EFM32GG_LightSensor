/***************************************************************************//**
 * @file
 * @brief LESENSE demo for EFM32GG_STK3700
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/



#include <stdint.h>
#include <stdbool.h>

#include "em_device.h"
#include "em_acmp.h"
#include "em_assert.h"
#include "em_chip.h"
#include "em_cmu.h"
#include "em_gpio.h"
#include "em_core.h"
#include "em_lcd.h"
#include "em_lesense.h"
#include "em_prs.h"
#include "segmentlcd.h"
#include "lightsense_conf.h"
#include "bsp_stk_buttons.h"

/***************************************************************************//**
 * Macro definitions
 ******************************************************************************/


#define LIGHTSENSE_EXCITE_PORT   gpioPortD
#define LIGHTSENSE_EXCITE_PIN    6U
#define LIGHTSENSE_SENSOR_PORT   gpioPortC
#define LIGHTSENSE_SENSOR_PIN    6U
#define LIGHTSENSE_BUTTON0_PORT  gpioPortB
#define LIGHTSENSE_BUTTON0_PIN   9U
#define LIGHTSENSE_BUTTON1_PORT  gpioPortB
#define LIGHTSENSE_BUTTON1_PIN   10U
#define LIGHTSENSE_BUTTON0_FLAG  (1 << LIGHTSENSE_BUTTON0_PIN)
#define LIGHTSENSE_BUTTON1_FLAG  (1 << LIGHTSENSE_BUTTON1_PIN)
#define LIGHT_PIN 				 0


/***************************************************************************//**
 * Datatypes
 ******************************************************************************/

enum Mode {

	Mode_manual,
	Mode_automatic

};
/***************************************************************************//**
 * Global variables
 ******************************************************************************/


static volatile enum Mode actual_mode = Mode_automatic;
static volatile bool trackButton1 = false;


/***************************************************************************//**
 * Prototypes
 ******************************************************************************/

void GPIO_ODD_IRQHandler(void);

void setupCMU(void);
void setupGPIO(void);
void setupACMP(void);
void setupLESENSE(void);
void setupPRS(void);

extern int BSP_LedSet(int ledNo);
extern int BSP_LedClear(int ledNo);
extern int BSP_LedsInit(void);
extern int BSP_LedToggle(int ledNo);
/***************************************************************************//**
 * @brief  Setup the CMU
 ******************************************************************************/
void setupCMU(void)
{
  /* Ensure core frequency has been updated */
  SystemCoreClockUpdate();

  /* Select clock source for HF clock. */
  CMU_ClockSelectSet(cmuClock_HF, cmuSelect_HFRCO);
  /* Select clock source for LFA clock. */
  CMU_ClockSelectSet(cmuClock_LFA, cmuSelect_LFRCO);
  /* Disable clock source for LFB clock. */
  CMU_ClockSelectSet(cmuClock_LFB, cmuSelect_Disabled);

  /* Enable HF peripheral clock. */
  CMU_ClockEnable(cmuClock_HFPER, true);
  /* Enable clock for GPIO. */
  CMU_ClockEnable(cmuClock_GPIO, true);
  /* Enable clock for LCD. */
  CMU_ClockEnable(cmuClock_LCD, true);
  /* Enable clock for ACMP0. */
  CMU_ClockEnable(cmuClock_ACMP0, true);
  /* Enable clock for PRS. */
  CMU_ClockEnable(cmuClock_PRS, true);
  /* Enable CORELE clock. */
  CMU_ClockEnable(cmuClock_CORELE, true);
  /* Enable clock for PCNT. */
  CMU_ClockEnable(cmuClock_PCNT0, true);
  /* Enable clock for LESENSE. */
  CMU_ClockEnable(cmuClock_LESENSE, true);
  /* Enable clock divider for LESENSE. */
  CMU_ClockDivSet(cmuClock_LESENSE, cmuClkDiv_1);

}

/***************************************************************************//**
 * @brief  Setup the GPIO
 ******************************************************************************/
void setupGPIO(void)
{
  /* Configure the drive strength of the ports for the light sensor. */
  GPIO_DriveModeSet(LIGHTSENSE_EXCITE_PORT, gpioDriveModeStandard);
  GPIO_DriveModeSet(LIGHTSENSE_SENSOR_PORT, gpioDriveModeStandard);

  /* Initialize the 2 GPIO pins of the light sensor setup. */
  GPIO_PinModeSet(LIGHTSENSE_EXCITE_PORT, LIGHTSENSE_EXCITE_PIN, gpioModePushPull, 0);
  GPIO_PinModeSet(LIGHTSENSE_SENSOR_PORT, LIGHTSENSE_SENSOR_PIN, gpioModeDisabled, 0); //The analog path, which is before the digital input path, remains active, i.e. a pin should be configured as GPIO disabled if you want to use it's analog input/output functionality).

  /* Enable push button 0 pin as input. */
  GPIO_PinModeSet(LIGHTSENSE_BUTTON0_PORT, LIGHTSENSE_BUTTON0_PIN, gpioModeInput, 0);
  /* Enable interrupts for that pin. */
  GPIO_IntConfig(LIGHTSENSE_BUTTON0_PORT, LIGHTSENSE_BUTTON0_PIN, false, true, true);
  /* Enable GPIO_ODD interrupt vector in NVIC. */
  NVIC_EnableIRQ(GPIO_ODD_IRQn);

  /* Enable push button 1 pin as input. */
  GPIO_PinModeSet(LIGHTSENSE_BUTTON1_PORT, LIGHTSENSE_BUTTON1_PIN, gpioModeInput, 0);
  /* Enable interrupts for that pin. */
  GPIO_IntConfig(LIGHTSENSE_BUTTON1_PORT, LIGHTSENSE_BUTTON1_PIN, false, true, true);
  /* Enable GPIO_EVEN interrupt vector in NVIC. */
  NVIC_EnableIRQ(GPIO_EVEN_IRQn);

}

/***************************************************************************//**
 * @brief  Setup the ACMP
 ******************************************************************************/
void setupACMP(void)
{
  /* ACMP configuration constant table. */
  static const ACMP_Init_TypeDef initACMP =
  {
    .fullBias = false,                 /* fullBias */
    .halfBias = true,                  /* halfBias */
    .biasProg =  0x0,                  /* biasProg */
    .interruptOnFallingEdge =  false,  /* interrupt on rising edge */
    .interruptOnRisingEdge =  false,   /* interrupt on falling edge */
    .warmTime = acmpWarmTime512,       /* 512 cycle warmup to be safe */
    .hysteresisLevel = acmpHysteresisLevel0, /* hysteresis level 0 */
    .inactiveValue = false,            /* inactive value */
    .lowPowerReferenceEnabled = false, /* low power reference */
    .vddLevel = 0x00,                  /* VDD level */
    .enable = false                    /* Don't request enabling. */
  };

  /* Configure ACMP. */
  ACMP_Init(ACMP0, &initACMP);
  /* Disable ACMP0 out to a pin. */
  ACMP_GPIOSetup(ACMP0, 0, false, false);
  /* Set up ACMP negSel to VDD, posSel is controlled by LESENSE. */
  ACMP_ChannelSet(ACMP0, acmpChannelVDD, acmpChannel0);
  /* LESENSE controls ACMP thus ACMP_Enable(ACMP0) should NOT be called in order
   * to ensure lower current consumption.*/


}

/***************************************************************************//**
 * @brief  Setup the LESENSE
 ******************************************************************************/
void setupLESENSE(void)
{
  /* LESENSE channel configuration constant table. */
  static const LESENSE_ChAll_TypeDef initChs = LESENSE_LIGHTSENSE_SCAN_CONF;
  /* LESENSE alternate excitation channel configuration constant table. */
  static const LESENSE_ConfAltEx_TypeDef initAltEx = LESENSE_LIGHTSENSE_ALTEX_CONF;
  /* LESENSE central configuration constant table. */
  static const LESENSE_Init_TypeDef initLESENSE =
  {
    .coreCtrl =
    {
      .scanStart = lesenseScanStartPeriodic,
      .prsSel = lesensePRSCh0,
      .scanConfSel = lesenseScanConfDirMap,
      .invACMP0 = false,
      .invACMP1 = false,
      .dualSample = false,
      .storeScanRes = false,
      .bufOverWr = true,
      .bufTrigLevel = lesenseBufTrigHalf,
      .wakeupOnDMA = lesenseDMAWakeUpDisable,
      .biasMode = lesenseBiasModeDutyCycle,
      .debugRun = false
    },

    .timeCtrl =
    {
      .startDelay = 0U
    },

    .perCtrl =
    {
      .dacCh0Data = lesenseDACIfData,
      .dacCh0ConvMode = lesenseDACConvModeDisable,
      .dacCh0OutMode = lesenseDACOutModeDisable,
      .dacCh1Data = lesenseDACIfData,
      .dacCh1ConvMode = lesenseDACConvModeDisable,
      .dacCh1OutMode = lesenseDACOutModeDisable,
      .dacPresc = 0U,
      .dacRef = lesenseDACRefBandGap,
      .acmp0Mode = lesenseACMPModeMuxThres,
      .acmp1Mode = lesenseACMPModeMuxThres,
      .warmupMode = lesenseWarmupModeNormal
    },

    .decCtrl =
    {
      .decInput = lesenseDecInputSensorSt,
      .initState = 0U,
      .chkState = false,
      .intMap = true,
      .hystPRS0 = false,
      .hystPRS1 = false,
      .hystPRS2 = false,
      .hystIRQ = false,
      .prsCount = true,
      .prsChSel0 = lesensePRSCh0,
      .prsChSel1 = lesensePRSCh1,
      .prsChSel2 = lesensePRSCh2,
      .prsChSel3 = lesensePRSCh3
    }
  };

  /* Initialize LESENSE interface with RESET. */
  LESENSE_Init(&initLESENSE, true);

  /* Configure scan channels. */
  LESENSE_ChannelAllConfig(&initChs);

  /* Configure alternate excitation channels. */
  LESENSE_AltExConfig(&initAltEx);

  /* Set scan frequency (in Hz). */
  (void)LESENSE_ScanFreqSet(0U, 20U);

  /* Set clock divisor for LF clock. */
  LESENSE_ClkDivSet(lesenseClkLF, lesenseClkDiv_1);

  /* Start scanning LESENSE channels. */
  LESENSE_ScanStart();
}

/***************************************************************************//**
 * @brief  Setup the PRS
 ******************************************************************************/
void setupPRS(void)
{
  /* Use PRS location 0 and output PRS channel 0 on GPIO PORTA0. */
  PRS->ROUTE = 0x01U;

  /* PRS channel 0 configuration. */
  PRS_SourceAsyncSignalSet(0U,
                           PRS_CH_CTRL_SOURCESEL_LESENSEL,
                           PRS_CH_CTRL_SIGSEL_LESENSESCANRES6);
}


/***************************************************************************//**
 * @brief  Main function
 ******************************************************************************/
int main(void)
{
  CORE_DECLARE_IRQ_STATE;

  /* Chip errata */
  CHIP_Init();

  /* Initialize LED driver */
  BSP_LedsInit();

  /* Disable interrupts */
  CORE_ENTER_ATOMIC();
  /* Setup CMU. */
  setupCMU();
  /* Setup GPIO. */
  setupGPIO();
  /* Setup ACMP. */
  setupACMP();
  /* Setup PRS. */
  setupPRS();
  /* Setup LESENSE. */
  setupLESENSE();


  /* Initialize segment LCD. */
  SegmentLCD_Init(false);


  /* Initialization done, enable interrupts globally. */
  CORE_EXIT_ATOMIC();



  /* Go to infinite loop. */
  while (1) {
	  uint32_t lastResult = LESENSE_ScanResultGet(); //returns the last sensor comparison against the threshold.
	  bool hasLight = (lastResult & LESENSE_IF_CH6) != 0; //check is the channel 6 bit is set (Light detector).

	  switch (actual_mode){
	  case Mode_automatic:
		  /*Changes the status of the LED based on the Light detector*/

		  if (!hasLight){
			  //BSP_LedSet(int ledNo)
			  BSP_LedSet(LIGHT_PIN);
		  }else {

			  BSP_LedClear(LIGHT_PIN);
		  }
		  SegmentLCD_Write("AUTO");
		  break;

	  case Mode_manual:
		  /*Changes the status of the LED based on the button*/
	  default:
		  SegmentLCD_Write("MANUAL");
		  if (trackButton1){
			  BSP_LedToggle(LIGHT_PIN);
			  trackButton1 = false;
		  }
		  break;
	  }



  }
}


/***************************************************************************//**
 * @brief  GPIO odd interrupt handler (for handling button events)
 ******************************************************************************/
void GPIO_ODD_IRQHandler(void)
{
  /* Clear interrupt flag */
  GPIO_IntClear(LIGHTSENSE_BUTTON0_FLAG);

  /* Change the mode */
  if (actual_mode == Mode_automatic){
	  actual_mode = Mode_manual;
  }
  else if (actual_mode == Mode_manual){
	  actual_mode = Mode_automatic;
  }
}

/***************************************************************************//**
 * @brief  GPIO even interrupt handler (for handling button events)
 ******************************************************************************/
void GPIO_EVEN_IRQHandler(void)
{
  /* Clear interrupt flag */
  GPIO_IntClear(LIGHTSENSE_BUTTON1_FLAG);

  trackButton1 = true;
}
