/**
 * @file syscalls.c
 * @brief System call stubs for STM32H753ZI bare-metal application
 * @author STM32H753ZI Project Team
 * @date 2025-08-05
 *
 * This file provides minimal implementations of system calls required by
 * newlib C library for bare-metal embedded systems.
 */

#include <sys/stat.h>
#include <errno.h>

/**
 * @brief Close file descriptor (stub)
 * @param file File descriptor
 * @return Always returns -1 (not implemented)
 */
int _close(int file) {
    (void)file; // Unused parameter
    errno = ENOSYS;
    return -1;
}

/**
 * @brief Get file status (stub)
 * @param file File descriptor
 * @param st Status structure
 * @return Always returns -1 (not implemented)
 */
int _fstat(int file, struct stat *st) {
    (void)file; // Unused parameter
    (void)st;   // Unused parameter
    errno = ENOSYS;
    return -1;
}

/**
 * @brief Check if file descriptor is a terminal (stub)
 * @param file File descriptor
 * @return Always returns 1 (assume terminal for stdout/stderr)
 */
int _isatty(int file) {
    // Return 1 for stdout/stderr, 0 for others
    return (file == 1 || file == 2) ? 1 : 0;
}

/**
 * @brief Seek in file (stub)
 * @param file File descriptor
 * @param ptr Offset
 * @param dir Direction
 * @return Always returns -1 (not implemented)
 */
int _lseek(int file, int ptr, int dir) {
    (void)file; // Unused parameter
    (void)ptr;  // Unused parameter
    (void)dir;  // Unused parameter
    errno = ENOSYS;
    return -1;
}

/**
 * @brief Read from file (stub)
 * @param file File descriptor
 * @param ptr Buffer
 * @param len Length
 * @return Always returns -1 (not implemented)
 */
int _read(int file, char *ptr, int len) {
    (void)file; // Unused parameter
    (void)ptr;  // Unused parameter
    (void)len;  // Unused parameter
    errno = ENOSYS;
    return -1;
}

/**
 * @brief Write to file
 * @param file File descriptor
 * @param ptr Buffer
 * @param len Length
 * @return Number of bytes written or -1 on error
 * 
 * @note This implementation redirects stdout/stderr to ITM debug output
 *       For other file descriptors, returns error.
 */
int _write(int file, char *ptr, int len) {
    if (file == 1 || file == 2) {  // stdout or stderr
        // TODO: Implement ITM debug output or UART redirect
        // For now, just return success
        return len;
    }
    errno = ENOSYS;
    return -1;
}

/**
 * @brief Get heap pointer (sbrk implementation)
 * @param incr Increment size
 * @return New heap pointer or -1 on error
 * 
 * @note Simple heap implementation using static memory region
 */
void *_sbrk(int incr) {
    extern char _end;           // Symbol from linker script
    extern char _estack;        // Symbol from linker script
    static char *heap_end = 0;
    char *prev_heap_end;

    if (heap_end == 0) {
        heap_end = &_end;
    }
    prev_heap_end = heap_end;

    // Check for heap overflow
    if (heap_end + incr > &_estack) {
        errno = ENOMEM;
        return (void *)-1;
    }

    heap_end += incr;
    return (void *)prev_heap_end;
}

/**
 * @brief Get process ID (stub)
 * @return Always returns 1
 */
int _getpid(void) {
    return 1;
}

/**
 * @brief Kill process (stub)
 * @param pid Process ID
 * @param sig Signal
 * @return Always returns -1 (not implemented)
 */
int _kill(int pid, int sig) {
    (void)pid; // Unused parameter
    (void)sig; // Unused parameter
    errno = ENOSYS;
    return -1;
}
