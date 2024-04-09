/*_____ I N C L U D E S ____________________________________________________*/
#include <stdio.h>
#include <string.h>
#include "Mini51Series.h"

#include "misc_config.h"

/*_____ D E C L A R A T I O N S ____________________________________________*/

struct flag_32bit flag_PROJ_CTL;
#define FLAG_PROJ_TIMER_PERIOD_1000MS                 	(flag_PROJ_CTL.bit0)
#define FLAG_PROJ_TIMER_PERIOD_SPECIFIC        			(flag_PROJ_CTL.bit1)
#define FLAG_PROJ_REVERSE2                 				(flag_PROJ_CTL.bit2)
#define FLAG_PROJ_REVERSE3                              (flag_PROJ_CTL.bit3)
#define FLAG_PROJ_REVERSE4                              (flag_PROJ_CTL.bit4)
#define FLAG_PROJ_REVERSE5                              (flag_PROJ_CTL.bit5)
#define FLAG_PROJ_REVERSE6                              (flag_PROJ_CTL.bit6)
#define FLAG_PROJ_REVERSE7                              (flag_PROJ_CTL.bit7)


/*_____ D E F I N I T I O N S ______________________________________________*/

volatile unsigned int counter_systick = 0;
volatile uint32_t counter_tick = 0;

uint32_t freq_target = 500;
uint32_t freq_detect = 0;
/*_____ M A C R O S ________________________________________________________*/

#define PWM_CH_0_MASK                                   (0x1UL)    /*!< BPWM channel 0 mask */
#define PWM_CH_1_MASK                                   (0x2UL)    /*!< BPWM channel 1 mask */
#define PWM_CH_2_MASK                                   (0x4UL)    /*!< BPWM channel 2 mask */
#define PWM_CH_3_MASK                                   (0x8UL)    /*!< BPWM channel 3 mask */
#define PWM_CH_4_MASK                                   (0x10UL)   /*!< BPWM channel 4 mask */
#define PWM_CH_5_MASK                                   (0x20UL)   /*!< BPWM channel 5 mask */


/*_____ F U N C T I O N S __________________________________________________*/

unsigned int get_systick(void)
{
	return (counter_systick);
}

void set_systick(unsigned int t)
{
	counter_systick = t;
}

void systick_counter(void)
{
	counter_systick++;
}

void SysTick_Handler(void)
{

    systick_counter();

    if (get_systick() >= 0xFFFFFFFF)
    {
        set_systick(0);      
    }

    // if ((get_systick() % 1000) == 0)
    // {
       
    // }

    #if defined (ENABLE_TICK_EVENT)
    TickCheckTickEvent();
    #endif    
}

void SysTick_delay(unsigned int delay)
{  
    
    unsigned int tickstart = get_systick(); 
    unsigned int wait = delay; 

    while((get_systick() - tickstart) < wait) 
    { 
    } 

}

void SysTick_enable(unsigned int ticks_per_second)
{
    set_systick(0);
    if (SysTick_Config(SystemCoreClock / ticks_per_second))
    {
        /* Setup SysTick Timer for 1 second interrupts  */
        printf("Set system tick error!!\n");
        while (1);
    }

    #if defined (ENABLE_TICK_EVENT)
    TickInitTickEvent();
    #endif
}

uint32_t get_tick(void)
{
	return (counter_tick);
}

void set_tick(uint32_t t)
{
	counter_tick = t;
}

void tick_counter(void)
{
	counter_tick++;
    if (get_tick() >= 60000)
    {
        set_tick(0);
    }
}

// void delay_ms(uint16_t ms)
// {
// 	TIMER_Delay(TIMER0, 1000*ms);
// }

void UART_Packet_send(void)
{
    const uint8_t buffer_len = 48;
    uint8_t R_TX_Buffer[buffer_len] = {0};        
    uint8_t i = 0 ;
    static uint8_t cnt = 0;

    // create data
    for ( i = 0 ; i < buffer_len ; i++)
    {
        R_TX_Buffer[i] = i;
    }
    R_TX_Buffer[0] = 0x5A;
    R_TX_Buffer[1] = 0x5A;
    R_TX_Buffer[2] = cnt;

    R_TX_Buffer[buffer_len-3] = cnt;
    R_TX_Buffer[buffer_len-2] = 0xA5;
    R_TX_Buffer[buffer_len-1] = 0xA5;

    #if 1
    for (i = 0; i < buffer_len ; i++)
    {
        while ((UART->FSR & UART_FSR_TX_FULL_Msk));

        UART->THR = R_TX_Buffer[i];
    }
    #else
    UART_Write(UART,R_TX_Buffer,buffer_len);
    #endif

    cnt++;
}


