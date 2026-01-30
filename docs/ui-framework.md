# User Interface Framework

This document describes the Universal User Interface (UUI) framework used in OpenDPS.

## Overview

The UUI framework provides:
- Screen-based UI organization
- Input handling (encoder, buttons)
- Widget rendering
- Focus management
- Event dispatching

## Architecture

```
+------------------+
|     uui_t        |  Global UI state
|  (UI Manager)    |
+--------+---------+
         |
         | manages
         v
+--------+---------+
|   ui_screen_t    |  Container for UI items
|    (Screens)     |
+--------+---------+
         |
         | contains
         v
+--------+---------+
|   ui_item_t      |  Base widget type
|    (Widgets)     |
+--------+---------+
         |
    +----+----+
    |         |
    v         v
+-------+ +--------+
|number | | icon   |  Concrete widgets
+-------+ +--------+
```

## Core Components

### UI Manager (`uui_t`)

The global UI state structure:

```c
typedef struct {
    ui_screen_t **screens;      // Array of screen pointers
    uint32_t num_screens;       // Number of screens
    uint32_t cur_screen;        // Current screen index
    bool is_visible;            // UI visibility flag
} uui_t;
```

### Screen (`ui_screen_t`)

A container for related UI items:

```c
typedef struct {
    char *name;                 // Screen name (for debugging)
    ui_item_t **items;          // Array of item pointers
    uint32_t num_items;         // Number of items
    int32_t cur_item;           // Currently focused item (-1 = none)
    void (*tick)(void);         // Periodic update callback
    void (*past_save)(void);    // Save settings callback
    void (*past_restore)(void); // Restore settings callback
} ui_screen_t;
```

### UI Item (`ui_item_t`)

Base structure for all widgets:

```c
typedef struct ui_item_t {
    uint16_t x, y;              // Position
    uint16_t width, height;     // Dimensions
    ui_item_type_t type;        // Item type
    bool can_focus;             // Can receive focus?
    bool is_visible;            // Currently visible?
    bool needs_redraw;          // Needs redraw?

    // Virtual functions
    void (*draw)(struct ui_item_t *item);
    void (*got_focus)(struct ui_item_t *item);
    void (*lost_focus)(struct ui_item_t *item);
    void (*handle_event)(struct ui_item_t *item, event_t event);
} ui_item_t;
```

## Widget Types

### Number Widget (`ui_number_t`)

Editable numeric value with digit selection:

```c
typedef struct ui_number_t {
    ui_item_t ui;               // Base item
    unit_t unit;                // Physical unit (V, A, etc.)
    uint16_t color;             // Text color
    tft_font_size_t font_size;  // Font size
    si_prefix_t si_prefix;      // SI prefix (milli, etc.)
    uint8_t num_digits;         // Integer digits
    uint8_t num_decimals;       // Decimal digits
    uint8_t cur_digit;          // Selected digit
    int32_t value;              // Current value
    int32_t min, max;           // Value limits
    void (*changed)(struct ui_number_t *item);  // Change callback
} ui_number_t;
```

**Usage Example:**

```c
static ui_number_t voltage_item = {
    .ui = {
        .x = 10, .y = 20,
        .type = ui_item_number,
        .can_focus = true,
    },
    .unit = unit_volt,
    .color = WHITE,
    .font_size = FONT_METER_LARGE,
    .num_digits = 2,
    .num_decimals = 2,
    .min = 0,
    .max = 50000,
    .value = 12000,
    .si_prefix = si_milli,
    .changed = on_voltage_changed,
};

number_init(&voltage_item);
```

### Icon Widget (`ui_icon_t`)

Selectable icon from a set:

```c
typedef struct ui_icon_t {
    ui_item_t ui;               // Base item
    uint16_t color;             // Icon color
    uint32_t icons_data_len;    // Icon data size
    uint32_t icons_width;       // Icon width
    uint32_t icons_height;      // Icon height
    uint32_t value;             // Selected index
    uint32_t num_icons;         // Total icons
    void (*changed)(struct ui_icon_t *item);  // Change callback
    const uint8_t *icons[];     // Icon data array
} ui_icon_t;
```

**Usage Example:**

```c
static ui_icon_t waveform_icon = {
    .ui = {
        .x = 100, .y = 50,
        .type = ui_item_icon,
        .can_focus = true,
    },
    .color = CYAN,
    .icons_width = 24,
    .icons_height = 24,
    .icons_data_len = 72,
    .num_icons = 4,
    .value = 0,
    .changed = on_waveform_changed,
    .icons = { gfx_sin, gfx_square, gfx_saw, gfx_triangle },
};

icon_init(&waveform_icon);
```

## Input Handling

### Event Types

