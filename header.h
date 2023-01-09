#ifndef HEADER_H
#define HEADER_H
using time_t = unsigned long;
template<typename ReturnT = void>
using func_t = ReturnT(*)();

template<typename T>
struct Option {
  private:

    T value;
    Option(): is_set(false) {}
    Option(T value): is_set(true), value(value) {}
  public:
    bool is_set;
    T get_value() {
      if (is_set) {
        return value;
      }
      Serial.println("tried to get value even though there is none");
    }
    static Option<T> None() {
      return Option();
    }
    static Option<T> Some(T value) {
      return Option(value);
    }

};
#endif