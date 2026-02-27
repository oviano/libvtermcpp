# libvtermcpp

A C++20 terminal emulator library, ported from [libvterm](http://www.leonerd.org.uk/code/libvterm/) by Paul Evans. Parses VT100/xterm escape sequences, maintains terminal state (cursor, pen attributes, scrollback), and provides programmatic access to the screen buffer. The original C implementation has been converted to idiomatic C++ — classes, RAII, type-safe enums, `std::span`, virtual callbacks — while preserving the same architecture and behaviour. Terminal emulation is decades old — the code doesn't have to be.

## Design overview

libvtermcpp retains libvterm's layered structure but uses C++ classes, RAII, `std::span`, `std::string_view`, `enum class`, and virtual callbacks instead of C function-pointer tables and manual memory management.

```
                 Terminal
                 /      \
              State     Screen
               |          |
            Parser    ScreenCallbacks
```

**Terminal** is the top-level object. It owns a **State** (cursor position, pen attributes, mode flags, color palette) and a **Screen** (cell buffer, damage tracking, scrollback integration). The terminal's built-in parser decodes incoming byte streams into control sequences, which State interprets to update internal state. Screen observes State via internal callbacks and maintains the cell grid.

All communication from the library to user code is through virtual callback structs. You subclass the callbacks you care about, register them, and receive notifications for events like cell damage, cursor movement, property changes, and scrollback.

Key design decisions:

- **No rendering.** The library maintains terminal state; your application renders it however it likes (GPU, CoreGraphics, ncurses, etc).
- **No I/O.** You feed bytes in via `write()` and receive output (terminal responses) via an output callback. The library never touches file descriptors.
- **No allocator customisation.** Uses standard `std::vector` and `std::unique_ptr` internally.
- **Value semantics where practical.** `Pos`, `Rect`, `Color`, `ScreenCell` are plain value types. `Terminal` is move-only.

## New features

### Built-in scrollback

libvtermcpp adds optional built-in scrollback storage that the original libvterm did not have. When enabled, the library manages scrollback internally — storing lines that scroll off the top of the screen, reflowing them on resize, and handling resize compensation (erasing orphaned duplicates when the terminal grows back after a shrink).

```cpp
vterm::Terminal vt(25, 80);
vterm::Scrollback& sb = vt.scrollback();
sb.set_capacity(10000);  // max lines to retain (0 = disabled, the default)

// Lines that scroll off screen are stored automatically.
// Read scrollback content:
for (size_t i = 0; i < sb.size(); i++) {
    const auto& line = sb.line(i);  // 0 = oldest
    // line.cells — vector of ScreenCell
    // line.continuation — true if this is a continuation of the previous logical line
}
```

Scrollback is disabled by default (capacity=0). When disabled, the library behaves exactly as before — scrollback is delegated entirely to the application via `ScreenCallbacks::on_sb_pushline`/`on_sb_popline`. When enabled, both the built-in storage and the callbacks fire, so applications can use the built-in storage while still observing scrollback events.

## Bug fixes over upstream libvterm

Over 40 bugs were found and fixed — first in the C codebase before porting, then during the C++ port and subsequent code review. AI-assisted analysis was used to systematically identify bugs, and all fixes have corresponding regression tests.

### Fixed in C before the port (25 bugs)

- **Resize buffer** — cursor corruption when reflow advanced past group boundary; infinite loop when scrollback cells had zero width; `old_row_start` going negative; cursor tracking missed the final row of a logical line; pushed spare lines from freed buffer instead of old buffer
- **Scroll damage tracking** — off-by-one in damaged region adjustment during scroll
- **Rect intersection** — used `>` instead of `>=` for half-open range comparison
- **Parser** — CSI parameter overflow (no cap on accumulated value); CSI argument index overflow past `CSI_ARGS_MAX`; OSC command number overflow
- **SGR pen attributes** — underline sub-parameter read without bounds check; SGR 38/48 colour commands off-by-one in minimum argument count; `CSI_ARG_HAS_MORE` loop could read past argument array
- **Keyboard** — `keyboard_unichar` used `%c` format for characters above 0x7F instead of UTF-8 encoding
- **UTF-8 decoder** — printable ASCII in mid-sequence didn't bounds-check output array
- **Screen popline** — resize path didn't use `sb_popline4` (continuation-aware variant) when available

### Fixed during C++ port and review (24 bugs)

Each fix has a regression test in `test_92_regression.cpp` and `test_93_regression_review.cpp`.

- **Wide character handling** — scrollback reflow, screen reflow, and resize could split double-width characters across row boundaries, producing corrupt cells
- **REP (CSI b)** — repeat count used character count instead of column width, and the loop could place a width-2 character past the right margin
- **UTF-8 decoder** — stale `bytes_remaining` state after C0 controls, DEL, or invalid bytes (0xFE/0xFF) interrupted multi-byte sequences; duplicate U+FFFD replacement characters on split sequences
- **Scrollback resize tracking** — `enforce_capacity` eviction didn't update the resize tracking counter; `commit_resize` unconditionally extended tracked ranges across non-contiguous regions
- **Scroll region** — `scrollregion_top` could exceed row count after vertical shrink; DECSLRM accepted left margin exceeding column count, causing undefined behaviour
- **Rect clipping** — `std::clamp` produced inverted rectangles when coordinates fell entirely outside bounds
- **Color comparison** — `operator==` only checked bit 0 of the type byte, making `default_fg` compare equal to `rgb(0,0,0)`
- **Parser** — OSC split across buffer boundaries left stale state; `string_len` underflow when escape flag was set at end of buffer
- **Constructor/initialisation** — zero or negative dimensions not validated; encoding array uninitialised before first `reset()`
- **Cursor positioning** — cursor row could go negative after reflow-shrink pushed all content to scrollback
- **Double-width row clamping** — UB when clamping cursor on a double-width row in a single-column terminal

## C++ features used

- **C++20 required** (`std::span`, `std::format`, `std::string_view`, designated initialisers, `std::to_array`)
- `enum class` with bitwise operator overloads for type-safe flags (`Modifier`, `AttrMask`, `SelectionMask`)
- `constexpr` constants and `std::array` for lookup tables
- Virtual callback structs with default no-op implementations (override only what you need)
- PIMPL pattern (`Terminal` owns `Impl` via `unique_ptr`; `State`/`Screen` are lightweight handles)
- `std::function` for the output callback
- No exceptions (error states indicated by return values)
- No RTTI required

## Building

### Requirements

- CMake 3.20+
- A C++20 compiler (Clang 16+, GCC 13+, MSVC 19.35+, Apple Clang 15+)

### As a standalone project

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)

