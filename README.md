# MINI51BSP_UART_TX_POLLING
 MINI51BSP_UART_TX_POLLING
 

udpate @ 2024/04/09

1. at NuTiny-SDK-Mini51 EVB , use TX send packet with 48 bytes per 200 ms

2. below is LA capture between each byte

![image](https://github.com/released/MINI51BSP_UART_TX_POLLING/blob/main/TX_byte_invertal.jpg)


3. below is LA capture between each byte , if enable DLY in UA_TOR register , 

refer to UART->TOR = (UART->TOR & ~UART_TOR_DLY_Msk)| (0x7F00);

![image](https://github.com/released/MINI51BSP_UART_TX_POLLING/blob/main/TX_byte_invertal_w_delay.jpg)

