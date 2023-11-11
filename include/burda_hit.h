#include <memory>
#include <vector>
#include <iostream>
#include "../utils.h"
#pragma once
//a memory efficient hit information format, resulting from Katherine readout
//however, it is difficult to analyze it directly
class burda_hit
{
    //linearized spatial coordinate of a pixel
    uint16_t linear_coord_;
    //number of ticks of the slow clock
    int64_t toa_;
    //number of ticks of te fast clock
    short fast_toa_;
    //time over threshold in ticks
    int16_t tot_; // can be zero because of chip error, so we set invalid value to -1
public:
    burda_hit(uint16_t linear_coord, int64_t toa, short fast_toa, int16_t tot) : linear_coord_(linear_coord),
                                                                                 toa_(toa),
                                                                                 fast_toa_(fast_toa),
                                                                                 tot_(tot) {}
    burda_hit(std::istream *in_stream)
    {
        toa_ = -1;
        io_utils::skip_comment_lines(*in_stream);
        if (in_stream->peek() == EOF)
            return;
        *in_stream >> linear_coord_;
        (*in_stream) >> toa_ >> fast_toa_ >> tot_;
    }
    static constexpr uint64_t avg_size()
    {
        return (sizeof(decltype(linear_coord_)) + sizeof(decltype(toa_)) + sizeof(decltype(fast_toa_)) + sizeof(decltype(tot_)));
    }
    static burda_hit end_token()
    {
        burda_hit end_token(0, -1, 0, 0);
        return end_token;
    }
    burda_hit()
    {
    }
    bool is_valid()
    {
        return toa_ >= 0;
    }
    uint16_t linear_coord() const
    {
        return linear_coord_;
    }
    static constexpr double fast_clock_dt = 1.5625;

    static constexpr double slow_clock_dt = 25.;
    uint64_t tick_toa() const
    {
        return toa_;
    }
    double toa() const
    {
        return slow_clock_dt * toa_ - fast_clock_dt * fast_toa_;
    }
    double time() const
    {
        return slow_clock_dt * toa_ - fast_clock_dt * fast_toa_;;
    }

    void update_time(uint64_t new_toa, short fast_toa)
    {
        toa_ = new_toa;
        fast_toa_ = fast_toa;
    }
    short fast_toa() const
    {
        return fast_toa_;
    }
    int16_t tot() const
    {
        return tot_;
    }
    uint32_t size() const
    {
        return avg_size();
    }
    bool operator<(const burda_hit &other) const
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
    bool operator==(const burda_hit &other) const
    {
        return !(*this < other || other < *this);
    }
};
//serialization of the hit
std::ostream &operator<<(std::ostream &os, const burda_hit &hit)
{
    os << hit.linear_coord() << " " << hit.toa() << " " << hit.fast_toa() << " " << hit.tot();
    return os;
}
//deserialization of the hit
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
