#!/usr/bin/env python3

import os

from ament_index_python.packages import get_package_share_directory

from launch import LaunchDescription, LaunchContext
from launch.substitutions import LaunchConfiguration, Command
from launch.actions import (
    IncludeLaunchDescription,
    DeclareLaunchArgument,
    TimerAction,
    OpaqueFunction,
    SetEnvironmentVariable,
    ExecuteProcess,
)
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch_ros.actions import Node

rob_x= ['0.0','1.0','2.0']#['13.5','14.0','0.5']
rob_y = ['0.0','0.0','0.0']#['5.0','5.0','0.0']
def robot_description(context: LaunchContext, robot_count, use_sim_time):
    action_list = []
    for i in range(int(context.perform_substitution(robot_count))):
        robot_description = Command(
            [
                "xacro ",
                os.path.join(
                    get_package_share_directory("simulation_bringup"),
                    "urdf",
                    "sensebeetle_original.xacro",
                ),
                " robot_namespace:=robot_{}".format(i),
            ]
        )

        start_joint_state_publisher_cmd = Node(
            package="joint_state_publisher",
            executable="joint_state_publisher",
            name="joint_state_publisher",
            namespace="robot_{}".format(i),
            parameters=[
                {"use_sim_time": use_sim_time, "robot_description": robot_description}
            ],
            output="screen",
        )

        start_robot_state_publisher_cmd = Node(
            package="robot_state_publisher",
            executable="robot_state_publisher",
            name="robot_state_publisher",
            namespace="robot_{}".format(i),
            parameters=[
                {"use_sim_time": use_sim_time, 
                 "robot_description": robot_description,
                 "frame_prefix": "robot_{}/".format(i)}
            ],
            output="screen",
        )

        start_spawn_entity_node = Node(
            package="gazebo_ros",
            executable="spawn_entity.py",
            arguments=[
                '-entity', 'robot_{}'.format(i),
                '-topic', 'robot_{}/robot_description'.format(i),
                '-robot_namespace', 'robot_{}'.format(i),
                '-x', rob_x[i],
                '-y', rob_y[i],
                '-z', '0.0',
                '-Y', '0.0'
            ],
        )

        action_list.append(start_joint_state_publisher_cmd)
        action_list.append(start_robot_state_publisher_cmd)
        action_list.append(start_spawn_entity_node)

    return action_list


def communication_client_launch(context: LaunchContext, robot_count, lidar_topic_name, lidar_pointcloud_topic_name, imu_topic_name, cmd_vel_topic_name ,use_sim_time):
    action_list = []
    for i in range(int(context.perform_substitution(robot_count))): 
        communication_client_launch = IncludeLaunchDescription(
            PythonLaunchDescriptionSource(
                os.path.join(
                    get_package_share_directory("communication_client"),
                    "launch",
                    "communication_client.launch.py",
                )
            ),
            launch_arguments={
                "robot_id": str(i),
                "lidar_topic_name": lidar_topic_name,
                "lidar_pointcloud_topic_name": lidar_pointcloud_topic_name,
                "imu_topic_name": imu_topic_name,
                "cmd_vel_topic_name": cmd_vel_topic_name,
            }.items(),
        )
        action_list.append(SetEnvironmentVariable("ROS_DOMAIN_ID", str(i + 1)))
        action_list.append(communication_client_launch)
    
    action_list.append(SetEnvironmentVariable("ROS_DOMAIN_ID", str(0)))
    return action_list


def world_launch(context: LaunchContext, world_name):
    name = context.perform_substitution(world_name)
    world_launch = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(
                get_package_share_directory("gazebo_ros"),
                "launch",
                "gzserver.launch.py",
            )
        ),
        launch_arguments={
            "world": os.path.join(
                get_package_share_directory("simulation_bringup"),
                "world",
                name,
                name + ".world",
            )
        }.items(),
    )
    
    # 启动 map_server 发布真实地图
    map_yaml_file = os.path.join(
        get_package_share_directory("simulation_bringup"),
        "world",
        name,
        name + ".yaml"
    )
    
    map_server_node = Node(
        package='nav2_map_server',
        executable='map_server',
        name='ground_truth_map_server',
        output='screen',
        parameters=[{
            'yaml_filename': map_yaml_file,
            'topic_name': 'ground_truth_map',
            'frame_id': 'world'
        }]
    )
    
    # 启动 lifecycle manager 来激活 map_server
    lifecycle_manager_node = Node(
        package='nav2_lifecycle_manager',
        executable='lifecycle_manager',
        name='ground_truth_map_lifecycle_manager',
        output='screen',
        parameters=[{
            'autostart': True,
            'node_names': ['ground_truth_map_server']
        }]
    )
    
    return [world_launch, map_server_node, lifecycle_manager_node]