# Run tests (enabled by default)
./build/test/libvtermcpp-test
```

To skip tests:

```bash
cmake -B build -DLIBVTERMCPP_BUILD_TESTS=OFF
```

### As a subdirectory in your project

```cmake
add_subdirectory(thirdparty/libvtermcpp)
target_link_libraries(your_target PRIVATE vtermcpp)
```

The `vtermcpp` target exports the public include path, so `#include <vterm/vterm.h>` works automatically.

### Produced artifact

A static library (`libvtermcpp.a` / `vtermcpp.lib`). No shared library option is provided.

## Testing

The test suite contains 649 tests covering parser behaviour, state management, screen operations, scrollback storage/reflow, and full vttest sequences. Some were ported from upstream libvterm; the rest were written from the terminal specs. The scrollback stress tests use golden output files to verify deterministic behaviour across resize sequences.

```bash
# Standard build + test
cmake --build build -j$(nproc) && ./build/test/libvtermcpp-test

# AddressSanitizer build
cmake -B build-asan -DCMAKE_CXX_FLAGS="-fsanitize=address -fno-omit-frame-pointer"
cmake --build build-asan -j$(nproc) && ./build-asan/test/libvtermcpp-test
```

## Usage

### Header

```cpp
#include <vterm/vterm.h>   // includes types.h, callbacks.h, terminal.h, state.h, screen.h
```

Everything is in `namespace vterm`.

### Creating a terminal

```cpp
vterm::Terminal vt(25, 80);   // rows, cols
vt.set_utf8(true);
```

`Terminal` is move-only. `State` and `Screen` are accessed by reference and must not outlive the `Terminal`.

### Feeding input

```cpp
std::string data = "Hello \x1b[1mBold\x1b[0m World";
vt.write({data.data(), data.size()});
```

`write()` returns the number of bytes consumed. Call it in a loop if you have a large buffer and want to process it incrementally.

### Capturing terminal output

When the terminal needs to send a response (e.g. cursor position report, device attributes), it calls the output callback:

```cpp
vt.set_output_callback([](std::span<const char> bytes) {
    // Forward to PTY or network socket
    ::write(pty_fd, bytes.data(), bytes.size());
});
```

