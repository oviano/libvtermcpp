#include "internal.h"
#include "utf8.h"

#include <algorithm>
#include <concepts>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <type_traits>

#undef DEBUG_REFLOW

namespace vterm {
namespace {

constexpr uint32_t unicode_space    = 0x20;
constexpr uint32_t unicode_linefeed = 0x0a;
constexpr uint32_t widechar_continuation = std::numeric_limits<uint32_t>::max();
constexpr int32_t  initial_logical_segments = 4;

// --- Internal types ---

// State of the pen at some moment in time, also used in a cell
struct ScreenPen {
    Color fg{}, bg{};

    uint32_t  bold      : 1 = 0;
    Underline underline : 2 = Underline::Off;
    uint32_t  italic    : 1 = 0;
    uint32_t  blink     : 1 = 0;
    uint32_t  reverse   : 1 = 0;
    uint32_t  conceal   : 1 = 0;
    uint32_t  strike    : 1 = 0;
    uint32_t  font      : 4 = 0; // 0 to 9
    uint32_t  small     : 1 = 0;
    Baseline  baseline  : 2 = Baseline::Normal;

    // Extra state storage that isn't strictly pen-related
    uint32_t protected_cell : 1 = 0;
    uint32_t dwl            : 1 = 0; // on a DECDWL or DECDHL line
    uint32_t dhl            : 2 = 0; // on a DECDHL line (1=top 2=bottom)
};

// Internal representation of a screen cell
struct InternalScreenCell {
    std::array<uint32_t, max_chars_per_cell> chars{};
    ScreenPen pen;
};

} // anonymous namespace

// --- Screen::Impl ---

struct Screen::Impl {
    explicit Impl(Terminal::Impl& vt_ref);

    Terminal::Impl& vt;
    State::Impl&    state;

    ScreenCallbacks* callbacks = nullptr;

    DamageSize damage_merge = DamageSize::Cell;
    // start_row == no_damage_row => no damage
    Rect damaged;
    Rect pending_scrollrect;
    int32_t  pending_scroll_downward  = 0;
    int32_t  pending_scroll_rightward = 0;

    int32_t rows = 0;
    int32_t cols = 0;

    uint32_t global_reverse : 1 = 0;
    uint32_t reflow         : 1 = 0;

    // Primary and Altscreen. buffers[1] is lazily allocated as needed
    std::array<std::vector<InternalScreenCell>, 2> buffers{};

    // buffer_idx selects buffers[0] or buffers[1], depending on altscreen
    int32_t buffer_idx = 0;

    // buffer for a single screen row used in scrollback storage callbacks
    std::vector<ScreenCell> sb_buffer;

    ScreenPen pen{};

    // The StateCallbacks subclass instance
    std::unique_ptr<StateCallbacks> state_cbs;

