#include <chrono>
#include <cstdint>
#include <cstring>
#include <functional>
#include <memory>
#include <rclcpp/qos.hpp>
#include <rclcpp/serialization.hpp>
#include <rclcpp/serialized_message.hpp>
#include <rclcpp/utilities.hpp>
#include <string>
#include <vector>
#include <algorithm>

#include "communication_client/communication_client.hpp"

namespace communication_client {
CommunicationClientNode::CommunicationClientNode(
  const rclcpp::NodeOptions& options)
    : Node("communication_client", options) {
  std::string lidar_topic_name, lidar_pointcloud_topic_name, imu_topic_name, cmd_vel_topic_name;
  this->declare_parameter<int>("robot_id", 0);
  this->declare_parameter<std::string>("lidar_topic_name",
                                       "livox/lidar_points");
  this->declare_parameter<std::string>("lidar_pointcloud_topic_name", "livox/lidar/pointcloud");
  this->declare_parameter<std::string>("imu_topic_name", "imu_data");
  this->declare_parameter<std::string>("cmd_vel_topic_name", "cmd_vel");

  this->get_parameter("robot_id", robot_id);
  this->get_parameter("lidar_topic_name", lidar_topic_name);
  this->get_parameter("lidar_pointcloud_topic_name", lidar_pointcloud_topic_name);
  this->get_parameter("imu_topic_name", imu_topic_name);
  this->get_parameter("cmd_vel_topic_name", cmd_vel_topic_name);

  rclcpp::QoS clock_qos(rclcpp::KeepLast(5));
  clock_qos.best_effort();
  clock_sub_ = this->create_subscription<builtin_interfaces::msg::Time>(
    "/changeable_clock", 5,
    std::bind(&CommunicationClientNode::ClockCallBack, this,
              std::placeholders::_1));
  livox_scan_sub_ =
    this->create_subscription<livox_ros_driver2::msg::CustomMsg>(
      "/robot_" + std::to_string(robot_id) + "/livox/lidar", 5,
      std::bind(&CommunicationClientNode::LivoxScanCallBack, this,
                std::placeholders::_1));
  livox_point_cloud_sub_ =
    this->create_subscription<sensor_msgs::msg::PointCloud2>(
      "/robot_" + std::to_string(robot_id) + "/livox/lidar/pointcloud", 5,
      std::bind(&CommunicationClientNode::LivoxPointCloudCallBack, this,
                std::placeholders::_1));
  livox_imu_sub_ = this->create_subscription<sensor_msgs::msg::Imu>(
    "/robot_" + std::to_string(robot_id) + "/livox/imu", 5,
    std::bind(&CommunicationClientNode::LivoxImuCallBack, this,
              std::placeholders::_1));
  // cmd_vel_stamped_sub_ =
  // this->create_subscription<geometry_msgs::msg::TwistStamped>(
  //   "/" + cmd_vel_topic_name, 5,
  //   std::bind(&CommunicationClientNode::CmdVelStampedCallBack, this,
  //             std::placeholders::_1));
  cmd_vel_sub_ = this->create_subscription<geometry_msgs::msg::Twist>(
    "/" + cmd_vel_topic_name, 5,
    std::bind(&CommunicationClientNode::CmdVelCallBack, this,
              std::placeholders::_1));
  
  // Subscribe to TF topics from Integration Service (on intermediate topics)
  rclcpp::QoS tf_qos(rclcpp::KeepLast(100));
  tf_qos.reliable();
  tf_sub_ = this->create_subscription<tf2_msgs::msg::TFMessage>(
    "/tf_raw", tf_qos,
    std::bind(&CommunicationClientNode::TfCallBack, this,
              std::placeholders::_1));
  
  rclcpp::QoS tf_static_qos(rclcpp::KeepLast(100));
  tf_static_qos.reliable();
  tf_static_qos.transient_local();
  tf_static_sub_ = this->create_subscription<tf2_msgs::msg::TFMessage>(
    "/tf_static_raw", tf_static_qos,
    std::bind(&CommunicationClientNode::TfStaticCallBack, this,
              std::placeholders::_1));

  clock_pub_ = this->create_publisher<rosgraph_msgs::msg::Clock>("/clock", clock_qos);
  livox_scan_pub_ = this->create_publisher<livox_ros_driver2::msg::CustomMsg>(
    "/" + lidar_topic_name, 5);
  livox_point_cloud_pub_ =
    this->create_publisher<sensor_msgs::msg::PointCloud2>(
      "/" + lidar_pointcloud_topic_name, 5);
  livox_imu_pub_ =
    this->create_publisher<sensor_msgs::msg::Imu>("/" + imu_topic_name, 5);
  cmd_vel_pub_ = this->create_publisher<geometry_msgs::msg::Twist>(
    "/robot_" + std::to_string(robot_id) + "/" + cmd_vel_topic_name, 5);
  
  // Publish TF with removed prefix - use same QoS as subscribers
  rclcpp::QoS tf_pub_qos(rclcpp::KeepLast(100));
  tf_pub_qos.reliable();
  tf_pub_ = this->create_publisher<tf2_msgs::msg::TFMessage>("/tf", tf_pub_qos);
  
  rclcpp::QoS tf_static_pub_qos(rclcpp::KeepLast(100));
  tf_static_pub_qos.reliable();
  tf_static_pub_qos.transient_local();
  tf_static_pub_ = this->create_publisher<tf2_msgs::msg::TFMessage>("/tf_static", tf_static_pub_qos);
}

void CommunicationClientNode::ClockCallBack(
  const builtin_interfaces::msg::Time::ConstSharedPtr clock_msg) {
  rosgraph_msgs::msg::Clock clock_msg_out;
  clock_msg_out.clock = *clock_msg;
  clock_pub_->publish(clock_msg_out);
}

void CommunicationClientNode::LivoxScanCallBack(
  const livox_ros_driver2::msg::CustomMsg::ConstSharedPtr livox_scan_msg) {
  livox_scan_pub_->publish(*livox_scan_msg);
}

void CommunicationClientNode::LivoxPointCloudCallBack(
  const sensor_msgs::msg::PointCloud2::ConstSharedPtr livox_point_cloud_msg) {
  livox_point_cloud_pub_->publish(*livox_point_cloud_msg);
}

void CommunicationClientNode::LivoxImuCallBack(
  const sensor_msgs::msg::Imu::ConstSharedPtr livox_imu_msg) {
  livox_imu_pub_->publish(*livox_imu_msg);
}

void CommunicationClientNode::CmdVelStampedCallBack(
  const geometry_msgs::msg::TwistStamped::ConstSharedPtr cmd_vel_msg) {
  geometry_msgs::msg::Twist twist = cmd_vel_msg->twist;
  cmd_vel_pub_->publish(twist);
}

void CommunicationClientNode::CmdVelCallBack(
  const geometry_msgs::msg::Twist::ConstSharedPtr cmd_vel_msg) {
  cmd_vel_pub_->publish(*cmd_vel_msg);
}

void CommunicationClientNode::TfCallBack(
  const tf2_msgs::msg::TFMessage::ConstSharedPtr tf_msg) {
  // Remove robot_X/ prefix from frame_id and child_frame_id
  std::string prefix = "robot_" + std::to_string(robot_id) + "/";
  
  tf2_msgs::msg::TFMessage tf_msg_out;
  tf_msg_out.transforms.reserve(tf_msg->transforms.size());
  
  for (const auto& transform : tf_msg->transforms) {
    // Only process transforms that belong to this robot
    if (transform.header.frame_id.find(prefix) == 0 || 
        transform.child_frame_id.find(prefix) == 0) {
      geometry_msgs::msg::TransformStamped transform_out = transform;
      
      // Remove prefix from frame_id
      if (transform_out.header.frame_id.find(prefix) == 0) {
        transform_out.header.frame_id = transform_out.header.frame_id.substr(prefix.length());
      }
      
      // Remove prefix from child_frame_id
      if (transform_out.child_frame_id.find(prefix) == 0) {
        transform_out.child_frame_id = transform_out.child_frame_id.substr(prefix.length());
      }
      
      tf_msg_out.transforms.push_back(transform_out);
    }
  }
  
  if (!tf_msg_out.transforms.empty()) {
    tf_pub_->publish(tf_msg_out);
  }
}

void CommunicationClientNode::TfStaticCallBack(
  const tf2_msgs::msg::TFMessage::ConstSharedPtr tf_static_msg) {
  // Remove robot_X/ prefix from frame_id and child_frame_id
  std::string prefix = "robot_" + std::to_string(robot_id) + "/";
  
  tf2_msgs::msg::TFMessage tf_static_msg_out;
  tf_static_msg_out.transforms.reserve(tf_static_msg->transforms.size());
  
  for (const auto& transform : tf_static_msg->transforms) {
    // Only process transforms that belong to this robot
    if (transform.header.frame_id.find(prefix) == 0 || 
        transform.child_frame_id.find(prefix) == 0) {
      geometry_msgs::msg::TransformStamped transform_out = transform;
      
      // Remove prefix from frame_id
      if (transform_out.header.frame_id.find(prefix) == 0) {
        transform_out.header.frame_id = transform_out.header.frame_id.substr(prefix.length());
      }
      
      // Remove prefix from child_frame_id
      if (transform_out.child_frame_id.find(prefix) == 0) {
        transform_out.child_frame_id = transform_out.child_frame_id.substr(prefix.length());
      }
      
      tf_static_msg_out.transforms.push_back(transform_out);
    }
  }
  
  if (!tf_static_msg_out.transforms.empty()) {
    tf_static_pub_->publish(tf_static_msg_out);
  }
}

}  // namespace communication_client

#include "rclcpp_components/register_node_macro.hpp"

// Register the component with class_loader.
// This acts as a sort of entry point, allowing the component to be discoverable
// when its library is being loaded into a running process.
RCLCPP_COMPONENTS_REGISTER_NODE(communication_client::CommunicationClientNode)