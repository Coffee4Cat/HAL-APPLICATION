import os
from launch import LaunchDescription
from launch_ros.actions import Node
from ament_index_python.packages import get_package_share_directory
from launch.actions import DeclareLaunchArgument, ExecuteProcess
from launch.substitutions import LaunchConfiguration
from launch.conditions import IfCondition, UnlessCondition

def generate_launch_description():
    # Path to the YAML configuration file
    config_file = os.path.join(
        get_package_share_directory('hal_communication'),
        'config',
        'comm.yaml'
    )

    return LaunchDescription([
        # Declare the parameter file
        DeclareLaunchArgument(
            'config_file',
            default_value=config_file,
            description='Path to the communication configuration file'
        ),
        DeclareLaunchArgument(name="wheels",
            default_value="True",
            description="Enable wheel interface"
        ),
        DeclareLaunchArgument(name="mani",
            default_value="True",
            description="Enable manipulator interface"
        ),

        # Communication server node
        Node(
            package='hal_communication',
            executable='hal_communication_node',
            name='communication',
            # namespace="hal",
            parameters=[config_file],
            output='screen',
            respawn=True
        ),

        # Wheel interface node (Python)
        Node(
            package='hal_communication',
            executable='wheel_interface.py',
            name='wheel_interface',
            # namespace="hal",
            output='screen',
            condition=IfCondition(LaunchConfiguration("wheels"))
        ),

        # Wheel mux node
        Node(
            package='hal_communication',
            executable='wheel_mux_node',
            name='wheel_mux_node',
            # namespace="hal",
            output='screen',
            condition=IfCondition(LaunchConfiguration("wheels"))
        ),

        # Manipulator interface node (Python)
        Node(
            package='hal_communication',
            executable='manipulator_interface.py',
            name='manipulator_interface',
            # namespace="hal",
            output='screen',
            condition=IfCondition(LaunchConfiguration("mani"))
        )
        
    ])
