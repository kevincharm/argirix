#include <stddef.h>
#include <kernel/irq.h>
#include <kernel/io.h>

struct idt_entry idt[256];
void idt_entry_set(struct idt_entry *entry, uint32_t base, uint8_t selector, uint8_t flags)
{
    entry->base_lo = (base >>  0u) & 0xffff;
    entry->base_hi = (base >> 16u) & 0xffff;
    entry->selector = selector;
    entry->flags = flags;
    entry->unused = 0;
}

#define PIC1_PORT_CMD   (0x20)
#define PIC1_PORT_DATA  (PIC1_PORT_CMD+1)
#define PIC2_PORT_CMD   (0xA0)
#define PIC2_PORT_DATA  (PIC2_PORT_CMD+1)

static void pic_remap()
{
    outb(PIC1_PORT_CMD, 0x11);
    io_wait();
    outb(PIC2_PORT_CMD, 0x11);
    io_wait();

    outb(PIC1_PORT_DATA, 0x20); // PIC1 -> 32
    io_wait();
    outb(PIC2_PORT_DATA, 0x28); // PIC2 -> 40
    io_wait();

    outb(PIC1_PORT_DATA, 0x04);
    io_wait();
    outb(PIC2_PORT_DATA, 0x02);
    io_wait();

    outb(PIC1_PORT_DATA, 0x01);
    io_wait();
    outb(PIC2_PORT_DATA, 0x01);
    io_wait();

    outb(PIC1_PORT_DATA, 0xff);
    io_wait();
    outb(PIC2_PORT_DATA, 0xff);
    io_wait();
}

extern void keyboard_irq_handler(void);
void irq0_handler(struct interrupt_frame *frame)
{
    __asm__("pushal");

    (void)frame;
    keyboard_irq_handler();

    if (frame->int_no >= 0x28) {
        // EOI to PIC2 (slave)
        outb(PIC2_PORT_CMD, 0x20);
    }

    // EOI to PIC1 (master)
    outb(PIC1_PORT_CMD, 0x20);

    __asm__("popal; leave; iret"); // bite me
}

void irq_init()
{
    // TODO: replace initialisation with memset
    for (size_t i=0; i<256; i++) {
        uint8_t *u8idt = (void *)(idt+i);
        for (size_t j=0; j<3; j++)
            u8idt[j] = 0;
    }

    /*
        IDT entry flags
           7    6..5         4..0
        [  P  |  DPL  |  always 01110  ]
     */
    uint8_t sel = IDT_SEL_KERNEL;
    uint8_t flags = IDT_FLAGS_BASE | ((0x0 & 0x3) << 4u) | (1u << 7u);
    idt_entry_set(idt+33, (uint32_t)irq0_handler, sel, flags); // IRQ1 + PIC1 offset (=32)

    pic_remap();
    lidt(&idt, (sizeof (struct idt_entry) * 256) - 1);
}