### Sending keyboard and mouse input

```cpp
// Printable character with modifier
vt.keyboard_unichar('c', vterm::Modifier::Ctrl);

// Special key
vt.keyboard_key(vterm::Key::Up, vterm::Modifier::None);

// Function key
vt.keyboard_key(vterm::key_function(5), vterm::Modifier::Shift);  // Shift+F5

// Bracketed paste
vt.keyboard_start_paste();
for (char c : clipboard_text)
    vt.keyboard_unichar(c, vterm::Modifier::None);
vt.keyboard_end_paste();

// Mouse
vt.mouse_button(1, true, vterm::Modifier::None);    // left press
vt.mouse_move(10, 20, vterm::Modifier::None);        // row 10, col 20
vt.mouse_button(1, false, vterm::Modifier::None);   // left release
```

### Reading screen content

```cpp
vterm::Screen& screen = vt.screen();

// Single cell
vterm::ScreenCell cell;
screen.get_cell({0, 0}, cell);
// cell.chars[0]  — first Unicode codepoint (0 = empty)
// cell.width     — display width (1 or 2)
// cell.attrs     — bold, italic, underline, etc.
// cell.fg / .bg  — foreground / background color

// Extract UTF-8 text from a row
char buf[512];
vterm::Rect row0 = {.start_row = 0, .end_row = 1, .start_col = 0, .end_col = 80};
size_t len = screen.get_text({buf, sizeof(buf)}, row0);
std::string text(buf, len);

// Extract Unicode codepoints
uint32_t codepoints[256];
size_t count = screen.get_chars({codepoints, 256}, row0);

// Check if position is past end of content
bool eol = screen.is_eol({0, 5});
```

### Reading pen attributes and cursor

```cpp
vterm::State& state = vt.state();

// Cursor position
vterm::Pos cursor = state.cursor_pos();

// Pen attributes
vterm::Value val;
state.get_penattr(vterm::Attr::Bold, val);       // val.boolean
state.get_penattr(vterm::Attr::Underline, val);   // val.number (0-3)
state.get_penattr(vterm::Attr::Foreground, val);   // val.color

// Resolve any color to RGB
vterm::Color fg = val.color;
state.convert_color_to_rgb(fg);
uint8_t r = fg.rgb.red, g = fg.rgb.green, b = fg.rgb.blue;
```

### Colors

```cpp
// Construct colors
vterm::Color red = vterm::Color::from_rgb(255, 0, 0);
vterm::Color ansi_green = vterm::Color::from_index(2);

// Check color type
if (col.is_rgb())        { /* col.rgb.red/green/blue */ }
if (col.is_indexed())    { /* col.indexed.idx (0-255) */ }
if (col.is_default_fg()) { /* terminal default foreground */ }
if (col.is_default_bg()) { /* terminal default background */ }

// Customise the 16-color ANSI palette
state.set_palette_color(0, vterm::Color::from_rgb(40, 40, 40));  // dark black

// Change default fg/bg
state.set_default_colors(
    vterm::Color::from_rgb(220, 220, 220),
    vterm::Color::from_rgb(30, 30, 30)
);
```

### Resizing

```cpp
vt.set_size(50, 120);  // new rows, new cols
// Screen content reflows if reflow is enabled
```

### Callbacks

Register callbacks to receive notifications. All callback methods return `bool` — return `true` if you handled the event, `false` to fall through to default behaviour. Every method has a default no-op implementation, so override only what you need.

#### ScreenCallbacks — rendering integration

```cpp
struct MyScreenCallbacks : vterm::ScreenCallbacks {
    // Region needs redrawing
    bool on_damage(vterm::Rect rect) override {
        schedule_redraw(rect);
        return true;
    }

    // Cursor moved
    bool on_movecursor(vterm::Pos pos, vterm::Pos oldpos, bool visible) override {
        update_cursor(pos, visible);
        return true;
    }

    // Terminal property changed (title, cursor shape, etc.)
    bool on_settermprop(vterm::Prop prop, const vterm::Value& val) override {
        if (prop == vterm::Prop::Title)
            set_window_title(val.string.str);
        return true;
    }

    // Line scrolled off top — store in scrollback buffer
    bool on_sb_pushline(std::span<const vterm::ScreenCell> cells, bool continuation) override {
        scrollback.push_back({cells.begin(), cells.end()});
        return true;
    }

    // Terminal wants to pull a line back from scrollback
    bool on_sb_popline(std::span<vterm::ScreenCell> cells, bool& continuation) override {
        if (scrollback.empty()) return false;
        auto& line = scrollback.back();
        size_t n = std::min(line.size(), cells.size());
        std::copy_n(line.begin(), n, cells.begin());
        continuation = false;
        scrollback.pop_back();
        return true;
    }
};

MyScreenCallbacks screen_cbs;
vt.screen().set_callbacks(screen_cbs);
vt.screen().enable_altscreen(true);     // allow programs to use alternate screen
vt.screen().enable_reflow(true);        // reflow content on resize

// To unregister callbacks:
// vt.screen().clear_callbacks();
```

