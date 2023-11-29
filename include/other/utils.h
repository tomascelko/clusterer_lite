#pragma once
#include <algorithm>
#include <climits>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <vector>

// utlility classes used across the whole project
//  for I/O and other are listed below
class not_implemented : public std::logic_error
{
public:
  not_implemented();
};

class io_utils
{
  static constexpr int CR = 13;
  static constexpr int LF = 10;

private:
  static bool is_separator(char character);

public:
  static void skip_eol(std::istream &stream);
  static void skip_comment_lines(std::istream &stream);
  static void skip_bom(std::istream &stream);
  static uint64_t read_to_buffer(const std::string &file_name, char *&buffer);
  static std::string strip(const std::string &value);
};

struct coord
{
  static constexpr uint16_t MAX_VALUE = 256;
  static constexpr uint16_t MIN_VALUE = 0;

  short x_;
  short y_;
  coord();
  coord(short x, short y);
  short x() const;
  short y() const;
  int32_t linearize() const; // use if untiled coord -> untiled linear

  int32_t
  linearize(uint32_t tile_size) const; // use if untiled coord -> tiled linear

  int32_t linearize_tiled(
      uint32_t tile_size) const; // use if tiled coord -> tiled linear

  bool is_valid_neighbor(const coord &neighbor, int32_t tile_size = 1) const;

  coord operator+(const coord &right);
};

template <typename number_type>
std::string double_to_str(number_type number, uint16_t precision = 6)
{
  std::string result;
  const uint16_t MAX_LENGTH = 20;
  int64_t number_rounded;
  std::string str;
  str.reserve(MAX_LENGTH);
  const double EPSILON = 0.0000000000000001;
  for (uint16_t i = 0; i < precision; ++i)
  {
    number *= 10;
  }
  number_rounded = (int64_t)std::round(number);
  for (uint16_t i = 0; i < MAX_LENGTH; ++i)
  {
    str.push_back((char)(48 + (number_rounded % 10)));
    if (i == precision - 1)
      str.push_back('.');
    number_rounded /= 10;
    // number = std::floor(number);
    if (number_rounded == 0)
    {
      if (i == precision - 1)
        str.push_back('0');
      break;
    }
  }
  std::reverse(str.begin(), str.end());
  return str;
}

template <typename T, class enable = void> class class_exists
{
  static constexpr bool value = false;
};

template <typename T> class class_exists<T, std::enable_if_t<(sizeof(T) > 0)>>
{
  static constexpr bool value = true;
};

static bool ends_with(const std::string &str, const std::string &suffix)
{
  return str.size() > suffix.size() &&
         str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}

template <typename T>
static typename std::vector<T>::iterator binary_find(std::vector<T> &array,
                                                     const T &item)
{
  if (array.size() == 0)
    return array.end();
  return binary_find_impl(array, item, 0, array.size());
}

template <typename T>
static typename std::vector<T>::iterator
binary_find_impl(std::vector<T> &array, const T &item, uint32_t left,
                 uint32_t right)
{
  uint32_t mid = (left + right) / 2;
  if (left == mid)
  {
    if (array[left] == item)
      return array.begin() + left;
    else
      return array.end();
  }
  if (array[mid] == item)
    return array.begin() + mid;
  if (array[mid] < item)
    return binary_find_impl(array, item, mid, right);
  return binary_find_impl(array, item, left, mid);
};

template <typename T, template <typename> class template_type>
struct is_instance_of_template
{
  template <typename U> static std::true_type test(const template_type<U> *);
  static std::false_type test(...);

  static constexpr bool value =
      decltype(test(static_cast<T *>(nullptr)))::value;
};

template <bool B, typename data_type>
typename std::enable_if<B, uint64_t>::type
get_processed_count(const data_type &data)
{
  return data.hits().size();
};

template <bool B, typename data_type>
typename std::enable_if<!B, uint64_t>::type
get_processed_count(const data_type &obj)
{
  return 1ULL;
}
