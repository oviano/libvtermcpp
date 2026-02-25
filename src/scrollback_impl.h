#ifndef VTERM_SCROLLBACK_IMPL_H
#define VTERM_SCROLLBACK_IMPL_H

#include "vterm/scrollback.h"

#include <deque>
#include <span>

namespace vterm {

struct Scrollback::Impl {
    std::deque<Line> lines;
    size_t capacity = 0;  // 0 = disabled (no scrollback storage)

    // Resize compensation state
    size_t push_track_start = 0;
    size_t push_track_count = 0;
    size_t sb_before_resize = 0;  // snapshot from begin_resize()

    // Internal operations (called by Screen and Terminal)
    void push_line(std::span<const ScreenCell> cells, bool continuation);
    bool pop_line(std::span<ScreenCell> cells, bool& continuation);
    void clear();
    void reflow(int32_t new_cols);
    void begin_resize();
    void commit_resize(int32_t old_rows, int32_t new_rows,
                       int32_t old_cols, int32_t new_cols);

    void enforce_capacity();
};

} // namespace vterm

#endif // VTERM_SCROLLBACK_IMPL_H
