#pragma once
#include <map>
#include <stdexcept>
#include <string>

// the class stores arguments for each node in the computational architecture
// by default it support four data types of arguments (int, double, bool and
// string)
class node_args
{

  using node_args_type = std::map<std::string, std::string>;

  std::map<std::string, node_args_type> args_data_;
  void check_existence_file(const std::string &node_name,
                            const std::string &arg_name) const;

public:
  template <typename arg_type>
  arg_type get_arg(const std::string &node_name,
                   const std::string &arg_name) const
  {
    return static_cast<arg_type>(args_data_.at(node_name).at(arg_name));
  }

  // safe accessor, throws exception on not_found
  const std::map<std::string, std::string> &
  at(const std::string &node_name) const;
  // can be used for modification from outside
  std::map<std::string, std::string> &operator[](const std::string &node_name);
  // default values for parameters
  node_args();
};

// parse integral argument from string
template <>
inline int node_args::get_arg<int>(const std::string &node_name,
                                   const std::string &arg_name) const
{

  return std::stoi(args_data_.at(node_name).at(arg_name));
};

// parse integral argument from double
template <>
inline double node_args::get_arg<double>(const std::string &node_name,
                                         const std::string &arg_name) const
{
  return std::stod(args_data_.at(node_name).at(arg_name));
};

// parse string argument
template <>
inline std::string
node_args::get_arg<std::string>(const std::string &node_name,
                                const std::string &arg_name) const
{
  check_existence_file(node_name, arg_name);
  return args_data_.at(node_name).at(arg_name);
};

// parse boolean argument
template <>
inline bool node_args::get_arg<bool>(const std::string &node_name,
                                     const std::string &arg_name) const
{
  return args_data_.at(node_name).at(arg_name) == "true" ? true : false;
};