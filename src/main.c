//
// This file is part of the GNU ARM Eclipse distribution.
// Copyright (c) 2014 Liviu Ionescu.
//

// ----------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include "cmsis/cmsis_device.h"
#include "stm32f4xx.h"
#include "ctype.h"
#include "diag/Trace.h"
#include <sys/stat.h>

// ----------------------------------------------------------------------------
//
// Standalone STM32F4 empty sample (trace via DEBUG).
//
// Trace support is enabled by adding the TRACE macro definition.
// By default the trace messages are forwarded to the DEBUG output,
// but can be rerouted to any device or completely suppressed, by
// changing the definitions required in system/src/diag/trace_impl.c
// (currently OS_USE_TRACE_ITM, OS_USE_TRACE_SEMIHOSTING_DEBUG/_STDOUT).
//

// ----- main() ---------------------------------------------------------------

// Sample pragmas to cope with warnings. Please note the related line at
// the end of this function, used to pop the compiler diagnostics status.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wmissing-declarations"
#pragma GCC diagnostic ignored "-Wreturn-type"

#define SEG_A_L1 GPIO_PIN_11
#define SEG_B_L2 GPIO_PIN_7
#define SEG_C_L3 GPIO_PIN_13
#define SEG_D GPIO_PIN_0
#define SEG_E GPIO_PIN_4
#define SEG_F GPIO_PIN_5
#define SEG_G GPIO_PIN_9
#define SEG_DP GPIO_PIN_6

#define CC_1 GPIO_PIN_14
#define CC_2 GPIO_PIN_12
#define CC_3 GPIO_PIN_10
#define CC_4 GPIO_PIN_8
#define CC_L GPIO_PIN_2

#define ON 1
#define OFF 0
#define TRUE 1
#define FALSE 0

void init_output_pins(GPIO_InitTypeDef GPIO_Init)
{
  GPIO_Init.Pin = SEG_A_L1 | SEG_B_L2 | SEG_C_L3 | SEG_D | SEG_E | SEG_F | SEG_G | SEG_DP |
      CC_1 | CC_2 | CC_3 | CC_4 | CC_L;
  GPIO_Init.Speed = GPIO_SPEED_MEDIUM;
  GPIO_Init.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_Init.Pull = GPIO_PULLUP;
  GPIO_Init.Alternate = 0;
  HAL_GPIO_Init(GPIOE, &GPIO_Init);
}

void write_digit(int digit)
{
  if (digit==CC_2 || digit==CC_3 || digit==CC_4 || digit==CC_L)
    HAL_GPIO_WritePin(GPIOE, SEG_A_L1, GPIO_PIN_SET);
  else
    HAL_GPIO_WritePin(GPIOE, SEG_A_L1, GPIO_PIN_RESET);
  if (digit==CC_1 || digit==CC_2 || digit==CC_3 || digit==CC_4 || digit==CC_L)
    HAL_GPIO_WritePin(GPIOE, SEG_B_L2, GPIO_PIN_SET);
  else
    HAL_GPIO_WritePin(GPIOE, SEG_B_L2, GPIO_PIN_RESET);
  if (digit==CC_1 || digit==CC_3 || digit==CC_4)
    HAL_GPIO_WritePin(GPIOE, SEG_C_L3, GPIO_PIN_SET);
  else
    HAL_GPIO_WritePin(GPIOE, SEG_C_L3, GPIO_PIN_RESET);
  if (digit==CC_2 || digit==CC_3 || digit==CC_4)
    HAL_GPIO_WritePin(GPIOE, SEG_D, GPIO_PIN_SET);
  else
    HAL_GPIO_WritePin(GPIOE, SEG_D, GPIO_PIN_RESET);
  if (digit==CC_2 || digit==CC_3 || digit==CC_4)
    HAL_GPIO_WritePin(GPIOE, SEG_E, GPIO_PIN_SET);
  else
    HAL_GPIO_WritePin(GPIOE, SEG_E, GPIO_PIN_RESET);
  if (digit==CC_3 || digit==CC_4)
    HAL_GPIO_WritePin(GPIOE, SEG_F, GPIO_PIN_SET);
  else
    HAL_GPIO_WritePin(GPIOE, SEG_F, GPIO_PIN_RESET);
  if (digit==CC_2)
    HAL_GPIO_WritePin(GPIOE, SEG_G, GPIO_PIN_SET);
  else
    HAL_GPIO_WritePin(GPIOE, SEG_G, GPIO_PIN_RESET);
  if (FALSE)
    HAL_GPIO_WritePin(GPIOE, SEG_DP, GPIO_PIN_SET);
  else
    HAL_GPIO_WritePin(GPIOE, SEG_DP, GPIO_PIN_RESET);
}

int main(int argc, char* argv[])
{
  HAL_Init();

  __HAL_RCC_GPIOA_CLK_ENABLE(); // input pins
  __HAL_RCC_GPIOE_CLK_ENABLE(); // output pins

  __HAL_RCC_TIM3_CLK_ENABLE();// enable clock for Timer 3

  GPIO_InitTypeDef GPIO_Init;

  // Initialize output pins for the display
  init_output_pins(GPIO_Init);

  // Set up timer 3 for limiting 7-segment frequency
  TIM_HandleTypeDef s_TimerInstance1;
  s_TimerInstance1.Instance = TIM3;
  s_TimerInstance1.Init.Prescaler = 8399;
  s_TimerInstance1.Init.Period = 20;
  s_TimerInstance1.Init.CounterMode = TIM_COUNTERMODE_DOWN;
  s_TimerInstance1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  s_TimerInstance1.Init.RepetitionCounter = 0;
  HAL_TIM_Base_Init(&s_TimerInstance1);
  HAL_TIM_Base_Start(&s_TimerInstance1);

  HAL_GPIO_WritePin(GPIOE, SEG_A_L1, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOE, SEG_B_L2, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOE, SEG_C_L3, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOE, SEG_D, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOE, SEG_E, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOE, SEG_F, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOE, SEG_G, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOE, SEG_DP, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOE, CC_1, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOE, CC_2, GPIO_PIN_SET);
  HAL_GPIO_WritePin(GPIOE, CC_3, GPIO_PIN_SET);
  HAL_GPIO_WritePin(GPIOE, CC_4, GPIO_PIN_SET);
  HAL_GPIO_WritePin(GPIOE, CC_L, GPIO_PIN_SET);

  int digits[] = { CC_1, CC_2, CC_3, CC_4, CC_L };
  int num_digits = sizeof(digits) / sizeof(digits[0]);
  int digit = 0;

  while (TRUE) {
    if (__HAL_TIM_GET_FLAG(&s_TimerInstance1, TIM_FLAG_UPDATE) == RESET) {
      // Continue looping until we reach the timer's period
      continue;
    }
    __HAL_TIM_CLEAR_FLAG(&s_TimerInstance1, TIM_IT_UPDATE); // Reset the timer

    for (int i = 0; i < num_digits; i++) {
      if (i == digit) {
        HAL_GPIO_WritePin(GPIOE, digits[i], GPIO_PIN_RESET);
      } else {
         HAL_GPIO_WritePin(GPIOE, digits[i], GPIO_PIN_SET);
      }
    }
    write_digit(digits[digit]);

    if (digit == num_digits-1) {
      digit = 0;
    } else {
      digit++;
    }

  }
}

#pragma GCC diagnostic pop

// ----------------------------------------------------------------------------
