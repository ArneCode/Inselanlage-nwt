size_t Pager::get_page_n() {
  int diff = curr_page_ptr - &pages[0];
  return diff + 1;
}
void Pager::display_title(AnimatableLcd* lcd) {
  clear_row(lcd, 0);
  lcd->setCursor(0, 0);
  lcd->print(get_page_n());
  lcd->print("/");
  lcd->print(n_pages);
  lcd->print(" ");
  String title = curr_content.title;
  lcd->print(title);
  //lcd->printCentered(title,title.length()-4);
}
void Pager::display_content(AnimatableLcd* lcd) {
  clear_row(lcd, 1);
  lcd->printCentered(curr_content.content,-1,1);
}
void Pager::clear_row(AnimatableLcd* lcd, int row) {
  lcd->setCursor(0,row);
  lcd->print("                ");
  lcd->setCursor(0,row);
}
void Pager::display(AnimatableLcd* lcd) {
  lcd->clear();
  display_title(lcd);
  display_content(lcd);
}
void Pager::update(AnimatableLcd* lcd) {
  PageUpdate update = (*curr_page_ptr)->update();
  if (update.new_title.is_set) {
    curr_content.title = update.new_title.get_value();
    display_title(lcd);
  }
  if (update.new_content.is_set) {
    //Serial.println("displaying new content");
    curr_content.content = update.new_content.get_value();
    display_content(lcd);
  }
}
void Pager::next_page() {
  curr_page_ptr++;
  if (get_page_n() > n_pages) {
    curr_page_ptr = pages;
  }
  curr_content = (*curr_page_ptr)->run();
}