#### StateCallbacks — low-level state observation

```cpp
struct MyStateCallbacks : vterm::StateCallbacks {
    bool on_bell() override {
        play_bell_sound();
        return true;
    }

    bool on_setpenattr(vterm::Attr attr, const vterm::Value& val) override {
        // Track pen changes for custom rendering
        return false;  // let Screen also handle it
    }
};
```

#### StateFallbacks — handle custom/unrecognised sequences

```cpp
struct MyFallbacks : vterm::StateFallbacks {
    bool on_osc(int32_t command, vterm::StringFragment frag) override {
        if (command == 7) {     // OSC 7: current directory
            if (frag.final_)
                set_cwd(frag.str);
            return true;
        }
        return false;
    }
};
```

### Damage merging

By default, the screen reports damage per-cell. For most rendering backends, coarser granularity is more efficient:

```cpp
// Report damage as scroll operations when possible (best for most UIs)
screen.set_damage_merge(vterm::DamageSize::Scroll);

// Or report damage per-row
screen.set_damage_merge(vterm::DamageSize::Row);

// Force pending damage to be emitted now
screen.flush_damage();
```

### Attribute extent queries

Find the contiguous region sharing the same attributes as a given cell, useful for text selection or syntax-aware rendering:

```cpp
vterm::Rect extent = {.start_col = 0, .end_col = -1};  // -1 = full row width
vterm::Pos pos = {.row = 3, .col = 10};
screen.get_attrs_extent(extent, pos, vterm::AttrMask::All);
// extent now contains the column range where all attributes match
```

Use specific masks to match only certain attributes:

```cpp
screen.get_attrs_extent(extent, pos,
    vterm::AttrMask::Bold | vterm::AttrMask::Foreground);
```

## Pseudocode example: minimal terminal emulator

```cpp
#include <vterm/vterm.h>

struct Emulator : vterm::ScreenCallbacks {
    vterm::Terminal vt;

    Emulator(int rows, int cols) : vt(rows, cols) {
        vt.set_utf8(true);
        vt.scrollback().set_capacity(10000);
        vt.screen().set_callbacks(*this);
        vt.screen().enable_altscreen(true);
        vt.screen().enable_reflow(true);
        vt.screen().set_damage_merge(vterm::DamageSize::Scroll);

        vt.set_output_callback([this](std::span<const char> data) {
            send_to_pty(data);
        });
    }

    // Called when data arrives from the child process (PTY)
    void on_pty_read(std::span<const char> data) {
        vt.write(data);
    }

    // Called when the user types
    void on_key_press(uint32_t codepoint, vterm::Modifier mod) {
        vt.keyboard_unichar(codepoint, mod);
    }

    void on_special_key(vterm::Key key, vterm::Modifier mod) {
        vt.keyboard_key(key, mod);
    }

    // Render the screen (called after damage)
    void render() {
        vterm::Screen& screen = vt.screen();
        vterm::ScreenCell cell;

        for (int32_t row = 0; row < vt.rows(); row++) {
            for (int32_t col = 0; col < vt.cols(); col++) {
                screen.get_cell({row, col}, cell);

                // Skip continuation cells behind wide characters
                if (cell.chars[0] == static_cast<uint32_t>(-1))
                    continue;

                vterm::Color fg = cell.fg, bg = cell.bg;
                screen.convert_color_to_rgb(fg);
                screen.convert_color_to_rgb(bg);

                draw_cell(row, col, cell.chars, cell.width,
                          fg.rgb.red, fg.rgb.green, fg.rgb.blue,
                          bg.rgb.red, bg.rgb.green, bg.rgb.blue,
                          cell.attrs);
            }
        }
    }

    // --- ScreenCallbacks ---

    bool on_damage(vterm::Rect rect) override {
        schedule_redraw();
        return true;
    }

    bool on_movecursor(vterm::Pos pos, vterm::Pos, bool visible) override {
        update_cursor_position(pos.row, pos.col, visible);
        return true;
    }

    bool on_settermprop(vterm::Prop prop, const vterm::Value& val) override {
        if (prop == vterm::Prop::Title)
            set_window_title(std::string(val.string.str));
        else if (prop == vterm::Prop::CursorShape)
            set_cursor_shape(static_cast<vterm::CursorShape>(val.number));
        return true;
    }

    // Scrollback is handled by the built-in Scrollback class.
    // Access via vt.scrollback().line(i) for rendering.

    // Stubs — your application provides these
    void send_to_pty(std::span<const char> data);
    void schedule_redraw();
    void update_cursor_position(int row, int col, bool visible);
    void set_window_title(std::string title);
    void set_cursor_shape(vterm::CursorShape shape);
    void draw_cell(int row, int col, const std::array<uint32_t, 6>& chars,
                   int width, uint8_t fr, uint8_t fg, uint8_t fb,
                   uint8_t br, uint8_t bg, uint8_t bb,
                   vterm::CellAttrs attrs);
};
```