    // Methods
    void clearcell(InternalScreenCell& cell) const;
    [[nodiscard]] InternalScreenCell* getcell(int32_t row, int32_t col);
    [[nodiscard]] const InternalScreenCell* getcell(int32_t row, int32_t col) const;
    [[nodiscard]] std::vector<InternalScreenCell> alloc_buffer(int32_t rows, int32_t cols);
    [[nodiscard]] bool get_cell_impl(Pos pos, ScreenCell& cell) const;
    [[nodiscard]] bool moverect_internal(Rect dest, Rect src);
    [[nodiscard]] bool erase_internal(Rect rect, bool selective);
    [[nodiscard]] bool moverect_user(Rect dest, Rect src);
    [[nodiscard]] bool erase_user(Rect rect, bool selective);
    void flush_damage_impl();
    void damagerect(Rect rect);
    void damagescreen();
    void sb_pushline_from_row(int32_t row, bool continuation);
    void resize_buffer(int32_t bufidx, int32_t new_rows, int32_t new_cols, bool active, StateFields& statefields);
    void reset_default_colours(std::span<InternalScreenCell> buf);
    template<typename T>
        requires (std::same_as<T, char> || std::same_as<T, uint32_t>)
    size_t get_chars_impl(std::span<T> buf, Rect rect) const;
};

// --- Terminal::Impl ctor/dtor (needs complete Screen::Impl) ---

Terminal::Impl::Impl() = default;

Terminal::Impl::~Impl() {
    // screen references state callbacks, destroy it first
    screen.reset();
    state.reset();
}

// --- Helpers ---

void Screen::Impl::clearcell(InternalScreenCell& cell) const {
    cell.chars[0] = 0;
    cell.pen = pen;
}

InternalScreenCell* Screen::Impl::getcell(int32_t row, int32_t col) {
    if(row < 0 || row >= rows)
        return nullptr;
    if(col < 0 || col >= cols)
        return nullptr;
    return &buffers[buffer_idx][cols * row + col];
}

const InternalScreenCell* Screen::Impl::getcell(int32_t row, int32_t col) const {
    if(row < 0 || row >= rows)
        return nullptr;
    if(col < 0 || col >= cols)
        return nullptr;
    return &buffers[buffer_idx][cols * row + col];
}

std::vector<InternalScreenCell> Screen::Impl::alloc_buffer(int32_t rows, int32_t cols) {
    std::vector<InternalScreenCell> new_buffer(rows * cols);

    for(auto& cell : new_buffer)
        clearcell(cell);

    return new_buffer;
}

namespace {

// Copy pen attributes from internal ScreenPen to external ScreenCell.
// The global_reverse flag is XORed into .reverse on the way out.
constexpr void pen_to_cell_attrs(const ScreenPen& pen, ScreenCell& cell, uint32_t global_reverse) {
    cell.attrs.bold      = pen.bold;
    cell.attrs.underline = pen.underline;
    cell.attrs.italic    = pen.italic;
    cell.attrs.blink     = pen.blink;
    cell.attrs.reverse   = pen.reverse ^ global_reverse;
    cell.attrs.conceal   = pen.conceal;
    cell.attrs.strike    = pen.strike;
    cell.attrs.font      = pen.font;
    cell.attrs.small     = pen.small;
    cell.attrs.baseline  = pen.baseline;

    cell.attrs.dwl = pen.dwl;
    cell.attrs.dhl = pen.dhl;

    cell.fg = pen.fg;
    cell.bg = pen.bg;
}

// Copy cell attributes from external ScreenCell to internal ScreenPen.
// The global_reverse flag is XORed into .reverse on the way in.
constexpr void cell_attrs_to_pen(const ScreenCell& cell, ScreenPen& pen, uint32_t global_reverse) {
    pen.bold      = cell.attrs.bold;
    pen.underline = cell.attrs.underline;
    pen.italic    = cell.attrs.italic;
    pen.blink     = cell.attrs.blink;
    pen.reverse   = cell.attrs.reverse ^ global_reverse;
    pen.conceal   = cell.attrs.conceal;
    pen.strike    = cell.attrs.strike;
    pen.font      = cell.attrs.font;
    pen.small     = cell.attrs.small;
    pen.baseline  = cell.attrs.baseline;

    pen.fg = cell.fg;
    pen.bg = cell.bg;
}

} // anonymous namespace

// Internal get_cell that operates on Screen::Impl* directly
bool Screen::Impl::get_cell_impl(Pos pos, ScreenCell& cell) const {
    const InternalScreenCell* intcell = getcell(pos.row, pos.col);
    if(!intcell)
        return false;

    cell.chars = intcell->chars;

    pen_to_cell_attrs(intcell->pen, cell, global_reverse);

    const InternalScreenCell* nextcell = (pos.col < (cols - 1)) ? getcell(pos.row, pos.col + 1) : nullptr;
    if(nextcell && nextcell->chars[0] == widechar_continuation)
        cell.width = 2;
    else
        cell.width = 1;

    return true;
}

// Internal flush_damage operating on Screen::Impl
void Screen::Impl::flush_damage_impl() {
    if(pending_scrollrect.start_row != no_damage_row) {
        scroll_rect(pending_scrollrect,
            pending_scroll_downward, pending_scroll_rightward,
            [this](Rect dest, Rect src) -> bool { return moverect_user(dest, src); },
            [this](Rect r, bool selective) -> bool { return erase_user(r, selective); });

        pending_scrollrect.start_row = no_damage_row;
    }

    if(damaged.start_row != no_damage_row) {
        if(callbacks)
            callbacks->on_damage(damaged);

        damaged.start_row = no_damage_row;
    }
}

// --- Damage ---

void Screen::Impl::damagerect(Rect rect) {
    Rect emit{};

    switch(damage_merge) {
    case DamageSize::Cell:
        // Always emit damage event
        emit = rect;
        break;

    case DamageSize::Row:
        // Emit damage longer than one row. Try to merge with existing damage in
        // the same row
        if(rect.end_row > rect.start_row + 1) {
            // Bigger than 1 line - flush existing, emit this
            flush_damage_impl();
            emit = rect;
        }
        else if(damaged.start_row == no_damage_row) {
            // None stored yet
            damaged = rect;
            return;
        }
        else if(rect.start_row == damaged.start_row) {
            // Merge with the stored line
            if(damaged.start_col > rect.start_col)
                damaged.start_col = rect.start_col;
            if(damaged.end_col < rect.end_col)
                damaged.end_col = rect.end_col;
            return;
        }
        else {
            // Emit the currently stored line, store a new one
            emit = damaged;
            damaged = rect;
        }
        break;

    case DamageSize::Screen:
    case DamageSize::Scroll:
        // Never emit damage event
        if(damaged.start_row == no_damage_row)
            damaged = rect;
        else {
            damaged.expand(rect);
        }
        return;

    default:
        DEBUG_LOG("TODO: Maybe merge damage for level {}\n", to_underlying(damage_merge));
        return;
    }

    if(callbacks)
        callbacks->on_damage(emit);
}

void Screen::Impl::damagescreen() {
    damagerect({.start_row = 0, .end_row = rows, .start_col = 0, .end_col = cols});
}

// --- State callback implementations ---

// Copy internal to external representation for pushline
void Screen::Impl::sb_pushline_from_row(int32_t row, bool continuation) {
    Pos pos{};
    pos.row = row;
    for(pos.col = 0; pos.col < cols; pos.col++)
        (void)get_cell_impl(pos, sb_buffer[pos.col]);

    callbacks->on_sb_pushline(sb_buffer, continuation);
}

// StateCallbacks subclass that routes state callbacks to screen logic
namespace {
struct ScreenStateCallbacks : public StateCallbacks {
    Screen::Impl& screen;

    explicit ScreenStateCallbacks(Screen::Impl& s) : screen(s) {}

    bool on_putglyph(const GlyphInfo& info, Pos pos) override {
        InternalScreenCell* cell = screen.getcell(pos.row, pos.col);
        if(!cell)
            return false;

        int32_t i = 0;
        for(; i < static_cast<int32_t>(info.chars.size()) && i < max_chars_per_cell; i++)
            cell->chars[i] = info.chars[i];
        if(i < max_chars_per_cell)
            cell->chars[i] = 0;
        cell->pen = screen.pen;

        for(int32_t col = 1; col < info.width; col++) {
            InternalScreenCell* cont = screen.getcell(pos.row, pos.col + col);
            if(cont) cont->chars[0] = widechar_continuation;
        }

        cell->pen.protected_cell = info.protected_cell;
        cell->pen.dwl            = info.dwl;
        cell->pen.dhl            = info.dhl;

        screen.damagerect({.start_row = pos.row, .end_row = pos.row + 1, .start_col = pos.col, .end_col = pos.col + info.width});

        return true;
    }

