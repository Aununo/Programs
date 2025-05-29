#include <S3C2440.H> // Or your specific S3C2440 header if different.
                     // If not available, use manual definitions below.

// Register definitions (as suggested in the PDF [cite: 31])
#define GPBCON   (*(volatile unsigned long *)0x56000010)
#define GPBDAT   (*(volatile unsigned long *)0x56000014)
#define GPBUP    (*(volatile unsigned long *)0x56000018)

// Simple delay function (implement a proper timer-based delay for real hardware)
void delay(volatile unsigned int count) {
    while(count--);
}

int main(void) {
    // Configure GPB pins
    // GPB1 (K1) as input
    // GPB5, GPB6, GPB7, GPB8 (LEDs) as output
    // GPBCON = 0x00015554; // Corrected: Example in PDF says 0x155551 [cite: 31]
                          // Bits [3:2] for GPB1 (input) = 00
                          // Bits [11:10] for GPB5 (output) = 01
                          // Bits [13:12] for GPB6 (output) = 01
                          // Bits [15:14] for GPB7 (output) = 01
                          // Bits [17:16] for GPB8 (output) = 01
                          // So, GPBCON should be set to configure GPB1 as input (00 at bits 3-2)
                          // and GPB5-8 as outputs (01 at bits 11-10, 13-12, 15-14, 17-16 respectively).
                          // Other bits should ideally be preserved or set to input if unused.
                          // A common value used in examples for this setup is 0x00015540 (if GPB0,2,3,4 are inputs)
                          // Let's use the PDF's suggested 0x155551 and assume it's correct for the specific MDK setup
                          // or that other bits don't cause issues in simulation.
    GPBCON = 0x00015540; // Configure GPB1 as input (bits 3:2 = 00)
                         // Configure GPB5-8 as outputs (bits 11:10, 13:12, 15:14, 17:16 = 01 each)
                         // Remaining bits for GPB0, GPB2, GPB3, GPB4, GPB9, GPB10 are set as input.
                         // 0x0001 0101 0101 0100 0000
                         // GPB8_OUT | GPB7_OUT | GPB6_OUT | GPB5_OUT | GPB4_IN | GPB3_IN | GPB2_IN | GPB1_IN | GPB0_IN

    // Disable pull-up resistor for GPB1 (optional, assuming external pull-up/pull-down as per diagram)
    // To enable pull-up for GPB1 (if needed, though diagram shows external resistors): GPBUP &= ~(1 << 1);
    // To disable pull-up for GPB1 (usually default for inputs or if external circuit manages it): GPBUP |= (1 << 1);
    // The diagram has an external 10K pull-up for GPB1. So, internal pull-up can be disabled.
    GPBUP = 0x07FF; // Disable all pull-ups for Port B (or selectively GPBUP |= (1 << 1);)

    unsigned int led_state_up[] = {
        ~(1 << 5), // LED1 ON (GPB5 low), others OFF (GPB6,7,8 high)
        ~(1 << 6), // LED2 ON (GPB6 low)
        ~(1 << 7), // LED3 ON (GPB7 low)
        ~(1 << 8)  // LED4 ON (GPB8 low)
    };
    // For lighting LEDs, we need to output LOW to the pin.
    // GPBDAT bits 5,6,7,8 control LED1,2,3,4.
    // To turn LED1 ON: GPB5=0. All other LEDs OFF: GPB6=1, GPB7=1, GPB8=1.
    // This means for GPBDAT, we want: ...1110.... (for LED1 ON)
    // For GPB5-8, initial state might be all OFF (all high).
    // Mask for LEDs: (0xF << 5) or 0x000001E0

    int current_led_index = 0;
    int direction = 1; // 1 for up (LED1->LED4), -1 for down (LED4->LED1)

    while(1) {
        // Read switch K1 state (GPB1)
        // If GPB1 is high (bit 1 of GPBDAT is 1)
        if (GPBDAT & (1 << 1)) {
            direction = 1; // Cycle LED1 -> LED4 [cite: 7]
        } else {
            direction = -1; // Cycle LED4 -> LED1 [cite: 7]
        }

        // Turn all LEDs OFF first (set GPB5-8 high)
        GPBDAT |= (0xF << 5); // Sets bits 5, 6, 7, 8 to 1

        // Turn on the current LED by setting its pin low
        // If direction is up (LED1 to LED4)
        if (direction == 1) {
            switch(current_led_index) {
                case 0: GPBDAT &= ~(1 << 5); break; // LED1 (GPB5) low
                case 1: GPBDAT &= ~(1 << 6); break; // LED2 (GPB6) low
                case 2: GPBDAT &= ~(1 << 7); break; // LED3 (GPB7) low
                case 3: GPBDAT &= ~(1 << 8); break; // LED4 (GPB8) low
            }
            current_led_index++;
            if (current_led_index > 3) {
                current_led_index = 0;
            }
        }
        // If direction is down (LED4 to LED1)
        else { // direction == -1
             switch(current_led_index) {
                case 0: GPBDAT &= ~(1 << 8); break; // LED4 (GPB8) low
                case 1: GPBDAT &= ~(1 << 7); break; // LED3 (GPB7) low
                case 2: GPBDAT &= ~(1 << 6); break; // LED2 (GPB6) low
                case 3: GPBDAT &= ~(1 << 5); break; // LED1 (GPB5) low
            }
            current_led_index++; // Still increment, but the mapping is reversed
            if (current_led_index > 3) {
                current_led_index = 0;
            }
        }

        delay(100000); // Adjust delay for simulation speed
				//
    }
    return 0;
}