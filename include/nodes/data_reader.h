#pragma once
#include <iostream>
#include <fstream>
#include <vector>
#include <memory>
#include "../data_structs/burda_hit.h"
// a node which reads the data from a stream using >> operator
template <typename istream_type>
class data_reader
{
protected:
  // uint64_t total_hits_read_;
  std::unique_ptr<istream_type> input_stream_;
  bool done_ = false;

public:
  data_reader(const std::string &file_name) : input_stream_(std::move(std::make_unique<istream_type>(file_name)))

  {
    check_input_stream(file_name);
    auto istream_optional = dynamic_cast<std::istream *>(input_stream_.get());
    if (istream_optional != nullptr)
    {
      // skip file header
      io_utils::skip_bom(*istream_optional);
      io_utils::skip_comment_lines(*istream_optional);
    }
  }
  bool done()
  {
    return done_;
  }
  std::string name()
  {
    return "reader";
  }
  // verify stream is opened
  void check_input_stream(const std::string &filename)
  {
    if (!input_stream_->is_open())
    {
      throw std::invalid_argument("Could not open selected input stream: '" + filename + "'");
    }
  }
  burda_hit process_hit()
  {
    burda_hit data;
    *input_stream_ >> data;
    if (!data.is_valid())
      done_ = true;
    return data;
  }
};
