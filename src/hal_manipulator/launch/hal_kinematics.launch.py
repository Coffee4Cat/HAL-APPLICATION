
import launch, os, xacro
from launch import LaunchDescription
from launch_ros.actions import Node
from ament_index_python import get_package_share_directory


def generate_launch_description():

    ld = LaunchDescription()

    config_file = os.path.join(
        get_package_share_directory('hal_manipulator'),
        'config',
        'dh_parameters.yaml'
    )



    
    hal_kinematics = Node(
        package='hal_manipulator',
        executable='hal_kinematics.py',
        name='hal_kinematics',
        parameters=[config_file]  
    )

    
    ld.add_action(hal_kinematics)

    return ld