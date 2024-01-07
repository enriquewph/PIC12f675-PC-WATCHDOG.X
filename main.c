/*
 * File:   main.c
 * Author: quiqu
 *
 * Created on December 25, 2023, 11:38 PM
 */

/* Funcionamiento del programa: PIC12F675
 * Cada 30 segundos leemos el estado de la entrada PC_ON (optoacoplador).
 * Un 1 representa que el led de encendido esta apagado, y un 0 que est� encendido.
 * En consecuencia: tendremos 1 si la m�quina est� encendida, y un 0 si est� apagada.
 * Si est� apagada, cuando hacemos el chequeo cada 30 segundos, se deber� enviar un pulso 0-1... 250ms ... 1-0 a la salida PC_BUTTON.
 *
 * Debemos copiar tambi�n el valor de !PC_ON en la salida STATUS_LED.
 *
 *
 * Entradas:
 * - PC_ON (GP2)
 * Salidas:
 * - STATUS_LED (GP0)
 * - PC_BUTTON (GP1)
 */

#include "header.h"
#include <stdint.h>

#define PC_ON GPIObits.GPIO2
#define STATUS_LED GPIObits.GPIO0
#define PC_BUTTON GPIObits.GPIO1


uint8_t counter = 0; //Cuando llega a 60, se chequea el estado de la entrada PC_ON (30 segundos)

// Interrupt Handler
void __interrupt() isr(void)
{
    // Timer1 Interrupt - Freq = 2.00 Hz - Period = 0.501192 seconds
    if (PIR1bits.TMR1IF) // timer 1 interrupt flag
    {
        CLRWDT();                   // clear the watchdog timer
        PIR1bits.TMR1IF = 0;        // interrupt must be cleared by software
        PIE1bits.TMR1IE = 1;        // reenable the interrupt
        TMR1H = 11;                 // preset for timer1 MSB register
        TMR1L = 71;                 // preset for timer1 LSB register
        counter++;

        
        // si PC_ON == 1, PARPADEAR STATUS_LED, caso contrario dejar encendido
        if (PC_ON == 1)
        {
            STATUS_LED = !STATUS_LED;
        }
        else
        {
            STATUS_LED = 1;
            // Reinicio el contador
            counter = 0;
        }
    }
}

void led_start_sequence()
{
    // 1 segundo de espera seguido de 10 parpadeos ultra rapidos
    __delay_ms(1000);
    for (uint8_t i = 0; i < 10; i++)
    {
        STATUS_LED = 1;
        __delay_ms(50);
        STATUS_LED = 0;
        __delay_ms(50);
    }
}

void main(void)
{
    INTCON = 0;        // clear the interrpt control register

    // watchdog setup
    // prescaler: 1:128
    // period: 2.1 seconds
    // on
    OPTION_REG = 0b11001111;


    // Configuramos los puertos
    TRISIO = 0b00001100; // MCLR,GP2 como entrada, el resto como salida
    GPIO = 0x00;   // Inicializamos todo a 0
    ANSEL = 0x00;  // No hay entradas analogicas
    CMCON = 0x07;       // Comparadores apagados 0b00000111
    VRCON = 0x00;       // Voltage Reference Control Register apagado
    ADCON0 = 0x00;      // ADC Control Register - ADC apagado

    // Timer1 Registers Prescaler= 8 - TMR1 Preset = 2887 - Freq = 2.00 Hz - Period = 0.501192 seconds
    T1CON = 0x3D;       // 0b00111101;
    TMR1H = 11;        // preset for timer1 MSB register
    TMR1L = 71;        // preset for timer1 LSB register

    // Interrupt Registers
    PIR1 = 0;          // clear the peripheral interrupt flag register
    PIE1 = 0x01;       // bit0 TMR1 overflow interrupt enable
    INTCON = 0xC0;     // bit7 GIE/GIEH - bit6 PEIE/GIEL

    led_start_sequence();

    while (1)
    {
        if (counter >= 60)
        {
            counter = 0;
            if (PC_ON == 1)
            {
                PC_BUTTON = 1;
                __delay_ms(250);
                PC_BUTTON = 0;
            }
        }
    }
}
