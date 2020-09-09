/**
  ******************************************************************************
  * @file    IAP/src/main.c 
  * @author  MCD Application Team
  * @version V3.3.0
  * @date    10/15/2010
  * @brief   Main program body
  ******************************************************************************
  * @copy
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2010 STMicroelectronics</center></h2>
  */ 

/** @addtogroup IAP
  * @{
  */

/* Includes ------------------------------------------------------------------*/
#include "common.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
extern pFunction Jump_To_Application;
extern uint32_t JumpAddress;
uint32_t Command_str;

/* Private function prototypes -----------------------------------------------*/
void SetSysClock(void);
void Key_Init(void);
uint8_t Sample_Input(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, uint8_t bState);
static void IAP_Init(void);

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Main program.
  * @param  None
  * @retval None
  */
int main(void)
{
    SetSysClock();
    Key_Init();
    /* Flash unlock */
    FLASH_Unlock();

    Command_str = *(__IO uint32_t*)0x08002800;
    /* Test if the app download flag set */
    if((Sample_Input(GPIOA, GPIO_Pin_0, 0) == 0) || (Command_str == 0xFFFFFFFF))
    {
        FLASH_ErasePage(0x08002800);

        /* If flag is not set */
        /* Execute the IAP driver in order to re-program the Flash */
        IAP_Init();
        SerialPutString("\r\n======================================================================");
        SerialPutString("\r\n=              (C) COPYRIGHT 2010 STMicroelectronics                 =");
        SerialPutString("\r\n=                                                                    =");
        SerialPutString("\r\n=     In-Application Programming Application  (Version 3.3.0)        =");
        SerialPutString("\r\n=                                                                    =");
        SerialPutString("\r\n=                                   By MCD Application Team          =");
        SerialPutString("\r\n======================================================================");
        SerialPutString("\r\n\r\n");
        Main_Menu();
    }
    /* Keep the user application running */
    else
    {
        /* Test if user code is programmed starting from address "APPLICATION_ADDRESS" */
        if (((*(__IO uint32_t*)APPLICATION_ADDRESS) & 0x2FFE0000 ) == 0x20000000)
        {
            /* Jump to user application */
            JumpAddress = *(__IO uint32_t*) (APPLICATION_ADDRESS + 4);
            Jump_To_Application = (pFunction) JumpAddress;
            /* Initialize user application's Stack Pointer */
            __set_MSP(*(__IO uint32_t*) APPLICATION_ADDRESS);
            Jump_To_Application();
        }
    }

    while (1)
    {}
}

/**
  * @brief  Sets System clock frequency to 64MHz and configure HCLK, PCLK2 
  *         and PCLK1 prescalers. 
  * @note   This function should be used only after reset.
  * @param  None
  * @retval None
  */
void SetSysClock(void)
{
    RCC_DeInit();                                          // 将外设RCC寄存器重设为缺省值
    RCC_HSICmd(ENABLE);
    while(RCC_GetFlagStatus(RCC_FLAG_HSIRDY)== RESET);     // 等待HSI就绪
    RCC_HCLKConfig(RCC_SYSCLK_Div1);                       // 设置AHB时钟（HCLK） RCC_SYSCLK_Div1——AHB时钟 = 系统时*/ 
    RCC_PCLK2Config(RCC_HCLK_Div1);                        // 设置高速AHB时钟（PCLK2)RCC_HCLK_Div1——APB2时钟 = HCLK 
    RCC_PCLK1Config(RCC_HCLK_Div2);                        // 设置低速AHB时钟（PCLK1）RCC_HCLK_Div2——APB1时钟 = HCLK / 2
    FLASH_SetLatency(FLASH_Latency_2);                     // 设置FLASH存储器延时时钟周期数FLASH_Latency_2  2延时周期
    FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);  // 选择FLASH预取指缓存的模,预取指缓存使能
    RCC_PLLConfig(RCC_PLLSource_HSI_Div2, RCC_PLLMul_16);  // 设置PLL时钟源及倍频系数，频率为8/2*16=64Mhz
    RCC_PLLCmd(ENABLE);                                    // 使能PLL
    while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET);    // 检查指定的RCC标志位(PLL准备好标志)设置与否
    RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);             // 设置系统时钟（SYSCLK）
    while(RCC_GetSYSCLKSource() != 0x08);                  // 0x08：PLL作为系统时钟
}

/**
  * @brief  Initialize the key.
  * @param  None
  * @retval None
  */
void Key_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    /* Enable GPIO clock */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
}

/**
  * @brief  key scan.
  * @param  GPIOx
  * @param  GPIO_Pin
  * @param  bState
  * @retval return 0 if state is true or return 1
  */
uint8_t Sample_Input(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, uint8_t bState)
{
    uint8_t bRet;
    uint8_t bCount = 0;
    uint8_t bIndex;

    while(GPIO_ReadInputDataBit(GPIOx, GPIO_Pin) == bState)
    {
        bCount++;
        if(bCount > 50)
        {
            break;
        }
        for(bIndex = 0; bIndex < 50; bIndex++);
    }

    if(bCount > 50)
    {
        bRet = 0;
    }
    else
    {
        bRet = 1;
    }

    return bRet;
}

/**
  * @brief  Initialize the IAP: Configure RCC, USART and GPIOs.
  * @param  None
  * @retval None
  */
void IAP_Init(void)
{
    USART_InitTypeDef USART_InitStructure;
    GPIO_InitTypeDef  GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    /* USART resources configuration (Clock, GPIO pins and USART registers) ----*/
    /* USART configured as follow:
          - BaudRate = 115200 baud  
          - Word Length = 8 Bits
          - One Stop Bit
          - No parity
          - Hardware flow control disabled (RTS and CTS signals)
          - Receive and transmit enabled
    */
    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART1, &USART_InitStructure);
    USART_Cmd(USART1, ENABLE);
}

#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif

/**
  * @}
  */

/******************* (C) COPYRIGHT 2010 STMicroelectronics *****END OF FILE****/
