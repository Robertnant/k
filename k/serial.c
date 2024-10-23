#include <k/types.h>
#include "io.h"
#include "serial.h"

#define COM1_BASE 0x3F8
#define DATA_OFFSET 0x00    // Data Register (RBR/THR).
#define LATCH_LOW 0x00      // Used when DLAB = 1.
#define IER_OFFSET 0x01
#define LATCH_HIGH 0x01     // Used when DLAB = 1.
#define IIR_OFFSET 0x02
#define FCR_OFFSET 0x02
#define LCR_OFFSET 0x03
#define MCR_OFFSET 0x04
#define LSR_OFFSET 0x05
#define FIFO_SIZE 16

/**
 * @brief Determines if the UART is ready to send new data.
 *
 * This function checks the Line Status Register (LSR) for the status of the
 * Transmitter Holding Register Empty (THRE) flag. If the flag is set, it indicates
 * that the UART is ready to accept a new byte for transmission.
 *
 * @return int Non-zero if the THR is empty and ready for new data, otherwise zero.
 */
int is_transmitter_holding_register_empty()
{
    return inb(COM1_BASE + LSR_OFFSET) & (1 << 5);
}

/**
 * @brief Tests whether the serial port is functioning correctly by performing a loopback test.
 *
 * This function enables loopback mode for testing, sends a byte, and checks if the same byte
 * is received. If the test passes, loopback mode is disabled and the function returns success.
 *
 * @return int Non-zero if the serial port test passes, zero if it fails.
 */
int is_serial_port_working()
{
    // Enable loopback mode for testing purposes while keeping DTR, RTS, and interrupt settings unchanged.
    outb(COM1_BASE + MCR_OFFSET, 0x1A);

    // Send a test byte and check if it is received (loopback test).
    outb(COM1_BASE + DATA_OFFSET, 0xAE);

    if (inb(COM1_BASE + DATA_OFFSET) != 0xAE)
    {
        return 0;
    }

    // Deactivate loopback mode (returning to normal operations).
    outb(COM1_BASE + MCR_OFFSET, 0x0B);

    return 1;
}

int init_serial()
{
    const u32 clock_rate = 115200;
    const u32 baud_rate = 38400;
    const u8 divisor_latch = clock_rate / baud_rate;

    // Disable all interrupts.
    outb(COM1_BASE + IER_OFFSET, 0x00);

    // Set divisor latch.
    outb(COM1_BASE + LCR_OFFSET, 0x80); // Set DLAB to 1.
    outb(COM1_BASE + LATCH_HIGH, divisor_latch >> 8);
    outb(COM1_BASE + LATCH_LOW, divisor_latch & 0xFF);

    // Clear DLAB as we've finished setting divisor latch, and set 8 bits, no parity and 1 stop bit.
    outb(COM1_BASE + LCR_OFFSET, 0x03);

    // (Enable FIFO) | (Interrupt trigger level 14 bytes) | (Clear transmit FIFO) | (Clear receive FIFO).
    outb(COM1_BASE + FCR_OFFSET, 0xC7);

    // Enable Transmiter Holding Register Empty Interrupt.
    outb(COM1_BASE + IER_OFFSET, 0x01);

    // Enable interrupts (IRQs) and set RTS and DTS.
    outb(COM1_BASE + MCR_OFFSET, 0x0B);

    return !is_serial_port_working(); // Negate result as 0 should indicate no error.
}

int write(const char *buf, size_t count)
{
    if (!is_serial_port_working()) {
        return -1;
    }

    size_t bytes_sent = 0;
    while (bytes_sent < count)
    {
        while (!is_transmitter_holding_register_empty())
        {
            ; // Wait for THR to be empty.
        }

        // Write as many bytes as possible to FIFO (up to its size) since it is used.
        for (int i = 0; i < FIFO_SIZE && bytes_sent < count; i++) {
            outb(COM1_BASE + DATA_OFFSET, buf[bytes_sent]);
            bytes_sent++;
        }
    }

    return bytes_sent;
}
