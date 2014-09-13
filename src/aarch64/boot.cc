extern "C" void __boot() {
  *(volatile char*)0x9000000 = 'H';
  *(volatile char*)0x9000000 = 'e';
  *(volatile char*)0x9000000 = 'l';
  *(volatile char*)0x9000000 = 'l';
  *(volatile char*)0x9000000 = 'o';
  *(volatile char*)0x9000000 = ' ';
  *(volatile char*)0x9000000 = 'W';
  *(volatile char*)0x9000000 = 'o';
  *(volatile char*)0x9000000 = 'r';
  *(volatile char*)0x9000000 = 'l';
  *(volatile char*)0x9000000 = 'd';
  *(volatile char*)0x9000000 = '!';
}
