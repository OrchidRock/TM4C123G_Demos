#include <stdbool.h>
#include <stdint.h>

#include "driverlib.h"
#include "morsecode.h"

#define LED_RED GPIO_PIN_1
#define LED_BLUE GPIO_PIN_2
#define LED_GREEN GPIO_PIN_3

static void systick_delay(uint32_t);
static void systick_delay1ms(void);
static void toggle_led(void);
void GPIOPortF_Handler(void);

static uint32_t clock_hz = 0;
static char message[11] = "HELLOWORLD";
static uint32_t BEAT = 200; // ms
static uint8_t MYLED = LED_BLUE;

int main()
{
    // 50M HZ
    ROM_SysCtlClockSet(SYSCTL_SYSDIV_4|SYSCTL_USE_PLL|SYSCTL_XTAL_16MHZ|SYSCTL_OSC_MAIN);
    
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    while (!ROM_SysCtlPeripheralReady( SYSCTL_PERIPH_GPIOF )) {}
    ROM_GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, LED_RED|LED_BLUE|LED_GREEN);
    GPIOUnlockPin(GPIO_PORTF_BASE, GPIO_PIN_4); // Important !!!
    ROM_GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, GPIO_PIN_4); //SW2: PF4
     
    // Interrupt
    GPIOIntTypeSet( GPIO_PORTF_BASE, GPIO_PIN_4, GPIO_HIGH_LEVEL );
    GPIOIntRegister( GPIO_PORTF_BASE, GPIOPortF_Handler);
    GPIOIntClear( GPIO_PORTF_BASE, GPIO_INT_PIN_4 ); 
    GPIOIntEnable( GPIO_PORTF_BASE, GPIO_INT_PIN_4 );
    IntPrioritySet( INT_GPIOF, 2 );
    IntEnable( INT_GPIOF ); 

    clock_hz = ROM_SysCtlClockGet();    
    ROM_SysTickEnable();
    
    int i, j;
    
    for (;;) {
        for(i = 0; message[i] != '\0' ; i++) {
            char* code = MorseCode[message[i] - 'A'];
            for (j = 0; code[j] != '\0'; j++) {
                if (code[j] == '_') {
                    toggle_led();
                    toggle_led();
                    toggle_led();
                }else if (code[j] == '.') {
                    toggle_led();
                }
                systick_delay(BEAT*2);
            }
            systick_delay(BEAT*6);
        }    
    }

}

void toggle_led(void) {
	ROM_GPIOPinWrite(GPIO_PORTF_BASE, LED_RED | LED_GREEN | LED_BLUE, MYLED);
	systick_delay(BEAT);
    ROM_GPIOPinWrite(GPIO_PORTF_BASE, LED_RED | LED_GREEN | LED_BLUE, 0);
	systick_delay(BEAT);
}

void systick_delay1ms(void) {
    ROM_SysTickPeriodSet((clock_hz / 1000UL) - 1);
    SysTickValueClear();
    while(!SysTickCountIsSet()) {}
}

void systick_delay(uint32_t delay) {
    uint32_t i = 0;
    for(; i < delay ; i++) {
        systick_delay1ms();
    }
}


void GPIOPortF_Handler(void) {
    GPIOIntClear( GPIO_PORTF_BASE, GPIO_INT_PIN_4 ); 
    //MYLED = (MYLED == LED_BLUE) ? LED_RED : LED_BLUE;
    MYLED = MYLED << 1;
    if (MYLED > LED_GREEN) MYLED = LED_RED;
}

