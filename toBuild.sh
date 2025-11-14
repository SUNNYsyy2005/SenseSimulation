#!/bin/bash

echo " 开始编译仿真环境..."

# 第一步：编译主要包，忽略不需要的包
echo " 步骤 1/4: 编译主要包..."
colcon build --symlink-install \
    --packages-ignore is-ros2-mix-generator is-examples realsense2_camera realsense2_camera_msgs realsense2_description \
    --cmake-args -DCMAKE_BUILD_TYPE=Release

if [ $? -ne 0 ]; then
    echo "❌ 主要包编译失败!"
    exit 1
fi

# 加载环境
echo " 加载编译环境..."
source install/setup.bash

# 第二步：确保 livox_ros_driver2 编译成功
echo " 步骤 2/4: 编译 livox_ros_driver2..."
colcon build --symlink-install \
    --packages-select livox_ros_driver2 \
    --cmake-args -DCMAKE_BUILD_TYPE=Release

if [ $? -ne 0 ]; then
    echo "❌ livox_ros_driver2 编译失败!"
    exit 1
fi

# 重新加载环境
echo " 重新加载环境..."
source install/setup.bash

# 第三步：编译 Integration Service 的 ROS2 混合生成器
# 关键：使用空格分隔包名（不是分号）
echo " 步骤 3/4: 编译 Integration Service mix 生成器..."
echo "⚙️  生成消息类型扩展: std_msgs, sensor_msgs, geometry_msgs, nav_msgs, tf2_msgs, builtin_interfaces, livox_ros_driver2"

colcon build --symlink-install \
    --packages-select is-ros2-mix-generator \
    --cmake-args \
        -DCMAKE_BUILD_TYPE=Release \
        "-DMIX_ROS2_PACKAGES=std_msgs sensor_msgs geometry_msgs nav_msgs tf2_msgs builtin_interfaces livox_ros_driver2"

if [ $? -ne 0 ]; then
    echo "❌ is-ros2-mix-generator 编译失败!"
    exit 1
fi

# 第四步：重新编译使用 Integration Service 的包
echo " 步骤 4/4: 重新编译 communication 相关包..."
source install/setup.bash

colcon build --symlink-install \
    --packages-select communication_server communication_client simulation_bringup \
    --cmake-args -DCMAKE_BUILD_TYPE=Release

if [ $? -ne 0 ]; then
    echo "⚠️  communication 包编译有警告，但继续..."
fi

# 最后加载完整环境
echo " 加载最终环境..."
source install/setup.bash

echo ""
echo "✅ 编译完成!"
echo " 请运行 'source install/setup.bash' 来加载环境"
echo " 然后运行 './run' 启动仿真"