## API reference

### Types

| Type | Description |
|------|-------------|
| `Pos` | `{row, col}` position |
| `Rect` | `{start_row, end_row, start_col, end_col}` rectangle (half-open) |
| `Color` | Union: RGB, indexed (0-255), or default fg/bg |
| `ScreenCell` | Cell content: up to 6 codepoints, width, attributes, colors |
| `CellAttrs` | Bitfield: bold, underline, italic, blink, reverse, conceal, strike, font, small, baseline |
| `GlyphInfo` | Glyph data passed to `on_putglyph` |
| `LineInfo` | Per-line flags: doublewidth, doubleheight, continuation |
| `Value` | Union of bool, int, string fragment, or color |
| `StringFragment` | Partial string delivery: `{str, initial, final_}` |

### Enums

| Enum | Values |
|------|--------|
| `Key` | `None`, `Enter`, `Tab`, `Backspace`, `Escape`, `Up`/`Down`/`Left`/`Right`, `Ins`/`Del`/`Home`/`End`/`PageUp`/`PageDown`, `Function0`..`FunctionMax`, `KP0`..`KP9`/`KPMult`/etc. |
| `Modifier` | `None`, `Shift`, `Alt`, `Ctrl` (bitwise combinable) |
| `Attr` | `Bold`, `Underline`, `Italic`, `Blink`, `Reverse`, `Conceal`, `Strike`, `Font`, `Foreground`, `Background`, `Small`, `Baseline` |
| `Prop` | `CursorVisible`, `CursorBlink`, `AltScreen`, `Title`, `IconName`, `Reverse`, `CursorShape`, `Mouse`, `FocusReport` |
| `DamageSize` | `Cell`, `Row`, `Screen`, `Scroll` |
| `AttrMask` | `Bold`, `Underline`, ..., `All` (bitwise combinable) |
| `CursorShape` | `Block`, `Underline`, `BarLeft` |
| `MouseProp` | `None`, `Click`, `Drag`, `Move` |
| `SelectionMask` | `Clipboard`, `Primary`, `Secondary`, `Select`, `Cut0` (bitwise combinable) |

### Terminal

| Method | Description |
|--------|-------------|
| `Terminal(rows, cols)` | Construct with initial dimensions |
| `rows()` / `cols()` | Current dimensions |
| `set_size(rows, cols)` | Resize (triggers reflow if enabled) |
| `utf8()` / `set_utf8(bool)` | UTF-8 encoding mode |
| `write(span)` | Feed bytes from child process; returns bytes consumed |
| `set_output_callback(fn)` | Register handler for terminal responses |
| `keyboard_unichar(c, mod)` | Send Unicode character |
| `keyboard_key(key, mod)` | Send special key |
| `keyboard_start_paste()` / `keyboard_end_paste()` | Bracketed paste markers |
| `mouse_move(row, col, mod)` | Report mouse movement |
| `mouse_button(btn, pressed, mod)` | Report mouse button (1-based) |
| `state()` / `screen()` / `scrollback()` | Access State, Screen, and Scrollback by reference |
| `parser_set_callbacks(cb)` | Low-level parser event hooks (pass by reference) |
| `parser_clear_callbacks()` | Unregister parser callbacks |