void TMR0_IRQHandler(void)
{
	
    if(TIMER_GetIntFlag(TIMER0) == 1)
    {
        TIMER_ClearIntFlag(TIMER0);
		tick_counter();

		if ((get_tick() % 1000) == 0)
		{
            FLAG_PROJ_TIMER_PERIOD_1000MS = 1;//set_flag(flag_timer_period_1000ms ,ENABLE);
		}

		if ((get_tick() % 200) == 0)
		{
            FLAG_PROJ_TIMER_PERIOD_SPECIFIC = 1;
		}	

		if ((get_tick() % 50) == 0)
		{

		}	
    }
}

void TIMER0_Init(void)
{
    TIMER_Open(TIMER0, TIMER_PERIODIC_MODE, 1000);
    TIMER_EnableInt(TIMER0);
    NVIC_EnableIRQ(TMR0_IRQn);	
    TIMER_Start(TIMER0);
}

void loop(void)
{
	// static uint32_t LOG1 = 0;
	// static uint32_t LOG2 = 0;

    if ((get_systick() % 1000) == 0)
    {
        // printf("%s(systick) : %4d\r\n",__FUNCTION__,LOG2++);    
    }

    if (FLAG_PROJ_TIMER_PERIOD_1000MS)//(is_flag_set(flag_timer_period_1000ms))
    {
        FLAG_PROJ_TIMER_PERIOD_1000MS = 0;//set_flag(flag_timer_period_1000ms ,DISABLE);

        // printf("%s(timer) : %4d\r\n",__FUNCTION__,LOG1++);
        P36 ^= 1;        
    }
 
    if (FLAG_PROJ_TIMER_PERIOD_SPECIFIC)
    {
        FLAG_PROJ_TIMER_PERIOD_SPECIFIC = 0;
        UART_Packet_send();
    }

}

void UARTx_Process(void)
{
	uint8_t res = 0;
	res = UART_READ(UART);


	if (res > 0x7F)
	{
		printf("invalid command\r\n");
	}
	else
	{
		printf("press : %c\r\n" , res);
		switch(res)
		{
			case '1':
				break;

			case 'X':
			case 'x':
			case 'Z':
			case 'z':
                SYS_UnlockReg();
				// NVIC_SystemReset();	// Reset I/O and peripherals , only check BS(FMC_ISPCTL[1])
                // SYS_ResetCPU();     // Not reset I/O and peripherals
                SYS_ResetChip();    // Reset I/O and peripherals ,  BS(FMC_ISPCTL[1]) reload from CONFIG setting (CBS)	
				break;
		}
	}
}

void UART_IRQHandler(void)
{
    if(UART_GET_INT_FLAG(UART, UART_IER_RDA_IEN_Msk | UART_IER_RTO_IEN_Msk))     /* UART receive data available flag */
    {
        while(UART_GET_RX_EMPTY(UART) == 0)
        {
			UARTx_Process();
        }
    }

    if(UART->FSR & (UART_FSR_BIF_Msk | UART_FSR_FEF_Msk | UART_FSR_PEF_Msk | UART_FSR_RX_OVER_IF_Msk))
    {
        UART_ClearIntFlag(UART, (UART_ISR_RLS_INT_Msk| UART_ISR_BUF_ERR_INT_Msk));
    }	
}

