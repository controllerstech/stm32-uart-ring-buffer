# stm32-uart-ring-buffer

## STEPS NEED TO BE FOLLOWED:-

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
3. Modify the Uart_IRQ_Handler according to uart that you are using.. like shown below

```
void USART2_IRQHandler(void)
{
  /* USER CODE BEGIN USART2_IRQn 0 */

	Uart_isr (&huart2);

  /* USER CODE END USART2_IRQn 0 */
//  HAL_UART_IRQHandler(&huart2);
  /* USER CODE BEGIN USART2_IRQn 1 */

  /* USER CODE END USART2_IRQn 1 */
}
```

4. put the ```Ringbuf_init ();``` in the main function and you are good to go
