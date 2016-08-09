#ifndef TERM_H
#define TERM_H

#define INITCODE    (0x4367)

/**
 * @brief Write the prompt to the serial port.
 *
 * @param HANDLE A pointer of the handle.
 */
#define PROMPT_WRITE(HANDLE)            SERIAL_WRITE((HANDLE), (HANDLE)->prompt, ntlibc_strlen((HANDLE)->prompt))

/**
 * @brief Read from the serial port.
 *
 * @param HANDLE A pointer of the handle.
 * @param BUF A pointer to the buffer.
 * @param CNT Read length.
 *
 * @return The number of bytes read.
 */
#define SERIAL_READ(HANDLE, BUF, CNT)   ((HANDLE)->func_read(BUF, CNT, (HANDLE)->extobj))

/**
 * @brief Write to the serial port.
 *
 * @param HANDLE A pointer of the handle.
 * @param BUF A pointer to the buffer.
 * @param CNT Write length.
 *
 * @return The number of bytes written.
 */
#define SERIAL_WRITE(HANDLE, BUF, CNT)  ((HANDLE)->func_write(BUF, CNT, (HANDLE)->extobj))

extern void terminalMode();

#endif