    bool on_premove(Rect rect) override {
        if(screen.callbacks &&
           rect.start_row == 0 && rect.start_col == 0 &&
           rect.end_col == screen.cols &&
           screen.buffer_idx == bufidx_primary) {
            for(int32_t row = 0; row < rect.end_row; row++) {
                const LineInfo& lineinfo = screen.state.get_lineinfo(row);
                screen.sb_pushline_from_row(row, lineinfo.continuation);
            }
        }
        return true;
    }

    bool on_movecursor(Pos pos, Pos oldpos, bool visible) override {
        if(screen.callbacks)
            return screen.callbacks->on_movecursor(pos, oldpos, visible);
        return false;
    }

    bool on_scrollrect(Rect rect, int32_t downward, int32_t rightward) override {
        if(screen.damage_merge != DamageSize::Scroll) {
            scroll_rect(rect, downward, rightward,
                [this](Rect dest, Rect src) -> bool { return screen.moverect_internal(dest, src); },
                [this](Rect r, bool selective) -> bool { return screen.erase_internal(r, selective); });

            screen.flush_damage_impl();

            scroll_rect(rect, downward, rightward,
                [this](Rect dest, Rect src) -> bool { return screen.moverect_user(dest, src); },
                [this](Rect r, bool selective) -> bool { return screen.erase_user(r, selective); });

            return true;
        }

        if(screen.damaged.start_row != no_damage_row &&
           !rect.intersects(screen.damaged)) {
            screen.flush_damage_impl();
        }

        if(screen.pending_scrollrect.start_row == no_damage_row) {
            screen.pending_scrollrect = rect;
            screen.pending_scroll_downward  = downward;
            screen.pending_scroll_rightward = rightward;
        }
        else if(screen.pending_scrollrect == rect &&
            ((screen.pending_scroll_downward  == 0 && downward  == 0) ||
             (screen.pending_scroll_rightward == 0 && rightward == 0))) {
            screen.pending_scroll_downward  += downward;
            screen.pending_scroll_rightward += rightward;
        }
        else {
            screen.flush_damage_impl();

            screen.pending_scrollrect = rect;
            screen.pending_scroll_downward  = downward;
            screen.pending_scroll_rightward = rightward;
        }

        scroll_rect(rect, downward, rightward,
            [this](Rect dest, Rect src) -> bool { return screen.moverect_internal(dest, src); },
            [this](Rect r, bool selective) -> bool { return screen.erase_internal(r, selective); });

        if(screen.damaged.start_row == no_damage_row)
            return true;

        if(rect.contains_rect(screen.damaged)) {
            // Scroll region entirely contains the damage; just move it
            screen.damaged.move(-downward, -rightward);
            screen.damaged.clip(rect);
        }
        // Common case: vertical scroll that neatly cuts the damage region in half
        else if(rect.start_col <= screen.damaged.start_col &&
                rect.end_col   >= screen.damaged.end_col &&
                rightward == 0) {
            if(screen.damaged.start_row >= rect.start_row &&
               screen.damaged.start_row  < rect.end_row) {
                screen.damaged.start_row -= downward;
                if(screen.damaged.start_row < rect.start_row)
                    screen.damaged.start_row = rect.start_row;
                if(screen.damaged.start_row > rect.end_row)
                    screen.damaged.start_row = rect.end_row;
            }
            if(screen.damaged.end_row > rect.start_row &&
               screen.damaged.end_row < rect.end_row) {
                screen.damaged.end_row -= downward;
                if(screen.damaged.end_row < rect.start_row)
                    screen.damaged.end_row = rect.start_row;
                if(screen.damaged.end_row > rect.end_row)
                    screen.damaged.end_row = rect.end_row;
            }
        }
        else {
            DEBUG_LOG("TODO: Just flush and redo damaged=({},{}-{},{}) rect=({},{}-{},{})\n",
                screen.damaged.start_row, screen.damaged.start_col, screen.damaged.end_row, screen.damaged.end_col,
                rect.start_row, rect.start_col, rect.end_row, rect.end_col);
        }

        return true;
    }

    bool on_erase(Rect rect, bool selective) override {
        (void)screen.erase_internal(rect, selective);
        (void)screen.erase_user(rect, false);
        return true;
    }

    bool on_initpen() override {
        return false;
    }

    bool on_setpenattr(Attr attr, const Value& val) override {
        switch(attr) {
        case Attr::Bold:       screen.pen.bold      = val.boolean; return true;
        case Attr::Underline:  screen.pen.underline  = static_cast<Underline>(val.number); return true;
        case Attr::Italic:     screen.pen.italic     = val.boolean; return true;
        case Attr::Blink:      screen.pen.blink      = val.boolean; return true;
        case Attr::Reverse:    screen.pen.reverse    = val.boolean; return true;
        case Attr::Conceal:    screen.pen.conceal    = val.boolean; return true;
        case Attr::Strike:     screen.pen.strike     = val.boolean; return true;
        case Attr::Font:       screen.pen.font       = val.number;  return true;
        case Attr::Foreground: screen.pen.fg         = val.color;   return true;
        case Attr::Background: screen.pen.bg         = val.color;   return true;
        case Attr::Small:      screen.pen.small      = val.boolean; return true;
        case Attr::Baseline:   screen.pen.baseline   = static_cast<Baseline>(val.number);  return true;
        case Attr::NAttrs:     return false;
        }
        return false;
    }

