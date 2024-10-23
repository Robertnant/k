#ifndef SERIAL_H
#define SERIAL_H

#include <k/types.h>

/**
 * @brief Initializes the serial port for communication at 38400 baud rate.
 *
 * This function configures the UART for serial communication. It sets the baud rate,
 * configures 8 data bits, no parity, 1 stop bit, enables FIFO with a 14-byte threshold,
 * and enables the Transmitter Holding Register Empty Interrupt.
 */
int init_serial();

/**
 * @brief Sends a buffer of data over the serial port.
 *
 * This function sends data from the buffer of size `count` over the serial port. It checks
 * the Transmitter Holding Register (THR) to ensure it is ready before writing each byte to the FIFO.
 *
 * @param buf The buffer containing the data to be sent.
 * @param count The number of bytes to send from the buffer.
 * @return int The number of bytes successfully sent.
 */
int write(const char* buf, size_t count);

#endif /* SERIAL_H */
