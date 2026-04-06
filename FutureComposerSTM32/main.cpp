#include "main.h"
#include "fc14synthesizer.hpp"
#include "music_data.h"
#include <cstdint>

// Sigma embedded devs never needed a heap
__asm(".global __use_no_heap\n");

#define ARRAY_SIZE(X) (sizeof(X)/sizeof((X)[0]))

uint8_t audio_buffer[4096] = {0};
volatile uint8_t bufferHalfToFill = 1;
volatile uint8_t bufferNeedsFill = 0;

DMA_HandleTypeDef hdma_tim_up;
TIM_HandleTypeDef htim;
UART_HandleTypeDef huart0;

alignas(4) constexpr auto MusicData = Fc14ByteorderInversion<KEIL_KEYGEN_MUSIC_DATA>();
FC14Synthesizer synthesizer(MusicData);

extern "C" void APB_DMA5_LIOINTC_IRQHandler();
extern "C" void DMA_HalfCpltCallback(DMA_HandleTypeDef *hdma_tim_up);
extern "C" void DMA_CpltCallback(DMA_HandleTypeDef *hdma_tim_up);
static void DMA_Init(void);
static void TIM_Init(void);
static void UART_Init(void);

int main() {
    HAL_Init();

    /* Initialize all configured peripherals */
    UART_Init();
    DMA_Init();
    TIM_Init();

    HAL_DMA_RegisterCallback(&hdma_tim_up, HAL_DMA_XFER_HALFCPLT_CB_ID, DMA_HalfCpltCallback);
    HAL_DMA_RegisterCallback(&hdma_tim_up, HAL_DMA_XFER_CPLT_CB_ID, DMA_CpltCallback);

    // Initial synthesis
    synthesizer.synthesize(audio_buffer, sizeof(audio_buffer));

    // Start DMA and Timer
    HAL_DMA_Start_IT(&hdma_tim_up,
                     (uint32_t)(uint64_t)audio_buffer,
                     (uint32_t)(uint64_t)(&ATIM->CCR3),
                     4096);
    __HAL_TIM_ENABLE_DMA(&htim, TIM_DMA_UPDATE);
    __HAL_TIM_MOE_ENABLE(&htim);
    TIM_CCxChannelCmd(htim.Instance, TIM_CHANNEL_3, TIM_CCxN_ENABLE);
    HAL_TIM_Base_Start(&htim);

    while (1) {
        if (bufferNeedsFill) {
            synthesizer.synthesize(audio_buffer + bufferHalfToFill * sizeof(audio_buffer) / 2,
                                   sizeof(audio_buffer) / 2);
            bufferNeedsFill = 0;
        }
    }
}

/**
  * @brief UART0 Initialization Function
  * @param None
  * @retval None
  */
static void UART_Init(void)
{
  huart0.Instance = UART0;
  huart0.Init.BaudRate = 115200;
  huart0.Init.WordLength = UART_WORDLENGTH_8B;
  huart0.Init.StopBits = UART_STOPBITS_1;
  huart0.Init.Parity = UART_PARITY_NONE;
  if (HAL_UART_Init(&huart0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief TIM Initialization Function
  * @param None
  * @retval None
  */
static void TIM_Init(void)
{
    // We use ATIM @ 200MHz, sample rate @ 41118 Hz
    // ARR = 0xFF (Uses full 8-bit values)
    // PSC = 19
    // Compare cycle = 200M / (PSC)19 / (ARR)256 / (RCR)1 = 41118 Hz, Repetition = 1, RCR = 0
    TIM_ClockConfigTypeDef sClockSourceConfig = {0};
    TIM_MasterConfigTypeDef sMasterConfig = {0};
    TIM_OC_InitTypeDef sConfigOC = {0};
    TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};
    htim.Instance = ATIM;
    htim.Init.Prescaler = 18;
    htim.Init.Period = 0xFF;
    htim.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim.Init.RepetitionCounter = 0;
    htim.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    if (HAL_TIM_Base_Init(&htim) != HAL_OK)
    {
        Error_Handler();
    }
    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    if (HAL_TIM_ConfigClockSource(&htim, &sClockSourceConfig) != HAL_OK)
    {
        Error_Handler();
    }
    if (HAL_TIM_PWM_Init(&htim) != HAL_OK)
    {
        Error_Handler();
    }
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&htim, &sMasterConfig) != HAL_OK)
    {
        Error_Handler();
    }
    sConfigOC.OCMode = TIM_OCMODE_PWM2;
    sConfigOC.Pulse = 0;
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
    sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
    if (HAL_TIM_PWM_ConfigChannel(&htim, &sConfigOC, TIM_CHANNEL_3) != HAL_OK)
    {
        Error_Handler();
    }
    sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
    sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
    sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
    sBreakDeadTimeConfig.DeadTime = 0;
    sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
    sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
    sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
    if (HAL_TIMEx_ConfigBreakDeadTime(&htim, &sBreakDeadTimeConfig) != HAL_OK)
    {
        Error_Handler();
    }
}

static void DMA_Init() {
    /* DMA interrupt init */
    /* APB_DMA2_LIOINTC_IRQn interrupt configuration */
    HAL_LIOINTC_Init(APB_DMA5_LIOINTC_IRQn / 32);
    HAL_LIOINTC_Config(APB_DMA5_LIOINTC_IRQn, LIOINTC_IRQ_TYPE_HIGH);
    HAL_LIOINTC_RegisterCallback(APB_DMA5_LIOINTC_IRQn, APB_DMA5_LIOINTC_IRQHandler);
    HAL_LIOINTC_Enable(APB_DMA5_LIOINTC_IRQn);
}

extern "C" void APB_DMA5_LIOINTC_IRQHandler() {
	HAL_DMA_IRQHandler(&hdma_tim_up);
}

extern "C" void DMA_HalfCpltCallback(DMA_HandleTypeDef *hdma_tim_up) {
    bufferHalfToFill = bufferHalfToFill ? 0 : 1; // Flip to another half of buffer
    bufferNeedsFill = 1;
}

extern "C" void DMA_CpltCallback(DMA_HandleTypeDef *hdma_tim_up) {
    bufferHalfToFill = bufferHalfToFill ? 0 : 1; // Flip to another half of buffer
    bufferNeedsFill = 1;
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
    __disable_irq();
    while (1)
    {
    }
}

// extern "C" void *malloc(size_t s) { return 0;}
// extern "C" void free(void *) { }

// Below is used to solve the C++ standard library malloc/free usage
// Found on: https://developer.arm.com/documentation/ka005021/latest/

// override the delete operator to avoid references to free.
void operator delete(void *pP) noexcept { }

// re-implement atexit calls due to static constructors
// and avoid allocation for the static destructors.
// This disables the static destructor calls
extern "C" {
    void __aeabi_atexit(void *) { }
    // disable static destructure call collection and avoid malloc
    // use only if application never terminates
    void __cxa_atexit(void (*func) (void *), void * arg, void * dso_handle) { }
}
