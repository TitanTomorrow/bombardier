/*
Copyright (c) 2015 Graham Chow

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_ints.h"
#include "inc/hw_types.h"
#include "inc/hw_adc.h"
#include "driverlib/pin_map.h"
#include "driverlib/debug.h"
#include "driverlib/sysctl.h"
#include "driverlib/adc.h"
#include "driverlib/rom.h"
#include "driverlib/interrupt.h"
#include "driverlib/gpio.h"
#include "driverlib/timer.h"
#include "driverlib/udma.h"
#include "driverlib/uart.h"
#include "usblib/usblib.h"
#include "usblib/usb-ids.h"
#include "usblib/device/usbdevice.h"
#include "usblib/device/usbdbulk.h"
#include "utils/uartstdio.h"
#include "utils/ustdlib.h"
#include "usb_bulk_structs.h"
#include "driverlib/pwm.h"
#include "global_def.h"

void ConfigureUART(void);

#define PWM_FREQUENCY (50)
#define PWM_MIN (00)
#define PWM_MAX (1000)
#define PWM_ZERO (1)

static uint32_t g_ui32Load;
static uint32_t g_ui32PWMClock;
static uint8_t g_ui8Adjust = PWM_ZERO;
static volatile uint8_t g_ui8AdjustPending = PWM_ZERO;
static volatile bool g_bUSBConfigured = false;
static uint32_t g_msg_index = 0;

/*
 * main.c
 */
int main(void)
{
	//uint32_t ui32Period;
	SysCtlClockSet(SYSCTL_SYSDIV_2_5|SYSCTL_USE_PLL|SYSCTL_OSC_MAIN|SYSCTL_XTAL_16MHZ);
	SysCtlPWMClockSet(SYSCTL_PWMDIV_64);

	// configure USB
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
    ROM_GPIOPinTypeUSBAnalog(GPIO_PORTD_BASE, GPIO_PIN_4 | GPIO_PIN_5);

	ConfigureUART();

	// gpio
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
	GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3);

	// configure pwm
	SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM0);
	GPIOPinTypePWM(GPIO_PORTB_BASE, GPIO_PIN_6);
	GPIOPinConfigure(GPIO_PB6_M0PWM0);
	g_ui32PWMClock = SysCtlClockGet() / 64;
	g_ui32Load = (g_ui32PWMClock / PWM_FREQUENCY) - 1;
	PWMGenConfigure(PWM0_BASE, PWM_GEN_0, PWM_GEN_MODE_DOWN);
	PWMGenPeriodSet(PWM0_BASE, PWM_GEN_0, g_ui32Load);

	PWMPulseWidthSet(PWM0_BASE, PWM_OUT_0, g_ui8Adjust * g_ui32Load / 1000);
	PWMOutputState(PWM0_BASE, PWM_OUT_0_BIT, true);
	PWMGenEnable(PWM0_BASE, PWM_GEN_0);

	// usb
	g_bUSBConfigured = false;
    USBBufferInit(&g_sTxBuffer);
    USBBufferInit(&g_sRxBuffer);
    USBStackModeSet(0, eUSBModeForceDevice, 0);
    USBDBulkInit(0, &g_sBulkDevice);

	IntMasterEnable();

	UARTprintf("Active\n");

	//int16_t debug = 1;
	while(true)
	{

		if(g_ui8AdjustPending != g_ui8Adjust)
		{
			GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 4);
			if(g_ui8AdjustPending < g_ui8Adjust)
			{
				g_ui8Adjust--;
				PWMPulseWidthSet(PWM0_BASE, PWM_OUT_0, g_ui8Adjust * g_ui32Load / 100);
			}
			else if(g_ui8AdjustPending > g_ui8Adjust)
			{
				g_ui8Adjust++;
				PWMPulseWidthSet(PWM0_BASE, PWM_OUT_0, g_ui8Adjust * g_ui32Load / 100);
			}
		}
		else
		{
			GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 2);
		}
        // Delay for a bit.
        //
		int i;
        for(i = 0; i < 20000; i++)
        {
        }

	}
}

