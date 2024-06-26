// Copyright 2019 Bold Hearts
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef V4L2_CAMERA__V4L2_CAMERA_HPP_
#define V4L2_CAMERA__V4L2_CAMERA_HPP_

#include "v4l2_camera/v4l2_camera.hpp"
#include "v4l2_camera/v4l2_camera_device.hpp"
#include "v4l2_camera/yuv422_yuy2_image_encodings.h"

#include <camera_info_manager/camera_info_manager.h>
#include <image_transport/image_transport.h>

#include <ostream>
#include <ros/ros.h>
#include <std_msgs/Time.h>

#include <memory>
#include <string>
#include <map>
#include <vector>
#include <thread>
#include <queue>
#include <deque>
#include <mutex>

#include "v4l2_camera/visibility_control.h"
#include "start_capture/capture_images.h"

#ifdef ENABLE_CUDA
#include <nppdefs.h>
#include <nppi_support_functions.h>
#endif

namespace v4l2_camera
{
#ifdef ENABLE_CUDA
struct GPUMemoryManager
{
 public:
  // helper structure to handle GPU memory
  Npp8u * dev_ptr;
  int step_bytes;
  int allocated_size;

  explicit GPUMemoryManager()
  {
    dev_ptr = nullptr;
    step_bytes = 0;
    allocated_size = 0;
  }

  ~GPUMemoryManager()
  {
    if (dev_ptr != nullptr) {
      nppiFree(dev_ptr);
    }
  }

  void Allocate(int width, int height)
  {
    if (dev_ptr == nullptr || allocated_size < height * step_bytes) {
      dev_ptr = nppiMalloc_8u_C3(width, height, &step_bytes);
      allocated_size = height * step_bytes;
    }
  }
};
#endif

class V4L2Camera
{
public:
  explicit V4L2Camera(ros::NodeHandle node, ros::NodeHandle private_nh);
  virtual ~V4L2Camera();

private:
  ros::NodeHandle node;
  ros::NodeHandle private_nh;
  std::string device;
  bool use_v4l2_buffer_timestamps;
  bool use_triggering;
  int timestamp_offset;
  std::mutex queue_mutex;
  ros::ServiceServer service_;

  std::queue<std::pair<sensor_msgs::ImagePtr, sensor_msgs::CameraInfoPtr>> image_queue;
  std::queue<std_msgs::TimeConstPtr> timestamp_queue;
  ros::Subscriber trigger_sub;
  void consumeTimestamp(const std_msgs::Time::ConstPtr& msg);
  void consumeImage(const sensor_msgs::ImagePtr img, const sensor_msgs::CameraInfoPtr ci);
  void publishImage(const sensor_msgs::ImagePtr img, const sensor_msgs::CameraInfoPtr ci, const std_msgs::TimeConstPtr& time);
  bool SetCaptureImages(start_capture::capture_images::Request& req, start_capture::capture_images::Response& res);
  std::atomic<bool> capture_images_;

  using ImageSize = std::vector<int64_t>;
  using TimePerFrame = std::vector<int64_t>;
  
  std::shared_ptr<V4l2CameraDevice> camera_;

  // Publisher used for intra process comm
  ros::Publisher image_pub_;
  ros::Publisher info_pub_;

  // Publisher used for inter process comm
  image_transport::ImageTransport image_transport_;
  image_transport::CameraPublisher camera_transport_pub_;

  std::shared_ptr<camera_info_manager::CameraInfoManager> cinfo_;

  std::thread capture_thread_;
  std::atomic<bool> canceled_;

  std::string output_encoding_;
  std::string camera_info_url_;
  std::string camera_frame_id_;
  std::string pixel_format_;
  
  int image_size_width;
  int image_size_height;

  int time_per_frame_first;
  int time_per_frame_second;

  std::map<std::string, int32_t> control_name_to_id_;


  ros::Timer image_pub_timer_;

  bool publish_next_frame_;
  bool use_image_transport_;

#ifdef ENABLE_CUDA
  // Memory region to communicate with GPU
  std::allocator<GPUMemoryManager> allocator_;
  std::shared_ptr<GPUMemoryManager> src_dev_;
  std::shared_ptr<GPUMemoryManager> dst_dev_;
#endif

  void createParameters();
  bool requestPixelFormat(std::string const & fourcc);
  bool requestImageSize(std::vector<int64_t> const & size);
  bool requestTimePerFrame(TimePerFrame const & tpf);

  bool checkCameraInfo(
    sensor_msgs::Image const & img,
    sensor_msgs::CameraInfo const & ci);
sensor_msgs::ImagePtr convert(sensor_msgs::Image& img);
#ifdef ENABLE_CUDA
sensor_msgs::ImagePtr convertOnGpu(sensor_msgs::Image& img);
#endif
};


}  // namespace v4l2_camera

#endif  // V4L2_CAMERA__V4L2_CAMERA_HPP_