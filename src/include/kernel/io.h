#ifndef __ARGIR__IRQ_H
#define __ARGIR__IRQ_H

#include <stdint.h>

#define IRQ_SCAN_CODE       (0x60)
#define IRQ_PORT_STATUS     (0x64)

static inline void outb(uint16_t port, uint8_t byte)
{
    __asm__ volatile (
        "outb %0, %1"
        :
        : "a" (byte), "Nd" (port)
    );
}

static inline uint8_t inb(uint16_t port)
{
    uint8_t ret;
    __asm__ volatile (
        "inb %1, %0"
        : "=r" (ret)
        : "Nd" (port)
    );
    return ret;
}

#endif /* __ARGIR__IRQ_H */