### State

| Method | Description |
|--------|-------------|
| `set_callbacks(cb)` / `clear_callbacks()` | Register/unregister `StateCallbacks` (pass by reference) |
| `set_fallbacks(fb)` / `clear_fallbacks()` | Register/unregister `StateFallbacks` for unrecognised sequences |
| `enable_premove()` | Enable delivery of `on_premove()` callbacks |
| `reset(hard)` | Reset terminal state |
| `cursor_pos()` | Current cursor `Pos` |
| `get_penattr(attr, val)` | Read current pen attribute |
| `get_lineinfo(row)` | Double-width/height flags for a row (returns `const LineInfo&`) |
| `get_default_colors()` | Returns `ColorPair{fg, bg}` with current defaults |
| `set_default_colors(fg, bg)` | Set default foreground and background colors |
| `get_palette_color(idx)` | Returns `Color` for palette index (0-255) |
| `set_palette_color(idx, col)` | Set palette color at index |
| `set_bold_highbright(bool)` | Map bold to bright ANSI colors |
| `convert_color_to_rgb(col)` | Resolve indexed/default to RGB |
| `set_termprop(prop, val)` | Set a terminal property |
| `focus_in()` / `focus_out()` | Report focus changes |
| `set_selection_callbacks(cb, buflen)` / `clear_selection_callbacks()` | Clipboard/selection integration (pass by reference) |
| `send_selection(mask, frag)` | Respond to selection query |

### Screen

| Method | Description |
|--------|-------------|
| `set_callbacks(cb)` / `clear_callbacks()` | Register/unregister `ScreenCallbacks` (pass by reference) |
| `set_fallbacks(fb)` / `clear_fallbacks()` | Register/unregister `StateFallbacks` |
| `enable_altscreen(bool)` | Enable alternate screen buffer |
| `enable_reflow(bool)` | Reflow content on resize |
| `set_damage_merge(size)` | Damage notification granularity |
| `flush_damage()` | Force pending damage emission |
| `reset(hard)` | Reset screen |
| `get_cell(pos, cell)` | Read a single cell |
| `get_chars(span, rect)` | Extract Unicode codepoints from region |
| `get_text(span, rect)` | Extract UTF-8 text from region |
| `get_attrs_extent(rect, pos, mask)` | Find contiguous same-attribute region |
| `is_eol(pos)` | All cells from pos to end of row are blank |
| `convert_color_to_rgb(col)` | Resolve indexed/default to RGB |
| `set_default_colors(fg, bg)` | Update default colors and refresh cells |

### Scrollback

| Method | Description |
|--------|-------------|
| `set_capacity(n)` | Set maximum number of stored lines (0 = disabled) |
| `capacity()` | Current capacity |
| `size()` | Number of stored lines |
| `empty()` | True if no stored lines |
| `line(index)` | Access line by index (0 = oldest, size()-1 = newest). Returns `const Line&` with `.cells` and `.continuation` |
| `clear()` | Remove all stored lines |

## Project structure

```
libvtermcpp/
  include/vterm/
    vterm.h          Umbrella header
    types.h          Pos, Rect, Color, ScreenCell, enums
    callbacks.h      ParserCallbacks, StateCallbacks, ScreenCallbacks, etc.
    terminal.h       Terminal class
    state.h          State class
    screen.h         Screen class
    scrollback.h     Scrollback class
  src/
    internal.h       Internal types (Pen, C1, parser state, Impl structs)
    scrollback_impl.h  Scrollback::Impl definition
    utf8.h           UTF-8 encoding helpers
    terminal.cpp     Terminal construction, output, write
    parser.cpp       VT escape sequence parser
    encoding.cpp     Character set encodings (UTF-8, single-94)
    unicode.cpp      Unicode width and combining character tables
    pen.cpp          Pen attribute handling (SGR)
    state.cpp        State machine (cursor, modes, CSI/OSC/DCS dispatch)
    screen.cpp       Screen buffer, damage tracking, resize/reflow
    scrollback.cpp   Scrollback storage, reflow, resize compensation
    keyboard.cpp     Keyboard input → escape sequence generation
    mouse.cpp        Mouse input → escape sequence generation
  test/
    test.h           Zero-dependency single-header test framework
    harness.h        Test helpers and assertion macros
    main.cpp         Test entry point
    golden/          Golden output files for scrollback stress tests
    test_*.cpp       93 files, 649 tests
  CMakeLists.txt
```
