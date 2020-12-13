# stm32-uart-ring-buffer

##STEPS NEED TO BE FOLLOWED:-

**1. Put the following statements in the interrupt.c file**
```
  extern void Uart_isr (UART_HandleTypeDef *huart);
  extern volatile uint16_t timeout;
```
**2. Change the systick handler in the interrupt.c file again**
```
void SysTick_Handler(void)
{
  /* USER CODE BEGIN SysTick_IRQn 0 */
  
  if(timeout >0)  timeout--;

  /* USER CODE END SysTick_IRQn 0 */
  HAL_IncTick();
  /* USER CODE BEGIN SysTick_IRQn 1 */

  /* USER CODE END SysTick_IRQn 1 */
}
```

3. put the ```Ringbuf_init ();``` in the main function and you are good to go
