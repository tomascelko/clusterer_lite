#include "other/utils.h"

// utlility classes used across the whole project
//  for I/O and other are listed below

not_implemented::not_implemented() : std::logic_error("not yet implemented") {}

std::filesystem::path path_;
std::filesystem::path current_path_ = std::filesystem::current_path();

basic_path::basic_path() {}

basic_path::basic_path(const std::string &path) : path_(path) {}

std::string basic_path::as_absolute() const
{
  return std::filesystem::absolute(path_).string();
}

std::string basic_path::as_relative() const
{
  return std::filesystem::relative(current_path_, path_).string();
}

std::string basic_path::last_folder() const
{
  const int32_t sep_position = path_.string().find_last_of("/\\");
  std::string temp = path_.string().substr(sep_position + 1);

  return temp;
}

std::string basic_path::parent() const
{
  return std::filesystem::absolute(path_.parent_path()).string();
}

file_path::file_path() {}

file_path::file_path(const std::string &path) : basic_path(path) {}

std::string file_path::filename() const { return path_.filename().string(); }

bool io_utils::is_separator(char character)
{
  const int SEPARATORS_ASCII_IDEX = 33;
  return character < SEPARATORS_ASCII_IDEX && character > 0;
}

void io_utils::skip_eol(std::istream &stream)
{
  if (stream.peek() < 0)
    return;
  char next_char = stream.peek();
  while (is_separator(next_char))
  {
    // FIXME only works with char = 1byte encoding (UTF8)
    next_char = stream.get();
    next_char = stream.peek();
  }
}

void io_utils::skip_comment_lines(std::istream &stream)
{
  // std::cout << "skipping comments" << std::endl;
  skip_eol(stream);
  while (stream.peek() == '#')
  {
    std::string line;
    std::getline((stream), line);
    skip_eol(stream);
  }
}

void io_utils::skip_bom(std::istream &stream)
{
  char *bom = new char[3];
  if (stream.peek() == 0xEF)
    stream.read(bom, 3);
  delete bom;
}

std::string io_utils::strip(const std::string &value)
{
  uint32_t start_index = value.size();
  uint32_t end_index = 0;

  for (auto it = value.begin(); it != value.end(); ++it)
  {
    if (!is_separator(*it))
    {
      start_index = it - value.begin();
      break;
    }
  }
  for (auto it = value.rend(); it != value.rbegin(); ++it)
  {
    if (!is_separator(*it))
    {
      end_index = it - value.rbegin();
      break;
    }
  }
  if (start_index >= end_index)
    return "";
  return value.substr(start_index, end_index + 1);
}

coord::coord() {}

coord::coord(short x, short y) : x_(x), y_(y) {}

short coord::x() const { return x_; }

short coord::y() const { return y_; }

int32_t coord::linearize() const // use if untiled coord -> untiled linear
{
  return (MAX_VALUE)*x_ + y_;
}

int32_t coord::linearize(
    uint32_t tile_size) const // use if untiled coord -> tiled linear
{
  return (MAX_VALUE / tile_size) * (x_ / tile_size) + (y_ / tile_size);
}

int32_t coord::linearize_tiled(
    uint32_t tile_size) const // use if tiled coord -> tiled linear
{
  return (MAX_VALUE / tile_size) * x_ + y_;
}

bool coord::is_valid_neighbor(const coord &neighbor, int32_t tile_size) const
{
  // Note expression obtained after multiplying the whole formula by tile_size
  return (x_ + neighbor.x_ * tile_size >= MIN_VALUE &&
          x_ + neighbor.x_ * tile_size < MAX_VALUE &&
          y_ + neighbor.y_ * tile_size >= MIN_VALUE &&
          y_ + neighbor.y_ * tile_size < MAX_VALUE);
}

coord coord::operator+(const coord &right)
{
  return coord(this->x() + right.x(), this->y() + right.y());
}
