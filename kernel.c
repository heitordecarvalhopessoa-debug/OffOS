typedef unsigned char uint8_t;
typedef signed char int8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef signed int int32_t;
typedef unsigned long long uint64_t;
typedef unsigned char bool;

#define true 1
#define false 0
#define NULL ((void *)0)
#define MULTIBOOT_BOOTLOADER_MAGIC 0x2BADB002u
#define SCREEN_W 800
#define SCREEN_H 600
#define PAINT_W 520
#define PAINT_H 270

typedef struct __attribute__((packed)) {
    uint32_t flags;
    uint32_t mem_lower;
    uint32_t mem_upper;
    uint32_t boot_device;
    uint32_t cmdline;
    uint32_t mods_count;
    uint32_t mods_addr;
    uint8_t syms[16];
    uint32_t mmap_length;
    uint32_t mmap_addr;
    uint32_t drives_length;
    uint32_t drives_addr;
    uint32_t config_table;
    uint32_t boot_loader_name;
    uint32_t apm_table;
    uint32_t vbe_control_info;
    uint32_t vbe_mode_info;
    uint16_t vbe_mode;
    uint16_t vbe_interface_seg;
    uint16_t vbe_interface_off;
    uint8_t vbe_interface_len;
    uint8_t reserved;
    uint64_t framebuffer_addr;
    uint32_t framebuffer_pitch;
    uint32_t framebuffer_width;
    uint32_t framebuffer_height;
    uint8_t framebuffer_bpp;
    uint8_t framebuffer_type;
    uint8_t red_field_position;
    uint8_t red_mask_size;
    uint8_t green_field_position;
    uint8_t green_mask_size;
    uint8_t blue_field_position;
    uint8_t blue_mask_size;
} multiboot_info_t;

typedef struct __attribute__((packed)) {
    uint16_t base_low;
    uint16_t selector;
    uint8_t always_zero;
    uint8_t flags;
    uint16_t base_high;
} idt_entry_t;

typedef struct __attribute__((packed)) {
    uint16_t limit;
    uint32_t base;
} idt_pointer_t;

typedef struct {
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    uint8_t day;
    uint8_t month;
    uint16_t year;
} rtc_time_t;

enum window_type {
    WIN_NONE,
    WIN_FILES,
    WIN_VIEWER,
    WIN_CALCULATOR,
    WIN_CALENDAR,
    WIN_CLOCK,
    WIN_ART,
    WIN_QUIT
};

typedef struct {
    int x;
    int y;
    int w;
    int h;
} rect_t;

static volatile uint32_t *framebuffer;
static uint32_t fb_pitch_pixels;
static uint32_t fb_width;
static uint32_t fb_height;
static uint32_t screen_scale;
static int fb_offset_x;
static int fb_offset_y;
static uint8_t red_pos;
static uint8_t green_pos;
static uint8_t blue_pos;
static idt_entry_t idt[256];
static idt_pointer_t idt_pointer;

static uint32_t color_desktop;
static uint32_t color_bar;
static uint32_t color_window;
static uint32_t color_title;
static uint32_t color_border;
static uint32_t color_text;
static uint32_t color_muted;
static uint32_t color_button;
static uint32_t color_white;
static uint32_t color_highlight;

static enum window_type active_window = WIN_NONE;
static rect_t window_rect;
static int menu_open;
static int viewer_file;

static volatile uint32_t timer_ticks;
static volatile int mouse_x = SCREEN_W / 2;
static volatile int mouse_y = SCREEN_H / 2;
static volatile uint8_t mouse_buttons;
static volatile uint8_t mouse_moved;
static volatile uint8_t click_pending;
static volatile int click_x;
static volatile int click_y;
static volatile uint8_t key_queue[64];
static volatile uint8_t key_head;
static volatile uint8_t key_tail;
static uint8_t mouse_packet[3];
static uint8_t mouse_packet_pos;
static uint8_t previous_mouse_buttons;
static uint8_t keyboard_shift;
static uint8_t keyboard_extended;
static uint8_t keyboard_caps;
static volatile uint8_t redraw_needed = 1;

static char calc_input[24] = "0";
static int32_t calc_left;
static char calc_operator;
static uint8_t calc_has_left;
static uint8_t calc_error;
static uint8_t calc_new_input;
static int art_color = 1;
static uint8_t paint_canvas[PAINT_W * PAINT_H];
static uint8_t paint_was_cleared = 1;
static int calendar_month;
static int calendar_year;

extern void irq0_stub(void);
extern void irq1_stub(void);
extern void irq12_stub(void);

static const uint8_t font[36][7] = {
    {14,17,17,31,17,17,17}, {30,17,17,30,17,17,30},
    {14,17,16,16,16,17,14}, {30,17,17,17,17,17,30},
    {31,16,16,30,16,16,31}, {31,16,16,30,16,16,16},
    {14,17,16,23,17,17,15}, {17,17,17,31,17,17,17},
    {14,4,4,4,4,4,14}, {7,2,2,2,18,18,12},
    {17,18,20,24,20,18,17}, {16,16,16,16,16,16,31},
    {17,27,21,21,17,17,17}, {17,25,21,19,17,17,17},
    {14,17,17,17,17,17,14}, {30,17,17,30,16,16,16},
    {14,17,17,17,21,18,13}, {30,17,17,30,20,18,17},
    {15,16,16,14,1,1,30}, {31,4,4,4,4,4,4},
    {17,17,17,17,17,17,14}, {17,17,17,17,17,10,4},
    {17,17,17,21,21,21,10}, {17,17,10,4,10,17,17},
    {17,17,10,4,4,4,4}, {31,1,2,4,8,16,31},
    {14,17,19,21,25,17,14}, {4,12,4,4,4,4,14},
    {14,17,1,2,4,8,31}, {30,1,1,14,1,1,30},
    {2,6,10,18,31,2,2}, {31,16,16,30,1,1,30},
    {14,16,16,30,17,17,14}, {31,1,2,4,8,8,8},
    {14,17,17,14,17,17,14}, {14,17,17,15,1,1,14}
};

static const char *month_names[12] = {
    "JANUARY", "FEBRUARY", "MARCH", "APRIL", "MAY", "JUNE",
    "JULY", "AUGUST", "SEPTEMBER", "OCTOBER", "NOVEMBER", "DECEMBER"
};
static const char *weekday_names[7] = {"SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"};
static const char *demo_names[3] = {"README.TXT", "NOTES.TXT", "WELCOME.TXT"};

