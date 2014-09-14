#include "exception"

#include <stdlib.h>

namespace std {

static terminate_handler termf = abort;

void terminate() noexcept {
  termf();
}

terminate_handler get_terminate() noexcept {
  return termf;
}

terminate_handler set_terminate(terminate_handler f) noexcept {
  terminate_handler temp = termf;
  termf = f;
  return temp;
}

}
