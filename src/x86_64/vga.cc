#include "vga.h"

#include "x86_64/x86_64.h"

namespace toy {
namespace x86_64 {

void Vga::Initialize(Color fcolor, Color bcolor) {
  // make cursor visible
  SendToPort(kCrtcIndexPort, 0xE0A);
  SendToPort(kCrtcIndexPort, 0xF0B);

  fcolor_ = fcolor;
  bcolor_ = bcolor;

  ClearScreen();
  SetCursorPos(0, 0);
}

void Vga::ClearScreen() {
  for (unsigned row = 0; row < kRows; ++row)
    for (unsigned col = 0; col < kCols; ++col) {
        volatile Cell& cell = GetCell(row, col);
        cell.chr = '\0';
        cell.fcolor = fcolor_;
        cell.bcolor = bcolor_;
    }
}

void Vga::GetCell(unsigned row, unsigned col,
                  char* chr, Color* fcolor, Color* bcolor) {
  if (row >= kRows || col >= kCols) return;

  volatile Cell& cell = GetCell(row, col);

  if (chr) *chr = cell.chr;
  if (fcolor) *fcolor = static_cast<Color>(cell.fcolor);
  if (bcolor) *bcolor = static_cast<Color>(cell.bcolor);
}

void Vga::SetCell(unsigned row, unsigned col,
                  const char* chr, const Color* fcolor, const Color* bcolor) {
  if (row >= kRows || col >= kCols) return;

  volatile Cell& cell = GetCell(row, col);

  if (chr) cell.chr = *chr;
  if (fcolor) cell.fcolor = *fcolor;
  if (bcolor) cell.bcolor = *bcolor;
}

void Vga::GetCursorPos(unsigned* row, unsigned* col) {
  if (row) *row = cursor_row_;
  if (col) *col = cursor_col_;
}

void Vga::SetCursorPos(unsigned row, unsigned col) {
  if (row >= kRows || col >= kCols) return;

  cursor_row_ = row;
  cursor_col_ = col;

  unsigned off = row * kCols + col;
  SendToPort(kCrtcIndexPort, 0x0F);
  SendToPort(kCrtcDataPort, off & 0xFF);
  SendToPort(kCrtcIndexPort, 0x0E);
  SendToPort(kCrtcDataPort, (off >> 8) & 0xFF);
}

void Vga::Putc(uint32_t chr) {
  switch (chr) {
    case '\r': {
      cursor_col_ = 0;
      break;
    }

    case '\n': {
new_line:
      cursor_col_ = 0;
      ++cursor_row_;
      if (cursor_row_ == kRows) {
        ShiftRowsUp();
        --cursor_row_;
      }
      break;
    }

    // TODO: implement other escapes

    default: {
      GetCell(cursor_row_, cursor_col_).chr = static_cast<char>(chr);
      if (++cursor_col_ == kCols) goto new_line;
      break;
    }
  }

  SetCursorPos(cursor_row_, cursor_col_);
}

void Vga::ShiftRowsUp() {
  for (unsigned row = 1; row < kRows; ++row)
    for (unsigned col = 0; col < kCols; ++col) {
      volatile Cell& tcell = GetCell(row - 1, col);
      volatile Cell& fcell = GetCell(row, col);
      tcell.chr = fcell.chr;
      tcell.fcolor = fcell.fcolor;
      tcell.bcolor = fcell.bcolor;
    }

  for (unsigned col = 0; col < kCols; ++col)
    GetCell(kRows - 1, col).chr = '\0';
}

Vga::Color Vga::fcolor_;
Vga::Color Vga::bcolor_;
unsigned Vga::cursor_row_;
unsigned Vga::cursor_col_;

}
}
