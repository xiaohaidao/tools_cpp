
#include "Raft.h"

/*
 *class StatusMachine {
 *public:
 *  // void(last_status_name);
 *  typedef std::function<void(const char *)> callback_type;
 *
 *  void add_substatus(const char *name, callback_type callbakc);
 *  void remove_substatus(const char *name);
 *
 *  void set_status_parent(const char *name, const char *parent);
 *  void set_status_child(const char *name, const char *child);
 *
 *private:
 *
 *};
 */

namespace detail {

struct AppendLog {
  // append entries
  int64_t term;
  int64_t leader_id;
  int64_t prev_log_index;
  int64_t prev_log_term;
  char *entries;
  int64_t leader_commit;
};

struct AppendLogRet {
  // return value
  int64_t term;
  bool success;
};

struct RequestVote {
  int64_t term;
  int64_t candidate_id;
  int64_t last_log_index;
  int64_t last_log_term;
};

struct RequestVoteRet {
  // return value
  int64_t term;
  bool vote_granted;
};

template <class Status, class RequestVote>
bool request_vote(Status &status, const RequestVote &vote) {
  if (vote.term < status.current_term) {
    return false;
  }
  if (status.vote_for != 0 && status.vote_for != vote.candidate_id) {
    return false;
  }
  if (!(vote.last_log_term >= status.last_log_term &&
        vote.last_log_index >= status.last_log_index)) {
    return false;
  }
  // vote it
  return true;
}

template <class T> struct Follower {
  typedef T Status;

  void recv_rpc() {
    // do server command
  }

  void refresh_timeout() {}
};

template <class T> struct Candidate {
  typedef T Status;

  void recv_rpc() {
    // do server command
  }

  void refresh_timeout_time() {}

  void on_timeout(Status &status) {
    // vote self
    status.current_term++;
    status.vote_for = status.current_term;
    refresh_timeout_time();
    send_requestvote();
  }

  void recv_requestvote() {
    if (vote_number > (numbers / 2)) {
      to_leader();
      return;
    }
  }

  void send_requestvote() {}
};

struct Leader {
  void on_timeout(Raft::StatusData &status) { send_append_log(); }

  void recv_client_log() {}

  void recv_follower_heartbeat() {}
  void refresh_follower_log() {}

  void send_append_log() {
    AppendLog append = {};
    AppendLogRet append_ret = {};
    rpc("AppendLog", append, append_ret);
  }
};

// TODO 状态机
// TODO LSM日志树
// TODO 快照

} /* namespace detail */

using namespace detail;

Raft::Raft() : raft_data_({}) {}

void Raft::recv_data() {
  switch (raft_data_.status) {
  case kFollower: {
    /* code */
    break;
  }
  case kCandidate: {
    /* code */
    break;
  }
  case kLeader: {
    /* code */
    break;
  }
  default:
    break;
  }
}

void Raft::timeout() {
  switch (raft_data_.status) {
  case kFollower: {
    /* code */
    break;
  }
  case kCandidate: {
    /* code */
    break;
  }
  case kLeader: {
    Leader{}.on_timeout(raft_data_);
    break;
  }
  default:
    break;
  }
}
