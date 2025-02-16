/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    subghz_phy_app.c
  * @author  MCD Application Team
  * @brief   Application of the SubGHz_Phy Middleware
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "platform.h"
#include "sys_app.h"
#include "subghz_phy_app.h"
#include "radio.h"

/* USER CODE BEGIN Includes */
//[JT]
#include "utilities_def.h"
#include "stm32_seq.h"
#include "stm32_timer.h"
#include "main.h"
#include "stdio.h"
/* USER CODE END Includes */

/* External variables ---------------------------------------------------------*/
/* USER CODE BEGIN EV */
//[JT]
#define RECEIVER
//#define TRANSMITTER
#define BUFFER_SIZE 	6
extern UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huart1;
/* USER CODE END EV */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
static UTIL_TIMER_Object_t timerTransmit;   //[JT]
static UTIL_TIMER_Object_t timerReceive;    //[JT]
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define TRANSMIT_PERIOD_MS 300  /* set Tx period of timer to 2 seconds */     //[JT]
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
#define TX_OUTPUT_POWER                             22       /* dBm */
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* Radio events function pointer */
static RadioEvents_t RadioEvents;

/* USER CODE BEGIN PV */
uint16_t BufferSize = BUFFER_SIZE;   //[JT]
uint8_t Buffer[BUFFER_SIZE];         //[JT]
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/*!
 * @brief Function to be executed on Radio Tx Done event
 */
static void OnTxDone(void);

/**
  * @brief Function to be executed on Radio Rx Done event
  * @param  payload ptr of buffer received
  * @param  size buffer size
  * @param  rssi
  * @param  LoraSnr_FskCfo
  */
static void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t LoraSnr_FskCfo);

/**
  * @brief Function executed on Radio Tx Timeout event
  */
static void OnTxTimeout(void);

/**
  * @brief Function executed on Radio Rx Timeout event
  */
static void OnRxTimeout(void);

/**
  * @brief Function executed on Radio Rx Error event
  */
static void OnRxError(void);

/* USER CODE BEGIN PFP */
static void Rx_Process(void);  //[JT]
static void TransmitPacket(void *context);    //[JT]
/* USER CODE END PFP */

/* Exported functions ---------------------------------------------------------*/
void SubghzApp_Init(void)
{
  /* USER CODE BEGIN SubghzApp_Init_1 */

  /* USER CODE END SubghzApp_Init_1 */

  /* Radio initialization */
  RadioEvents.TxDone = OnTxDone;
  RadioEvents.RxDone = OnRxDone;
  RadioEvents.TxTimeout = OnTxTimeout;
  RadioEvents.RxTimeout = OnRxTimeout;
  RadioEvents.RxError = OnRxError;

  Radio.Init(&RadioEvents);

  /* USER CODE BEGIN SubghzApp_Init_2 */
  //[JT]
#ifdef TRANSMITTER
  APP_LOG(TS_ON, VLEVEL_L, "******TRANSMITTER******\n\r");
  Radio.SetTxConfig(MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                      LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                      LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                      true, 0, 0, LORA_IQ_INVERSION_ON, TX_TIMEOUT_VALUE);
#endif
#ifdef RECEIVER
  APP_LOG(TS_ON, VLEVEL_L, "******RECEIVER******\n\r");
  Radio.SetRxConfig(MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
                    LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
                    LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
                    0, true, 0, 0, LORA_IQ_INVERSION_ON, true);
#endif

  Radio.SetMaxPayloadLength(MODEM_LORA, BUFFER_SIZE);
  //Radio.SetChannel(RF_FREQUENCY);
  Radio.SetChannel(914900000);


#ifdef TRANSMITTER
  Buffer[0] = 'S';
  Buffer[1] = 'T';
  Buffer[2] = 'M';
  Buffer[3] = '3';
  Buffer[4] = '2';
  Buffer[5] = 'W';
  Buffer[6] = 'L';
  Buffer[7] = '_';
  Buffer[8] = 'T';
  Buffer[9] = 'X';

  /* Add delay between TX and RX =
  time Busy_signal is ON in RX opening window */
  HAL_Delay(Radio.GetWakeupTime() + TCXO_WORKAROUND_TIME_MARGIN);

  UTIL_TIMER_Create(&timerTransmit, 0xFFFFFFFFU, UTIL_TIMER_ONESHOT, TransmitPacket, NULL);
  UTIL_TIMER_SetPeriod(&timerTransmit, TRANSMIT_PERIOD_MS);
  UTIL_TIMER_Start(&timerTransmit);  // start transmitting packets every 2s
#endif

#ifdef RECEIVER

  UTIL_SEQ_RegTask((1 << CFG_SEQ_Task_Rx_Process), UTIL_SEQ_RFU, Rx_Process);
  UTIL_SEQ_SetTask((1 << CFG_SEQ_Task_Rx_Process), CFG_SEQ_Prio_0);

#endif
  /* USER CODE END SubghzApp_Init_2 */
}

