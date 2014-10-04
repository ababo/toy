#ifndef __X86_64_VGA_H_
#define __X86_64_VGA_H_

#include <cstdint>

namespace toy {
namespace x86_64 {

class Vga {
 public:
  enum Color: uint8_t {
    kBlack = 0,
    kLowBlue = 1,
    kLowGreen = 2,
    kLowCyan = 3,
    kLowRed = 4,
    kLowMagenta = 5,
    kBrown = 6,
    kLightGray = 7,
    kDarkGray = 8,
    kHighBlue = 9,
    kHighGreen = 10,
    kHighCyan = 11,
    kHighRed = 12,
    kHighMagenta = 13,
    kHighYellow = 14,
    kWhite = 15
  };

  static const unsigned kRows = 25;
  static const unsigned kCols = 80;

  static void Initialize(Color fcolor, Color bcolor);

  static void GetCell(unsigned row, unsigned col,
                      char* chr, Color* fcolor, Color* bcolor);
  static void SetCell(unsigned row, unsigned col,
                      const char* chr, const Color* fcolor,
                      const Color* bcolor);

  static void GetCursorPos(unsigned* row, unsigned* col);
  static void SetCursorPos(unsigned row, unsigned col);

  static void Putc(uint32_t chr); // for KLog

 private:
  struct Cell {
    int8_t chr;
    uint8_t fcolor : 4;
    uint8_t bcolor : 4;
  };

  static const uint16_t kCrtcIndexPort = 0x3D4;
  static const uint16_t kCrtcDataPort = 0x3D5;

  static volatile Cell& GetCell(unsigned row, unsigned col) {
    return *(reinterpret_cast<Cell*>(0xB8000) + row * kCols + col);
  }

  static void ClearScreen();
  static void ShiftRowsUp();

  static Color fcolor_;
  static Color bcolor_;
  static unsigned cursor_row_;
  static unsigned cursor_col_;
};

}
}

#endif // __X86_64_VGA_H_
