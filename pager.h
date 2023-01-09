#ifndef PAGER_H
#define PAGER_H
#include "header.h"
#include "animLcd.h"
class Pager {
      Page** pages;
    size_t n_pages;
    Page** curr_page_ptr;
    PageContent curr_content;
    size_t get_page_n();
    void display_title(AnimatableLcd* lcd);
    void display_content(AnimatableLcd* lcd);
    void clear_row(AnimatableLcd* lcd, int row);
  public:
    Pager(Page* pages[], size_t n_pages): pages(pages), n_pages(n_pages){
      curr_page_ptr = pages;
      curr_content = (*curr_page_ptr)->run();
    }
    Pager(){}
    void display(AnimatableLcd* lcd);
    void update(AnimatableLcd* lcd);
    void next_page();
};
#endif