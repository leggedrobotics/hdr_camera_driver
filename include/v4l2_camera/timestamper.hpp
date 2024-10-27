#pragma once

#include<tuple>
#include<string>

#include <rclcpp/rclcpp.hpp>

using Timestamp = std::pair<int, int>;

class Timestamper
{
  public:
    Timestamper() = default;
    void init(std::string const& ts_buffer_fn);
    Timestamp get_last_timestamp(const rclcpp::Time& buffer_time) const;
    std::vector<rclcpp::Time> get_timestamps_since_last_published(rclcpp::Time const& last_published);

  private:
    std::string _ts_buffer_fn;
};
