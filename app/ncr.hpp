#ifndef NCR_HPP
#define NCR_HPP

#include <ncurses.h>
#include "helper.hpp"

namespace ncr {
    void init();
    void end();
    void print_top_bar(const char *fmt);
    void print_bottom_bar(const char *fmt);
}


#endif // NCR_HPP