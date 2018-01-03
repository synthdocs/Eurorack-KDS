/* ###################################################################
**     This component module is generated by Processor Expert. Do not modify it.
**     Filename    : ADMAIN.c
**     Project     : Flask
**     Processor   : MK22FN256VLH12
**     Component   : ADC
**     Version     : Component 01.697, Driver 01.00, CPU db: 3.00.000
**     Repository  : Kinetis
**     Compiler    : GNU C Compiler
**     Date/Time   : 2017-11-17, 00:46, # CodeGen: 0
**     Abstract    :
**         This device "ADC" implements an A/D converter,
**         its control methods and interrupt/event handling procedure.
**     Settings    :
**          Component name                                 : ADMAIN
**          A/D converter                                  : ADC0
**          Sharing                                        : Disabled
**          ADC_LDD                                        : ADC_LDD
**          Interrupt service/event                        : Enabled
**            A/D interrupt                                : INT_ADC0
**            A/D interrupt priority                       : medium priority
**          A/D channels                                   : 2
**            Channel0                                     : 
**              A/D channel (pin)                          : ADC0_SE6b/PTD5/SPI0_PCS2/UART0_CTS_b/FTM0_CH5/EWM_OUT_b/SPI1_SCK
**              Mode select                                : Single Ended
**            Channel1                                     : 
**              A/D channel (pin)                          : ADC0_SE7b/PTD6/LLWU_P15/SPI0_PCS3/UART0_RX/FTM0_CH6/FTM0_FLT0/SPI1_SOUT
**              Mode select                                : Single Ended
**          A/D resolution                                 : 16 bits
**          Conversion time                                : 6.25 �s
**          Low-power mode                                 : Disabled
**          High-speed conversion mode                     : Disabled
**          Asynchro clock output                          : Disabled
**          Sample time                                    : 0 = short
**          Internal trigger                               : Disabled
**          Number of conversions                          : 4
**          Initialization                                 : 
**            Enabled in init. code                        : yes
**            Events enabled in init.                      : yes
**          CPU clock/speed selection                      : 
**            High speed mode                              : This component enabled
**            Low speed mode                               : This component disabled
**            Slow speed mode                              : This component disabled
**     Contents    :
**         Measure    - byte ADMAIN_Measure(bool WaitForResult);
**         GetValue16 - byte ADMAIN_GetValue16(word *Values);
**         Calibrate  - byte ADMAIN_Calibrate(bool WaitForResult);
**
**     Copyright : 1997 - 2015 Freescale Semiconductor, Inc. 
**     All Rights Reserved.
**     
**     Redistribution and use in source and binary forms, with or without modification,
**     are permitted provided that the following conditions are met:
**     
**     o Redistributions of source code must retain the above copyright notice, this list
**       of conditions and the following disclaimer.
**     
**     o Redistributions in binary form must reproduce the above copyright notice, this
**       list of conditions and the following disclaimer in the documentation and/or
**       other materials provided with the distribution.
**     
**     o Neither the name of Freescale Semiconductor, Inc. nor the names of its
**       contributors may be used to endorse or promote products derived from this
**       software without specific prior written permission.
**     
**     THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
**     ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
**     WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
**     DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
**     ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
**     (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
**     LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
**     ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
**     (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
**     SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**     
**     http: www.freescale.com
**     mail: support@freescale.com
** ###################################################################*/
/*!
** @file ADMAIN.c
** @version 01.00
** @brief
**         This device "ADC" implements an A/D converter,
**         its control methods and interrupt/event handling procedure.
*/         
/*!
**  @addtogroup ADMAIN_module ADMAIN module documentation
**  @{
*/         

/* MODULE ADMAIN. */

#include "Events.h"
#include "ADMAIN.h"

