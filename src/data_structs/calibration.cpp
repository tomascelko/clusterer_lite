#include "data_structs/calibration.h"

// object which is capable of converting ToT in ticks to deposited enegy in keV

void calibration::load_calib_vector(std::string &&name, const coord &chip_size,
                                    std::vector<std::vector<double>> &vector)
{
  std::ifstream calib_stream(name);
  if (!calib_stream.is_open())
  {
    throw std::invalid_argument("The file '" + name +
                                "' can not be used for calibration");
  }
  vector.reserve(chip_size.x());
  for (uint32_t i = 0; i < chip_size.x(); i++)
  {
    std::vector<double> line;
    line.reserve(chip_size.y());
    for (uint32_t j = 0; j < chip_size.y(); j++)
    {
      double x;
      calib_stream >> x;
      line.push_back(x);
    }
    vector.emplace_back(std::move(line));
  }
}

bool calibration::has_last_folder_separator(const std::string &path)
{
  return path[path.size() - 1] == '/' || path[path.size() - 1] == '\\';
}

std::string calibration::add_last_folder_separator(const std::string &path)
{
  if (path.find('\\') != std::string::npos)
  {
    return path + '\\';
  }
  else
  {
    return path + "/";
  }
}

calibration::calibration(const std::string &calib_folder,
                         const coord &chip_size)
{
  std::string formatted_folder;
  formatted_folder = has_last_folder_separator(calib_folder)
                         ? calib_folder
                         : add_last_folder_separator(calib_folder);
  load_calib_vector(formatted_folder + std::string(a_suffix), chip_size, a_);
  load_calib_vector(formatted_folder + std::string(b_suffix), chip_size, b_);
  load_calib_vector(formatted_folder + std::string(c_suffix), chip_size, c_);
  load_calib_vector(formatted_folder + std::string(t_suffix), chip_size, t_);
}

calibration::calibration(const std::vector<std::vector<double>> &a,
                         const std::vector<std::vector<double>> &b,
                         const std::vector<std::vector<double>> &c,
                         const std::vector<std::vector<double>> &t)
  : a_(a), b_(b), c_(c), t_(t)
{
}

void calibration::set_calib(
    const std::vector<std::vector<double>> &calib_matrix,
    calibration::calib_type matrix_name)
{
  switch (matrix_name)
  {
  case calibration::calib_type::A:
    a_ = calib_matrix;
    break;
  case calibration::calib_type::B:
    b_ = calib_matrix;
    break;
  case calibration::calib_type::C:
    c_ = calib_matrix;
    break;
  case calibration::calib_type::T:
    t_ = calib_matrix;
    break;
  }
}

// does the conversion from ToT to E[keV]
double calibration::compute_energy(short x, short y, double tot)
{
  double a = a_[y][x];
  double c = c_[y][x];
  double t = t_[y][x];
  double b = b_[y][x];
  const double epsilon = 0.000000000001;
  double a2 = a;
  double b2 = (-a * t + b - tot);
  double c2 = tot * t - b * t - c;
  const double invalid_value = 0.1;
  if (std::abs(a2) < epsilon || b2 * b2 - 4 * a2 * c2 < 0)
    return invalid_value;
  return std::max((-b2 + std::sqrt(b2 * b2 - 4 * a2 * c2)) / (2 * a2),
                  invalid_value); // greater root of quad formula
}
