#pragma once
#include <vector>
// print the data type to a file using << operator
template <typename data_type, typename stream_type>
class data_printer
{
  std::unique_ptr<stream_type> out_stream_;
  using data_iterator = typename std::vector<data_type>::iterator;

public:
  data_printer(stream_type *print_stream) : out_stream_(std::unique_ptr<stream_type>(print_stream))
  {
  }
  std::string name()
  {
    return "printer";
  }
  void process_single_data(data_type &&data)
  {

    (*out_stream_) << data;
  }
  void process_data(data_iterator first, data_iterator last)
  {
    for (auto data_it = first; data_it != last; ++data_it)
    {
      process_single_data(std::move(*data_it));
    }
  }
  void close()
  {
    out_stream_->close();
  }
  virtual ~data_printer() = default;
};