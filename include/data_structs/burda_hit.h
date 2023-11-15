#pragma once
#include <memory>
#include <vector>
#include <iostream>
#include "../other/utils.h"
// a memory efficient hit information format, resulting from Katherine readout
// however, it is difficult to analyze it directly
class burda_hit
{
  // linearized spatial coordinate of a pixel
  uint16_t linear_coord_;
  // number of ticks of the slow clock
  int64_t toa_;
  // number of ticks of te fast clock
  short fast_toa_;
  // time over threshold in ticks
  int16_t tot_; // can be zero because of chip error, so we set invalid value to -1
public:
  burda_hit(uint16_t linear_coord, int64_t toa, short fast_toa, int16_t tot);
  burda_hit(std::istream *in_stream);
  static constexpr uint64_t avg_size();

  static burda_hit end_token();
  burda_hit();
  bool is_valid();
  uint16_t linear_coord() const;
  static constexpr double fast_clock_dt = 1.5625;

  static constexpr double slow_clock_dt = 25.;
  uint64_t tick_toa() const;
  double toa() const;
  double time() const;

  void update_time(uint64_t new_toa, short fast_toa);
  short fast_toa() const;
  int16_t tot() const;
  uint32_t size() const;
  bool operator<(const burda_hit &other) const;
  bool operator==(const burda_hit &other) const;
};
// serialization of the hit
std::ostream &operator<<(std::ostream &os, const burda_hit &hit);
// deserialization of the hit
std::istream &operator>>(std::istream &is, burda_hit &hit);