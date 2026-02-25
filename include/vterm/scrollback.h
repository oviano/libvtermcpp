#ifndef VTERM_SCROLLBACK_H
#define VTERM_SCROLLBACK_H

#include "types.h"
#include <vector>

namespace vterm {

class Terminal;
class Screen;

class Scrollback {
public:
    struct Line {
        std::vector<ScreenCell> cells;
        bool continuation = false;
    };

    void set_capacity(size_t max_lines);
    [[nodiscard]] size_t capacity() const;
    [[nodiscard]] size_t size() const;
    [[nodiscard]] bool empty() const;

    // Line access (0 = oldest, size()-1 = newest)
    [[nodiscard]] const Line& line(size_t index) const;

    void clear();

    struct Impl;

private:
    friend class Terminal;
    friend class Screen;
    Scrollback() = default;
    ~Scrollback() = default;
    Scrollback(const Scrollback&) = delete;
    Scrollback& operator=(const Scrollback&) = delete;

    Impl* impl_ = nullptr;  // non-owning; lifetime managed by Terminal
};

} // namespace vterm

#endif // VTERM_SCROLLBACK_H
