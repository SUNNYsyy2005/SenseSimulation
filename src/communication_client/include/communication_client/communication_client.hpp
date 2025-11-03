#ifndef COMMUNICATION_CLIENT_HPP
#define COMMUNICATION_CLIENT_HPP

#include <pcl/point_cloud.h>
#include <pcl/point_types.h>
#include <pcl_conversions/pcl_conversions.h>

#include <cstdint>
#include <geometry_msgs/msg/twist.hpp>
#include <geometry_msgs/msg/twist_stamped.hpp>
#include <livox_ros_driver2/msg/custom_msg.hpp>
#include <memory>
#include <queue>
#include <rclcpp/node.hpp>
#include <rclcpp/node_options.hpp>
#include <rclcpp/publisher.hpp>
#include <rclcpp/publisher_base.hpp>
#include <rclcpp/rclcpp.hpp>
#include <rclcpp/subscription.hpp>
#include <sensor_msgs/msg/imu.hpp>
#include <sensor_msgs/msg/point_cloud2.hpp>
#include <tf2_msgs/msg/tf_message.hpp>
#include <vector>

#include "rosgraph_msgs/msg/clock.hpp"

namespace communication_client {
class CommunicationClientNode : public rclcpp::Node {
 public:
  CommunicationClientNode(const rclcpp::NodeOptions& options);

 private:
  int robot_id;

  void ClockCallBack(
    const builtin_interfaces::msg::Time::ConstSharedPtr time_msg);
  void LivoxScanCallBack(
    const livox_ros_driver2::msg::CustomMsg::ConstSharedPtr livox_scan_msg);
  void LivoxPointCloudCallBack(
    const sensor_msgs::msg::PointCloud2::ConstSharedPtr livox_point_cloud_msg);
  void LivoxImuCallBack(
    const sensor_msgs::msg::Imu::ConstSharedPtr livox_imu_msg);
  void CmdVelStampedCallBack(
    const geometry_msgs::msg::TwistStamped::ConstSharedPtr cmd_vel_msg);
  void CmdVelCallBack(
    const geometry_msgs::msg::Twist::ConstSharedPtr cmd_vel_msg);
  void TfCallBack(
    const tf2_msgs::msg::TFMessage::ConstSharedPtr tf_msg);
  void TfStaticCallBack(
    const tf2_msgs::msg::TFMessage::ConstSharedPtr tf_static_msg);

  rclcpp::Subscription<builtin_interfaces::msg::Time>::SharedPtr clock_sub_;
  rclcpp::Subscription<livox_ros_driver2::msg::CustomMsg>::SharedPtr
    livox_scan_sub_;
  rclcpp::Subscription<sensor_msgs::msg::PointCloud2>::SharedPtr
    livox_point_cloud_sub_;
  rclcpp::Subscription<sensor_msgs::msg::Imu>::SharedPtr livox_imu_sub_;
  rclcpp::Subscription<geometry_msgs::msg::TwistStamped>::SharedPtr
    cmd_vel_stamped_sub_;
  rclcpp::Subscription<geometry_msgs::msg::Twist>::SharedPtr cmd_vel_sub_;
  rclcpp::Subscription<tf2_msgs::msg::TFMessage>::SharedPtr tf_sub_;
  rclcpp::Subscription<tf2_msgs::msg::TFMessage>::SharedPtr tf_static_sub_;

  rclcpp::Publisher<rosgraph_msgs::msg::Clock>::SharedPtr clock_pub_;
  rclcpp::Publisher<livox_ros_driver2::msg::CustomMsg>::SharedPtr
    livox_scan_pub_;
  rclcpp::Publisher<sensor_msgs::msg::PointCloud2>::SharedPtr
    livox_point_cloud_pub_;
  rclcpp::Publisher<sensor_msgs::msg::Imu>::SharedPtr livox_imu_pub_;
  rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr cmd_vel_pub_;
  rclcpp::Publisher<tf2_msgs::msg::TFMessage>::SharedPtr tf_pub_;
  rclcpp::Publisher<tf2_msgs::msg::TFMessage>::SharedPtr tf_static_pub_;
};
}  // namespace communication_client

#endif