colcon build --symlink-install --packages-ignore is-ros2-mix-generator is-examples --cmake-args -DCMAKE_BUILD_TYPE=Release
source install/setup.bash
colcon build --symlink-install --packages-select is-ros2-mix-generator --cmake-args -DCMAKE_BUILD_TYPE=Release -DMIX_ROS2_PACKAGES="sensor_msgs geometry_msgs livox_ros_driver2 builtin_interfaces tf2_msgs nav_msgs"
