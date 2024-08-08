#include <fstream>
#include <vector>

#include "v4l2_camera/timestamper.hpp"

std::vector<Timestamp> read_timestamps(std::string const& timestamps_fn)
{
  std::vector<Timestamp> ret {};

  std::ifstream timestamps_file(timestamps_fn);
  int secs, nsecs;
  char dot;
  while(timestamps_file >> secs >> dot >> nsecs){
    ret.emplace_back(secs, nsecs);
  }

  return ret;
}

void Timestamper::init(std::string const& ts_buffer_fn)
{
  _ts_buffer_fn = ts_buffer_fn;
}

Timestamp Timestamper::get_last_timestamp(const rclcpp::Time& buffer_time) const
{
    //std::this_thread::sleep_for(std::chrono::milliseconds(10));

    auto const& tss = read_timestamps(_ts_buffer_fn);
    const auto nanos = buffer_time.nanoseconds();

    if (tss.empty()) {
        return Timestamp();  // Return a default constructed Timestamp if the list is empty
    }
    auto k = 0;
    for (auto it = tss.rbegin(); it != tss.rend(); ++it) {
        int64_t nr = it->first * 1e9 + it->second;
        if (nr< nanos) {

            
            //std::cout << "K iters: " << k << " ts   - difference: " << nr - nanos << std::endl; //,  nr  << "  capture: " << nanos << std::endl;

            return *it;
        }

        k++;
    }

    return Timestamp();  // Return a default constructed Timestamp if no valid timestamp is found
}
