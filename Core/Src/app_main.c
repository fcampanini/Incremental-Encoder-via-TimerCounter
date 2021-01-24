/*
*Copyright 2021 Filippo Campanini
*
*CC BY-NC-SA License
*
*http://creativecommons.org/licenses/by-nc-sa/4.0/
*
*NOTICES
*THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
*INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
*IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
*WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
*OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/


#include "app_main.h"
#include "stdbool.h"

#define PWM_ENCODER_MULTIPLIER	200 //multiplier for the PWM pulse

bool 			wasPressed; 		//store press condition of rotary encoder switch
bool 			isPWMBlink;			//store blinking condition for LEDs
uint8_t			PWMBlinkIndex;		//index to count the blinks on LEDs
uint32_t 		counterEncoder;		//incremental current value coming from timer counter
uint32_t 		oldCounterEncoder;	//store previous incremental value coming from timer counter
uint32_t 		toTrigger;			//set the PWM channel currently enabled

/*
 * enumeration for storing the rotary encoder direction
 * CW	-	Clock Wise
 * CCW	-	Counter Clock Wise
 */
enum RotEncDirection{CW = false, CCW = true} direction, oldDirection;

/**
  * @brief  Launch LED dimming via PWM
  * @param  count 	- 	value for the PWM dimming
  * @param	channel	-	PWM channel to manage
  */
static void setPWMLightEncoder(uint32_t count,uint32_t channel);

/**
  * @brief  Manage the Rotary Encoder value change
  * @param  value 	- 	current incremental value
  */
static void EncoderValueChanged(uint32_t value);

/**
  * @brief  Manage the Rotary Encoder direction change
  * @param  value 	- 	current direction
  */
static void EncoderDirectionChanged(bool direction);

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

		counterEncoder = (TIM4->CNT / 4);
		direction = (TIM4->CR1 >> 4) & 0x0001;

		if(counterEncoder == 50 && oldCounterEncoder == 0)
		{
			TIM4->CNT = 0u;
		}
		else if(counterEncoder == 0 && oldCounterEncoder == 50)
		{
			TIM4->CNT = 200u;
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
				  setPWMLightEncoder(200, toTrigger);
				  PWMBlinkIndex ++;
			  }
			  else if(PWMBlinkIndex == 1)
			  {
				  setPWMLightEncoder(0, toTrigger);
				  PWMBlinkIndex ++;
			  }
			  else if(PWMBlinkIndex == 2)
			  {
				  setPWMLightEncoder(200, toTrigger);
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
static void EncoderValueChanged(uint32_t value)
{
	setPWMLightEncoder(value, toTrigger);
}
static void EncoderDirectionChanged(bool direction)
{

}