    bool on_settermprop(Prop prop, const Value& val) override {
        switch(prop) {
        case Prop::AltScreen:
            if(val.boolean && screen.buffers[bufidx_altscreen].empty())
                return false;
            screen.buffer_idx = val.boolean
                ? bufidx_altscreen
                : bufidx_primary;
            // only send a damage event on disable; because during enable there's an
            // erase that sends a damage anyway
            if(!val.boolean)
                screen.damagescreen();
            break;
        case Prop::Reverse:
            screen.global_reverse = val.boolean;
            screen.damagescreen();
            break;
        default:
            ; // ignore
        }

        if(screen.callbacks)
            return screen.callbacks->on_settermprop(prop, val);

        return true;
    }

    bool on_bell() override {
        if(screen.callbacks)
            return screen.callbacks->on_bell();
        return false;
    }

    bool on_resize(int32_t new_rows, int32_t new_cols, StateFields& fields) override;

    bool on_setlineinfo(int32_t row, const LineInfo& newinfo, const LineInfo& oldinfo) override {
        if(newinfo.doublewidth != oldinfo.doublewidth ||
           newinfo.doubleheight != oldinfo.doubleheight) {
            for(int32_t col = 0; col < screen.cols; col++) {
                InternalScreenCell* cell = screen.getcell(row, col);
                if(!cell) continue;
                cell->pen.dwl = newinfo.doublewidth;
                cell->pen.dhl = newinfo.doubleheight;
            }

            screen.damagerect({.start_row = row, .end_row = row + 1, .start_col = 0, .end_col = newinfo.doublewidth ? screen.cols / 2 : screen.cols});

            if(newinfo.doublewidth)
                (void)screen.erase_internal({.start_row = row, .end_row = row + 1, .start_col = screen.cols / 2, .end_col = screen.cols}, false);
        }
        return true;
    }

