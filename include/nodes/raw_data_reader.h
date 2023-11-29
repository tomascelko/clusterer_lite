#pragma once
#include "../data_structs/burda_hit.h"
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>
#include <type_traits>
#include <vector>
enum class measurement_data_type
{
  KATHERINE_1_ID = 0x0,
  KATHERINE_2_ID = 0x1,
  PIXEL_MEASUREMENT_DATA = 0x4,
  PIXEL_TIMESTAMP_OFFSET = 0x5,
  CURRENT_FRAME_FINISHED = 0xC,
  LOST_PIXEL_COUNT = 0xD

};

class raw_char_buffer
{
  uint64_t index_offset = 0;
  char *data;
  uint64_t size;

public:
  raw_char_buffer() {}

  raw_char_buffer(char *data, uint64_t size) : data(data), size(size) {}

  bool read(char *read_bytes, uint32_t byte_count)
  {
    if (index_offset >= size)
      return false;

    std::memcpy(read_bytes, data + index_offset, byte_count);
    index_offset += byte_count;
    return true;
  }

  virtual ~raw_char_buffer() = default;

  bool is_open() { return size > 0; }
};

// a node which reads the data from a stream using >> operator
template <typename istream_type> class raw_data_reader
{

protected:
  // uint64_t total_hits_read_;
  std::unique_ptr<istream_type> input_source_;

  bool done_ = false;
  int64_t time_offset_ = 0;
  const uint32_t HIT_BYTE_SIZE = 6;
  bool use_io_;

public:
  raw_data_reader(const std::string &file_name)
    : input_source_(std::move(
          std::make_unique<istream_type>(file_name, std::ios::binary))),
      use_io_(true)

  {
    check_input_stream(file_name);
    auto istream_optional = dynamic_cast<std::istream *>(input_source_.get());
    if (istream_optional != nullptr)
    {
      // skip file header
      io_utils::skip_bom(*istream_optional);
      io_utils::skip_comment_lines(*istream_optional);
    }
  }

  raw_data_reader(char *hit_byte_array, uint64_t byte_array_size)
    : use_io_(false), input_source_(std::move(std::make_unique<raw_char_buffer>(
                          hit_byte_array, byte_array_size)))
  {
  }

  bool done() { return done_; }

  std::string name() { return "raw_reader"; }

  // verify stream is opened
  void check_input_stream(const std::string &filename)
  {
    if (!input_source_->is_open())
    {
      throw std::invalid_argument("Could not open selected input stream: '" +
                                  filename + "'");
    }
  }

  burda_hit parse_burda_hit(int64_t data)
  {

    data = data & 0xfffffffffffL;
    uint16_t linear_coord = (data & 0xfffffffffffL) >> 28;
    int64_t toa = ((data >> 14ULL) & 0x3fffULL) + 4096ULL * time_offset_;
    int16_t tot = (data & 0x3fffL) >> 4;
    uint8_t fast_toa = data & 0xfL;
    return burda_hit{linear_coord, toa, fast_toa, tot};
  }

  uint64_t processed = 0;

  bool parse_6byte_data_frame(uint64_t data_frame, burda_hit &hit)
  {

    switch (static_cast<measurement_data_type>(data_frame >> 44))
    {
    case measurement_data_type::PIXEL_MEASUREMENT_DATA:
    case measurement_data_type::KATHERINE_1_ID:
    case measurement_data_type::KATHERINE_2_ID:
      hit = parse_burda_hit(data_frame);
      ++processed;
      // std::cout << hit.linear_coord() << " " << hit.toa() << " " << hit.tot()
      //           << std::endl;
      return true;
      break;
    case measurement_data_type::PIXEL_TIMESTAMP_OFFSET:
      time_offset_ = data_frame & 0xffffffffUL;
      // std::cout << "Time offset updated to " << time_offset_ << std::endl;
      return false;
      break;
    case measurement_data_type::LOST_PIXEL_COUNT:
      done_ = true;
      std::cout << "Lost pixels in Katherine: " << (data_frame & 0x0fffffffff)
                << std::endl;
    default:
      // std::cout << std::hex << ( data_frame >> 44) << std::endl;
      // std:: cout << "FRAME " << (data_frame >> 44) << std::endl;
      break;
    }
    return false;
  }

  burda_hit process_hit()
  {

    uint64_t data = 0;
    burda_hit hit;
    input_source_->read(reinterpret_cast<char *>(&data), HIT_BYTE_SIZE);
    while (!parse_6byte_data_frame(data, hit) || done())
    {
      if (done())
        return burda_hit::end_token();
      if (!input_source_->read(reinterpret_cast<char *>(&data), HIT_BYTE_SIZE))
        done_ = true;
    }
    return hit;
  }
};
