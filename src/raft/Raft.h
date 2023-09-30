
#ifndef RAFT_RAFT_H
#define RAFT_RAFT_H

#include <stdint.h>

class Raft {
public:
  enum Status {
    kFollower,
    kCandidate,
    kLeader,
  };

  struct StatusStruct {
    int64_t current_term;
    int64_t vote_for;
    char *log;

    int64_t commit_index;
    int64_t last_applied;

    char *next_index;
    char *match_index;
  };

  struct StatusData {
    Status status;
    StatusStruct term;
  };

public:
  Raft();

private:
  StatusData raft_data_;

  void recv_data();

  void timeout();
};

#endif // RAFT_RAFT_H
