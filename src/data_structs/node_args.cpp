#include "data_structs/node_args.h"
// the class stores arguments for each node in the computational architecture
// by default it support four data types of arguments (int, double, bool and string)

void node_args::check_existence_file(const std::string &node_name, const std::string &arg_name) const
{
  if (args_data_.count(node_name) == 0)
  {
    throw std::invalid_argument("Argument '" + arg_name + "' for node '" + node_name + "' not foound");
  }
  if (args_data_.at(node_name).count(arg_name) == 0)
  {
    throw std::invalid_argument("Argument '" + arg_name + "' for node '" + node_name + "' not foound");
  }
}
// safe accessor, throws exception on not_found
const std::map<std::string, std::string> &node_args::at(const std::string &node_name) const
{

  return args_data_.at(node_name);
}
// can be used for modification from outside
std::map<std::string, std::string> &node_args::operator[](const std::string &node_name)
{
  return args_data_[node_name];
}
// default values for parameters
node_args::node_args()
{
  args_data_ = {
      {"reader",
       node_args_type({{"sleep_duration_full_memory", "100"}})},
      {"clusterer",
       node_args_type({{"tile_size", "1"},
                       {"max_dt", "200"},
                       {"type", "standard"}})},
  };
}
