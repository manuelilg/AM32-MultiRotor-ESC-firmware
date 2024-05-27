/*
 * peripherals.c
 *
 *  Created on: Sep. 26, 2020
 *      Author: Alka
 */


// PERIPHERAL SETUP

#include "peripherals.h"
#include "targets.h"
#include "serial_telemetry.h"

#if defined(USE_USART_TX)
extern uint8_t txBuffer[3];
#endif
#if defined(USE_USART_RX)
extern uint8_t rxBuffer[6];
#endif

void initCorePeripherals(void) {
	LL_APB1_GRP2_EnableClock(LL_APB1_GRP2_PERIPH_SYSCFG);
	LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);
	SystemClock_Config();

	FLASH->ACR |= FLASH_ACR_PRFTBE;   //// prefetch buffer enable
	UN_GPIO_Init();
	MX_DMA_Init();
	MX_TIM14_Init();
	MX_TIM1_Init();
	MX_TIM_Interval_Init();
	MX_TIM17_Init();
	TEN_KHZ_Timer_Init();
	UN_TIM_Init();
#ifdef USE_SERIAL_TELEMETRY
	telem_UART_Init();
#endif
}

void initAfterJump() {
	volatile uint32_t *VectorTable = (volatile uint32_t *)0x20000000;
	uint32_t vector_index = 0;
	for(vector_index  = 0; vector_index  < 48; vector_index++)
	{
	  VectorTable[vector_index ] = *(__IO uint32_t*)(APPLICATION_ADDRESS + (vector_index <<2));  // no VTOR on cortex-MO so need to copy vector table
	}
	//	  /* Enable the SYSCFG peripheral clock*/
	do {
		volatile uint32_t tmpreg;
		((((RCC_TypeDef *) ((((uint32_t)0x40000000U) + 0x00020000) + 0x00001000))->APB2ENR) |= ((0x1U << (0U))));
		/* Delay after an RCC peripheral clock enabling */
		tmpreg = ((((RCC_TypeDef *) ((((uint32_t)0x40000000U) + 0x00020000) + 0x00001000))->APB2ENR) & ((0x1U << (0U))));
		((void)(tmpreg));
	  } while(0U);
	//	  /* Remap SRAM at 0x00000000 */
	do {((SYSCFG_TypeDef *) (((uint32_t)0x40000000U) + 0x00010000))->CFGR1 &= ~((0x3U << (0U)));
			 ((SYSCFG_TypeDef *) (((uint32_t)0x40000000U) + 0x00010000))->CFGR1 |= ((0x1U << (0U)) | (0x2U << (0U)));
			}while(0);

	if (SysTick_Config(SystemCoreClock / 1000))
	{
		/* Capture error */
		while (1)
		{
		}
	}
	__enable_irq();
}

void SystemClock_Config(void) {
	LL_FLASH_SetLatency(LL_FLASH_LATENCY_1);

	if(LL_FLASH_GetLatency() != LL_FLASH_LATENCY_1)
	{
		// Error_Handler();
	}
	LL_RCC_HSI_Enable();

	 /* Wait till HSI is ready */
	while(LL_RCC_HSI_IsReady() != 1)
	{

	}
	LL_RCC_HSI_SetCalibTrimming(16);
	LL_RCC_HSI14_Enable();

	 /* Wait till HSI14 is ready */
	while(LL_RCC_HSI14_IsReady() != 1)
	{

	}
	LL_RCC_HSI14_SetCalibTrimming(16);
	LL_RCC_LSI_Enable();

	 /* Wait till LSI is ready */
	while(LL_RCC_LSI_IsReady() != 1)
	{

	}
	LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSI_DIV_2, LL_RCC_PLL_MUL_12);
	LL_RCC_PLL_Enable();

	 /* Wait till PLL is ready */
	while(LL_RCC_PLL_IsReady() != 1)
	{

	}
	LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
	LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);
	LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);

	 /* Wait till System clock is ready */
	while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL)
	{

	}
	LL_Init1msTick(48000000);
	LL_SetSystemCoreClock(48000000);
	LL_RCC_HSI14_EnableADCControl();
}

