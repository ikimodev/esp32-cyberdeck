#pragma once
#include <stdint.h>
#include <stddef.h>

namespace Lab {
constexpr uint32_t DEBOUNCE_MS = 30;
constexpr int VISIBLE_ROWS = 3;
namespace Oled {
constexpr int HEADER_HEIGHT = 16;
constexpr int HEADER_TEXT_Y = 4;
constexpr int CONTENT_Y = 19;
constexpr int TEXT_ROW_HEIGHT = 9;
constexpr int LIST_TEXT_Y = 20;
constexpr int LIST_ROW_HEIGHT = 11;
constexpr int FOOTER_Y = 54;
}
enum class Page { List, Details, Message, Drawing };
constexpr int DRAWING_WIDTH = 128, DRAWING_HEIGHT = 64;
constexpr size_t DRAWING_BYTES = DRAWING_WIDTH * DRAWING_HEIGHT / 8;

constexpr int hexDigit(char c) {
  return c >= '0' && c <= '9' ? c - '0' :
         c >= 'a' && c <= 'f' ? c - 'a' + 10 :
         c >= 'A' && c <= 'F' ? c - 'A' + 10 : -1;
}

// Row-major, leftmost pixel in bit 7: Adafruit_GFX drawBitmap format.
// Validate everything first so rejected input cannot corrupt the current picture.
constexpr bool decodeDrawing(const char* hex, size_t length, uint8_t* out) {
  if (length != DRAWING_BYTES * 2) return false;
  for (size_t i = 0; i < length; ++i) if (hexDigit(hex[i]) < 0) return false;
  for (size_t i = 0; i < DRAWING_BYTES; ++i)
    out[i] = uint8_t((hexDigit(hex[i * 2]) << 4) | hexDigit(hex[i * 2 + 1]));
  return true;
}

// GPIO-independent logic: the same code is exercised by firmware self-tests.
struct Button {
  bool raw = false, pressed = false;
  uint32_t changedAt = 0, count = 0;
  constexpr void begin(bool initial, uint32_t now) {
    raw = pressed = initial;
    changedAt = now;
    count = 0;
  }
  constexpr bool update(bool sample, uint32_t now) {
    if (sample != raw) { raw = sample; changedAt = now; }
    if (pressed != raw && uint32_t(now - changedAt) >= DEBOUNCE_MS) {
      pressed = raw;
      if (pressed) { ++count; return true; }
    }
    return false;
  }
};

struct UI {
  Page page = Page::List;
  int count = 0, selected = 0, first = 0;
  uint32_t messageAt = 0;
  constexpr void results(int n) {
    count = n > 0 ? n : 0;
    selected = first = 0;
    if (page == Page::Details) page = Page::List;
  }
  constexpr void back() { page = Page::List; }
  constexpr void showMessage(uint32_t now) { page = Page::Message; messageAt = now; }
  constexpr void showDrawing(uint32_t now) { page = Page::Drawing; messageAt = now; }
  constexpr bool temporary() const { return page == Page::Message || page == Page::Drawing; }
  constexpr bool expire(uint32_t now, uint32_t duration) {
    if (!temporary() || uint32_t(now - messageAt) < duration) return false;
    back(); return true;
  }
  constexpr bool input(bool up, bool down, bool ok) {
    if (ok) {
      if (page != Page::List) back();
      else if (count) page = Page::Details;
      return true;
    }
    if (page != Page::List || up == down || !count) return false;
    int next = selected + (down ? 1 : -1);
    if (next < 0 || next >= count) return false;
    selected = next;
    if (selected < first) first = selected;
    if (selected >= first + VISIBLE_ROWS) first = selected - VISIBLE_ROWS + 1;
    return true;
  }
};

// At size 1, 21 columns x 4 rows fit above the OLED footer.
constexpr bool validMessage(const char* text, size_t length) {
  if (!length || length > 84) return false;
  int row = 0, column = 0;
  bool visible = false;
  for (size_t i = 0; i < length; ++i) {
    const uint8_t c = uint8_t(text[i]);
    if (c == '\n') { ++row; column = 0; }
    else {
      if (c < 32 || c > 126) return false;
      if (column == 21) { ++row; column = 0; }
      ++column;
      if (c != ' ') visible = true;
    }
    if (row >= 4) return false;
  }
  return visible;
}
}