def generate_launch_description():
    robot_count = LaunchConfiguration("robot_count")
    lidar_topic_name = LaunchConfiguration("lidar_topic_name")
    lidar_pointcloud_topic_name = LaunchConfiguration("lidar_pointcloud_topic_name")
    imu_topic_name = LaunchConfiguration("imu_topic_name")
    cmd_vel_topic_name = LaunchConfiguration("cmd_vel_topic_name")
    use_sim_time = LaunchConfiguration("use_sim_time")
    world_name = LaunchConfiguration("world_name")

    declare_robot_count = DeclareLaunchArgument(
        "robot_count", default_value="3", description=""
    )
    declare_lidar_topic_name_cmd = DeclareLaunchArgument(
        "lidar_topic_name", default_value="livox/lidar", description=""
    )
    declare_lidar_pointcloud_topic_name = DeclareLaunchArgument(
        "lidar_pointcloud_topic_name",
        default_value="livox/lidar/pointcloud",
        description="",
    )
    declare_imu_topic_name_cmd = DeclareLaunchArgument(
        "imu_topic_name", default_value="livox/imu", description=""
    )
    declare_cmd_vel_topic_name_cmd = DeclareLaunchArgument(
        "cmd_vel_topic_name", default_value="cmd_vel", description=""
    )

    declare_use_sim_time_cmd = DeclareLaunchArgument(
        "use_sim_time",
        default_value="True",
        description="Use simulation (Gazebo) clock if true",
    )

    declare_world_cmd = DeclareLaunchArgument(
        "world_name", default_value="office", description="Choose world"
    )

    gazebo_client_launch = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(
                get_package_share_directory("gazebo_ros"),
                "launch",
                "gzclient.launch.py",
            )
        ),
    )

    communication_server_launch = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(
                get_package_share_directory("communication_server"),
                "launch",
                "communication_server.launch.py",
            )
        ),
    )

    # Integration Service 用于转发真实 odom 到 Domain 10
    truth_odom_yaml_file = os.path.join(
        get_package_share_directory("simulation_bringup"),
        "yaml",
        "robot_10_original.yaml"
    )
    
    truth_odom_relay_service = ExecuteProcess(
        cmd=["integration-service", truth_odom_yaml_file],
        output="screen",
        shell=False,
    )

    ld = LaunchDescription()

    # 设置 GAZEBO_MODEL_PATH 以包含 room2 模型目录
    gazebo_model_path = os.path.join(
        get_package_share_directory("simulation_bringup"),
        "world",
        "room2"
    )
    set_gazebo_model_path = SetEnvironmentVariable(
        'GAZEBO_MODEL_PATH',
        gazebo_model_path + ':' + os.environ.get('GAZEBO_MODEL_PATH', '')
    )
    ld.add_action(set_gazebo_model_path)

    ld.add_action(declare_robot_count)
    ld.add_action(declare_lidar_topic_name_cmd)
    ld.add_action(declare_lidar_pointcloud_topic_name)
    ld.add_action(declare_imu_topic_name_cmd)
    ld.add_action(declare_cmd_vel_topic_name_cmd)
    ld.add_action(declare_use_sim_time_cmd)
    ld.add_action(declare_world_cmd)

    ld.add_action(gazebo_client_launch)
    ld.add_action(communication_server_launch)
    ld.add_action(truth_odom_relay_service)  # 添加真实 odom 转发服务
    ld.add_action(OpaqueFunction(function=world_launch, args=[world_name]))
    ld.add_action(
        OpaqueFunction(function=robot_description, args=[robot_count, use_sim_time])
    )
    ld.add_action(
        OpaqueFunction(function=communication_client_launch, args=[robot_count, lidar_topic_name, lidar_pointcloud_topic_name, imu_topic_name, cmd_vel_topic_name, use_sim_time])
    )

    return ld