void MX_IWDG_Init(void) {
	LL_IWDG_Enable(IWDG);
	LL_IWDG_EnableWriteAccess(IWDG);
	LL_IWDG_SetPrescaler(IWDG, LL_IWDG_PRESCALER_8);
	LL_IWDG_SetReloadCounter(IWDG, 4095);
	while (LL_IWDG_IsReady(IWDG) != 1)
	{
	}

	LL_IWDG_SetWindow(IWDG, 4095);
	LL_IWDG_ReloadCounter(IWDG);
}

void MX_TIM1_Init(void) {
	LL_TIM_InitTypeDef TIM_InitStruct = {0};
	LL_TIM_OC_InitTypeDef TIM_OC_InitStruct = {0};
	LL_TIM_BDTR_InitTypeDef TIM_BDTRInitStruct = {0};

	LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

	/* Peripheral clock enable */
	LL_APB1_GRP2_EnableClock(LL_APB1_GRP2_PERIPH_TIM1);

	TIM_InitStruct.Prescaler = 0;
	TIM_InitStruct.CounterMode = LL_TIM_COUNTERMODE_UP;
	TIM_InitStruct.Autoreload = 1999;
	TIM_InitStruct.ClockDivision = LL_TIM_CLOCKDIVISION_DIV1;
	TIM_InitStruct.RepetitionCounter = 0;
	LL_TIM_Init(TIM1, &TIM_InitStruct);
	LL_TIM_EnableARRPreload(TIM1);
	LL_TIM_OC_EnablePreload(TIM1, LL_TIM_CHANNEL_CH1);
	TIM_OC_InitStruct.OCMode = LL_TIM_OCMODE_PWM1;
	TIM_OC_InitStruct.OCState = LL_TIM_OCSTATE_DISABLE;
	TIM_OC_InitStruct.OCNState = LL_TIM_OCSTATE_DISABLE;
	TIM_OC_InitStruct.CompareValue = 0;
	TIM_OC_InitStruct.OCPolarity = LL_TIM_OCPOLARITY_HIGH;
	TIM_OC_InitStruct.OCNPolarity = LL_TIM_OCPOLARITY_HIGH;
	TIM_OC_InitStruct.OCIdleState = LL_TIM_OCIDLESTATE_LOW;
	TIM_OC_InitStruct.OCNIdleState = LL_TIM_OCIDLESTATE_LOW;
	LL_TIM_OC_Init(TIM1, LL_TIM_CHANNEL_CH1, &TIM_OC_InitStruct);
	LL_TIM_OC_DisableFast(TIM1, LL_TIM_CHANNEL_CH1);
	LL_TIM_OC_EnablePreload(TIM1, LL_TIM_CHANNEL_CH2);
	TIM_OC_InitStruct.OCState = LL_TIM_OCSTATE_DISABLE;
	TIM_OC_InitStruct.OCNState = LL_TIM_OCSTATE_DISABLE;
	LL_TIM_OC_Init(TIM1, LL_TIM_CHANNEL_CH2, &TIM_OC_InitStruct);
	LL_TIM_OC_DisableFast(TIM1, LL_TIM_CHANNEL_CH2);
	LL_TIM_OC_EnablePreload(TIM1, LL_TIM_CHANNEL_CH3);
	TIM_OC_InitStruct.OCState = LL_TIM_OCSTATE_DISABLE;
	TIM_OC_InitStruct.OCNState = LL_TIM_OCSTATE_DISABLE;
	LL_TIM_OC_Init(TIM1, LL_TIM_CHANNEL_CH3, &TIM_OC_InitStruct);
	LL_TIM_OC_DisableFast(TIM1, LL_TIM_CHANNEL_CH3);
	LL_TIM_OC_EnablePreload(TIM1, LL_TIM_CHANNEL_CH4);
	TIM_OC_InitStruct.OCState = LL_TIM_OCSTATE_DISABLE;
	TIM_OC_InitStruct.OCNState = LL_TIM_OCSTATE_DISABLE;
	LL_TIM_OC_Init(TIM1, LL_TIM_CHANNEL_CH4, &TIM_OC_InitStruct);
	LL_TIM_OC_DisableFast(TIM1, LL_TIM_CHANNEL_CH4);
	LL_TIM_SetTriggerOutput(TIM1, LL_TIM_TRGO_RESET);
	LL_TIM_DisableMasterSlaveMode(TIM1);
	TIM_BDTRInitStruct.OSSRState = LL_TIM_OSSR_DISABLE;
	TIM_BDTRInitStruct.OSSIState = LL_TIM_OSSI_DISABLE;
	TIM_BDTRInitStruct.LockLevel = LL_TIM_LOCKLEVEL_OFF;
	TIM_BDTRInitStruct.DeadTime = DEAD_TIME;
	TIM_BDTRInitStruct.BreakState = LL_TIM_BREAK_DISABLE;
	TIM_BDTRInitStruct.BreakPolarity = LL_TIM_BREAK_POLARITY_HIGH;
	TIM_BDTRInitStruct.AutomaticOutput = LL_TIM_AUTOMATICOUTPUT_DISABLE;
	LL_TIM_BDTR_Init(TIM1, &TIM_BDTRInitStruct);
	/* USER CODE BEGIN TIM1_Init 2 */

	/* USER CODE END TIM1_Init 2 */
	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);
	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
	/**TIM1 GPIO Configuration
	PB13   ------> TIM1_CH1N
	PB14   ------> TIM1_CH2N
	PB15   ------> TIM1_CH3N
	PA8   ------> TIM1_CH1
	PA9   ------> TIM1_CH2
	PA10   ------> TIM1_CH3
	*/
	GPIO_InitStruct.Pin = LL_GPIO_PIN_13;
	GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
	GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
	GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
	GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
	GPIO_InitStruct.Alternate = LL_GPIO_AF_2;
	LL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = LL_GPIO_PIN_14;
	GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
	GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
	GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
	GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
	GPIO_InitStruct.Alternate = LL_GPIO_AF_2;
	LL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = LL_GPIO_PIN_15;
	GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
	GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
	GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
	GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
	GPIO_InitStruct.Alternate = LL_GPIO_AF_2;
	LL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = LL_GPIO_PIN_8;
	GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
	GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
	GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
	GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
	GPIO_InitStruct.Alternate = LL_GPIO_AF_2;
	LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = LL_GPIO_PIN_9;
	GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
	GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
	GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
	GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
	GPIO_InitStruct.Alternate = LL_GPIO_AF_2;
	LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = LL_GPIO_PIN_10;
	GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
	GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
	GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
	GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
	GPIO_InitStruct.Alternate = LL_GPIO_AF_2;
	LL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