#ifdef __cplusplus
extern "C" {
#endif 
#define STOP            0x00U          /* STOP state           */
#define MEASURE         0x01U          /* MESURE state         */
#define CONTINUOUS      0x02U          /* CONTINUOUS state     */
#define SINGLE          0x03U          /* SINGLE state         */
#define CALIBRATING     0x04U          /* CALIBRATING state    */

static volatile byte SumChan;          /* Counter of measured channels */
static volatile byte ModeFlg;          /* Current state of device */
static volatile byte SumCnt;           /* Counter of conversions */
LDD_TDeviceData *AdcLdd2_DeviceDataPtr; /* Device data pointer */
/* Sample group configuration */
static LDD_ADC_TSample SampleGroup[ADMAIN_SAMPLE_GROUP_SIZE];
/* Measure multiple channels flags  */
/* Temporary buffer for converting results */
volatile dword ADMAIN_OutV[ADMAIN_SAMPLE_GROUP_SIZE]; /* Sum of measured values */
/* Calibration in progress flag */
static volatile bool OutFlg;           /* Measurement finish flag */

/*
** ===================================================================
**     Method      :  ClrSumV (component ADC)
**
**     Description :
**         The method clears the internal buffers used to store sum of a 
**         number of last conversions.
**         This method is internal. It is used by Processor Expert only.
** ===================================================================
*/
static void ClrSumV(void)
{
  ADMAIN_OutV[0] = 0U;                 /* Set variable for storing measured values to 0 */
  ADMAIN_OutV[1] = 0U;                 /* Set variable for storing measured values to 0 */
}

/*
** ===================================================================
**     Method      :  ADMAIN_HWEnDi (component ADC)
**
**     Description :
**         Enables or disables the peripheral(s) associated with the 
**         component. The method is called automatically as a part of the 
**         Enable and Disable methods and several internal methods.
**         This method is internal. It is used by Processor Expert only.
** ===================================================================
*/
void ADMAIN_HWEnDi(void)
{
  if (ModeFlg) {                       /* Start or stop measurement? */
    SumCnt = 0U;                       /* Set counter of conversions to 0 */
    OutFlg = FALSE;                    /* Output values aren't available */
    SumChan = 0U;                      /* Set the counter of measured channels to 0 */
    ClrSumV();                         /* Clear measured values */
    SampleGroup[0].ChannelIdx = 0U;
    (void)AdcLdd2_CreateSampleGroup(AdcLdd2_DeviceDataPtr, (LDD_ADC_TSample *)SampleGroup, 1U); /* Configure sample group */
    (void)AdcLdd2_StartSingleMeasurement(AdcLdd2_DeviceDataPtr);
  }
}

/*
** ===================================================================
**     Method      :  ADMAIN_Measure (component ADC)
*/
/*!
**     @brief
**         This method performs one measurement on all channels that
**         are set in the component inspector. (Note: If the [number of
**         conversions] is more than one the conversion of A/D channels
**         is performed specified number of times.)
**     @param
**         WaitForResult   - Wait for a result of a
**                           conversion. If [interrupt service] is
**                           disabled, A/D peripheral doesn't support
**                           measuring all channels at once or Autoscan
**                           mode property isn't enabled and at the same
**                           time the [number of channels] is greater
**                           than 1, then the WaitForResult parameter is
**                           ignored and the method waits for each
**                           result every time. If the [interrupt
**                           service] is disabled and a [number of
**                           conversions] is greater than 1, the
**                           parameter is ignored and the method also
**                           waits for each result every time.
**     @return
**                         - Error code, possible codes:
**                           ERR_OK - OK
**                           ERR_SPEED - This device does not work in
**                           the active speed mode
**                           ERR_DISABLED - Device is disabled
**                           ERR_BUSY - A conversion is already running
*/
/* ===================================================================*/
byte ADMAIN_Measure(bool WaitForResult)
{
  if (ModeFlg != STOP) {               /* Is the device in different mode than "stop"? */
    return ERR_BUSY;                   /* If yes then error */
  }
  ModeFlg = MEASURE;                   /* Set state of device to the measure mode */
  ADMAIN_HWEnDi();                     /* Enable the device */
  if (WaitForResult) {                 /* Is WaitForResult TRUE? */
    while (ModeFlg == MEASURE) {}      /* If yes then wait for end of measurement */
  }
  return ERR_OK;                       /* OK */
}

/*
** ===================================================================
**     Method      :  ADMAIN_GetValue16 (component ADC)
*/
/*!
**     @brief
**         This method returns the last measured values of all channels.
**         Compared with [GetValue] method this method returns more
**         accurate result if the [number of conversions] is greater
**         than 1 and [AD resolution] is less than 16 bits. In addition,
**         the user code dependency on [AD resolution] is eliminated.
**     @param
**         Values          - Pointer to the array that contains
**                           the measured data.
**     @return
**                         - Error code, possible codes:
**                           ERR_OK - OK
**                           ERR_SPEED - This device does not work in
**                           the active speed mode
**                           ERR_NOTAVAIL - Requested value not
**                           available
**                           ERR_OVERRUN - External trigger overrun flag
**                           was detected after the last value(s) was
**                           obtained (for example by GetValue). This
**                           error may not be supported on some CPUs
**                           (see generated code).
*/
/* ===================================================================*/
byte ADMAIN_GetValue16(word *Values)
{
  if (!OutFlg) {                       /* Is output flag set? */
    return ERR_NOTAVAIL;               /* If no then error */
  }
  Values[0] = (word)((word)(ADMAIN_OutV[0] >> 2U)); /* Save measured values to the output buffer */
  Values[1] = (word)((word)(ADMAIN_OutV[1] >> 2U)); /* Save measured values to the output buffer */
  return ERR_OK;                       /* OK */
}

/*
** ===================================================================
**     Method      :  ADMAIN_Calibrate (component ADC)
**     Description :
**         This method starts self calibration process. Calibration is
**         typically used to remove the effects of the gain and offset
**         from a specific reading.
**     Parameters  :
**         NAME            - DESCRIPTION
**         WaitForResult   - Wait for a result of
**                           calibration. If the <interrupt service> is
**                           disabled, the WaitForResult parameter is
**                           ignored and the method waits for
**                           calibration result every time.
**     Returns     :
**         ---             - Error code
**                           ERR_OK - OK
**                           ERR_BUSY - A conversion is already running
**                           ERR_SPEED - This device does not work in
**                           the active speed mode
**                           ERR_DISABLED - Device is disabled
**                           ERR_FAILED - Calibration hasn't been
**                           finished correctly
** ===================================================================
*/
byte ADMAIN_Calibrate(bool WaitForResult)
{
  if (ModeFlg != STOP) {               /* Is the device in different mode than "stop"? */
    return ERR_BUSY;                   /* If yes then error */
  }
  ModeFlg = CALIBRATING;               /* Set state of device to the calibration mode */
  (void)AdcLdd2_GetMeasurementCompleteStatus(AdcLdd2_DeviceDataPtr); /* Clear measurement complete status */
  (void)AdcLdd2_StartCalibration(AdcLdd2_DeviceDataPtr); /* Start calibration */
  if (!WaitForResult) {                /* If doesn't wait for result */
    return ERR_OK;                     /* then return ERR_OK, but user have to check the result of calibration e.g. by GetCalibrationStatus method */
  }
  while (!AdcLdd2_GetMeasurementCompleteStatus(AdcLdd2_DeviceDataPtr)) {}; /* Wait until calibration ends */
  if (AdcLdd2_GetCalibrationResultStatus(AdcLdd2_DeviceDataPtr) != ERR_OK) { /* If calibration failed flag is set */
    ModeFlg = STOP;                    /* Set the device to the stop mode */
    return ERR_FAILED;                 /* Return ERR_FAILED error code */
  }
  return ERR_OK;                       /* ADC device is now calibrated */
}

/*
** ===================================================================
**     Method      :  ADMAIN_OnMeasurementComplete (component ADC)
**
**     Description :
**         The method services the conversion complete interrupt of the 
**         selected peripheral(s) and eventually invokes the beans 
**         event(s).
**         This method is internal. It is used by Processor Expert only.
** ===================================================================
*/
void AdcLdd2_OnMeasurementComplete(LDD_TUserData *UserDataPtr)
{
  AdcLdd2_TResultData ResultData;      /* Temporary result data */
    
  (void)UserDataPtr;                   /* Parameter is not used, suppress unused argument warning */
  if (ModeFlg == CALIBRATING) {        /* If the driver is in CALIBRATING mode */
    (void)AdcLdd2_GetCalibrationResultStatus(AdcLdd2_DeviceDataPtr);
    ModeFlg = STOP;                    /* Set the device to the stop mode */
    ADMAIN_OnCalibrationEnd();         /* If yes then invoke user event */
    return;                            /* Return from interrupt */
  }
  AdcLdd2_GetMeasuredValues(AdcLdd2_DeviceDataPtr, (LDD_TData *)&ResultData);
  ADMAIN_OutV[SumChan] += ResultData;
  SumChan++;                           /* Increase counter of measured channels*/
  if (SumChan == 2U) {                 /* Is number of measured channels equal to the number of channels used in the component? */
    SumChan = 0U;                      /* If yes then set the counter of measured channels to 0 */
    SumCnt++;                          /* Increase counter of conversions*/
    if (SumCnt == 4U) {                /* Is number of conversions on each channel equal to the number of conversions defined in the component? */
      OutFlg = TRUE;                   /* Measured values are available */
      ADMAIN_OnEnd();                  /* If yes then invoke user event */
      ModeFlg = STOP;                  /* Set the device to the stop mode */
      return;                          /* Return from interrupt */
    }
  }
  SampleGroup[0].ChannelIdx = SumChan; /* Start measurement of next channel */
  (void)AdcLdd2_CreateSampleGroup(AdcLdd2_DeviceDataPtr, (LDD_ADC_TSample *)SampleGroup, 1U); /* Configure sample group */
  (void)AdcLdd2_StartSingleMeasurement(AdcLdd2_DeviceDataPtr);
}

/*
** ===================================================================
**     Method      :  ADMAIN_Init (component ADC)
**
**     Description :
**         Initializes the associated peripheral(s) and the component 
**         internal variables. The method is called automatically as a 
**         part of the application initialization code.
**         This method is internal. It is used by Processor Expert only.
** ===================================================================
*/
void ADMAIN_Init(void)
{
  OutFlg = FALSE;                      /* No measured value */
  ModeFlg = STOP;                      /* Device isn't running */
  AdcLdd2_DeviceDataPtr = AdcLdd2_Init(NULL); /* Calling init method of the inherited component */
}

/* END ADMAIN. */

#ifdef __cplusplus
}  /* extern "C" */
#endif 

/*!
** @}
*/
/*
** ###################################################################
**
**     This file was created by Processor Expert 10.5 [05.21]
**     for the Freescale Kinetis series of microcontrollers.
**
** ###################################################################
*/