/* USER CODE BEGIN EF */
//[JT]
static void Rx_Process(void)
{
  APP_LOG(TS_OFF, VLEVEL_L, "\n\r");
  APP_LOG(TS_ON, VLEVEL_L, "Radio Rx\n\r");
  Radio.Rx(0);
}

static void TransmitPacket(void *context)
{
	uint8_t i =0;
	for(i=0; i<10;i++)
	{
		Buffer[i]++;
	}
  Radio.Send(Buffer, BufferSize);
}
/* USER CODE END EF */

/* Private functions ---------------------------------------------------------*/
static void OnTxDone(void)
{
  /* USER CODE BEGIN OnTxDone */
  //[JT]
  APP_LOG(TS_ON, VLEVEL_L, "OnTxDone\n\r");
  UTIL_TIMER_Start(&timerTransmit);  //Transmit packet in 2s
  /* USER CODE END OnTxDone */
}

static void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t LoraSnr_FskCfo)
{
  /* USER CODE BEGIN OnRxDone */
  //[JT]
	  char Calkowita[4];
	  char Ulamek[4];
	  uint8_t i;
	  // Send LoRa parametres via USB
  APP_LOG(TS_ON, VLEVEL_L, "OnRxDone\n\r");
  APP_LOG(TS_ON, VLEVEL_L,  "RssiValue=%d dBm, SnrValue=%d\n\r", rssi, LoraSnr_FskCfo);

  Radio.Sleep();// set LoRa module in sleep mode

  BufferSize = size;
  memcpy(Buffer, payload, BufferSize);

  	uint16_t RecievedData16bit;
  	// convert received data according to SHT30 datasheet
	RecievedData16bit=(Buffer[0]<<8 | Buffer[1]);
	float Temperatura = -45.0f + 175.0f *(( (float)RecievedData16bit)/((1<<16) -1));
	RecievedData16bit=(Buffer[3]<<8 | Buffer[4]);
	float Wilgotnosc = 100.0f*(float)(RecievedData16bit)/((1<<16) -1);

	uint16_t TempCalkowita = (uint16_t)Temperatura;
	uint16_t TempUlamek = (uint16_t)(Temperatura*100)%100;
	itoa(TempCalkowita,Calkowita,10);
	itoa(TempUlamek,Ulamek,10);
	// send received data to console
	  APP_LOG(TS_OFF, VLEVEL_L,  "Temperatura: %d.%d ^C \n\r", TempCalkowita,TempUlamek);
		uint16_t WilgCalkowita = (uint16_t)Wilgotnosc;
		uint16_t WilgUlamek = (uint16_t)(Wilgotnosc*100)%100;
		itoa(WilgCalkowita,Calkowita,10);
		itoa(WilgUlamek,Ulamek,10);
	  APP_LOG(TS_OFF, VLEVEL_L,  "Wilgotnosc: %d.%d % \n\r", WilgCalkowita,WilgUlamek);


  UTIL_SEQ_SetTask((1 << CFG_SEQ_Task_Rx_Process), CFG_SEQ_Prio_0);
  /* USER CODE END OnRxDone */
}

static void OnTxTimeout(void)
{
  /* USER CODE BEGIN OnTxTimeout */
  /* USER CODE END OnTxTimeout */
}

static void OnRxTimeout(void)
{
  /* USER CODE BEGIN OnRxTimeout */
  /* USER CODE END OnRxTimeout */
}

static void OnRxError(void)
{
  /* USER CODE BEGIN OnRxError */
  /* USER CODE END OnRxError */
}

/* USER CODE BEGIN PrFD */
/* USER CODE END PrFD */
