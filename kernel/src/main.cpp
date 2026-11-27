#include <cstdint>
#include <cstddef>
#include <limine.h>

#include "serial.h"
#include "kprintf.h"
#include "fb.h"
#include "console.h"
#include "keyboard.h"
#include "input.h"
// Set the base revision to 6, this is recommended as this is the latest
// base revision described by the Limine boot protocol specification.
// See specification for further info.

namespace {

__attribute__((used, section(".limine_requests")))
volatile std::uint64_t limine_base_revision[] = LIMINE_BASE_REVISION(6);

}

// The Limine requests can be placed anywhere, but it is important that
// the compiler does not optimise them away, so, usually, they should
// be made volatile or equivalent, _and_ they should be accessed at least
// once or marked as used with the "used" attribute as done here.

namespace {

__attribute__((used, section(".limine_requests")))
volatile limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST_ID,
    .revision = 0,
    .response = nullptr
};

}

// Finally, define the start and end markers for the Limine requests.
// These can also be moved anywhere, to any .cpp file, as seen fit.

namespace {

__attribute__((used, section(".limine_requests_start")))
volatile std::uint64_t limine_requests_start_marker[] = LIMINE_REQUESTS_START_MARKER;

__attribute__((used, section(".limine_requests_end")))
volatile std::uint64_t limine_requests_end_marker[] = LIMINE_REQUESTS_END_MARKER;

}

// Halt and catch fire function, defined below.
namespace {

void hcf();

}

// The following stubs are required by the Itanium C++ ABI (the one we use,
// regardless of the "Itanium" nomenclature).
// Like the memory functions in memory.cpp, these stubs can be moved to a different .cpp file,
// but should not be removed, unless you know what you are doing.
extern "C" {
    int __cxa_atexit(void (*)(void *), void *, void *) { return 0; }
    void __cxa_pure_virtual() { hcf(); }
    void __cxa_deleted_virtual() { hcf(); }
    void *__dso_handle;
    // Not thread safe: replace before statics can be initialised concurrently.
    int __cxa_guard_acquire(std::uint64_t *guard) { return *reinterpret_cast<std::uint8_t *>(guard) == 0; }
    void __cxa_guard_release(std::uint64_t *guard) { *reinterpret_cast<std::uint8_t *>(guard) = 1; }
}

// Extern declarations for global constructors array.
extern void (*__init_array_start[])();
extern void (*__init_array_end[])();

// The same for .ctors, which Clang 10 and older emit for x86 and which runs backwards.
extern void (*__ctors_start[])();
extern void (*__ctors_end[])();

// Halt and catch fire function.
namespace {

void hcf() {
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

}

// The following will be our kernel's entry point.
// If renaming kmain() to something else, make sure to change the
// linker script accordingly.
extern "C" void kmain() {
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
