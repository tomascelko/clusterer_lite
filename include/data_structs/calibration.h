#include "../other/utils.h"
#include <vector>
#include <string>
#include <iomanip>
#include <string_view>
#include <iostream>
#include <cmath>
#pragma once
// object which is capable of converting ToT in ticks to deposited enegy in keV
class calibration
{
  // the files with constants required for energy computation
  std::vector<std::vector<double>> a_;
  std::vector<std::vector<double>> b_;
  std::vector<std::vector<double>> c_;
  std::vector<std::vector<double>> t_;
  static constexpr std::string_view a_suffix = "a.txt";
  static constexpr std::string_view b_suffix = "b.txt";
  static constexpr std::string_view c_suffix = "c.txt";
  static constexpr std::string_view t_suffix = "t.txt";

  void load_calib_vector(std::string &&name, const coord &chip_size, std::vector<std::vector<double>> &vector);
  bool has_last_folder_separator(const std::string &path);
  std::string add_last_folder_separator(const std::string &path);

public:
  calibration(const std::string &calib_folder, const coord &chip_size);
  // does the conversion from ToT to E[keV]
  double compute_energy(short x, short y, double tot);
};