void MX_TIM_Interval_Init(void) {
	// INTERVAL_TIMER
	LL_TIM_InitTypeDef TIM_InitStruct = {0};
	if (INTERVAL_TIMER == TIM2) {
	  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM2);
	}
	else {
	  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM3);
	}

	TIM_InitStruct.Prescaler = 23;
	TIM_InitStruct.CounterMode = LL_TIM_COUNTERMODE_UP;
	TIM_InitStruct.Autoreload = 65535;
	TIM_InitStruct.ClockDivision = LL_TIM_CLOCKDIVISION_DIV1;
	LL_TIM_Init(INTERVAL_TIMER, &TIM_InitStruct);
	LL_TIM_DisableARRPreload(INTERVAL_TIMER);
	LL_TIM_SetClockSource(INTERVAL_TIMER, LL_TIM_CLOCKSOURCE_INTERNAL);
	LL_TIM_SetTriggerOutput(INTERVAL_TIMER, LL_TIM_TRGO_RESET);
	LL_TIM_DisableMasterSlaveMode(INTERVAL_TIMER);
}

void MX_TIM14_Init(void) {
	LL_TIM_InitTypeDef TIM_InitStruct = {0};

	/* Peripheral clock enable */
	LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM14);

	/* TIM14 interrupt Init */
	NVIC_SetPriority(TIM14_IRQn, 0);
	NVIC_EnableIRQ(TIM14_IRQn);
	TIM_InitStruct.Prescaler = 23;
	TIM_InitStruct.CounterMode = LL_TIM_COUNTERMODE_UP;
	TIM_InitStruct.Autoreload = 0xFFFF;
	TIM_InitStruct.ClockDivision = LL_TIM_CLOCKDIVISION_DIV1;
	LL_TIM_Init(TIM14, &TIM_InitStruct);
	LL_TIM_DisableARRPreload(TIM14);
}

