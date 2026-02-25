#ifndef VTERM_CALLBACKS_H
#define VTERM_CALLBACKS_H

#include "types.h"

#include <span>
#include <string_view>

namespace vterm {

struct ParserCallbacks {
    virtual ~ParserCallbacks() = default;
    virtual int32_t on_text(std::span<const char> bytes) { return 0; }
    virtual bool on_control(uint8_t control) { return false; }
    virtual bool on_escape(std::string_view bytes) { return false; }
    virtual bool on_csi(std::string_view leader, std::span<const int64_t> args, std::string_view intermed, char command) { return false; }
    virtual bool on_osc(int32_t command, StringFragment frag) { return false; }
    virtual bool on_dcs(std::string_view command, StringFragment frag) { return false; }
    virtual bool on_apc(StringFragment frag) { return false; }
    virtual bool on_pm(StringFragment frag) { return false; }
    virtual bool on_sos(StringFragment frag) { return false; }
    virtual bool on_resize(int32_t rows, int32_t cols) { return false; }
};

struct StateCallbacks {
    virtual ~StateCallbacks() = default;
    virtual bool on_putglyph(const GlyphInfo& info, Pos pos) { return false; }
    virtual bool on_movecursor(Pos pos, Pos oldpos, bool visible) { return false; }
    virtual bool on_scrollrect(Rect rect, int32_t downward, int32_t rightward) { return false; }
    virtual bool on_moverect(Rect dest, Rect src) { return false; }
    virtual bool on_erase(Rect rect, bool selective) { return false; }
    virtual bool on_initpen() { return false; }
    virtual bool on_setpenattr(Attr attr, const Value& val) { return false; }
    virtual bool on_settermprop(Prop prop, const Value& val) { return false; }
    virtual bool on_bell() { return false; }
    virtual bool on_resize(int32_t rows, int32_t cols, StateFields& fields) { return false; }
    virtual bool on_setlineinfo(int32_t row, const LineInfo& newinfo, const LineInfo& oldinfo) { return false; }
    virtual bool on_sb_clear() { return false; }
    virtual bool on_premove(Rect dest) { return false; }
};

struct StateFallbacks {
    virtual ~StateFallbacks() = default;
    virtual bool on_control(uint8_t control) { return false; }
    virtual bool on_csi(std::string_view leader, std::span<const int64_t> args, std::string_view intermed, char command) { return false; }
    virtual bool on_osc(int32_t command, StringFragment frag) { return false; }
    virtual bool on_dcs(std::string_view command, StringFragment frag) { return false; }
    virtual bool on_apc(StringFragment frag) { return false; }
    virtual bool on_pm(StringFragment frag) { return false; }
    virtual bool on_sos(StringFragment frag) { return false; }
};

struct ScreenCallbacks {
    virtual ~ScreenCallbacks() = default;
    virtual bool on_damage(Rect rect) { return false; }
    virtual bool on_moverect(Rect dest, Rect src) { return false; }
    virtual bool on_movecursor(Pos pos, Pos oldpos, bool visible) { return false; }
    virtual bool on_settermprop(Prop prop, const Value& val) { return false; }
    virtual bool on_bell() { return false; }
    virtual bool on_resize(int32_t rows, int32_t cols) { return false; }
    virtual bool on_sb_pushline(std::span<const ScreenCell> cells, bool continuation) { return false; }
    virtual bool on_sb_popline(std::span<ScreenCell> cells, bool& continuation) { return false; }
    virtual bool on_sb_clear() { return false; }
};

struct SelectionCallbacks {
    virtual ~SelectionCallbacks() = default;
    virtual bool on_set(SelectionMask mask, StringFragment frag) { return false; }
    virtual bool on_query(SelectionMask mask) { return false; }
};

} // namespace vterm

#endif // VTERM_CALLBACKS_H
