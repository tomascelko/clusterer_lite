#pragma once
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <iomanip>
#include "../other/utils.h"
// suring conversion, the burda_hit is often converted to mm_hit
// which is easier to use for analysis
class mm_hit
{
  // spatial pixel coordinates (x,y)
  coord coord_;
  // time of arrival in ns
  double toa_;
  // deposited energy in keV
  double e_;

public:
  mm_hit(short x, short y, double toa, double e);
  mm_hit();

  const coord &coordinates() const;
  short x() const;
  short y() const;
  double time() const;
  double toa() const;
  double e() const;
  bool approx_equals(const mm_hit &other);
};
// hit serialization
template <typename stream_type>
stream_type &operator<<(stream_type &os, const mm_hit &hit)
{
  os << hit.x() << " " << hit.y() << " " << double_to_str(hit.toa()) << " ";
  os << double_to_str(hit.e(), 2) << "\n";

  return os;
}
// hit deserialization
template <typename stream_type>
stream_type &operator>>(stream_type &istream, mm_hit &hit)
{
  short x, y;
  double toa, e;
  istream >> x >> y >> toa >> e;
  hit = mm_hit(x, y, toa, e);

  return istream;
}
