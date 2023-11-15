#pragma once
#include "../data_structs/burda_hit.h"
#include "../data_structs/calibration.h"
#include "../data_structs/mm_hit.h"
#include "../devices/current_device.h"
#include "../other/utils.h"
#include <algorithm>
#include <cassert>
#include <queue>
#include <type_traits>

// converts the burda hit to mm_hit
// computes energy, and time in nanoseconds
class burda_to_mm_hit_adapter
{
  bool calibrate_;
  uint16_t chip_width_;
  uint16_t chip_height_;
  uint16_t last_x;
  uint16_t last_y;
  std::unique_ptr<calibration> calibrator_;
  uint64_t repetition_counter = 0;
  // an option to ignore repetetive hits (indication of chip failure)
  bool ignore_repeating_pixel = false;
  uint16_t max_faulty_repetition_count = 10;

public:
  const double fast_clock_dt = 1.5625; // nanoseconds
  const double slow_clock_dt = 25.;

  // compute energy in keV and toa in nanoseconds
  mm_hit process_hit(const burda_hit &in_hit)
  {
    double toa = in_hit.toa();
    short y = in_hit.linear_coord() / chip_width_;
    short x = in_hit.linear_coord() % chip_width_;
    return mm_hit(x, y, toa, calibrator_->compute_energy(x, y, (in_hit.tot())));
  }

  burda_to_mm_hit_adapter(calibration &&calib)
    : chip_height_(current_chip::chip_type::size_x()),
      chip_width_(current_chip::chip_type::size_y()), calibrate_(true),
      calibrator_(std::make_unique<calibration>(std::move(calib)))
  {
  }

  std::string name() { return "burda_to_mm_adapter"; }

  virtual ~burda_to_mm_hit_adapter() = default;
};