#include "data_structs/mm_hit.h"

// suring conversion, the burda_hit is often converted to mm_hit
// which is easier to use for analysis

mm_hit::mm_hit(short x, short y, double toa, double e)
  : coord_(x, y), toa_(toa), e_(e)
{
}

mm_hit::mm_hit() {}

const coord &mm_hit::coordinates() const { return coord_; }

short mm_hit::x() const { return coord_.x_; }

short mm_hit::y() const { return coord_.y_; }

double mm_hit::time() const { return toa(); }

double mm_hit::toa() const { return toa_; }

double mm_hit::e() const { return e_; }

bool mm_hit::approx_equals(const mm_hit &other)
{
  const double epsilon = 0.01;
  return (x() == other.x() && y() == other.y() &&
          std::abs(toa() - other.toa()) < epsilon &&
          std::abs(e() - other.e() < epsilon));
}