//*****************************************************************************
//
// Configure the UART and its pins.  This must be called before UARTprintf().
//
//*****************************************************************************
void ConfigureUART(void)
{
    //
    // Enable the GPIO Peripheral used by the UART.
    //
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    //
    // Enable UART0
    //
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);

    //
    // Configure GPIO Pins for UART mode.
    //
    ROM_GPIOPinConfigure(GPIO_PA0_U0RX);
    ROM_GPIOPinConfigure(GPIO_PA1_U0TX);
    ROM_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    //
    // Use the internal 16MHz oscillator as the UART clock source.
    //
    UARTClockSourceSet(UART0_BASE, UART_CLOCK_PIOSC);

    //
    // Initialize the UART for console I/O.
    //
    UARTStdioConfig(0, 115200, 16000000);
}

//*****************************************************************************
//
// Handles bulk driver notifications related to the transmit channel (data to
// the USB host).
//
// \param pvCBData is the client-supplied callback pointer for this channel.
// \param ui32Event identifies the event we are being notified about.
// \param ui32MsgValue is an event-specific value.
// \param pvMsgData is an event-specific pointer.
//
// This function is called by the bulk driver to notify us of any events
// related to operation of the transmit data channel (the IN channel carrying
// data to the USB host).
//
// \return The return value is event-specific.
//
//*****************************************************************************
uint32_t
TxHandler(void *pvCBData, uint32_t ui32Event, uint32_t ui32MsgValue,
          void *pvMsgData)
{
    //
    // We are not required to do anything in response to any transmit event
    // in this example. All we do is update our transmit counter.
    //
    if(ui32Event == USB_EVENT_TX_COMPLETE)
    {
        //g_ui32TxCount += ui32MsgValue;
    }

    //
    // Dump a debug message.
    //
    //DEBUG_PRINT("TX complete %d\n", ui32MsgValue);

    return(0);
}

//*****************************************************************************
//
// Handles bulk driver notifications related to the receive channel (data from
// the USB host).
//
// \param pvCBData is the client-supplied callback pointer for this channel.
// \param ui32Event identifies the event we are being notified about.
// \param ui32MsgValue is an event-specific value.
// \param pvMsgData is an event-specific pointer.
//
// This function is called by the bulk driver to notify us of any events
// related to operation of the receive data channel (the OUT channel carrying
// data from the USB host).
//
// \return The return value is event-specific.
//
//*****************************************************************************
uint32_t
RxHandler(void *pvCBData, uint32_t ui32Event,
               uint32_t ui32MsgValue, void *pvMsgData)
{
    //
    // Which event are we being sent?
    //
    switch(ui32Event)
    {
        //
        // We are connected to a host and communication is now possible.
        //
        case USB_EVENT_CONNECTED:
        {
            g_bUSBConfigured = true;
            UARTprintf("Host connected.\n");

            //
            // Flush our buffers.
            //
            USBBufferFlush(&g_sTxBuffer);
            USBBufferFlush(&g_sRxBuffer);

            break;
        }

        //
        // The host has disconnected.
        //
        case USB_EVENT_DISCONNECTED:
        {
            g_bUSBConfigured = false;
            UARTprintf("Host disconnected.\n");
            break;
        }

        //
        // A new packet has been received.
        //
        case USB_EVENT_RX_AVAILABLE:
        {
            int i;

            //
            // Read the command 0xAA
            //
            // byte 0: command = 0xAA read
            // byte 1: MSB buffer size
            // byte 2: LSB buffer size

            uint8_t *data = (uint8_t *)pvMsgData;
            uint8_t pwm;
			for(i = ui32MsgValue;i>0;i--)
			{
				switch(g_msg_index)
				{
				default:
				case 0:
					if(*data == 0xAA)
					{
						g_msg_index++;
					}
					break;
				case 1:
					if(*data == 0xAA)
						g_msg_index++;
					else
						g_msg_index = 0;
					break;
				case 2:
					pwm = *data;
					pwm = min(pwm, PWM_MAX);
					pwm = max(pwm, PWM_MIN);
					g_ui8AdjustPending = pwm;
					g_msg_index = 0;
					break;
				}
				data++;
				if(data >= &g_pui8USBRxBuffer[BULK_RX_BUFFER_SIZE])
				{
					data = &g_pui8USBRxBuffer[0];
				}

            }
			USBBufferDataRemoved(&g_sRxBuffer, ui32MsgValue);
            break;
        }

        //
        // Ignore SUSPEND and RESUME for now.
        //
        case USB_EVENT_SUSPEND:
        case USB_EVENT_RESUME:
        {
            break;
        }

        //
        // Ignore all other events and return 0.
        //
        default:
        {
            break;
        }
    }

    return(0);
}