void MX_TIM17_Init(void) {
	LL_TIM_InitTypeDef TIM_InitStruct = {0};

	/* Peripheral clock enable */
	LL_APB1_GRP2_EnableClock(LL_APB1_GRP2_PERIPH_TIM17);
	TIM_InitStruct.Prescaler = 47;
	TIM_InitStruct.CounterMode = LL_TIM_COUNTERMODE_UP;
	TIM_InitStruct.Autoreload = 0xFFFF;
	TIM_InitStruct.ClockDivision = LL_TIM_CLOCKDIVISION_DIV1;
	TIM_InitStruct.RepetitionCounter = 0;
	LL_TIM_Init(TIM17, &TIM_InitStruct);
	LL_TIM_DisableARRPreload(TIM17);
}

/**
  * Enable DMA controller clock
  */
void MX_DMA_Init(void) {
	/* Init with LL driver */
	/* DMA controller clock enable */
	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA1);

	/* DMA interrupt init */
	/* DMA1_Channel2_3_IRQn interrupt configuration */
	// Interrupt configuration for ADC DMA channel done in ADC configuration
	/* DMA1_Channel4_5_IRQn interrupt configuration */
	NVIC_SetPriority(IC_DMA_IRQ_NAME, 1);
	NVIC_EnableIRQ(IC_DMA_IRQ_NAME);
}

void TEN_KHZ_Timer_Init() {
#ifdef USE_TIMER_16
	  LL_TIM_InitTypeDef TIM_InitStruct = {0};
	  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM2);
	  /* Peripheral clock enable */
	  TIM_InitStruct.Prescaler = 47;
	  TIM_InitStruct.CounterMode = LL_TIM_COUNTERMODE_UP;
	  TIM_InitStruct.Autoreload = 100;
	  TIM_InitStruct.ClockDivision = LL_TIM_CLOCKDIVISION_DIV1;
	  TIM_InitStruct.RepetitionCounter = 0;
	  LL_TIM_Init(TIM2, &TIM_InitStruct);
	  LL_TIM_EnableARRPreload(TIM2);
	  NVIC_SetPriority(TIM2_IRQn, 2);
	  NVIC_EnableIRQ(TIM2_IRQn);
#else
	  LL_TIM_InitTypeDef TIM_InitStruct = {0};
	  /* Peripheral clock enable */
	  LL_APB1_GRP2_EnableClock(LL_APB1_GRP2_PERIPH_TIM16);
	  TIM_InitStruct.Prescaler = 47;
	  TIM_InitStruct.CounterMode = LL_TIM_COUNTERMODE_UP;
	  TIM_InitStruct.Autoreload = 100;
	  TIM_InitStruct.ClockDivision = LL_TIM_CLOCKDIVISION_DIV1;
	  TIM_InitStruct.RepetitionCounter = 0;
	  LL_TIM_Init(TIM16, &TIM_InitStruct);
	  LL_TIM_EnableARRPreload(TIM16);
	  NVIC_SetPriority(TIM16_IRQn, 2);
	  NVIC_EnableIRQ(TIM16_IRQn);
#endif
}

void UN_TIM_Init(void) {
	LL_TIM_InitTypeDef TIM_InitStruct = {0};
	LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

#if defined(USE_TIMER_16)
	LL_APB1_GRP2_EnableClock(LL_APB1_GRP2_PERIPH_TIM16);
	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
	/**TIM16 GPIO Configuration
	PA6   ------> TIM16_CH1
	*/
	GPIO_InitStruct.Pin = LL_GPIO_PIN_6;
	GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
	GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
	GPIO_InitStruct.Pull = LL_GPIO_PULL_UP;
	GPIO_InitStruct.Alternate = LL_GPIO_AF_5;
	LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

#elif defined(USE_TIMER_2_CHANNEL_3)

	LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM2);
	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);

	GPIO_InitStruct.Pin = INPUT_PIN;
	GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
	GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
	GPIO_InitStruct.Pull = LL_GPIO_PULL_UP;
	GPIO_InitStruct.Alternate = LL_GPIO_AF_2;
	LL_GPIO_Init(INPUT_PIN_PORT, &GPIO_InitStruct);

