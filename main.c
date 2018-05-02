/*******************************************************************************
* File Name: main.c
*
* Version: 1.10
*
* Description:
* Graupner SUMD decoder for the highr 4 channels
*******************************************************************************/
#include "project.h"
#define SUMD_SYNCBYTE 0xA8 
#define SUMD_MAXCHAN 8
#define SUMD_BUFFSIZE SUMD_MAXCHAN*2 + 5 // 6 channels + 5 -> 17 bytes for 6 channels
static uint8_t sumdIndex=0;
static uint8_t sumdSize=0;
static uint8_t sumd[SUMD_BUFFSIZE]={0}; 
volatile uint16_t rcValue[8] = {4400, 4400, 4400, 4400, 4400, 4400, 4400, 4400}; // interval [1000;2000]




long map(long x, long in_min, long in_max, long out_min, long out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void readSumD(uint8_t ch) {
  uint8_t cont=1;
  sumdSize=1;
  uint8_t val;
  uint8_t b;
    
  sumd[sumdIndex++]=ch;
  while (cont) {
    val= UART_UartGetChar();
    if (0u==val)
        continue;
    
    if(sumdIndex == 2) sumdSize = val;
    if(sumdIndex < SUMD_BUFFSIZE) sumd[sumdIndex] = val;
    sumdIndex++;
    
    if(sumdIndex == sumdSize*2+5) 
    {
      sumdIndex = 0;
      if (sumdSize > SUMD_MAXCHAN) 
            sumdSize = SUMD_MAXCHAN; 
      for (b = 0; b < sumdSize; b++) 
        {
        // rc pulse width from 4320 to 4540 (220 steps resolution)
        // need to translate 1000-2000 to those values
        rcValue[b] = map( (((sumd[2*b+3]<<8) | sumd[2*b+4])>>3),1000,2000,4320,4540);
        } 
     cont=0; // exit while loop
     }
  }
}




int main()
{
    uint8_t ch;
    /* Start SCB (UART mode) operation */
    UART_Start();
    PWM_1_Start();
    PWM_2_Start();
    PWM_3_Start();
    PWM_4_Start();
    
    //UART_UartPutString("\r\n***********************************************************************************\r\n");
    //UART_UartPutString("This is the decoder for Graupner SUMD highest 4 channels\r\n");
    //UART_UartPutString("\r\n***********************************************************************************\r\n");
    
    CyGlobalIntEnable;  /* Uncomment this line to enable global interrupts. */    
    for (;;)
    {
        /* Get received character or zero if nothing has been received yet */
        ch = UART_UartGetChar();
        if (0u != ch) //character present
        {
        sumdIndex = 0;
        if(ch != SUMD_SYNCBYTE) 
            continue;
        readSumD(ch);
      
        PWM_1_WriteCompare(rcValue[4]); 
        PWM_2_WriteCompare(rcValue[5]); 
        PWM_3_WriteCompare(rcValue[6]); 
        PWM_4_WriteCompare(rcValue[7]); 
        }
            
    }
}










/* [] END OF FILE */
