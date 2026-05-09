from launch import LaunchDescription
from launch_ros.actions import Node
from launch.actions import IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource

from ament_index_python.packages import get_package_share_directory

import os


def generate_launch_description():

    communication_launch = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(
                get_package_share_directory('hal_communication'),
                'launch',
                'communication.launch.py'
            )
        )
    )

    manipulator_launch = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(
                get_package_share_directory('hal_manipulator'),
                'launch',
                'hal_kinematics.launch.py'
            )
        )
    )

    application_node = Node(
        package='hal_application',
        executable='application'
    )

    gamepad_node = Node(
        package='hal_application',
        executable='gamepad_interface.py'
    )

    return LaunchDescription([
        communication_launch,
        manipulator_launch,
        application_node,
        gamepad_node
    ])