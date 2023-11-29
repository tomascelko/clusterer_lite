
#include "data_structs/burda_hit.h"

burda_hit::burda_hit(uint16_t linear_coord, int64_t toa, short fast_toa,
                     int16_t tot)
  : linear_coord_(linear_coord), toa_(toa), fast_toa_(fast_toa), tot_(tot)
{
}

burda_hit::burda_hit(std::istream *in_stream)
{
  io_utils::skip_comment_lines(*in_stream);
  if (in_stream->peek() == EOF)
  {
    toa_ = -1;
    return;
  }
  *in_stream >> linear_coord_;
  (*in_stream) >> toa_ >> fast_toa_ >> tot_;
}

constexpr uint64_t burda_hit::avg_size()
{
  return (sizeof(decltype(linear_coord_)) + sizeof(decltype(toa_)) +
          sizeof(decltype(fast_toa_)) + sizeof(decltype(tot_)));
}

burda_hit burda_hit::end_token()
{
  burda_hit end_token(0, -1, 0, 0);
  return end_token;
}

burda_hit::burda_hit() {}

bool burda_hit::is_valid() { return toa_ >= 0; }

uint16_t burda_hit::linear_coord() const { return linear_coord_; }

uint64_t burda_hit::tick_toa() const { return toa_; }

double burda_hit::toa() const
{
  return slow_clock_dt * toa_ - fast_clock_dt * fast_toa_;
}

double burda_hit::time() const
{
  return slow_clock_dt * toa_ - fast_clock_dt * fast_toa_;
  ;
}

void burda_hit::update_time(uint64_t new_toa, short fast_toa)
{
  toa_ = new_toa;
  fast_toa_ = fast_toa;
}

short burda_hit::fast_toa() const { return fast_toa_; }

int16_t burda_hit::tot() const { return tot_; }

uint32_t burda_hit::size() const { return avg_size(); }

bool burda_hit::operator<(const burda_hit &other) const
{
  const double EPSILON = 0.0001;
  if (toa() < other.toa() - EPSILON)
  {
    return true;
  }
  if (other.toa() < toa() - EPSILON)
  {
    return false;
  }
  if (linear_coord() < other.linear_coord())
  {
    return true;
  }
  if (linear_coord() > other.linear_coord())
  {
    return false;
  }
  return false;
}

bool burda_hit::operator==(const burda_hit &other) const
{
  return !(*this < other || other < *this);
}

// serialization of the hit

/*std::ofstream &operator<<(std::ofstream &os, const burda_hit &hit)
{

  uint64_t packet_bytes6 = 0;
  packet_bytes6 |= 0x4UL << 44;
  packet_bytes6 |= hit.linear_coord() << 28;
  auto moduled_tick_toa = hit.tick_toa() % burda_hit::offset_modulus;
  if (hit.tick_toa() > burda_hit::offset_modulus >> 1 &&
      moduled_tick_toa < burda_hit::offset_modulus >> 1)
    moduled_tick_toa += burda_hit::offset_modulus >> 1;
  packet_bytes6 |= (moduled_tick_toa << 14) & 0xfffffff;
  packet_bytes6 |= (hit.tot() << 4) & 0x3fff;
  packet_bytes6 |= hit.fast_toa() & 0xf;
  os.write(reinterpret_cast<const char *>(&packet_bytes6), 6);
  return os;
}
*/

std::ostream &operator<<(std::ostream &os, const burda_hit &hit)
{
  os << hit.linear_coord() << " " << hit.toa() << " " << hit.fast_toa() << " "
     << hit.tot();
  return os;
}

// deserialization of the hit
std::istream &operator>>(std::istream &is, burda_hit &hit)
{

  uint32_t lin_coord;
  uint64_t toa;
  short ftoa;
  int16_t tot;
  io_utils::skip_comment_lines(is);
  if (is.peek() == EOF)
  {
    hit = burda_hit::end_token();
    return is;
  }

  is >> lin_coord >> toa >> ftoa >> tot;
  hit = burda_hit(lin_coord, toa, ftoa, tot);

  return is;
}
