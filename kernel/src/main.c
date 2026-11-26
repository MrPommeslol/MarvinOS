#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <limine.h>

//---------- Eigene includes ----------
#include "serial.h"
#include "kprintf.h"
#include "fb.h"
#include "console.h"
#include "keyboard.h"
#include "input.h"
//---------- Eigene includes ----------

// Set the base revision to 6, this is recommended as this is the latest
// base revision described by the Limine boot protocol specification.
// See specification for further info.

__attribute__((used, section(".limine_requests")))
static volatile uint64_t limine_base_revision[] = LIMINE_BASE_REVISION(6);

// The Limine requests can be placed anywhere, but it is important that
// the compiler does not optimise them away, so, usually, they should
// be made volatile or equivalent, _and_ they should be accessed at least
// once or marked as used with the "used" attribute as done here.

__attribute__((used, section(".limine_requests")))
static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST_ID,
    .revision = 0
};

// Finally, define the start and end markers for the Limine requests.
// These can also be moved anywhere, to any .c file, as seen fit.

__attribute__((used, section(".limine_requests_start")))
static volatile uint64_t limine_requests_start_marker[] = LIMINE_REQUESTS_START_MARKER;

__attribute__((used, section(".limine_requests_end")))
static volatile uint64_t limine_requests_end_marker[] = LIMINE_REQUESTS_END_MARKER;

// Halt and catch fire function.
static void hcf(void) {
    for (;;) {
#if defined (__x86_64__)
        asm ("hlt");
#elif defined (__aarch64__) || defined (__riscv)
        asm ("wfi");
#elif defined (__loongarch64)
        asm ("idle 0");
#endif
    }
}

// The following will be our kernel's entry point.
// If renaming kmain() to something else, make sure to change the
// linker script accordingly.
void kmain(void) {
    //  Versteht Limine die Version
    if (LIMINE_BASE_REVISION_SUPPORTED(limine_base_revision) == false) {
        hcf();
    }

    // Serielle ausgabe initialisieren
    if (serial_init() != 0) {
        hcf();
    }
    serial_puts("\033[2J\033[H");
    kprintf("Serial output is working\n");

    // Existiert ein Bildschirm
    if (framebuffer_request.response == NULL
     || framebuffer_request.response->framebuffer_count < 1) {
        kprintf("Fehler: Kein Framebuffer von Limine!\n");
        hcf();
    }

    struct limine_framebuffer *framebuffer = framebuffer_request.response->framebuffers[0];

    // Passt das Format
    if (framebuffer->memory_model != LIMINE_FRAMEBUFFER_RGB || framebuffer->bpp != 32) {
        kprintf("Fehler: Framebuffer-Format wird nicht unterstuetzt!\n");
        hcf();
    }

    // Grafik-Modul starten
    if (fb_init(framebuffer) != 0) {
        kprintf("Fehler: fb_init fehlgeschlagen!\n");
        hcf();
    }

    kprintf("\n(Info: Bildschirm: %u x %u Pixel, %u Bit pro Pixel, Pitch %u Bytes)\n",
            (unsigned int)framebuffer->width,
            (unsigned int)framebuffer->height,
            (unsigned int)framebuffer->bpp,
            (unsigned int)framebuffer->pitch);

    // Konsole starten
    console_init(0xFFFFFF, 0x000040, 2, 16);
    tprintf("Terminal output is working\n");

    // Polling Tasteninput abfragen
    keyboard_init();
    char line[128];
    while (1)
    {
        size_t len = input_readline(line, sizeof(line));
        tprintf("(%u)\n", (unsigned int)len);
    }

    hcf(); // Fertig, für immer anhalten
}