#elif defined(USE_TIMER_3_CHANNEL_1)

	LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM3);
	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);

	GPIO_InitStruct.Pin = INPUT_PIN;
	GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
	GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
	GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
	GPIO_InitStruct.Pull = LL_GPIO_PULL_UP;
	GPIO_InitStruct.Alternate = LL_GPIO_AF_1;
	LL_GPIO_Init(INPUT_PIN_PORT, &GPIO_InitStruct);

#endif

	/* TIM Input DMA Init */
	LL_DMA_SetDataTransferDirection(DMA1, INPUT_DMA_CHANNEL, LL_DMA_DIRECTION_PERIPH_TO_MEMORY);
	LL_DMA_SetChannelPriorityLevel(DMA1, INPUT_DMA_CHANNEL, LL_DMA_PRIORITY_LOW);
	LL_DMA_SetMode(DMA1, INPUT_DMA_CHANNEL, LL_DMA_MODE_NORMAL);
	LL_DMA_SetPeriphIncMode(DMA1, INPUT_DMA_CHANNEL, LL_DMA_PERIPH_NOINCREMENT);
	LL_DMA_SetMemoryIncMode(DMA1, INPUT_DMA_CHANNEL, LL_DMA_MEMORY_INCREMENT);
	LL_DMA_SetPeriphSize(DMA1, INPUT_DMA_CHANNEL, LL_DMA_PDATAALIGN_HALFWORD);
	LL_DMA_SetMemorySize(DMA1, INPUT_DMA_CHANNEL, LL_DMA_MDATAALIGN_WORD);


#ifdef USE_TIMER_3_CHANNEL_1
	NVIC_SetPriority(IC_DMA_IRQ_NAME, 1);
	NVIC_EnableIRQ(IC_DMA_IRQ_NAME);
#endif

	TIM_InitStruct.Prescaler = 0;
	TIM_InitStruct.CounterMode = LL_TIM_COUNTERMODE_UP;
	TIM_InitStruct.Autoreload = 63;
	TIM_InitStruct.ClockDivision = LL_TIM_CLOCKDIVISION_DIV1;
	TIM_InitStruct.RepetitionCounter = 0;
	LL_TIM_Init(IC_TIMER_REGISTER, &TIM_InitStruct);
	LL_TIM_DisableARRPreload(IC_TIMER_REGISTER);
	LL_TIM_IC_SetActiveInput(IC_TIMER_REGISTER, IC_TIMER_CHANNEL, LL_TIM_ACTIVEINPUT_DIRECTTI);
	LL_TIM_IC_SetPrescaler(IC_TIMER_REGISTER, IC_TIMER_CHANNEL, LL_TIM_ICPSC_DIV1);
	LL_TIM_IC_SetFilter(IC_TIMER_REGISTER, IC_TIMER_CHANNEL, LL_TIM_IC_FILTER_FDIV1);
	LL_TIM_IC_SetPolarity(IC_TIMER_REGISTER, IC_TIMER_CHANNEL, LL_TIM_IC_POLARITY_BOTHEDGE);
}

