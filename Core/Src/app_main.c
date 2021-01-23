/*
 * app_main.c
 *
 *  Created on: 12 Jan 2021
 *  Author: Filippo Campanini
 */

#include "app_main.h"
#include "stdbool.h"

#define PWM_COURSE				200
#define PWM_MULTIPLIER			10
#define PWM_ENCODER_MULTIPLIER	200
#define	PWM_LIGHT_PULSE_SPEED	1


uint32_t count1, count2, count3, count4;
bool isUp1, isUp2, isUp3, isUp4;
uint32_t toTrigger;

uint32_t counterEncoder, oldCounterEncoder, delta;
bool direction, oldDirection;
bool isEncoderChanged, isDirectionChanged;

bool isSWPress, wasPressed, isPWMBlink;
uint8_t PWMBlinkIndex;


static void setPWMLightEncoder(uint32_t count,uint32_t channel);
static void setPWMLight(uint32_t *pCount, bool *pIsUp, uint32_t nextToTrigger, uint32_t channel);
void EncoderValueChanged(uint32_t value);
void EncoderDirectionChanged(bool direction);

void app_main()
{
	HAL_GPIO_WritePin(LD1_GPIO_Port, LD1_Pin, GPIO_PIN_SET);

	 //start timer
	 HAL_TIM_Base_Start_IT(&htim1);

	 //start second timer
	 HAL_TIM_Base_Start_IT(&htim6);

	 //start timer encoder
	 HAL_TIM_Encoder_Start(&htim4, TIM_CHANNEL_ALL);

	 toTrigger = TIM_CHANNEL_1;

	while(1)
	{

		/*
		if(toTrigger == TIM_CHANNEL_2)
		{
			setPWMLight(&count2, &isUp2, TIM_CHANNEL_3, TIM_CHANNEL_2);
		}
		else if(toTrigger == TIM_CHANNEL_3)
		{
			setPWMLight(&count3, &isUp3, TIM_CHANNEL_4, TIM_CHANNEL_3);
		}
		else if(toTrigger == TIM_CHANNEL_4)
		{
			setPWMLight(&count4, &isUp4, TIM_CHANNEL_2, TIM_CHANNEL_4);
		}

		HAL_Delay(PWM_LIGHT_PULSE_SPEED);
		*/


		counterEncoder = (TIM4->CNT / 4);
		direction = (TIM4->CR1 >> 4) & 0x0001;

		if(counterEncoder == 100 && oldCounterEncoder == 0)
		{
			TIM4->CNT = 0u;
		}
		else if(counterEncoder == 0 && oldCounterEncoder == 100)
		{
			TIM4->CNT = 400u;
		}
		else
		{
			if(counterEncoder != oldCounterEncoder)
			{
				oldCounterEncoder = counterEncoder;
				EncoderValueChanged(counterEncoder);
			}

			if(direction != oldDirection)
			{
				oldDirection = direction;
				EncoderDirectionChanged(direction);
			}
		}

		if(HAL_GPIO_ReadPin(RC_SW_GPIO_Port, RC_SW_Pin) == false && wasPressed == false)
		{
			HAL_Delay(2);
			wasPressed = true;
			TIM4->CNT = 0u;
			counterEncoder = 0;
			oldCounterEncoder = 0;

			switch(toTrigger)
			{
				case TIM_CHANNEL_1:
					toTrigger = TIM_CHANNEL_2;
					setPWMLightEncoder(0, TIM_CHANNEL_1);
				break;

				case TIM_CHANNEL_2:
					toTrigger = TIM_CHANNEL_3;
					setPWMLightEncoder(0, TIM_CHANNEL_2);
				break;

				case TIM_CHANNEL_3:
					toTrigger = TIM_CHANNEL_4;
					setPWMLightEncoder(0, TIM_CHANNEL_3);
				break;

				case TIM_CHANNEL_4:
					toTrigger = TIM_CHANNEL_1;
					setPWMLightEncoder(0, TIM_CHANNEL_4);
				break;
			}
			isPWMBlink = true;
		}
		else if(HAL_GPIO_ReadPin(RC_SW_GPIO_Port, RC_SW_Pin) == true && wasPressed == true)
		{
			HAL_Delay(2);
			wasPressed = false;
		}
	}
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	  if (htim == &htim1)
	  {
		  HAL_GPIO_TogglePin(LD1_GPIO_Port, LD1_Pin);
	  }
	  else if(htim == &htim6)
	  {
		  //HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);

		  if(isPWMBlink == true)
		  {
			  if(PWMBlinkIndex == 0)
			  {
				  setPWMLightEncoder(400, toTrigger);
				  PWMBlinkIndex ++;
			  }
			  else if(PWMBlinkIndex == 1)
			  {
				  setPWMLightEncoder(0, toTrigger);
				  PWMBlinkIndex ++;
			  }
			  else if(PWMBlinkIndex == 2)
			  {
				  setPWMLightEncoder(400, toTrigger);
				  PWMBlinkIndex ++;
			  }
			  else if(PWMBlinkIndex == 3)
			  {
				  setPWMLightEncoder(0, toTrigger);
				  PWMBlinkIndex = 0;
				  isPWMBlink = false;
			  }
		  }
	  }
}

static void setPWMLightEncoder(uint32_t count, uint32_t channel)
{
	TIM_OC_InitTypeDef sConfigOC = {0};
	sConfigOC.OCMode = TIM_OCMODE_PWM1;
	sConfigOC.Pulse = PWM_ENCODER_MULTIPLIER * count;
	HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, channel);
	HAL_TIM_PWM_Start(&htim3, channel);
}

static void setPWMLight(uint32_t *pCount, bool *pIsUp, uint32_t nextToTrigger, uint32_t channel)
{
	TIM_OC_InitTypeDef sConfigOC = {0};
	sConfigOC.OCMode = TIM_OCMODE_PWM1;


	if(*pCount < PWM_COURSE && *pIsUp == true)
	{
		(*pCount)++;
	}
	else if(*pCount == PWM_COURSE && *pIsUp == true)
	{
		*pCount -= 1;
		*pIsUp = false;
	}
	else if(*pCount > 0 && *pIsUp == false)
	{
		(*pCount)--;
	}
	else if(*pCount == 0 && *pIsUp == false)
	{
		*pIsUp = true;
		toTrigger = nextToTrigger;
	}

	sConfigOC.Pulse = PWM_MULTIPLIER * *pCount;
	HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, channel);
	HAL_TIM_PWM_Start(&htim3, channel);
}


void EncoderValueChanged(uint32_t value)
{
	setPWMLightEncoder(value, toTrigger);
}


void EncoderDirectionChanged(bool direction)
{

}
