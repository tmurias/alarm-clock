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

#define BUTTON GPIO_PIN_0
#define BTN_HOURS GPIO_PIN_3
#define BTN_MINS GPIO_PIN_5
#define BTN_ALARMOFF GPIO_PIN_7
#define BTN_TIMESET GPIO_PIN_0
#define BTN_24HR GPIO_PIN_2
#define BTN_WHATEVER GPIO_PIN_4
#define BTN_SNOOZE GPIO_PIN_6

#define ON 1
#define OFF 0
#define TRUE 1
#define FALSE 0

#define DIGIT_0 0b11111100
#define DIGIT_1 0b01100000
#define DIGIT_2 0b11011010
#define DIGIT_3 0b11110010
#define DIGIT_4 0b01100110
#define DIGIT_5 0b10110110
#define DIGIT_6 0b10111110
#define DIGIT_7 0b11100000
#define DIGIT_8 0b11111110
#define DIGIT_9 0b11100110

#define COLON 0b11000000

const int digit_values[10] = {
    DIGIT_0, DIGIT_1,
    DIGIT_2, DIGIT_3,
    DIGIT_4, DIGIT_5,
    DIGIT_6, DIGIT_7,
    DIGIT_8, DIGIT_9
};

void init_output_pins(GPIO_InitTypeDef GPIO_Init);
void init_input_pins(GPIO_InitTypeDef GPIO_Init);
void write_digit(int digit);
void write_digit_value(int value);
void increment_hours();
void increment_mins();
void increment_secs();

int hours = 2;
int mins = 41;
int secs = 0;

int main(int argc, char* argv[])
{
  HAL_Init();

  __HAL_RCC_GPIOA_CLK_ENABLE(); // input pins
  __HAL_RCC_GPIOE_CLK_ENABLE(); // output pins

  __HAL_RCC_TIM3_CLK_ENABLE();// enable clock for Timer 3
  __HAL_RCC_TIM6_CLK_ENABLE();// enable clock for Timer 6
  __HAL_RCC_TIM4_CLK_ENABLE();// enable clock for Timer 4

  GPIO_InitTypeDef GPIO_Init;

  // Initialize output pins for the display
  init_output_pins(GPIO_Init);

  // Initialize input pins for buttons
  init_input_pins(GPIO_Init);

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

  // Set up timer 6 for button debouncing
  TIM_HandleTypeDef s_TimerInstance2;
  s_TimerInstance2.Instance = TIM6;
  s_TimerInstance2.Init.Prescaler = 8399;
  s_TimerInstance2.Init.Period = 5000;
  s_TimerInstance2.Init.CounterMode = TIM_COUNTERMODE_DOWN;
  s_TimerInstance2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  s_TimerInstance2.Init.RepetitionCounter = 0;
  HAL_TIM_Base_Init(&s_TimerInstance2);
  HAL_TIM_Base_Start(&s_TimerInstance2);

  // Set up timer 4 for counting minutes
  TIM_HandleTypeDef s_TimerInstance3;
  s_TimerInstance3.Instance = TIM4;
  s_TimerInstance3.Init.Prescaler = 16799;
  s_TimerInstance3.Init.Period = 4999;
  s_TimerInstance3.Init.CounterMode = TIM_COUNTERMODE_DOWN;
  s_TimerInstance3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  s_TimerInstance3.Init.RepetitionCounter = 0;
  HAL_TIM_Base_Init(&s_TimerInstance3);
  HAL_TIM_Base_Start(&s_TimerInstance3);

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

  int button_state;
  int last_button_state = OFF; // Flag for edge detection when button is pressed
  int debounce_time_elapsed = TRUE; //Whether sufficient time has passed to press the button again

  int digits[] = { CC_1, CC_2, CC_3, CC_4, CC_L };
  int num_digits = sizeof(digits) / sizeof(digits[0]);
  int digit = 0;

  while (TRUE) {

    // Limits frequency for updating the 7-segment display
    if (__HAL_TIM_GET_FLAG(&s_TimerInstance1, TIM_FLAG_UPDATE) != RESET) {
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

    button_state = HAL_GPIO_ReadPin(GPIOA, BUTTON);

    // Flag is set when the debounce timer reaches its period
    debounce_time_elapsed = (__HAL_TIM_GET_FLAG(&s_TimerInstance2, TIM_FLAG_UPDATE) != RESET);

    if (button_state && !last_button_state && debounce_time_elapsed) {
      //increment_mins();
      __HAL_TIM_CLEAR_FLAG(&s_TimerInstance2, TIM_IT_UPDATE);
    }
    last_button_state = button_state;

    // Flag is set when a minute is reached (increment the clock)
    if (__HAL_TIM_GET_FLAG(&s_TimerInstance3, TIM_FLAG_UPDATE) != RESET) {
      increment_secs();
      __HAL_TIM_CLEAR_FLAG(&s_TimerInstance3, TIM_IT_UPDATE);
    }

  }
}

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

void init_input_pins(GPIO_InitTypeDef GPIO_Init)
{
  GPIO_Init.Pin = BUTTON;
  GPIO_Init.Speed = GPIO_SPEED_MEDIUM;
  GPIO_Init.Mode = GPIO_MODE_INPUT;
  GPIO_Init.Pull = GPIO_PULLDOWN;
  GPIO_Init.Alternate = 0;
  HAL_GPIO_Init(GPIOA, &GPIO_Init);
}

/**
 * Lights up all required segments for the given digit based
 * on the values of the global hours and mins variables.
 */
void write_digit(int digit)
{
  if (digit == CC_1) {
    write_digit_value(digit_values[hours / 10]);
  } else if (digit == CC_2) {
    write_digit_value(digit_values[hours % 10]);
  } else if (digit == CC_3) {
    write_digit_value(digit_values[mins / 10]);
  } else if (digit == CC_4) {
    write_digit_value(digit_values[mins % 10]);
  } else if (digit == CC_L) {
    write_digit_value(COLON);
  }
}

/**
 * Writes the given value to whatever digit is currently selected.
 */
void write_digit_value(int value)
{
  HAL_GPIO_WritePin(GPIOE, SEG_A_L1, (value & 0b10000000) ? GPIO_PIN_SET : GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOE, SEG_B_L2, (value & 0b01000000) ? GPIO_PIN_SET : GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOE, SEG_C_L3, (value & 0b00100000) ? GPIO_PIN_SET : GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOE, SEG_D, (value & 0b00010000) ? GPIO_PIN_SET : GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOE, SEG_E, (value & 0b00001000) ? GPIO_PIN_SET : GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOE, SEG_F, (value & 0b00000100) ? GPIO_PIN_SET : GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOE, SEG_G, (value & 0b00000010) ? GPIO_PIN_SET : GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOE, SEG_DP, (value & 0b00000001) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void increment_hours()
{
  hours++;
  if (hours > 12) {
    hours = 1;
  }
}

void increment_mins()
{
  mins++;
  if (mins > 59) {
    mins = 0;
    increment_hours();
  }
}

void increment_secs()
{
  secs++;
  if (secs > 59) {
    secs = 0;
    increment_mins();
  }
}

#pragma GCC diagnostic pop

// ----------------------------------------------------------------------------
