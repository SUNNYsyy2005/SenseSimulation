#!/bin/bash

# 第一步：编译主要包，忽略不需要的包
colcon build --symlink-install \
    --packages-ignore is-ros2-mix-generator is-examples realsense2_camera realsense2_camera_msgs realsense2_description \
    --cmake-args -DCMAKE_BUILD_TYPE=Release

# 加载环境（bash 使用 source，sh 使用 .）
. install/setup.bash

# 第二步：确保 livox_ros_driver2 编译成功
colcon build --symlink-install \
    --packages-select livox_ros_driver2 \
    --cmake-args -DCMAKE_BUILD_TYPE=Release

# 重新加载环境
. install/setup.bash

# 第三步：编译 Integration Service 的 ROS2 混合生成器
# 需要在加载了 livox_ros_driver2 的环境下编译
colcon build --symlink-install \
    --packages-select is-ros2-mix-generator \
    --cmake-args -DCMAKE_BUILD_TYPE=Release \
    -DMIX_ROS2_PACKAGES="sensor_msgs geometry_msgs livox_ros_driver2 builtin_interfaces tf2_msgs nav_msgs"

# 最后加载完整环境
. install/setup.bash

echo "Build completed successfully!"

