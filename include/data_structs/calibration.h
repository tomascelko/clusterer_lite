#include "../other/utils.h"
#include <cmath>
#include <iomanip>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>
#pragma once

// object which is capable of converting ToT in ticks to deposited enegy in keV
class calibration
{
  enum class calib_type
  {
    A,
    B,
    C,
    T
  };
  // the files with constants required for energy computation
  std::vector<std::vector<double>> a_;
  std::vector<std::vector<double>> b_;
  std::vector<std::vector<double>> c_;
  std::vector<std::vector<double>> t_;
  static constexpr std::string_view a_suffix = "a.txt";
  static constexpr std::string_view b_suffix = "b.txt";
  static constexpr std::string_view c_suffix = "c.txt";
  static constexpr std::string_view t_suffix = "t.txt";

  void load_calib_vector(std::string &&name, const coord &chip_size,
                         std::vector<std::vector<double>> &vector);
  bool has_last_folder_separator(const std::string &path);
  std::string add_last_folder_separator(const std::string &path);

public:
  calibration(const std::string &calib_folder, const coord &chip_size);
  calibration(const std::vector<std::vector<double>> &a,
              const std::vector<std::vector<double>> &b,
              const std::vector<std::vector<double>> &c,
              const std::vector<std::vector<double>> &t);
  // does the conversion from ToT to E[keV]
  double compute_energy(short x, short y, double tot);
  void set_calib(const std::vector<std::vector<double>> &calib_matrix,
                 calib_type matrix_name);
};