    bool on_sb_clear() override {
        if(screen.callbacks)
            if(screen.callbacks->on_sb_clear())
                return true;
        return false;
    }
};
} // anonymous namespace

// --- Scroll helpers ---

bool Screen::Impl::moverect_internal(Rect dest, Rect src) {
    int32_t ncols = src.end_col - src.start_col;
    int32_t downward = src.start_row - dest.start_row;

    int32_t init_row, test_row, inc_row;
    if(downward < 0) {
        init_row = dest.end_row - 1;
        test_row = dest.start_row - 1;
        inc_row  = -1;
    }
    else {
        init_row = dest.start_row;
        test_row = dest.end_row;
        inc_row  = +1;
    }

    auto& buf = buffers[buffer_idx];
    for(int32_t row = init_row; row != test_row; row += inc_row) {
        auto dst = buf.begin() + row * cols + dest.start_col;
        auto srci = buf.begin() + (row + downward) * cols + src.start_col;
        if(dst < srci)
            std::copy(srci, srci + ncols, dst);
        else
            std::copy_backward(srci, srci + ncols, dst + ncols);
    }

    return true;
}

bool Screen::Impl::moverect_user(Rect dest, Rect src) {
    if(callbacks) {
        if(damage_merge != DamageSize::Scroll) {
            // Avoid an infinite loop
            flush_damage_impl();
        }

        if(callbacks->on_moverect(dest, src))
            return true;
    }

    damagerect(dest);

    return true;
}

bool Screen::Impl::erase_internal(Rect rect, bool selective) {

    for(int32_t row = rect.start_row; row < state.rows && row < rect.end_row; row++) {
        const LineInfo& info = state.get_lineinfo(row);

        for(int32_t col = rect.start_col; col < rect.end_col; col++) {
            InternalScreenCell* cell = getcell(row, col);
            if(!cell)
                continue;

            if(selective && cell->pen.protected_cell)
                continue;

            cell->chars[0] = 0;
            // Only copy .fg and .bg; leave things like rv in reset state
            ScreenPen newpen{};
            newpen.fg = pen.fg;
            newpen.bg = pen.bg;
            cell->pen = newpen;
            cell->pen.dwl = info.doublewidth;
            cell->pen.dhl = info.doubleheight;
        }
    }

    return true;
}

bool Screen::Impl::erase_user(Rect rect, [[maybe_unused]] bool selective) {
    damagerect(rect);
    return true;
}

// --- resize_buffer ---

namespace {

// How many cells are non-blank
// Returns the position of the first blank cell in the trailing blank end
[[nodiscard]] constexpr int32_t line_popcount(std::span<const InternalScreenCell> buffer, int32_t row, int32_t cols) {
    int32_t col = cols - 1;
    while(col >= 0 && buffer[row * cols + col].chars[0] == 0)
        col--;
    return col + 1;
}

} // anonymous namespace

void Screen::Impl::resize_buffer(int32_t bufidx, int32_t new_rows, int32_t new_cols, bool active, StateFields& statefields) {
    int32_t old_rows = rows;
    int32_t old_cols = cols;

    std::vector<InternalScreenCell>& old_buffer = buffers[bufidx];
    std::vector<LineInfo>& old_lineinfo_vec = *statefields.lineinfos[bufidx];

    std::vector<InternalScreenCell> new_buffer(new_rows * new_cols);
    std::vector<LineInfo> new_lineinfo(new_rows);

    int32_t old_row = old_rows - 1;
    int32_t new_row = new_rows - 1;

    Pos old_cursor = statefields.pos;
    Pos new_cursor = {.row = cursor_unset, .col = cursor_unset};

#ifdef DEBUG_REFLOW
    std::cerr << std::format("Resizing from {}x{} to {}x{}; cursor was at ({},{})\n",
        old_cols, old_rows, new_cols, new_rows, old_cursor.col, old_cursor.row);
#endif

    // Keep track of the final row that is known to be blank, so we know what
    // spare space we have for scrolling into
    int32_t final_blank_row = new_rows;

    while(old_row >= 0) {
        int32_t old_row_end = old_row;
        // TODO: Stop if dwl or dhl
        while(reflow && old_row >= 0 && old_lineinfo_vec[old_row].continuation)
            old_row--;
        int32_t old_row_start = old_row;
        if(old_row_start < 0)
            old_row_start = 0;

        int32_t width = 0;
        for(int32_t row = old_row_start; row <= old_row_end; row++) {
            if(reflow && row < (old_rows - 1) && old_lineinfo_vec[row + 1].continuation)
                width += old_cols;
            else
                width += line_popcount(old_buffer, row, old_cols);
        }

        if(final_blank_row == (new_row + 1) && width == 0)
            final_blank_row = new_row;

        int32_t new_height = reflow
            ? (width ? (width + new_cols - 1) / new_cols : 1)
            : 1;

        int32_t new_row_end   = new_row;
        int32_t new_row_start = new_row - new_height + 1;

        old_row = old_row_start;
        int32_t old_col = 0;

        int32_t spare_rows = new_rows - final_blank_row;

        if(new_row_start < 0 &&       // we'd fall off the top
            spare_rows >= 0 &&         // we actually have spare rows
            (!active || new_cursor.row == cursor_unset || (new_cursor.row - new_row_start) < new_rows))
        {
            // Attempt to scroll content down into the blank rows at the bottom to
            // make it fit
            int32_t downwards = -new_row_start;
            if(downwards > spare_rows)
                downwards = spare_rows;
            int32_t rowcount = new_rows - downwards;

#ifdef DEBUG_REFLOW
            std::cerr << std::format("  scroll {} rows +{} downwards\n", rowcount, downwards);
#endif

            std::copy_backward(new_buffer.begin(), new_buffer.begin() + rowcount * new_cols,
                new_buffer.begin() + (rowcount + downwards) * new_cols);
            std::copy_backward(new_lineinfo.begin(), new_lineinfo.begin() + rowcount,
                new_lineinfo.begin() + rowcount + downwards);

            new_row       += downwards;
            new_row_start += downwards;
            new_row_end   += downwards;

            if(new_cursor.row >= 0)
                new_cursor.row += downwards;

            final_blank_row += downwards;
        }

#ifdef DEBUG_REFLOW
        std::cerr << std::format("  rows [{}..{}] <- [{}..{}] width={}\n",
            new_row_start, new_row_end, old_row_start, old_row_end, width);
#endif

        if(new_row_start < 0) {
            if(old_row_start <= old_cursor.row && old_cursor.row <= old_row_end) {
                new_cursor.row = 0;
                new_cursor.col = old_cursor.col;
                if(new_cursor.col >= new_cols)
                    new_cursor.col = new_cols - 1;
            }
            break;
        }

        for(new_row = new_row_start, old_row = old_row_start; new_row <= new_row_end; new_row++) {
            int32_t count = width >= new_cols ? new_cols : width;
            width -= count;

            int32_t new_col = 0;

            while(count) {
                // TODO: Could batch-copy contiguous runs of cells instead of one-at-a-time
                new_buffer[new_row * new_cols + new_col] = old_buffer[old_row * old_cols + old_col];

                if(old_cursor.row == old_row && old_cursor.col == old_col) {
                    new_cursor.row = new_row;
                    new_cursor.col = new_col;
                }

                old_col++;
                if(old_col == old_cols) {
                    old_row++;

                    if(!reflow) {
                        new_col++;
                        break;
                    }
                    old_col = 0;
                }

                new_col++;
                count--;
            }

            if(old_row <= old_row_end && old_cursor.row == old_row && old_cursor.col >= old_col) {
                new_cursor.row = new_row;
                new_cursor.col = (old_cursor.col - old_col + new_col);
                if(new_cursor.col >= new_cols)
                    new_cursor.col = new_cols - 1;
            }

            while(new_col < new_cols) {
                clearcell(new_buffer[new_row * new_cols + new_col]);
                new_col++;
            }

            new_lineinfo[new_row].continuation = (new_row > new_row_start);
        }

        old_row = old_row_start - 1;
        new_row = new_row_start - 1;
    }

    if(old_cursor.row <= old_row) {
        // cursor would have moved entirely off the top of the screen; lets just
        // bring it within range
        new_cursor.row = 0;
        new_cursor.col = old_cursor.col;
        if(new_cursor.col >= new_cols)
            new_cursor.col = new_cols - 1;
    }

    // We really expect the cursor position to be set by now
    if(active && (new_cursor.row == cursor_unset || new_cursor.col == cursor_unset)) {
        std::cerr << "screen_resize failed to update cursor position\n";
        std::abort();
    }

    if(old_row >= 0 && bufidx == bufidx_primary) {
        // Push spare lines to scrollback buffer
        if(callbacks) {
            int32_t saved_buffer_idx = buffer_idx;
            buffer_idx = bufidx;
            for(int32_t row = 0; row <= old_row; row++) {
                const LineInfo& lineinfo = old_lineinfo_vec[row];
                sb_pushline_from_row(row, lineinfo.continuation);
            }
            buffer_idx = saved_buffer_idx;
        }
        if(active)
            statefields.pos.row -= (old_row + 1);
    }

    if(new_row >= 0 && bufidx == bufidx_primary && callbacks) {
        if(reflow) {
            // Reflow-aware backfill: pop complete logical lines from scrollback,
            // join continuation segments, and re-split at new column width
            std::vector<ScreenCell> logical_cells(old_cols * initial_logical_segments);

            while(new_row >= 0) {
                // Pop one complete logical line. Since scrollback is LIFO, the first
                // pop gives the last segment. Keep popping while continuation=true
                // to collect all segments of the logical line.
                int32_t total_segs = 0;
                bool cont = false;

                bool popresult = callbacks->on_sb_popline(
                    std::span(sb_buffer).first(static_cast<size_t>(old_cols)), cont);
                if(!popresult)
                    break;

                // Ensure capacity for first segment
                if(static_cast<int32_t>(logical_cells.size()) < old_cols) {
                    logical_cells.assign(old_cols * initial_logical_segments, ScreenCell{});
                }
                std::copy_n(sb_buffer.data(), old_cols, logical_cells.data());
                total_segs = 1;

                // If this segment is a continuation, keep popping to find the start
                while(cont) {
                    popresult = callbacks->on_sb_popline(
                        std::span(sb_buffer).first(static_cast<size_t>(old_cols)), cont);
                    if(!popresult)
                        break;

                    int32_t needed = old_cols * (total_segs + 1);
                    if(static_cast<int32_t>(logical_cells.size()) < needed) {
                        logical_cells.resize(needed * 2);
                    }
                    std::copy_n(sb_buffer.data(), old_cols, &logical_cells[old_cols * total_segs]);
                    total_segs++;
                }

                // Segments are in reverse order (last popped = first in logical line).
                // Reverse them using sb_buffer as temporary swap space.
                for(int32_t i = 0; i < total_segs / 2; i++) {
                    int32_t j = total_segs - 1 - i;
                    std::swap_ranges(&logical_cells[i * old_cols],
                        &logical_cells[i * old_cols] + old_cols,
                        &logical_cells[j * old_cols]);
                }

                // Compute content width: full old_cols for all segments except last,
                // where we trim trailing blank cells
                int32_t total_width = 0;
                for(int32_t seg = 0; seg < total_segs; seg++) {
                    if(seg < total_segs - 1) {
                        total_width += old_cols;
                    }
                    else {
                        int32_t col = old_cols - 1;
                        while(col >= 0 && logical_cells[seg * old_cols + col].chars[0] == 0)
                            col--;
                        total_width += col + 1;
                    }
                }

                int32_t new_height = total_width > 0
                    ? (total_width + new_cols - 1) / new_cols : 1;

                if(new_row - new_height + 1 < 0) {
                    // Not enough space - push the logical line back to scrollback
                    for(int32_t seg = 0; seg < total_segs; seg++)
                        callbacks->on_sb_pushline(
                            std::span(logical_cells).subspan(static_cast<size_t>(seg * old_cols), static_cast<size_t>(old_cols)),
                            seg > 0);
                    break;
                }

                // Place the reflowed logical line into buffer rows
                // [new_row - new_height + 1 .. new_row]
                int32_t src_seg = 0, src_col = 0;
                int32_t remaining = total_width;
                int32_t row_start = new_row - new_height + 1;

                for(int32_t row = row_start; row <= new_row; row++) {
                    new_lineinfo[row].continuation = (row > row_start);

                    int32_t count = remaining >= new_cols ? new_cols : remaining;
                    remaining -= count;

                    Pos pos{};
                    pos.row = row;
                    for(pos.col = 0; count > 0; pos.col++, count--) {
                        ScreenCell& src = logical_cells[src_seg * old_cols + src_col];
                        InternalScreenCell& dst = new_buffer[pos.row * new_cols + pos.col];

                        dst.chars = src.chars;

                        cell_attrs_to_pen(src, dst.pen, global_reverse);

                        if(src.width == 2 && pos.col < (new_cols - 1))
                            new_buffer[pos.row * new_cols + pos.col + 1].chars[0] = widechar_continuation;

                        src_col++;
                        if(src_col >= old_cols) {
                            src_seg++;
                            src_col = 0;
                        }
                    }

                    for( ; pos.col < new_cols; pos.col++)
                        clearcell(new_buffer[pos.row * new_cols + pos.col]);
                }

                if(active)
                    statefields.pos.row += new_height;

                new_row -= new_height;
            }

        }
        else {
            // Non-reflow backfill (original path for sb_popline without reflow)
            while(new_row >= 0) {
                bool continuation = false;
                bool popresult = callbacks->on_sb_popline(
                    std::span(sb_buffer).first(static_cast<size_t>(old_cols)), continuation);
                if(!popresult)
                    break;
                new_lineinfo[new_row].continuation = continuation;

                Pos pos{};
                pos.row = new_row;
                for(pos.col = 0; pos.col < old_cols && pos.col < new_cols; ) {
                    int32_t w = sb_buffer[pos.col].width;
                    if(w < 1) w = 1;
                    ScreenCell& src = sb_buffer[pos.col];
                    InternalScreenCell& dst = new_buffer[pos.row * new_cols + pos.col];

                    dst.chars = src.chars;

                    cell_attrs_to_pen(src, dst.pen, global_reverse);

                    if(src.width == 2 && pos.col < (new_cols - 1))
                        new_buffer[pos.row * new_cols + pos.col + 1].chars[0] = widechar_continuation;

                    pos.col += w;
                }
                for( ; pos.col < new_cols; pos.col++)
                    clearcell(new_buffer[pos.row * new_cols + pos.col]);
                new_row--;

                if(active)
                    statefields.pos.row++;
            }
        }
    }

    if(new_row >= 0) {
        // Scroll new rows back up to the top and fill in blanks at the bottom
        int32_t moverows = new_rows - new_row - 1;
        std::copy(new_buffer.begin() + (new_row + 1) * new_cols,
            new_buffer.begin() + (new_row + 1 + moverows) * new_cols,
            new_buffer.begin());
        std::copy(new_lineinfo.begin() + new_row + 1,
            new_lineinfo.begin() + new_row + 1 + moverows,
            new_lineinfo.begin());

        new_cursor.row -= (new_row + 1);

        for(new_row = moverows; new_row < new_rows; new_row++) {
            for(int32_t col = 0; col < new_cols; col++)
                clearcell(new_buffer[new_row * new_cols + col]);
            new_lineinfo[new_row] = LineInfo{};
        }
    }

    buffers[bufidx] = std::move(new_buffer);

    *statefields.lineinfos[bufidx] = std::move(new_lineinfo);

    if(active)
        statefields.pos = new_cursor;
}

// --- resize (StateCallbacks::on_resize implementation) ---

bool ScreenStateCallbacks::on_resize(int32_t new_rows, int32_t new_cols, StateFields& fields) {
    bool altscreen_active = (!screen.buffers[bufidx_altscreen].empty() &&
                             screen.buffer_idx == bufidx_altscreen);

    int32_t old_rows = screen.rows;
    int32_t old_cols = screen.cols;

    if(new_cols > old_cols) {
        // Ensure that .sb_buffer is large enough for a new or old row
        screen.sb_buffer.resize(new_cols);
    }

    screen.resize_buffer(0, new_rows, new_cols, !altscreen_active, fields);
    if(!screen.buffers[bufidx_altscreen].empty())
        screen.resize_buffer(1, new_rows, new_cols, altscreen_active, fields);
    else if(new_rows != old_rows) {
        // We don't need a full resize of the altscreen because it isn't enabled
        // but we should at least keep the lineinfo the right size
        fields.lineinfos[bufidx_altscreen]->resize(new_rows);
    }

    screen.buffer_idx = altscreen_active
        ? bufidx_altscreen
        : bufidx_primary;

    screen.rows = new_rows;
    screen.cols = new_cols;

    if(new_cols <= old_cols) {
        screen.sb_buffer.resize(new_cols);
    }

    // TODO: Maaaaybe we can optimise this if there's no reflow happening
    screen.damagescreen();

    if(screen.callbacks)
        screen.callbacks->on_resize(new_rows, new_cols);

    return true;
}

// --- Screen creation / destruction ---

Screen::Impl::Impl(Terminal::Impl& vt_ref)
    : vt(vt_ref), state(vt_ref.obtain_state()),
      rows(vt_ref.rows), cols(vt_ref.cols)
{
    global_reverse = false;
    reflow         = false;

    damaged.start_row = no_damage_row;
    pending_scrollrect.start_row = no_damage_row;

    buffers[bufidx_primary] = alloc_buffer(rows, cols);
    buffer_idx = bufidx_primary;

    sb_buffer.resize(cols);

    // Create and install state callbacks
    auto cbs = std::make_unique<ScreenStateCallbacks>(*this);
    state.callbacks = cbs.get();
    state_cbs = std::move(cbs);
    state.callbacks_has_premove = true;
    if(state.callbacks)
        state.callbacks->on_initpen();
}

Screen::Impl& Terminal::Impl::obtain_screen() {
    if(screen)
        return *screen;

    screen = std::make_unique<Screen::Impl>(*this);
    return *screen;
}

// --- get_chars_impl template ---

template<typename T>
    requires (std::same_as<T, char> || std::same_as<T, uint32_t>)
size_t Screen::Impl::get_chars_impl(std::span<T> buf, Rect rect) const {
    size_t outpos = 0;
    int32_t padding = 0;

    auto put = [&](uint32_t c) {
        if constexpr(std::is_same_v<T, char>) {
            size_t thislen = utf8_seqlen(c);
            if(!buf.empty() && outpos + thislen <= buf.size())
                outpos += fill_utf8(c, buf.subspan(outpos));
            else
                outpos += thislen;
        }
        else {
            if(!buf.empty() && outpos + 1 <= buf.size())
                buf[outpos++] = c;
            else
                outpos++;
        }
    };

    for(int32_t row = rect.start_row; row < rect.end_row; row++) {
        for(int32_t col = rect.start_col; col < rect.end_col; col++) {
            const InternalScreenCell* cell = getcell(row, col);
            if(!cell) continue;

            if(cell->chars[0] == 0)
                // Erased cell, might need a space
                padding++;
            else if(cell->chars[0] == widechar_continuation)
                // Gap behind a double-width char, do nothing
                ;
            else {
                while(padding) {
                    put(unicode_space);
                    padding--;
                }
                for(int32_t i = 0; i < max_chars_per_cell && cell->chars[i]; i++) {
                    put(cell->chars[i]);
                }
            }
        }

        if(row < rect.end_row - 1) {
            put(unicode_linefeed);
            padding = 0;
        }
    }

    return outpos;
}

// --- attrs_differ ---

namespace {

[[nodiscard]] constexpr bool has_attr(AttrMask mask, AttrMask flag) {
    return to_underlying(mask & flag) != 0;
}

[[nodiscard]] constexpr bool attrs_differ(AttrMask attrs, const InternalScreenCell& a, const InternalScreenCell& b) {
    if(has_attr(attrs, AttrMask::Bold)       && (a.pen.bold      != b.pen.bold))      return true;
    if(has_attr(attrs, AttrMask::Underline)  && (a.pen.underline  != b.pen.underline)) return true;
    if(has_attr(attrs, AttrMask::Italic)     && (a.pen.italic     != b.pen.italic))    return true;
    if(has_attr(attrs, AttrMask::Blink)      && (a.pen.blink      != b.pen.blink))     return true;
    if(has_attr(attrs, AttrMask::Reverse)    && (a.pen.reverse    != b.pen.reverse))   return true;
    if(has_attr(attrs, AttrMask::Conceal)    && (a.pen.conceal    != b.pen.conceal))   return true;
    if(has_attr(attrs, AttrMask::Strike)     && (a.pen.strike     != b.pen.strike))    return true;
    if(has_attr(attrs, AttrMask::Font)       && (a.pen.font       != b.pen.font))      return true;
    if(has_attr(attrs, AttrMask::Foreground) && a.pen.fg != b.pen.fg)  return true;
    if(has_attr(attrs, AttrMask::Background) && a.pen.bg != b.pen.bg)  return true;
    if(has_attr(attrs, AttrMask::Small)      && (a.pen.small      != b.pen.small))     return true;
    if(has_attr(attrs, AttrMask::Baseline)   && (a.pen.baseline   != b.pen.baseline))  return true;

    return false;
}

} // anonymous namespace

// --- reset_default_colours ---

void Screen::Impl::reset_default_colours(std::span<InternalScreenCell> buf) {
    for(int32_t row = 0; row < rows; row++)
        for(int32_t col = 0; col < cols; col++) {
            InternalScreenCell& cell = buf[row * cols + col];
            if(cell.pen.fg.is_default_fg())
                cell.pen.fg = pen.fg;
            if(cell.pen.bg.is_default_bg())
                cell.pen.bg = pen.bg;
        }
}

// ============================================================
// Screen public API methods
// ============================================================

void Screen::set_callbacks(ScreenCallbacks& cb) {
    impl_->callbacks = &cb;
}

void Screen::clear_callbacks() {
    impl_->callbacks = nullptr;
}

void Screen::set_fallbacks(StateFallbacks& fb) {
    impl_->state.fallbacks = &fb;
}

void Screen::clear_fallbacks() {
    impl_->state.fallbacks = nullptr;
}

void Screen::enable_altscreen(bool enabled) {
    if(impl_->buffers[bufidx_altscreen].empty() && enabled) {
        int32_t rows = impl_->vt.rows;
        int32_t cols = impl_->vt.cols;

        impl_->buffers[bufidx_altscreen] = impl_->alloc_buffer(rows, cols);
    }
}

void Screen::enable_reflow(bool enabled) {
    impl_->reflow = enabled;
}

void Screen::set_damage_merge(DamageSize size) {
    flush_damage();
    impl_->damage_merge = size;
}

void Screen::flush_damage() {
    impl_->flush_damage_impl();
}

void Screen::reset(bool hard) {
    impl_->damaged.start_row = no_damage_row;
    impl_->pending_scrollrect.start_row = no_damage_row;
    impl_->state.reset(hard);
    flush_damage();
}

bool Screen::get_cell(Pos pos, ScreenCell& cell) const {
    return impl_->get_cell_impl(pos, cell);
}

size_t Screen::get_chars(std::span<uint32_t> chars, Rect rect) const {
    return impl_->get_chars_impl(chars, rect);
}

size_t Screen::get_text(std::span<char> str, Rect rect) const {
    return impl_->get_chars_impl(str, rect);
}

bool Screen::get_attrs_extent(Rect& extent, Pos pos, AttrMask attrs) const {
    auto* target_ptr = impl_->getcell(pos.row, pos.col);
    if(!target_ptr)
        return false;
    const auto& target = *target_ptr;

    extent.start_row = pos.row;
    extent.end_row   = pos.row + 1;

    if(extent.start_col < 0)
        extent.start_col = 0;
    if(extent.end_col < 0)
        extent.end_col = impl_->cols;

    int32_t col;

    for(col = pos.col - 1; col >= extent.start_col; col--) {
        const InternalScreenCell* c = impl_->getcell(pos.row, col);
        if(!c || attrs_differ(attrs, target, *c))
            break;
    }
    extent.start_col = col + 1;

    for(col = pos.col + 1; col < extent.end_col; col++) {
        const InternalScreenCell* c = impl_->getcell(pos.row, col);
        if(!c || attrs_differ(attrs, target, *c))
            break;
    }
    extent.end_col = col;

    return true;
}

bool Screen::is_eol(Pos pos) const {
    // This cell is EOL if this and every cell to the right is blank
    for(; pos.col < impl_->cols; pos.col++) {
        const InternalScreenCell* cell = impl_->getcell(pos.row, pos.col);
        if(!cell || cell->chars[0] != 0)
            return false;
    }
    return true;
}

void Screen::convert_color_to_rgb(Color& col) const {
    if(col.is_indexed())
        (void)impl_->state.lookup_colour_palette(col.indexed.idx, col);
    col.type &= color_type::type_mask;
}

void Screen::set_default_colors(const Color& default_fg, const Color& default_bg) {
    // Set on state directly
    impl_->state.default_fg = default_fg;
    impl_->state.default_fg.type = (impl_->state.default_fg.type & ~color_type::default_mask) | color_type::default_fg;
    impl_->state.default_bg = default_bg;
    impl_->state.default_bg.type = (impl_->state.default_bg.type & ~color_type::default_mask) | color_type::default_bg;

    if(impl_->pen.fg.is_default_fg()) {
        impl_->pen.fg = default_fg;
        impl_->pen.fg.type = (impl_->pen.fg.type & ~color_type::default_mask)
                            | color_type::default_fg;
    }

    if(impl_->pen.bg.is_default_bg()) {
        impl_->pen.bg = default_bg;
        impl_->pen.bg.type = (impl_->pen.bg.type & ~color_type::default_mask)
                            | color_type::default_bg;
    }

    impl_->reset_default_colours(impl_->buffers[0]);
    if(!impl_->buffers[1].empty())
        impl_->reset_default_colours(impl_->buffers[1]);
}

} // namespace vterm
