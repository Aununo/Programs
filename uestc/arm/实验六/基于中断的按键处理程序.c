/*
 * main.c - Experiment 6: Interrupt-based Key Handling
 * For S3C2440 Processor
 */
#include "2440addr.h" // Provided header for S3C2440 register definitions [cite: 13]

// Define bit for EINT4_23 in INTMSK, SRCPND, INTPND (typically bit 5)
#define INT_EINT4_23_BIT (1 << 5)

/*
 * LED Control Functions
 * LEDs are connected to GPB5-GPB8 and are active low [cite: 5]
 */

// Turn on a specific LED (1 to 4)
void Led_On(int led_num) {
    if (led_num >= 1 && led_num <= 4) {
        // GPB5 is LED1, GPB6 is LED2, etc. So, pin = led_num + 4
        rGPBDAT &= ~(1 << (led_num + 4)); // Clear bit to turn LED on
    }
}

// Turn off a specific LED (1 to 4)
void Led_Off(int led_num) {
    if (led_num >= 1 && led_num <= 4) {
        rGPBDAT |= (1 << (led_num + 4));  // Set bit to turn LED off
    }
}

// Turn all LEDs on
void All_Leds_On(void) {
    rGPBDAT &= ~((1<<5) | (1<<6) | (1<<7) | (1<<8)); // Clear GPB5-8
}

// Turn all LEDs off
void All_Leds_Off(void) {
    rGPBDAT |= ((1<<5) | (1<<6) | (1<<7) | (1<<8));  // Set GPB5-8
}

/*
 * External Interrupt Handler for EINT8-EINT13
 * This C function is expected to be called by the assembly routine in IRQ_ISR.s
 * when an IRQ from EINT4-23 occurs.
 */
void EINT8_23_IRQHandler(void) {
    unsigned long pend_val = rEINTPEND; // Read pending external interrupts

    // Check which EINT (8-13) caused the interrupt [cite: 16]
    // K1 (EINT8) -> LED1 (GPB5) ON [cite: 5]
    if (pend_val & (1 << 8)) {
        Led_On(1);
        rEINTPEND = (1 << 8); // Clear EINT8 pending bit by writing 1 [cite: 16, 17]
    }
    // K2 (EINT9) -> LED2 (GPB6) ON [cite: 5]
    else if (pend_val & (1 << 9)) {
        Led_On(2);
        rEINTPEND = (1 << 9); // Clear EINT9 pending bit
    }
    // K3 (EINT10) -> LED3 (GPB7) ON [cite: 5]
    else if (pend_val & (1 << 10)) {
        Led_On(3);
        rEINTPEND = (1 << 10); // Clear EINT10 pending bit
    }
    // K4 (EINT11) -> LED4 (GPB8) ON [cite: 5]
    else if (pend_val & (1 << 11)) {
        Led_On(4);
        rEINTPEND = (1 << 11); // Clear EINT11 pending bit
    }
    // K5 (EINT12) -> All LEDs ON [cite: 5]
    else if (pend_val & (1 << 12)) {
        All_Leds_On();
        rEINTPEND = (1 << 12); // Clear EINT12 pending bit
    }
    // K6 (EINT13) -> All LEDs OFF [cite: 5]
    else if (pend_val & (1 << 13)) {
        All_Leds_Off();
        rEINTPEND = (1 << 13); // Clear EINT13 pending bit
    }

    // Clear the main interrupt source pending (EINT4_23) in SRCPND and INTPND [cite: 16, 18]
    ClearPending(INT_EINT4_23_BIT);
}

/*
 * Main Function
 */
int main(void) {
    // 1. Initialize LED GPIOs (GPB5-GPB8)
    // Configure GPB5, GPB6, GPB7, GPB8 as output pins.
    // GPBCON: bits [11:10] for GPB5, [13:12] for GPB6, [15:14] for GPB7, [17:16] for GPB8.
    // Set to '01' for output mode.
    rGPBCON &= ~((0x3 << 10) | (0x3 << 12) | (0x3 << 14) | (0x3 << 16)); // Clear current settings for GPB5-8
    rGPBCON |= ((0x1 << 10) | (0x1 << 12) | (0x1 << 14) | (0x1 << 16));  // Set GPB5-8 as output

    // Disable pull-up resistors for GPB5-8 (good practice for outputs).
    rGPBUP |= ((1 << 5) | (1 << 6) | (1 << 7) | (1 << 8));

    // Initially, turn all LEDs off. (LEDs are active low, so write 1 to data reg) [cite: 5]
    All_Leds_Off();

    // 2. Initialize External Interrupt GPIOs (EINT8-EINT13 / GPG0-GPG5)
    // EINT8=GPG0, EINT9=GPG1, EINT10=GPG2, EINT11=GPG3, EINT12=GPG4, EINT13=GPG5
    // Configure GPG0-GPG5 pins as external interrupt inputs ('10').
    // GPGCON: bits [1:0] for GPG0, ..., [11:10] for GPG5.
    rGPGCON &= ~((0x3 << 0) | (0x3 << 2) | (0x3 << 4) | (0x3 << 6) | (0x3 << 8) | (0x3 << 10)); // Clear GPG0-5
    rGPGCON |= ((0x2 << 0) | (0x2 << 2) | (0x2 << 4) | (0x2 << 6) | (0x2 << 8) | (0x2 << 10));  // Set GPG0-5 to EINT mode

    // Disable pull-up/down for GPG0-GPG5 as external pull-ups are used (schematic [cite: 4]).
    rGPGUP |= ((1 << 0) | (1 << 1) | (1 << 2) | (1 << 3) | (1 << 4) | (1 << 5));

    // 3. Configure External Interrupt Controller
    // Set EINT8-EINT13 to falling edge trigger ('010'b = 0x2)[cite: 21].
    // These are configured in EXTINT1 register.
    // EINT8:   EXTINT1[3:0]
    // EINT9:   EXTINT1[7:4]
    // EINT10:  EXTINT1[11:8]
    // EINT11:  EXTINT1[15:12]
    // EINT12:  EXTINT1[19:16]
    // EINT13:  EXTINT1[23:20]
    // Use read-modify-write: first clear relevant bits, then set them [cite: 14]
    rEXTINT1 &= ~((0xF << 0)  | (0xF << 4)  | (0xF << 8) | \
                  (0xF << 12) | (0xF << 16) | (0xF << 20)); // Clear trigger mode for EINT8-13
    rEXTINT1 |= ((0x2 << 0)  | (0x2 << 4)  | (0x2 << 8) | \
                 (0x2 << 12) | (0x2 << 16) | (0x2 << 20)); // Set falling edge for EINT8-13

    // Unmask EINT8-EINT13 in EINTMASK register (0 = unmasked, 1 = masked).
    rEINTMASK &= ~((1 << 8) | (1 << 9) | (1 << 10) | (1 << 11) | (1 << 12) | (1 << 13));

    // 4. Initialize Main Interrupt Controller
    // The assembly IRQ_ISR.s is expected to handle the jump from the IRQ vector
    // and call our C handler (EINT8_23_IRQHandler).
    // We need to unmask the EINT4_23 interrupt source in the main INTMSK register.
    // EINT4_23 is typically bit 5.
    rINTMSK &= ~INT_EINT4_23_BIT; // Unmask the EINT4-23 interrupt group

    // Enabling IRQs in CPSR is usually handled by the startup code (S3C2440.s)
    // or by the IRQ_ISR.s when it exits.

    // 5. Enter main loop
    // All actions are interrupt-driven.
    while (1) {
        // Idle loop, or low-power mode could be entered here.
    }

    return 0; // Should not be reached in bare-metal embedded
}    