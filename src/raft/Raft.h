
#ifndef RAFT_RAFT_H
#define RAFT_RAFT_H

#include "min_heap/timer.h"

class raft {
public:
private:
  enum Status {
    kVoter,
    kCandidate,
    kMaster,
  };

  struct {
    Status status;

  } raft_data_;

  timer timer_task_;
};

#endif // RAFT_RAFT_H
