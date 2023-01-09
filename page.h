#ifndef PAGE_H
#define PAGE_H
#include "header.h"
struct PageContent {
  String title;
  String content;
};
struct PageUpdate {
  Option<String> new_title;
  Option<String> new_content;
  PageUpdate()
    : new_title(Option<String>::None()), new_content(Option<String>::None()) {}
  PageUpdate(Option<String> new_title, Option<String> new_content)
    : new_title(new_title), new_content(new_content) {}
};
class Page {
  public:
    virtual PageContent run() {
    }
    virtual void update(Pager* pager) {}
    virtual PageUpdate update() {}
};

class InfoPage: public Page {
    PageContent content;
  public:
    PageContent run() {
      return content;
    }
    PageUpdate update() {
      return PageUpdate();
    }
    InfoPage(PageContent content): content(content) {}
};
class DynamicContentPage: public Page {
    String title;
    func_t<String> update_fn;
    time_t last_update_t = 0;
  public:
    PageContent run() {
      last_update_t = millis();
      return PageContent{title, update_fn()};
    }
    PageUpdate update() {
      String new_content = update_fn();
      Option<String> content_update = Option<String>::None();
      if(millis()-last_update_t > 1000){
        content_update = Option<String>::Some(new_content);
        last_update_t = millis();
      }
      return PageUpdate(Option<String>::None(), content_update);
    }
    DynamicContentPage(String title, func_t<String> update_fn)
      : title(title), update_fn(update_fn) {}
};
#endif