```c
typedef enum {
    event_none = 0,
    event_rot_left,      // Encoder rotated left
    event_rot_right,     // Encoder rotated right
    event_rot_press,     // Encoder pressed
    event_rot_left_set,  // Encoder+Set rotated left
    event_rot_right_set, // Encoder+Set rotated right
    event_button_m1,     // M1 button pressed
    event_button_m2,     // M2 button pressed
    event_button_sel,    // Select button pressed
    event_ocp,           // Overcurrent event
} event_t;
```

### Event Flow

```
Button/Encoder ISR
        |
        v
  event_put(event)
        |
        v
   Event Queue
        |
        v
  Main Loop: event_get()
        |
        v
  uui_handle_event()
        |
        +---> Screen handler
        |         |
        |         v
        +---> Item handler (focused)
```

### Focus Navigation

Focus moves between items using encoder with SEL held:

```c
// In main event loop
switch (event) {
    case event_rot_left_set:
        uui_focus_prev();  // Move focus to previous item
        break;
    case event_rot_right_set:
        uui_focus_next();  // Move focus to next item
        break;
    case event_rot_left:
    case event_rot_right:
        // Pass to focused item
        if (current_item) {
            current_item->handle_event(current_item, event);
        }
        break;
}
```

## Display Rendering

### TFT Interface

The display is a 128x160 pixel ILI9163C TFT:

```c
// Coordinate system
// (0,0) at top-left
// X: 0-127 (horizontal)
// Y: 0-159 (vertical)

// Color format: BGR565
#define BLACK   0x0000
#define WHITE   0xFFFF
#define RED     0x001F
#define GREEN   0x07E0
#define BLUE    0xF800
```

### Drawing Functions

```c
// Fill rectangle
tft_fill(x, y, width, height, color);

// Draw text
tft_puts(font_size, str, x, y, width, height, color, invert);

// Draw bitmap
tft_blit(bits, x, y, width, height, color);

// Clear screen
tft_clear();
```

### Font Sizes

```c
typedef enum {
    FONT_FULL_SMALL,    // 6x8 pixels, full character set
    FONT_METER_SMALL,   // 10x16 pixels, digits only
    FONT_METER_MEDIUM,  // 14x24 pixels, digits only
    FONT_METER_LARGE,   // 22x36 pixels, digits only
} tft_font_size_t;
```

## Screen Management

### Switching Screens

```c
// Switch to screen by index
uui_set_screen(&ui, screen_index);

// Get current screen
ui_screen_t *current = uui_get_current_screen(&ui);
```

### Screen Lifecycle

```c
// Screen activation
1. Previous screen's past_save() called
2. Current screen index updated
3. New screen's past_restore() called
4. All items marked for redraw

// Screen tick (100Hz)
1. Screen's tick() callback called
2. Each item's draw() called if needs_redraw
```

## Creating a Custom Screen

### Step 1: Define Items

```c
static ui_number_t my_value = {
    .ui = { .x = 10, .y = 30, .type = ui_item_number, .can_focus = true },
    .unit = unit_volt,
    .color = WHITE,
    .font_size = FONT_METER_LARGE,
    .num_digits = 2,
    .num_decimals = 2,
    .min = 0, .max = 10000,
    .value = 5000,
    .changed = value_changed,
};
```

### Step 2: Define Screen

```c
static ui_item_t *my_items[] = {
    (ui_item_t *)&my_value,
};

static ui_screen_t my_screen = {
    .name = "My Screen",
    .items = my_items,
    .num_items = 1,
    .cur_item = 0,
    .tick = my_screen_tick,
    .past_save = my_screen_save,
    .past_restore = my_screen_restore,
};
```

### Step 3: Initialize

```c
void my_screen_init(void) {
    number_init(&my_value);
}
```

### Step 4: Register with UI

```c
static ui_screen_t *screens[] = {
    &main_screen,
    &my_screen,
    &settings_screen,
};

uui_t ui = {
    .screens = screens,
    .num_screens = 3,
    .cur_screen = 0,
};

uui_init(&ui);
```

## Units and Prefixes

### Physical Units

```c
typedef enum {
    unit_none,
    unit_volt,      // V
    unit_ampere,    // A
    unit_watt,      // W
    unit_hertz,     // Hz
    unit_second,    // s
} unit_t;
```

### SI Prefixes

```c
typedef enum {
    si_none,
    si_milli,   // m (10^-3)
    si_micro,   // u (10^-6)
    si_kilo,    // k (10^3)
    si_mega,    // M (10^6)
} si_prefix_t;
```

## Performance Considerations

- Only redraw items when `needs_redraw` is set
- Use DMA for SPI transfers to TFT
- Minimize full-screen clears
- Batch multiple item updates before refresh