static inline void outb(uint16_t port, uint8_t value) {
    __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t value;
    __asm__ volatile ("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

static inline void outw(uint16_t port, uint16_t value) {
    __asm__ volatile ("outw %0, %1" : : "a"(value), "Nd"(port));
}

static inline void wait_io(void) {
    outb(0x80, 0);
}

static void halt_forever(void) {
    __asm__ volatile ("cli");
    for (;;) {
        __asm__ volatile ("hlt");
    }
}

static void boot_error(const char *message) {
    volatile uint16_t *vga = (volatile uint16_t *)0xB8000;
    int i = 0;
    while (message[i] && i < 80) {
        vga[i] = (uint16_t)(0x4F00u | (uint8_t)message[i]);
        outb(0xE9, (uint8_t)message[i]);
        i++;
    }
    halt_forever();
}

static uint32_t pack_rgb(uint8_t red, uint8_t green, uint8_t blue) {
    return ((uint32_t)blue << red_pos) |
           ((uint32_t)red << green_pos) |
           ((uint32_t)green << blue_pos);
}

static void draw_pixel(int x, int y, uint32_t color) {
    uint32_t px;
    uint32_t py;
    if (x < 0 || y < 0 || x >= SCREEN_W || y >= SCREEN_H || framebuffer == NULL) {
        return;
    }
    px = (uint32_t)fb_offset_x + ((uint32_t)x * screen_scale >> 16);
    py = (uint32_t)fb_offset_y + ((uint32_t)y * screen_scale >> 16);
    if (px < fb_width && py < fb_height) {
        framebuffer[py * fb_pitch_pixels + px] = color;
    }
}

static void fill_rect(int x, int y, int w, int h, uint32_t color) {
    int yy;
    int xx;
    uint32_t left;
    uint32_t top;
    uint32_t right;
    uint32_t bottom;
    if (w <= 0 || h <= 0) {
        return;
    }
    if (x < 0) { w += x; x = 0; }
    if (y < 0) { h += y; y = 0; }
    if (x + w > SCREEN_W) { w = SCREEN_W - x; }
    if (y + h > SCREEN_H) { h = SCREEN_H - y; }
    if (w <= 0 || h <= 0) {
        return;
    }
    left = (uint32_t)fb_offset_x + ((uint32_t)x * screen_scale >> 16);
    top = (uint32_t)fb_offset_y + ((uint32_t)y * screen_scale >> 16);
    right = (uint32_t)fb_offset_x + ((uint32_t)(x + w) * screen_scale >> 16);
    bottom = (uint32_t)fb_offset_y + ((uint32_t)(y + h) * screen_scale >> 16);
    if (right > fb_width) right = fb_width;
    if (bottom > fb_height) bottom = fb_height;
    for (yy = (int)top; yy < (int)bottom; yy++) {
        uint32_t offset = (uint32_t)yy * fb_pitch_pixels + left;
        for (xx = (int)left; xx < (int)right; xx++) {
            framebuffer[offset + (uint32_t)(xx - (int)left)] = color;
        }
    }
}

static void draw_rect(int x, int y, int w, int h, uint32_t color) {
    int i;
    if (w <= 0 || h <= 0) {
        return;
    }
    for (i = 0; i < w; i++) {
        draw_pixel(x + i, y, color);
        draw_pixel(x + i, y + h - 1, color);
    }
    for (i = 0; i < h; i++) {
        draw_pixel(x, y + i, color);
        draw_pixel(x + w - 1, y + i, color);
    }
}

static uint8_t glyph_row(char character, int row) {
    int index;
    if (character >= 'a' && character <= 'z') {
        character = (char)(character - 'a' + 'A');
    }
    if (character >= 'A' && character <= 'Z') {
        index = character - 'A';
        return font[index][row];
    }
    if (character >= '0' && character <= '9') {
        index = 26 + character - '0';
        return font[index][row];
    }
    switch (character) {
        case '-': return row == 3 ? 14 : 0;
        case '+': return row == 3 ? 14 : (row == 1 || row == 2 || row == 4 || row == 5 ? 4 : 0);
        case '=': return row == 2 || row == 4 ? 14 : 0;
        case ':': return row == 2 || row == 5 ? 4 : 0;
        case '.': return row == 6 ? 4 : 0;
        case ',': return row == 5 ? 4 : (row == 6 ? 8 : 0);
        case '/': return (uint8_t)(1u << (row > 4 ? 0 : 4 - row));
        case '_': return row == 6 ? 31 : 0;
        case '!': return row < 5 ? 4 : (row == 6 ? 4 : 0);
        case '?':
            if (row == 0 || row == 1) return 14;
            if (row == 2) return 1;
            if (row == 3) return 6;
            if (row == 4) return 4;
            return row == 6 ? 4 : 0;
        case '(': return row == 0 || row == 6 ? 2 : (row == 1 || row == 5 ? 4 : 8);
        case ')': return row == 0 || row == 6 ? 8 : (row == 1 || row == 5 ? 4 : 2);
        case '<': return row == 0 || row == 6 ? 2 : (row == 1 || row == 5 ? 4 : 8);
        case '>': return row == 0 || row == 6 ? 8 : (row == 1 || row == 5 ? 4 : 2);
        case '\'': return row < 2 ? 4 : 0;
        case ' ': return 0;
        default: return 0;
    }
}

static void draw_char(int x, int y, char character, uint32_t color, int scale) {
    int row;
    int column;
    for (row = 0; row < 7; row++) {
        uint8_t bits = glyph_row(character, row);
        for (column = 0; column < 5; column++) {
            if (bits & (1u << (4 - column))) {
                fill_rect(x + column * scale, y + row * scale, scale, scale, color);
            }
        }
    }
}

static void draw_text(int x, int y, const char *text, uint32_t color, int scale) {
    int cursor = x;
    while (*text) {
        draw_char(cursor, y, *text, color, scale);
        cursor += 6 * scale;
        text++;
    }
}

static void draw_line(int x0, int y0, int x1, int y1, uint32_t color) {
    int dx = x1 > x0 ? x1 - x0 : x0 - x1;
    int sx = x0 < x1 ? 1 : -1;
    int dy = y1 > y0 ? y0 - y1 : y1 - y0;
    int sy = y0 < y1 ? 1 : -1;
    int error = dx + dy;
    for (;;) {
        draw_pixel(x0, y0, color);
        if (x0 == x1 && y0 == y1) {
            break;
        }
        {
            int twice = error * 2;
            if (twice >= dy) {
                error += dy;
                x0 += sx;
            }
            if (twice <= dx) {
                error += dx;
                y0 += sy;
            }
        }
    }
}

static void draw_button(int x, int y, int w, int h, const char *label, bool selected) {
    fill_rect(x, y, w, h, selected ? color_highlight : color_button);
    draw_rect(x, y, w, h, color_border);
    draw_line(x + 2, y + h - 3, x + w - 3, y + h - 3, color_white);
    draw_line(x + w - 3, y + 2, x + w - 3, y + h - 3, color_white);
    draw_text(x + 8, y + (h - 14) / 2, label, color_text, 2);
}

static void set_idt_gate(uint8_t number, void (*handler)(void)) {
    uint32_t address = (uint32_t)handler;
    idt[number].base_low = (uint16_t)(address & 0xFFFF);
    /* GRUB enters the Multiboot kernel with its 32-bit code segment at 0x10. */
    idt[number].selector = 0x10;
    idt[number].always_zero = 0;
    idt[number].flags = 0x8E;
    idt[number].base_high = (uint16_t)((address >> 16) & 0xFFFF);
}

static void idt_init(void) {
    int i;
    for (i = 0; i < 256; i++) {
        idt[i].base_low = 0;
        idt[i].selector = 0;
        idt[i].always_zero = 0;
        idt[i].flags = 0;
        idt[i].base_high = 0;
    }
    set_idt_gate(32, irq0_stub);
    set_idt_gate(33, irq1_stub);
    set_idt_gate(44, irq12_stub);
    idt_pointer.limit = (uint16_t)(sizeof(idt) - 1);
    idt_pointer.base = (uint32_t)&idt;
    __asm__ volatile ("lidt %0" : : "m"(idt_pointer));
}

static void pic_remap(void) {
    uint8_t master_mask = inb(0x21);
    uint8_t slave_mask = inb(0xA1);
    outb(0x20, 0x11);
    wait_io();
    outb(0xA0, 0x11);
    wait_io();
    outb(0x21, 0x20);
    wait_io();
    outb(0xA1, 0x28);
    wait_io();
    outb(0x21, 0x04);
    wait_io();
    outb(0xA1, 0x02);
    wait_io();
    outb(0x21, 0x01);
    wait_io();
    outb(0xA1, 0x01);
    wait_io();
    outb(0x21, (uint8_t)(master_mask & ~0x07u));
    outb(0xA1, (uint8_t)(slave_mask & ~0x10u));
}

static void pit_init(void) {
    uint16_t divisor = (uint16_t)(1193182u / 100u);
    outb(0x43, 0x36);
    outb(0x40, (uint8_t)(divisor & 0xFF));
    outb(0x40, (uint8_t)(divisor >> 8));
}

static void ps2_wait_write(void) {
    int timeout = 100000;
    while ((inb(0x64) & 0x02) && timeout-- > 0) { }
}

static void ps2_wait_read(void) {
    int timeout = 100000;
    while (!(inb(0x64) & 0x01) && timeout-- > 0) { }
}

static void mouse_command(uint8_t command) {
    ps2_wait_write();
    outb(0x64, 0xD4);
    ps2_wait_write();
    outb(0x60, command);
    ps2_wait_read();
    (void)inb(0x60);
}

static void mouse_init(void) {
    uint8_t command_byte;
    ps2_wait_write();
    outb(0x64, 0xA8);
    ps2_wait_write();
    outb(0x64, 0x20);
    ps2_wait_read();
    command_byte = inb(0x60);
    command_byte = (uint8_t)((command_byte | 0x02u) & ~0x20u);
    ps2_wait_write();
    outb(0x64, 0x60);
    ps2_wait_write();
    outb(0x60, command_byte);
    mouse_command(0xF6);
    mouse_command(0xF4);
}

static char scancode_to_ascii(uint8_t code, uint8_t shifted) {
    static const char normal[58] = {
        0, 0, '1','2','3','4','5','6','7','8','9','0','-','=', '\b',
        '\t','q','w','e','r','t','y','u','i','o','p','[',']','\n',0,
        'a','s','d','f','g','h','j','k','l',';','\'','`',0,'\\',
        'z','x','c','v','b','n','m',',','.','/',0,'*',0,' '
    };
    static const char shifted_keys[58] = {
        0, 0, '!','@','#','$','%','^','&','*','(',')','_','+', '\b',
        '\t','Q','W','E','R','T','Y','U','I','O','P','{','}','\n',0,
        'A','S','D','F','G','H','J','K','L',':','"','~',0,'|',
        'Z','X','C','V','B','N','M','<','>','?',0,'*',0,' '
    };
    if (code >= 58) {
        return 0;
    }
    if (keyboard_caps && code >= 16 && code <= 50) {
        if (normal[code] >= 'a' && normal[code] <= 'z') {
            shifted = (uint8_t)!shifted;
        }
    }
    return shifted ? shifted_keys[code] : normal[code];
}

static void queue_key(uint8_t value) {
    uint8_t next = (uint8_t)((key_head + 1u) & 63u);
    if (next != key_tail) {
        key_queue[key_head] = value;
        key_head = next;
    }
}

void timer_irq_handler(void) {
    timer_ticks++;
    outb(0x20, 0x20);
}

void keyboard_irq_handler(void) {
    uint8_t code = inb(0x60);
    if (code == 0xE0) {
        keyboard_extended = 1;
        outb(0x20, 0x20);
        return;
    }
    if (code == 0x2A || code == 0x36) {
        keyboard_shift = (uint8_t)((code & 0x80u) == 0);
    } else if (code == 0x3A && !(code & 0x80u)) {
        keyboard_caps = (uint8_t)!keyboard_caps;
    } else if (!(code & 0x80u)) {
        if (keyboard_extended) {
            if (code == 0x4B) queue_key('[');
            if (code == 0x4D) queue_key(']');
            if (code == 0x48) queue_key('^');
            if (code == 0x50) queue_key('v');
            keyboard_extended = 0;
        } else {
            char key = scancode_to_ascii(code, keyboard_shift);
            if (key) queue_key((uint8_t)key);
        }
    } else {
        keyboard_extended = 0;
    }
    outb(0x20, 0x20);
}

void mouse_irq_handler(void) {
    uint8_t value = inb(0x60);
    if (mouse_packet_pos == 0 && !(value & 0x08u)) {
        outb(0xA0, 0x20);
        outb(0x20, 0x20);
        return;
    }
    mouse_packet[mouse_packet_pos++] = value;
    if (mouse_packet_pos == 3) {
        int dx = (int8_t)mouse_packet[1];
        int dy = (int8_t)mouse_packet[2];
        uint8_t buttons = (uint8_t)(mouse_packet[0] & 0x07u);
        int next_x;
        int next_y;
        mouse_packet_pos = 0;
        next_x = mouse_x + dx;
        next_y = mouse_y - dy;
        if (next_x < 0) next_x = 0;
        if (next_y < 0) next_y = 0;
        if (next_x >= SCREEN_W) next_x = SCREEN_W - 1;
        if (next_y >= SCREEN_H) next_y = SCREEN_H - 1;
        if (next_x != mouse_x || next_y != mouse_y) {
            mouse_x = next_x;
            mouse_y = next_y;
            mouse_moved = 1;
            redraw_needed = 1;
        }
        if ((buttons & 1u) && !(previous_mouse_buttons & 1u)) {
            click_x = next_x;
            click_y = next_y;
            click_pending = 1;
        }
        mouse_buttons = buttons;
        previous_mouse_buttons = buttons;
    }
    outb(0xA0, 0x20);
    outb(0x20, 0x20);
}

static uint8_t bcd_to_binary(uint8_t value) {
    return (uint8_t)((value & 0x0Fu) + ((value >> 4) * 10u));
}

static uint8_t read_cmos(uint8_t index) {
    outb(0x70, index);
    return inb(0x71);
}

static rtc_time_t read_rtc(void) {
    rtc_time_t result;
    uint8_t status;
    uint8_t raw_hour;
    uint8_t century;
    int tries = 1000;
    while ((read_cmos(0x0A) & 0x80u) && tries-- > 0) { }
    status = read_cmos(0x0B);
    result.second = read_cmos(0x00);
    result.minute = read_cmos(0x02);
    raw_hour = read_cmos(0x04);
    result.day = read_cmos(0x07);
    result.month = read_cmos(0x08);
    result.year = read_cmos(0x09);
    century = read_cmos(0x32);

    if (!(status & 0x04u)) {
        result.second = bcd_to_binary(result.second);
        result.minute = bcd_to_binary(result.minute);
        result.day = bcd_to_binary(result.day);
        result.month = bcd_to_binary(result.month);
        result.year = bcd_to_binary(result.year);
        century = bcd_to_binary(century);
    }
    result.hour = (uint8_t)(raw_hour & 0x7Fu);
    if (!(status & 0x04u)) {
        result.hour = bcd_to_binary(result.hour);
    }
    if (!(status & 0x02u) && (raw_hour & 0x80u)) {
        result.hour = (uint8_t)((result.hour + 12u) % 24u);
    }
    if (century >= 19 && century <= 99) {
        result.year = (uint16_t)(century * 100u + result.year);
    } else {
        result.year = (uint16_t)(2000u + result.year);
    }
    if (result.month < 1 || result.month > 12) result.month = 1;
    if (result.day < 1 || result.day > 31) result.day = 1;
    return result;
}

static void format_two_digits(char *buffer, uint8_t value) {
    buffer[0] = (char)('0' + (value / 10u) % 10u);
    buffer[1] = (char)('0' + value % 10u);
    buffer[2] = 0;
}

static void format_integer(char *buffer, int32_t value) {
    char reversed[12];
    uint32_t magnitude;
    int count = 0;
    int output = 0;
    if (value < 0) {
        buffer[output++] = '-';
        magnitude = 0u - (uint32_t)value;
    } else {
        magnitude = (uint32_t)value;
    }
    do {
        reversed[count++] = (char)('0' + magnitude % 10u);
        magnitude /= 10u;
    } while (magnitude && count < 11);
    while (count > 0) buffer[output++] = reversed[--count];
    buffer[output] = 0;
}

static int parse_integer(const char *text) {
    int sign = 1;
    int value = 0;
    if (*text == '-') {
        sign = -1;
        text++;
    }
    while (*text >= '0' && *text <= '9') {
        value = value * 10 + (*text - '0');
        text++;
    }
    return value * sign;
}

static bool is_leap_year(int year) {
    return (year % 4 == 0 && year % 100 != 0) || year % 400 == 0;
}

static int days_in_month(int month, int year) {
    static const uint8_t days[12] = {31,28,31,30,31,30,31,31,30,31,30,31};
    if (month == 1 && is_leap_year(year)) return 29;
    return days[month];
}

static int first_weekday(int year, int month) {
    int a = (14 - (month + 1)) / 12;
    int y = year - a;
    int m = (month + 1) + 12 * a - 2;
    return (1 + y + y / 4 - y / 100 + y / 400 + (31 * m) / 12) % 7;
}

static void set_window(enum window_type type) {
    active_window = type;
    menu_open = 0;
    switch (type) {
        case WIN_FILES:
        case WIN_VIEWER:
            window_rect = (rect_t){150, 88, 500, 405};
            break;
        case WIN_CALCULATOR:
            window_rect = (rect_t){215, 82, 370, 410};
            break;
        case WIN_CALENDAR:
            window_rect = (rect_t){195, 78, 410, 420};
            break;
        case WIN_CLOCK:
            window_rect = (rect_t){190, 130, 420, 280};
            break;
        case WIN_ART:
            window_rect = (rect_t){88, 48, 624, 510};
            break;
        case WIN_QUIT:
            window_rect = (rect_t){260, 205, 280, 175};
            break;
        default:
            active_window = WIN_NONE;
            break;
    }
    redraw_needed = 1;
}

static void draw_window_frame(const char *title) {
    int x = window_rect.x;
    int y = window_rect.y;
    int w = window_rect.w;
    int h = window_rect.h;
    fill_rect(x, y, w, h, color_window);
    draw_rect(x, y, w, h, color_border);
    fill_rect(x + 2, y + 2, w - 4, 32, color_title);
    draw_text(x + 12, y + 10, title, color_white, 2);
    fill_rect(x + w - 30, y + 5, 22, 24, color_button);
    draw_rect(x + w - 30, y + 5, 22, 24, color_border);
    draw_text(x + w - 24, y + 10, "X", color_text, 2);
}

static void draw_home_panel(void) {
    static const char *apps[4] = {"CALCULATOR", "CALENDAR", "CLOCK", "ARTAPP"};
    int i;
    fill_rect(28, 108, 230, 188, color_window);
    draw_rect(28, 108, 230, 188, color_border);
    fill_rect(33, 113, 220, 32, color_title);
    draw_text(43, 122, "APPS", color_white, 2);
    for (i = 0; i < 4; i++) {
        draw_text(50, 158 + i * 29, "-", color_text, 2);
        draw_text(70, 158 + i * 29, apps[i], color_text, 2);
    }
}

static void draw_popup_menu(void) {
    int x;
    int rows;
    const char **items;
    static const char *file_items[5] = {
        "OPEN FILES", "README.TXT", "NOTES.TXT", "WELCOME.TXT", "CLOSE WINDOW"
    };
    static const char *tool_items[4] = {"CALCULATOR", "CALENDAR", "CLOCK", "ARTAPP"};
    int i;
    if (menu_open == 1) {
        x = 596;
        rows = 5;
        items = file_items;
    } else if (menu_open == 2) {
        x = 674;
        rows = 4;
        items = tool_items;
    } else {
        return;
    }
    fill_rect(x, 36, 194, rows * 29 + 4, color_window);
    draw_rect(x, 36, 194, rows * 29 + 4, color_border);
    for (i = 0; i < rows; i++) {
        draw_text(x + 10, 44 + i * 29, items[i], color_text, 2);
    }
}

static void draw_file_window(void) {
    int i;
    draw_window_frame("FILES - HOME");
    draw_text(window_rect.x + 22, window_rect.y + 55, "VIRTUAL FILES", color_muted, 2);
    for (i = 0; i < 3; i++) {
        fill_rect(window_rect.x + 18, window_rect.y + 91 + i * 45,
                  window_rect.w - 36, 35, i == 0 ? color_highlight : color_window);
        draw_text(window_rect.x + 32, window_rect.y + 103 + i * 45, demo_names[i], color_text, 2);
    }
    draw_text(window_rect.x + 22, window_rect.y + 250, "CLICK A FILE TO OPEN IT", color_muted, 2);
}

static void draw_viewer_window(void) {
    const char *line1;
    const char *line2;
    const char *line3;
    const char *line4;
    const char *line5;
    const char *line6;
    draw_window_frame(demo_names[viewer_file]);
    if (viewer_file == 0) {
        line1 = "WELCOME TO OFFOS";
        line2 = "A TINY GRAPHICAL OS";
        line3 = "MADE WITH C AND ASM";
        line4 = "BY HEITOR PESSOA";
        line5 = "USE FILE AND TOOLS TO";
        line6 = "EXPLORE";
    } else if (viewer_file == 1) {
        line1 = "THIS IS VERSION 0.1 TEST";
        line2 = "SOME THINGS ARE BUGGED";
        line3 = "YOU CAN TEST EVERYTHING";
        line4 = "IF YOU WANT THE SOURCE CODE";
        line5 = "GO TO heitordecarvalhopessoa-debug";
        line6 = "IN GITHUB";
    } else {
        line1 = "TEST FILE";
        line2 = "TEST FILE";
        line3 = "TEST FILE";
        line4 = "TEST FILE";
        line5 = "TEST FILE";
        line6 = "TEST FILE";
    }
    draw_text(window_rect.x + 25, window_rect.y + 82, line1, color_text, 2);
    draw_text(window_rect.x + 25, window_rect.y + 122, line2, color_text, 2);
    draw_text(window_rect.x + 25, window_rect.y + 162, line3, color_text, 2);
    draw_text(window_rect.x + 25, window_rect.y + 202, line4, color_text, 2);
    draw_text(window_rect.x + 25, window_rect.y + 242, line5, color_text, 2);
    draw_text(window_rect.x + 25, window_rect.y + 282, line6, color_text, 2);
    draw_button(window_rect.x + 25, window_rect.y + 300, 105, 38, "BACK", false);
}

static void calc_push_digit(char digit) {
    int length = 0;
    if (calc_new_input) {
        calc_input[0] = '0';
        calc_input[1] = 0;
        calc_new_input = 0;
    }
    while (calc_input[length]) length++;
    if (calc_error) {
        calc_input[0] = 0;
        calc_error = 0;
        length = 0;
    }
    if (length >= 10) return;
    if (length == 1 && calc_input[0] == '0') {
        calc_input[0] = digit;
        calc_input[1] = 0;
    } else {
        calc_input[length] = digit;
        calc_input[length + 1] = 0;
    }
}

static void calc_set_operator(char operation) {
    if (calc_error) return;
    calc_new_input = 0;
    if (!calc_has_left) {
        calc_left = parse_integer(calc_input);
        calc_has_left = 1;
    } else if (calc_operator) {
        int32_t right = parse_integer(calc_input);
        if (calc_operator == '+') calc_left += right;
        if (calc_operator == '-') calc_left -= right;
        if (calc_operator == '*') calc_left *= right;
        if (calc_operator == '/' && right != 0) calc_left /= right;
        if (calc_operator == '/' && right == 0) {
            calc_error = 1;
            calc_input[0] = 'E'; calc_input[1] = 'R'; calc_input[2] = 'R'; calc_input[3] = 0;
            return;
        }
    }
    calc_operator = operation;
    calc_input[0] = 0;
}

static void calc_press(char key) {
    if (key >= '0' && key <= '9') {
        calc_push_digit(key);
    } else if (key == 'C' || key == 'c') {
        calc_input[0] = '0'; calc_input[1] = 0;
        calc_left = 0; calc_operator = 0; calc_has_left = 0; calc_error = 0; calc_new_input = 0;
    } else if (key == '+' || key == '-' || key == '*' || key == '/') {
        calc_set_operator(key);
    } else if (key == '\n' || key == '=') {
        if (calc_has_left && calc_operator && !calc_error) {
            int32_t right = parse_integer(calc_input);
            int32_t result = calc_left;
            if (calc_operator == '+') result += right;
            if (calc_operator == '-') result -= right;
            if (calc_operator == '*') result *= right;
            if (calc_operator == '/' && right != 0) result /= right;
            if (calc_operator == '/' && right == 0) {
                calc_error = 1;
                calc_input[0] = 'E'; calc_input[1] = 'R'; calc_input[2] = 'R'; calc_input[3] = 0;
            } else {
                format_integer(calc_input, result);
                calc_new_input = 1;
            }
            calc_has_left = 0;
            calc_operator = 0;
        }
    } else if (key == '\b') {
        int length = 0;
        while (calc_input[length]) length++;
        if (length > 1) calc_input[length - 1] = 0;
        else { calc_input[0] = '0'; calc_input[1] = 0; }
    }
    redraw_needed = 1;
}

static void draw_calculator(void) {
    static const char keys[16] = {
        '7','8','9','÷',
        '4','5','6','X',
        '1','2','3','-',
        '0','C','=','+'
    };
    int i;
    int x = window_rect.x + 20;
    int y = window_rect.y + 70;
    draw_window_frame("CALCULATOR");
    fill_rect(x, y, window_rect.w - 40, 52, color_white);
    draw_rect(x, y, window_rect.w - 40, 52, color_border);
    draw_text(x + 12, y + 17, calc_input, color_text, 2);
    for (i = 0; i < 16; i++) {
        char label[2];
        int col = i % 4;
        int row = i / 4;
        label[0] = keys[i];
        label[1] = 0;
        draw_button(x + col * 78, y + 70 + row * 63, 68, 52, label, false);
    }
    draw_text(x + 2, window_rect.y + window_rect.h - 26, "CALCULATOR v.test", color_muted, 1);
}

static void draw_calendar(void) {
    rtc_time_t now = read_rtc();
    int first;
    int days;
    int i;
    int x = window_rect.x + 21;
    int y = window_rect.y + 125;
    draw_window_frame("CALENDAR");
    fill_rect(window_rect.x + 20, window_rect.y + 53, 38, 34, color_button);
    draw_rect(window_rect.x + 20, window_rect.y + 53, 38, 34, color_border);
    draw_text(window_rect.x + 32, window_rect.y + 63, "<", color_text, 2);
    fill_rect(window_rect.x + window_rect.w - 58, window_rect.y + 53, 38, 34, color_button);
    draw_rect(window_rect.x + window_rect.w - 58, window_rect.y + 53, 38, 34, color_border);
    draw_text(window_rect.x + window_rect.w - 46, window_rect.y + 63, ">", color_text, 2);
    {
        char year_text[12];
        format_integer(year_text, calendar_year);
        draw_text(window_rect.x + 92, window_rect.y + 64, month_names[calendar_month], color_text, 2);
        draw_text(window_rect.x + 92, window_rect.y + 86, year_text, color_muted, 1);
    }
    for (i = 0; i < 7; i++) {
        draw_text(x + i * 52, y, weekday_names[i], color_muted, 1);
    }
    first = first_weekday(calendar_year, calendar_month);
    days = days_in_month(calendar_month, calendar_year);
    for (i = 1; i <= days; i++) {
        char number[12];
        int cell = first + i - 1;
        int cell_x = x + (cell % 7) * 52;
        int cell_y = y + 25 + (cell / 7) * 39;
        format_integer(number, i);
        if (calendar_year == now.year && calendar_month + 1 == now.month && i == now.day) {
            fill_rect(cell_x - 5, cell_y - 5, 42, 30, color_highlight);
        }
        draw_text(cell_x, cell_y, number, color_text, 2);
    }
    draw_text(window_rect.x + 22, window_rect.y + window_rect.h - 25, "CALENDAR v.test", color_muted, 1);
}

static void draw_clock(void) {
    rtc_time_t now = read_rtc();
    char hour[3];
    char minute[3];
    char second[3];
    char date[16];
    draw_window_frame("CLOCK");
    format_two_digits(hour, now.hour);
    format_two_digits(minute, now.minute);
    format_two_digits(second, now.second);
    draw_text(window_rect.x + 45, window_rect.y + 88, hour, color_text, 5);
    draw_text(window_rect.x + 125, window_rect.y + 88, ":", color_muted, 5);
    draw_text(window_rect.x + 155, window_rect.y + 88, minute, color_text, 5);
    draw_text(window_rect.x + 235, window_rect.y + 88, ":", color_muted, 5);
    draw_text(window_rect.x + 265, window_rect.y + 88, second, color_text, 5);
    format_two_digits(date, now.day);
    date[2] = '/';
    format_two_digits(date + 3, now.month);
    date[5] = '/';
    format_integer(date + 6, now.year);
    draw_text(window_rect.x + 105, window_rect.y + 190, date, color_muted, 2);
    draw_text(window_rect.x + 92, window_rect.y + 229, "CLOCK v.test", color_muted, 1);
}

static void clear_paint_canvas(void) {
    int i;
    for (i = 0; i < PAINT_W * PAINT_H; i++) paint_canvas[i] = 0;
    paint_was_cleared = 1;
}

static uint32_t paint_color(uint8_t index) {
    switch (index) {
        case 1: return pack_rgb(20, 20, 27);
        case 2: return pack_rgb(230, 61, 73);
        case 3: return pack_rgb(37, 92, 218);
        case 4: return pack_rgb(31, 147, 92);
        case 5: return pack_rgb(240, 190, 47);
        default: return color_white;
    }
}

static void draw_art(void) {
    int i;
    static const uint8_t palette[6] = {0,1,2,3,4,5};
    int palette_x = window_rect.x + 30;
    int palette_y = window_rect.y + 67;
    int canvas_x = window_rect.x + 30;
    int canvas_y = window_rect.y + 139;
    draw_window_frame("ARTAPP");
    draw_text(window_rect.x + 30, window_rect.y + 48, "COLOR", color_muted, 1);
    for (i = 0; i < 6; i++) {
        int sx = palette_x + i * 35;
        fill_rect(sx, palette_y, 25, 25, paint_color(palette[i]));
        draw_rect(sx, palette_y, 25, 25, art_color == palette[i] ? color_text : color_border);
    }
    draw_button(window_rect.x + window_rect.w - 116, palette_y - 1, 82, 29, "CLEAR", false);
    fill_rect(canvas_x, canvas_y, PAINT_W, PAINT_H, color_white);
    draw_rect(canvas_x, canvas_y, PAINT_W, PAINT_H, color_border);
    for (i = 0; i < PAINT_W * PAINT_H; i++) {
        if (paint_canvas[i]) {
            int px = i % PAINT_W;
            int py = i / PAINT_W;
            draw_pixel(canvas_x + px, canvas_y + py, paint_color(paint_canvas[i]));
        }
    }
    draw_text(window_rect.x + 30, window_rect.y + 480, "ARTAPP v.test", color_muted, 1);
}

static void draw_quit_dialog(void) {
    draw_window_frame("QUIT OFFOS");
    draw_text(window_rect.x + 25, window_rect.y + 58, "SHUT DOWN THE PC?", color_text, 2);
    draw_button(window_rect.x + 28, window_rect.y + 112, 92, 36, "CANCEL", false);
    draw_button(window_rect.x + 157, window_rect.y + 112, 92, 36, "QUIT", true);
}

static void draw_cursor(void) {
    int x = mouse_x;
    int y = mouse_y;
    uint32_t black = pack_rgb(10, 13, 24);
    uint32_t edge = color_white;

    draw_line(x, y, x, y + 15, edge);
    draw_line(x, y, x + 11, y + 10, edge);
    draw_line(x, y + 15, x + 5, y + 11, edge);
    draw_line(x + 5, y + 11, x + 10, y + 18, edge);
    draw_line(x + 10, y + 18, x + 14, y + 15, edge);
    draw_line(x + 14, y + 15, x + 9, y + 8, edge);
    draw_line(x + 9, y + 8, x + 15, y + 8, edge);
    draw_line(x + 15, y + 8, x, y, edge);
    draw_line(x + 1, y + 2, x + 1, y + 13, black);
    draw_line(x + 1, y + 2, x + 11, y + 9, black);
    draw_line(x + 2, y + 13, x + 5, y + 10, black);
    draw_line(x + 5, y + 10, x + 10, y + 17, black);
    draw_line(x + 10, y + 17, x + 12, y + 15, black);
    draw_line(x + 12, y + 15, x + 7, y + 8, black);
    draw_line(x + 7, y + 8, x + 13, y + 8, black);
}

static void render(void) {
    uint32_t y;
    uint32_t bar_height = (37u * screen_scale) >> 16;
    for (y = 0; y < fb_height; y++) {
        uint32_t x;
        uint32_t offset = y * fb_pitch_pixels;
        for (x = 0; x < fb_width; x++) framebuffer[offset + x] = color_desktop;
    }
    if (bar_height > fb_height) bar_height = fb_height;
    for (y = 0; y < bar_height; y++) {
        uint32_t x;
        uint32_t offset = y * fb_pitch_pixels;
        for (x = 0; x < fb_width; x++) framebuffer[offset + x] = color_bar;
    }
    if (bar_height >= 2) {
        uint32_t x;
        uint32_t border_row = (bar_height - 2u) * fb_pitch_pixels;
        for (x = 0; x < fb_width; x++) framebuffer[border_row + x] = color_border;
    }
    draw_text(10, 10, "OffOS", pack_rgb(49, 53, 156), 2);
    draw_text(610, 11, "FILE", pack_rgb(49, 53, 156), 1);
    draw_text(675, 11, "TOOLS", pack_rgb(49, 53, 156), 1);
    draw_text(754, 11, "QUIT", pack_rgb(49, 53, 156), 1);
    draw_home_panel();
    switch (active_window) {
        case WIN_FILES: draw_file_window(); break;
        case WIN_VIEWER: draw_viewer_window(); break;
        case WIN_CALCULATOR: draw_calculator(); break;
        case WIN_CALENDAR: draw_calendar(); break;
        case WIN_CLOCK: draw_clock(); break;
        case WIN_ART: draw_art(); break;
        case WIN_QUIT: draw_quit_dialog(); break;
        default: break;
    }
    draw_popup_menu();
    draw_cursor();
    redraw_needed = 0;
}

static void open_tool(int index) {
    if (index == 0) set_window(WIN_CALCULATOR);
    else if (index == 1) {
        rtc_time_t now = read_rtc();
        calendar_month = now.month - 1;
        calendar_year = now.year;
        set_window(WIN_CALENDAR);
    } else if (index == 2) set_window(WIN_CLOCK);
    else if (index == 3) set_window(WIN_ART);
}

static void shutdown_pc(void) {
    __asm__ volatile ("cli");
    outw(0x604, 0x2000);
    outw(0xB004, 0x2000);
    outw(0x4004, 0x3400);
    halt_forever();
}

static void process_menu_click(int x, int y) {
    int row;
    if (menu_open == 1 && x >= 596 && x < 790 && y >= 36 && y < 36 + 5 * 29) {
        row = (y - 36) / 29;
        menu_open = 0;
        if (row == 0) set_window(WIN_FILES);
        else if (row >= 1 && row <= 3) {
            viewer_file = row - 1;
            set_window(WIN_VIEWER);
        } else if (row == 4) {
            active_window = WIN_NONE;
            redraw_needed = 1;
        }
        return;
    }
    if (menu_open == 2 && x >= 674 && x < 800 && y >= 36 && y < 36 + 4 * 29) {
        row = (y - 36) / 29;
        menu_open = 0;
        open_tool(row);
        return;
    }
    menu_open = 0;
}

static void handle_click(int x, int y) {
    int i;
    if (menu_open) {
        process_menu_click(x, y);
        return;
    }
    if (y < 37) {
        if (x >= 600 && x < 665) menu_open = 1;
        else if (x >= 670 && x < 744) menu_open = 2;
        else if (x >= 748) set_window(WIN_QUIT);
        redraw_needed = 1;
        return;
    }
    if (active_window == WIN_NONE && x >= 28 && x < 258 && y >= 151 && y < 281) {
        int app = (y - 151) / 29;
        if (app >= 0 && app < 4) open_tool(app);
        return;
    }
    if (active_window == WIN_NONE) return;
    if (x < window_rect.x || x >= window_rect.x + window_rect.w ||
        y < window_rect.y || y >= window_rect.y + window_rect.h) {
        return;
    }
    if (x >= window_rect.x + window_rect.w - 36 && y < window_rect.y + 34) {
        if (active_window == WIN_QUIT) active_window = WIN_NONE;
        else active_window = WIN_NONE;
        redraw_needed = 1;
        return;
    }
    switch (active_window) {
        case WIN_FILES:
            for (i = 0; i < 3; i++) {
                int row_y = window_rect.y + 91 + i * 45;
                if (y >= row_y && y < row_y + 35) {
                    viewer_file = i;
                    set_window(WIN_VIEWER);
                    return;
                }
            }
            break;
        case WIN_VIEWER:
            if (y >= window_rect.y + 290) set_window(WIN_FILES);
            break;
        case WIN_CALCULATOR: {
            int px = window_rect.x + 20;
            int py = window_rect.y + 140;
            int col = (x - px) / 78;
            int row = (y - py) / 63;
            static const char keys[16] = {
                '7','8','9','÷',
                '4','5','6','X',
                '1','2','3','-',
                '0','C','=','+'
            };
            if (col >= 0 && col < 4 && row >= 0 && row < 4 &&
                x < px + col * 78 + 68 && y < py + row * 63 + 52) {
                calc_press(keys[row * 4 + col]);
            }
            break;
        }
        case WIN_CALENDAR:
            if (x < window_rect.x + 65 && y < window_rect.y + 100) {
                calendar_month--;
                if (calendar_month < 0) { calendar_month = 11; calendar_year--; }
                redraw_needed = 1;
            } else if (x > window_rect.x + window_rect.w - 65 && y < window_rect.y + 100) {
                calendar_month++;
                if (calendar_month > 11) { calendar_month = 0; calendar_year++; }
                redraw_needed = 1;
            }
            break;
        case WIN_ART: {
            int py = window_rect.y + 67;
            int palette_x = window_rect.x + 30;
            int canvas_x = window_rect.x + 30;
            int canvas_y = window_rect.y + 139;
            if (y >= py && y < py + 27 && x >= palette_x && x < palette_x + 210) {
                art_color = (x - palette_x) / 35;
                redraw_needed = 1;
            } else if (y >= py - 1 && y < py + 28 &&
                       x >= window_rect.x + window_rect.w - 116) {
                clear_paint_canvas();
                redraw_needed = 1;
            } else if (x >= canvas_x && x < canvas_x + PAINT_W &&
                       y >= canvas_y && y < canvas_y + PAINT_H) {
                int cx = x - canvas_x;
                int cy = y - canvas_y;
                if (cx >= 0 && cy >= 0 && cx < PAINT_W && cy < PAINT_H) {
                    paint_canvas[cy * PAINT_W + cx] = (uint8_t)art_color;
                    paint_was_cleared = 0;
                    redraw_needed = 1;
                }
            }
            break;
        }
        case WIN_QUIT:
            if (y >= window_rect.y + 105 && y < window_rect.y + 155) {
                if (x >= window_rect.x + 150) shutdown_pc();
                active_window = WIN_NONE;
                redraw_needed = 1;
            }
            break;
        default:
            break;
    }
}

static void handle_key(uint8_t key) {
    if (key == 'f' || key == 'F') {
        menu_open = 1;
        redraw_needed = 1;
        return;
    }
    if (key == 't' || key == 'T') {
        menu_open = 2;
        redraw_needed = 1;
        return;
    }
    if (key == 'q' || key == 'Q') {
        set_window(WIN_QUIT);
        return;
    }
    if (key == 27) {
        if (menu_open) menu_open = 0;
        else active_window = WIN_NONE;
        redraw_needed = 1;
        return;
    }
    if (menu_open) {
        if (key >= '1' && key <= '4' && menu_open == 2) {
            open_tool(key - '1');
        } else if (key == '\n' && menu_open == 1) {
            set_window(WIN_FILES);
        }
        return;
    }
    if (active_window == WIN_CALCULATOR) {
        calc_press((char)key);
    } else if (active_window == WIN_CALENDAR) {
        if (key == '[') {
            calendar_month--;
            if (calendar_month < 0) { calendar_month = 11; calendar_year--; }
            redraw_needed = 1;
        } else if (key == ']') {
            calendar_month++;
            if (calendar_month > 11) { calendar_month = 0; calendar_year++; }
            redraw_needed = 1;
        }
    } else if (active_window == WIN_ART) {
        if (key == 'c' || key == 'C') {
            clear_paint_canvas();
            redraw_needed = 1;
        } else if (key >= '1' && key <= '5') {
            art_color = key - '0';
            redraw_needed = 1;
        }
    } else if (active_window == WIN_QUIT) {
        if (key == 'y' || key == 'Y') shutdown_pc();
        if (key == 'n' || key == 'N') {
            active_window = WIN_NONE;
            redraw_needed = 1;
        }
    }
}

static void paint_at_mouse(void) {
    int x = mouse_x;
    int y = mouse_y;
    int canvas_x = window_rect.x + 30;
    int canvas_y = window_rect.y + 139;
    int dx;
    int dy;
    if (active_window != WIN_ART || !(mouse_buttons & 1u) ||
        x < canvas_x || x >= canvas_x + PAINT_W ||
        y < canvas_y || y >= canvas_y + PAINT_H) {
        return;
    }
    dx = x - canvas_x;
    dy = y - canvas_y;
    if (dx > 0 && dy > 0 && dx < PAINT_W - 1 && dy < PAINT_H - 1) {
        paint_canvas[dy * PAINT_W + dx] = (uint8_t)art_color;
        paint_canvas[dy * PAINT_W + dx + 1] = (uint8_t)art_color;
        paint_canvas[(dy + 1) * PAINT_W + dx] = (uint8_t)art_color;
        paint_canvas[(dy + 1) * PAINT_W + dx + 1] = (uint8_t)art_color;
        paint_was_cleared = 0;
        redraw_needed = 1;
    }
}

static void enable_interrupts(void) {
    __asm__ volatile ("sti");
}

void kernel_main(uint32_t magic, uint32_t info_address) {
    multiboot_info_t *info = (multiboot_info_t *)info_address;
    uint32_t second_seen = 0xFFFFFFFFu;
    if (magic != MULTIBOOT_BOOTLOADER_MAGIC) {
        boot_error("OFFOS: INVALID MULTIBOOT BOOT");
    }
    if (!(info->flags & (1u << 12)) || info->framebuffer_type != 1 ||
        info->framebuffer_bpp != 32) {
        boot_error("OFFOS NEEDS A 32-BIT GRUB FRAMEBUFFER");
    }
    if (info->framebuffer_addr > 0xFFFFFFFFull ||
        info->framebuffer_width < SCREEN_W || info->framebuffer_height < SCREEN_H) {
        boot_error("OFFOS NEEDS AN 800X600 FRAMEBUFFER");
    }
    framebuffer = (volatile uint32_t *)(uint32_t)info->framebuffer_addr;
    fb_pitch_pixels = info->framebuffer_pitch / 4u;
    fb_width = info->framebuffer_width;
    fb_height = info->framebuffer_height;
    {
        uint32_t scale_x = fb_width * 65536u / SCREEN_W;
        uint32_t scale_y = fb_height * 65536u / SCREEN_H;
        screen_scale = scale_x < scale_y ? scale_x : scale_y;
        fb_offset_x = (int)((fb_width - (SCREEN_W * screen_scale >> 16)) / 2u);
        fb_offset_y = (int)((fb_height - (SCREEN_H * screen_scale >> 16)) / 2u);
    }
    red_pos = info->red_field_position;
    green_pos = info->green_field_position;
    blue_pos = info->blue_field_position;
    color_desktop = pack_rgb(62, 72, 205);
    color_bar = pack_rgb(250, 250, 252);
    color_window = pack_rgb(111, 148, 190);
    color_title = pack_rgb(61, 83, 112);
    color_border = pack_rgb(30, 38, 51);
    color_text = pack_rgb(20, 24, 31);
    color_muted = pack_rgb(50, 61, 80);
    color_button = pack_rgb(184, 198, 213);
    color_white = pack_rgb(255, 255, 255);
    color_highlight = pack_rgb(145, 176, 215);

    idt_init();
    pic_remap();
    pit_init();
    mouse_init();
    {
        rtc_time_t now = read_rtc();
        calendar_month = now.month - 1;
        calendar_year = now.year;
    }
    enable_interrupts();
    for (;;) {
        uint8_t pending;
        uint8_t key;
        uint32_t ticks = timer_ticks;
        __asm__ volatile ("cli");
        pending = click_pending;
        click_pending = 0;
        if (key_tail != key_head) {
            key = key_queue[key_tail];
            key_tail = (uint8_t)((key_tail + 1u) & 63u);
        } else {
            key = 0;
        }
        __asm__ volatile ("sti");

        if (pending) handle_click(click_x, click_y);
        if (key) handle_key(key);
        if (active_window == WIN_ART && (mouse_buttons & 1u)) paint_at_mouse();
        if ((active_window == WIN_CLOCK || active_window == WIN_CALENDAR) &&
            ticks / 100u != second_seen) {
            second_seen = ticks / 100u;
            redraw_needed = 1;
        }
        if (redraw_needed) render();
        __asm__ volatile ("hlt");
    }
}