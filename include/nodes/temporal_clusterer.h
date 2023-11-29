#include "data_structs/mm_hit.h"

#include "data_structs/cluster.h"

class temporal_clusterer
{
  std::vector<cluster<mm_hit>> result_clusters_;
  using hit_vect_iterator = std::vector<mm_hit>::iterator;
  double current_toa = 0;
  const double MAX_JOIN_TIME = 200.;

public:
  temporal_clusterer() : result_clusters_(){};

  std::vector<cluster<mm_hit>> &result_clusters() { return result_clusters_; }

  void process_hits(hit_vect_iterator first, hit_vect_iterator last)
  {

    for (hit_vect_iterator hit_it = first; hit_it != last; ++hit_it)
    {
      if ((hit_it->toa() - current_toa > MAX_JOIN_TIME) ||
          (result_clusters_.size() == 0))
      {
        result_clusters_.emplace_back();
      }
      current_toa = hit_it->toa();
      result_clusters_.back().add_hit(std::move(*hit_it));
    }
  }

  std::vector<cluster<mm_hit>> process_remaining() { return result_clusters_; }
};