void UN_GPIO_Init(void) {
	LL_EXTI_InitTypeDef EXTI_InitStruct = {0};
	LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

	/* GPIO Ports Clock Enable */
	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOF);
	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);

	// OC_SEL
	LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_11);
	GPIO_InitStruct.Pin = LL_GPIO_PIN_11;
	GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
	GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
	GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
	GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
	LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	// OC_TH_STBY2
	LL_GPIO_ResetOutputPin(GPIOF, LL_GPIO_PIN_6);
	GPIO_InitStruct.Pin = LL_GPIO_PIN_6;
	GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
	GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
	GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
	GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
	LL_GPIO_Init(GPIOF, &GPIO_InitStruct);

	// OC_TH_STBY1
	LL_GPIO_ResetOutputPin(GPIOF, LL_GPIO_PIN_7);
	GPIO_InitStruct.Pin = LL_GPIO_PIN_7;
	GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
	GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
	GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
	GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
	LL_GPIO_Init(GPIOF, &GPIO_InitStruct);

	//LL_SYSCFG_SetEXTISource(SYSCFG_EXTI_PORTA, SYSCFG_EXTI_LINEA);
	//LL_SYSCFG_SetEXTISource(SYSCFG_EXTI_PORTB, SYSCFG_EXTI_LINEB);
	//LL_SYSCFG_SetEXTISource(SYSCFG_EXTI_PORTC, SYSCFG_EXTI_LINEC);

	EXTI_InitStruct.Line_0_31 = PHASE_A_LL_EXTI_LINE;
	EXTI_InitStruct.LineCommand = ENABLE;
	EXTI_InitStruct.Mode = LL_EXTI_MODE_IT;
	EXTI_InitStruct.Trigger = LL_EXTI_TRIGGER_RISING;
	LL_EXTI_Init(&EXTI_InitStruct);

	EXTI_InitStruct.Line_0_31 = PHASE_B_LL_EXTI_LINE;
	EXTI_InitStruct.LineCommand = ENABLE;
	EXTI_InitStruct.Mode = LL_EXTI_MODE_IT;
	EXTI_InitStruct.Trigger = LL_EXTI_TRIGGER_RISING;
	LL_EXTI_Init(&EXTI_InitStruct);

	EXTI_InitStruct.Line_0_31 = PHASE_C_LL_EXTI_LINE;
	EXTI_InitStruct.LineCommand = ENABLE;
	EXTI_InitStruct.Mode = LL_EXTI_MODE_IT;
	EXTI_InitStruct.Trigger = LL_EXTI_TRIGGER_RISING;
	LL_EXTI_Init(&EXTI_InitStruct);

	//LL_EXTI_DisableIT_0_31(PHASE_A_LL_EXTI_LINE);
	//LL_EXTI_DisableIT_0_31(PHASE_B_LL_EXTI_LINE);
	//LL_EXTI_DisableIT_0_31(PHASE_C_LL_EXTI_LINE);

	GPIO_InitStruct.Pin = PHASE_A_EXTI_PIN;
	GPIO_InitStruct.Mode = LL_GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
	LL_GPIO_Init(PHASE_A_EXTI_PORT, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = PHASE_B_EXTI_PIN;
	GPIO_InitStruct.Mode = LL_GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
	LL_GPIO_Init(PHASE_B_EXTI_PORT, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = PHASE_C_EXTI_PIN;
	GPIO_InitStruct.Mode = LL_GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
	LL_GPIO_Init(PHASE_C_EXTI_PORT, &GPIO_InitStruct);

	/* EXTI interrupt init*/
	NVIC_SetPriority(EXTI_IRQA_NAME, 0);
	NVIC_SetPriority(EXTI_IRQB_NAME, 0);
	NVIC_SetPriority(EXTI_IRQC_NAME, 0);
	NVIC_EnableIRQ(EXTI_IRQA_NAME);
	NVIC_EnableIRQ(EXTI_IRQB_NAME);
	NVIC_EnableIRQ(EXTI_IRQC_NAME);

#ifdef USE_HALL_SENSOR
	GPIO_InitStruct.Pin = HALL_A_PIN;
	GPIO_InitStruct.Mode = LL_GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = LL_GPIO_PULL_UP;
	LL_GPIO_Init(HALL_A_PORT, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = HALL_B_PIN;
	GPIO_InitStruct.Mode = LL_GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = LL_GPIO_PULL_UP;
	LL_GPIO_Init(HALL_B_PORT, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = HALL_C_PIN;
	GPIO_InitStruct.Mode = LL_GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = LL_GPIO_PULL_UP;
	LL_GPIO_Init(HALL_C_PORT, &GPIO_InitStruct);
#endif

	// PB6 -> additional DSHOT input with EXTI (TIM16_CH1N can not be used as IC)
//	GPIO_InitStruct.Pin = LL_GPIO_PIN_6;
//	GPIO_InitStruct.Mode = LL_GPIO_MODE_INPUT;
//	GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
//	LL_GPIO_Init(GPIOB, &GPIO_InitStruct);
//
//	EXTI_InitStruct.Line_0_31 = LL_EXTI_LINE_6;
//	EXTI_InitStruct.LineCommand = ENABLE;
//	EXTI_InitStruct.Mode = LL_EXTI_MODE_IT;
//	EXTI_InitStruct.Trigger = LL_EXTI_TRIGGER_RISING;
//	LL_EXTI_Init(&EXTI_InitStruct);
//
//	LL_EXTI_DisableIT_0_31(LL_EXTI_LINE_6);
//
//	LL_SYSCFG_SetEXTISource(LL_SYSCFG_EXTI_PORTB, LL_SYSCFG_EXTI_LINE6);

//	NVIC_SetPriority(EXTI4_15_IRQn, 0);
//	NVIC_EnableIRQ(EXTI4_15_IRQn);


	// PB6 and PB7 debug output
//	GPIO_InitStruct.Pin = LL_GPIO_PIN_6;
//	GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
//	GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
//	GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
//	LL_GPIO_Init(GPIOB, &GPIO_InitStruct);
//
//	GPIO_InitStruct.Pin = LL_GPIO_PIN_7;
//	GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
//	GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
//	GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
//	LL_GPIO_Init(GPIOB, &GPIO_InitStruct);

#if defined(USE_USART_TX)
	// PB6 USART1_TX
	LL_GPIO_SetPinMode(GPIOB, LL_GPIO_PIN_6, LL_GPIO_MODE_ALTERNATE);
	LL_GPIO_SetAFPin_0_7(GPIOB, LL_GPIO_PIN_6, LL_GPIO_AF_0);
	LL_GPIO_SetPinSpeed(GPIOB, LL_GPIO_PIN_6, LL_GPIO_SPEED_FREQ_HIGH);
	LL_GPIO_SetPinOutputType(GPIOB, LL_GPIO_PIN_6, LL_GPIO_OUTPUT_PUSHPULL);
	LL_GPIO_SetPinPull(GPIOB, LL_GPIO_PIN_6, LL_GPIO_PULL_UP);

	LL_APB1_GRP2_EnableClock(LL_APB1_GRP2_PERIPH_USART1);
	LL_RCC_SetUSARTClockSource(LL_RCC_USART1_CLKSOURCE_PCLK1);
	LL_USART_SetTransferDirection(USART1, LL_USART_DIRECTION_TX);
	LL_USART_ConfigCharacter(USART1, LL_USART_DATAWIDTH_8B, LL_USART_PARITY_NONE, LL_USART_STOPBITS_1);

	LL_USART_SetOverSampling(USART1, LL_USART_OVERSAMPLING_8);
	LL_USART_SetBaudRate(USART1, SystemCoreClock, LL_USART_OVERSAMPLING_8, 1000000);

	LL_USART_EnableDMAReq_TX(USART1);

	LL_USART_Enable(USART1);

	while(!(LL_USART_IsActiveFlag_TEACK(USART1)))
	{	}

	// DMA Channel 2
	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA1);

	NVIC_SetPriority(DMA1_Channel2_3_IRQn, 3);
	NVIC_EnableIRQ(DMA1_Channel2_3_IRQn);

	LL_DMA_ConfigTransfer(DMA1, LL_DMA_CHANNEL_2,
						  LL_DMA_DIRECTION_MEMORY_TO_PERIPH |
						  LL_DMA_PRIORITY_HIGH              |
						  LL_DMA_MODE_NORMAL                |
						  LL_DMA_PERIPH_NOINCREMENT         |
						  LL_DMA_MEMORY_INCREMENT           |
						  LL_DMA_PDATAALIGN_BYTE            |
						  LL_DMA_MDATAALIGN_BYTE);
	LL_DMA_ConfigAddresses(DMA1, LL_DMA_CHANNEL_2,
						   (uint32_t)txBuffer,
						   LL_USART_DMA_GetRegAddr(USART1, LL_USART_DMA_REG_DATA_TRANSMIT),
						   LL_DMA_GetDataTransferDirection(DMA1, LL_DMA_CHANNEL_2));
	LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_2, 3);

	LL_DMA_EnableIT_TC(DMA1, LL_DMA_CHANNEL_2);
	LL_DMA_EnableIT_TE(DMA1, LL_DMA_CHANNEL_2);

#endif

#if defined(USE_USART_RX)
	// PB7 USART1_RX
	LL_GPIO_SetPinMode(GPIOB, LL_GPIO_PIN_7, LL_GPIO_MODE_ALTERNATE);
	LL_GPIO_SetAFPin_0_7(GPIOB, LL_GPIO_PIN_7, LL_GPIO_AF_0);
	LL_GPIO_SetPinSpeed(GPIOB, LL_GPIO_PIN_7, LL_GPIO_SPEED_FREQ_HIGH);
	LL_GPIO_SetPinOutputType(GPIOB, LL_GPIO_PIN_7, LL_GPIO_MODE_INPUT);
	LL_GPIO_SetPinPull(GPIOB, LL_GPIO_PIN_7, LL_GPIO_PULL_UP);

	LL_APB1_GRP2_EnableClock(LL_APB1_GRP2_PERIPH_USART1);
	LL_RCC_SetUSARTClockSource(LL_RCC_USART1_CLKSOURCE_PCLK1);
	LL_USART_SetTransferDirection(USART1, LL_USART_DIRECTION_RX);
	LL_USART_ConfigCharacter(USART1, LL_USART_DATAWIDTH_8B, LL_USART_PARITY_NONE, LL_USART_STOPBITS_1);

	LL_USART_SetOverSampling(USART1, LL_USART_OVERSAMPLING_8);
	LL_USART_SetBaudRate(USART1, SystemCoreClock, LL_USART_OVERSAMPLING_8, 1000000);

	LL_USART_EnableDMAReq_RX(USART1);

	LL_USART_Enable(USART1);

	while(!(LL_USART_IsActiveFlag_REACK(USART1)))
	{	}

	// DMA Channel 2
	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA1);

	NVIC_SetPriority(DMA1_Channel2_3_IRQn, 3);
	NVIC_EnableIRQ(DMA1_Channel2_3_IRQn);

	LL_DMA_ConfigTransfer(DMA1, LL_DMA_CHANNEL_3,
						  LL_DMA_DIRECTION_PERIPH_TO_MEMORY |
						  LL_DMA_PRIORITY_HIGH              |
						  LL_DMA_MODE_NORMAL                |
						  LL_DMA_PERIPH_NOINCREMENT         |
						  LL_DMA_MEMORY_INCREMENT           |
						  LL_DMA_PDATAALIGN_BYTE            |
						  LL_DMA_MDATAALIGN_BYTE);
	LL_DMA_ConfigAddresses(DMA1, LL_DMA_CHANNEL_3,
						   LL_USART_DMA_GetRegAddr(USART1, LL_USART_DMA_REG_DATA_RECEIVE),
						   (uint32_t)rxBuffer,
						   LL_DMA_GetDataTransferDirection(DMA1, LL_DMA_CHANNEL_3));
	LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_3, 6);

	LL_DMA_EnableIT_TC(DMA1, LL_DMA_CHANNEL_3);
	LL_DMA_EnableIT_HT(DMA1, LL_DMA_CHANNEL_3);
	LL_DMA_EnableIT_TE(DMA1, LL_DMA_CHANNEL_3);

	LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_3);
#endif
}

#ifdef USE_RGB_LED
void LED_GPIO_init() {
	LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
#ifdef LED_RED_PORT
	LL_GPIO_SetOutputPin(LED_RED_PORT, LED_RED_PIN);
	GPIO_InitStruct.Pin = LED_RED_PIN;
	GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
	GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_OPENDRAIN;
	GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
	LL_GPIO_Init(LED_RED_PORT, &GPIO_InitStruct);
#endif
#ifdef LED_GREEN_PORT
	LL_GPIO_SetOutputPin(LED_GREEN_PORT, LED_GREEN_PIN);
	GPIO_InitStruct.Pin = LED_GREEN_PIN;
	GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
	GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_OPENDRAIN;
	GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
	LL_GPIO_Init(LED_GREEN_PORT, &GPIO_InitStruct);
#endif
#ifdef LED_BLUE_PORT
	LL_GPIO_SetOutputPin(LED_BLUE_PORT, LED_BLUE_PIN);
	GPIO_InitStruct.Pin = LED_BLUE_PIN;
	GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
	GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_OPENDRAIN;
	GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
	LL_GPIO_Init(LED_BLUE_PORT, &GPIO_InitStruct);
#endif
}
#endif

void Error_Handler(void) {
// do absolutely nothing
}
