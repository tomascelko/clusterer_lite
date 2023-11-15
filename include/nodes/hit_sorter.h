#pragma once
#include <algorithm>
#include <queue>
#include "../data_structs/burda_hit.h"
// uses heap sorting (priority queue) to guarantee temporal orderedness of hits
template <typename data_type>
class hit_sorter
{
  // sorts mm hits temporally
  struct toa_comparer
  {
    auto operator()(const data_type &left, const data_type &right) const
    {
      if (left.toa() < right.toa())
        return false;
      if (right.toa() < left.toa())
        return true;
      return false;
    }
  };
  // for sorting burda hits temporally
  struct burda_toa_comparer
  {
    auto operator()(const burda_hit &left, const burda_hit &right) const
    {
      if (left.toa() < right.toa())
        return false;
      if (right.toa() < left.toa())
        return true;
      if (left.fast_toa() < right.fast_toa())
        return false;
      if (right.fast_toa() < left.fast_toa())
        return true;
      return false;
    }
  };
  using hits_optional_type = std::optional<std::vector<data_type>>;
  std::priority_queue<data_type, std::vector<data_type>, toa_comparer> priority_queue_;
  // maximal unorderedness of the datastream
  const double DEQUEUE_TIME = 500000.;
  // check for outputting the hits every DEQUEUE_CHECK_INTERVAL hits
  const uint32_t DEQUEUE_CHECK_INTERVAL = 64;
  uint64_t processed_hit_count_;

public:
  hit_sorter()
  {
    toa_comparer less_comparer;
    priority_queue_ = std::priority_queue<data_type, std::vector<data_type>, toa_comparer>(less_comparer);
  }
  std::string name()
  {
    return "hit_sorter";
  }

  hits_optional_type process_hit(data_type &&hit)
  {
    priority_queue_.push(hit);
    ++processed_hit_count_;
    if (processed_hit_count_ % DEQUEUE_CHECK_INTERVAL == 0)
    {
      std::vector<data_type> hits;
      while (priority_queue_.top().toa() < hit.toa() - DEQUEUE_TIME)
      {
        data_type old_hit = priority_queue_.top();
        hits.emplace_back(std::move(old_hit));
        priority_queue_.pop();
      }
      return hits;
    }
    else
      return std::nullopt;
  }
  std::vector<data_type> process_remaining()
  {
    std::vector<data_type> hits;
    // remove the remaining hits at the end of datastream
    while (!priority_queue_.empty())
    {
      data_type old_hit = priority_queue_.top();
      priority_queue_.pop();
      hits.emplace_back(std::move(old_hit));
    }
    return hits;
  }

  virtual ~hit_sorter() = default;
};