void UART_Init(void)
{
    SYS_ResetModule(UART_RST);

    /* Configure UART0 and set UART0 baud rate */
    UART_Open(UART, 115200);
    NVIC_SetPriority(UART_IRQn, 3);

    UART_EnableInt(UART, UART_IER_RDA_IEN_Msk | UART_IER_RTO_IEN_Msk);
    NVIC_EnableIRQ(UART_IRQn);
	
	#if (_debug_log_UART_ == 1)	//debug
	printf("\r\nCLK_GetCPUFreq : %8d\r\n",CLK_GetCPUFreq());
	printf("CLK_GetHCLKFreq : %8d\r\n",CLK_GetHCLKFreq());
	printf("CLK_GetHXTFreq : %8d\r\n",CLK_GetHXTFreq());
	printf("CLK_GetLXTFreq : %8d\r\n",CLK_GetLXTFreq());
	#endif	

    #if 0
    printf("FLAG_PROJ_TIMER_PERIOD_1000MS : 0x%2X\r\n",FLAG_PROJ_TIMER_PERIOD_1000MS);
    printf("FLAG_PROJ_REVERSE1 : 0x%2X\r\n",FLAG_PROJ_REVERSE1);
    printf("FLAG_PROJ_REVERSE2 : 0x%2X\r\n",FLAG_PROJ_REVERSE2);
    printf("FLAG_PROJ_REVERSE3 : 0x%2X\r\n",FLAG_PROJ_REVERSE3);
    printf("FLAG_PROJ_REVERSE4 : 0x%2X\r\n",FLAG_PROJ_REVERSE4);
    printf("FLAG_PROJ_REVERSE5 : 0x%2X\r\n",FLAG_PROJ_REVERSE5);
    printf("FLAG_PROJ_REVERSE6 : 0x%2X\r\n",FLAG_PROJ_REVERSE6);
    printf("FLAG_PROJ_REVERSE7 : 0x%2X\r\n",FLAG_PROJ_REVERSE7);
    #endif
}

void GPIO_Init(void)
{
    SYS->P3_MFP = (SYS->P3_MFP & ~(SYS_MFP_P36_Msk)) | (SYS_MFP_P36_GPIO);

    GPIO_SetMode(P3, BIT6, GPIO_PMD_OUTPUT);
}

void SYS_Init(void)
{
    /* Unlock protected registers */
    SYS_UnlockReg();

   SYS->P5_MFP &= ~(SYS_MFP_P50_Msk | SYS_MFP_P51_Msk);
   SYS->P5_MFP |= (SYS_MFP_P50_XTAL1 | SYS_MFP_P51_XTAL2);

   /* Enable external 12MHz XTAL (UART), and internal 22.1184MHz */
   CLK->PWRCON = CLK_PWRCON_XTL12M ;

   /* Waiting for clock ready */
   CLK_WaitClockReady(CLK_CLKSTATUS_XTL_STB_Msk );

     /* Enable external 12MHz XTAL (UART), and internal 22.1184MHz */
    CLK->PWRCON =  CLK_PWRCON_IRC22M_EN_Msk /*| CLK_PWRCON_IRC10K_EN_Msk*/;

    /* Waiting for clock ready */
    CLK_WaitClockReady( CLK_CLKSTATUS_IRC22M_STB_Msk /*| CLK_CLKSTATUS_IRC10K_STB_Msk*/);   

    /* Switch HCLK clock source to XTL */
    CLK_SetHCLK(CLK_CLKSEL0_HCLK_S_IRC22M,CLK_CLKDIV_HCLK(1));

    /* STCLK to XTL STCLK to XTL */
    CLK_SetSysTickClockSrc(CLK_CLKSEL0_HCLK_S_IRC22M);

    CLK_EnableModuleClock(UART_MODULE);
    CLK_SetModuleClock(UART_MODULE,CLK_CLKSEL1_UART_S_IRC22M,CLK_CLKDIV_UART(1));

    CLK_EnableModuleClock(TMR0_MODULE);
    CLK_SetModuleClock(TMR0_MODULE,CLK_CLKSEL1_TMR0_S_IRC22M,0);

    SYS->P0_MFP &= ~(SYS_MFP_P01_Msk | SYS_MFP_P00_Msk);
    SYS->P0_MFP |= (SYS_MFP_P01_RXD | SYS_MFP_P00_TXD);


    /* Update System Core Clock */
    /* User can use SystemCoreClockUpdate() to calculate SystemCoreClock and cyclesPerUs automatically. */
    SystemCoreClockUpdate();

    /* Lock protected registers */
    SYS_LockReg();
}


int main()
{
    SYS_Init();

	GPIO_Init();
	UART_Init();
	TIMER0_Init();

    SysTick_enable(1000);
    #if defined (ENABLE_TICK_EVENT)
    TickSetTickEvent(1000, TickCallback_processA);  // 1000 ms
    TickSetTickEvent(5000, TickCallback_processB);  // 5000 ms
    #endif

    /* Got no where to go, just loop forever */
    while(1)
    {
        loop();

    }
}

/*** (C) COPYRIGHT 2013 Nuvoton Technology Corp. ***/
