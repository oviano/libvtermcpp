#include "scrollback_impl.h"

#include <algorithm>

namespace vterm {

// --- Scrollback::Impl method definitions ---

void Scrollback::Impl::push_line(std::span<const ScreenCell> cells, bool continuation) {
    Line line;
    line.cells.assign(cells.begin(), cells.end());
    line.continuation = continuation;

    lines.push_back(std::move(line));
    enforce_capacity();
}

bool Scrollback::Impl::pop_line(std::span<ScreenCell> cells, bool& continuation) {
    if(lines.empty())
        return false;

    const auto& line = lines.back();
    const size_t copy_cols = std::min(cells.size(), line.cells.size());

    std::copy_n(line.cells.begin(), copy_cols, cells.begin());

    for(size_t i = copy_cols; i < cells.size(); ++i) {
        cells[i] = ScreenCell{};
        cells[i].width = 1;
    }

    continuation = line.continuation;
    lines.pop_back();

    return true;
}

void Scrollback::Impl::clear() {
    lines.clear();
    push_track_start = 0;
    push_track_count = 0;
    sb_before_resize = 0;
}

void Scrollback::Impl::reflow(int32_t new_cols) {
    if(lines.empty() || new_cols <= 0)
        return;

    std::deque<Line> reflowed;

    // Process logical lines: a logical line starts with continuation=false and includes
    // all subsequent lines with continuation=true
    std::vector<ScreenCell> logical_line;

    auto flush_logical_line = [&]() {
        if(logical_line.empty())
            return;

        // Strip trailing blank cells (chars[0] == 0), matching vterm's line_popcount
        while(!logical_line.empty() && logical_line.back().chars[0] == 0)
            logical_line.pop_back();

        if(logical_line.empty()) {
            // Was an empty line — emit one blank row
            Line blank;
            blank.cells.resize(static_cast<size_t>(new_cols));
            for(auto& c : blank.cells) {
                c = ScreenCell{};
                c.width = 1;
            }
            blank.continuation = false;
            reflowed.push_back(std::move(blank));
        }
        else {
            // Split into chunks of new_cols
            size_t offset = 0;
            bool first = true;

            while(offset < logical_line.size()) {
                const size_t chunk_size = std::min(static_cast<size_t>(new_cols),
                                                   logical_line.size() - offset);

                Line row;
                row.cells.assign(logical_line.begin() + static_cast<ptrdiff_t>(offset),
                                 logical_line.begin() + static_cast<ptrdiff_t>(offset + chunk_size));

                // Pad to new_cols
                while(row.cells.size() < static_cast<size_t>(new_cols)) {
                    ScreenCell blank_cell{};
                    blank_cell.width = 1;
                    row.cells.push_back(blank_cell);
                }

                row.continuation = !first;
                first = false;

                reflowed.push_back(std::move(row));
                offset += chunk_size;
            }
        }

        logical_line.clear();
    };

    for(size_t i = 0; i < lines.size(); i++) {
        if(i > 0 && !lines[i].continuation)
            flush_logical_line();

        logical_line.insert(logical_line.end(),
                            lines[i].cells.begin(), lines[i].cells.end());
    }

    flush_logical_line();

    lines = std::move(reflowed);

    // Trim to capacity
    while(capacity > 0 && lines.size() > capacity)
        lines.pop_front();
}

void Scrollback::Impl::begin_resize() {
    sb_before_resize = lines.size();
}

void Scrollback::Impl::commit_resize(int32_t old_rows, int32_t new_rows,
                                      int32_t old_cols, int32_t new_cols) {
    if(old_cols == new_cols) {
        // Same column width — resize compensation only
        size_t sb_after = lines.size();
        size_t sb_before = sb_before_resize;

        if(new_rows < old_rows && sb_after > sb_before) {
            // Shrink: track pushed lines
            if(push_track_count == 0)
                push_track_start = sb_before;
            push_track_count += (sb_after - sb_before);
        }
        else if(new_rows > old_rows && push_track_count > 0) {
            // Grow: erase tracked pushed lines (they're orphaned duplicates)
            const size_t erase_start = push_track_start;
            const size_t erase_end = std::min(erase_start + push_track_count, lines.size());
            if(erase_start < lines.size()) {
                lines.erase(lines.begin() + static_cast<ptrdiff_t>(erase_start),
                            lines.begin() + static_cast<ptrdiff_t>(erase_end));
            }
            push_track_count = 0;
            push_track_start = 0;
        }
    }
    else {
        // Column width changed — reset tracking and reflow
        push_track_count = 0;
        push_track_start = 0;
        reflow(new_cols);
    }
}

void Scrollback::Impl::enforce_capacity() {
    while(capacity > 0 && lines.size() > capacity) {
        lines.pop_front();
        if(sb_before_resize > 0)
            sb_before_resize--;
        if(push_track_count > 0) {
            if(push_track_start > 0) {
                push_track_start--;
            }
            else {
                push_track_count--;
            }
        }
    }
}

// --- Scrollback public API ---

void Scrollback::set_capacity(size_t max_lines) {
    if(!impl_) return;
    impl_->capacity = max_lines;
    impl_->enforce_capacity();
}

size_t Scrollback::capacity() const {
    if(!impl_) return 0;
    return impl_->capacity;
}

size_t Scrollback::size() const {
    if(!impl_) return 0;
    return impl_->lines.size();
}

bool Scrollback::empty() const {
    if(!impl_) return true;
    return impl_->lines.empty();
}

const Scrollback::Line& Scrollback::line(size_t index) const {
    return impl_->lines[index];
}

void Scrollback::clear() {
    if(!impl_) return;
    impl_->clear();
}

} // namespace vterm
