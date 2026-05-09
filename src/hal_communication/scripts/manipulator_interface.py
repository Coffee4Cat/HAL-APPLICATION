#!/usr/bin/env python3
import rclpy
from rclpy.node import Node
import struct
from sensor_msgs.msg import JointState
from hal_interfaces.srv import Communication
from example_interfaces.msg import String
from example_interfaces.msg import Bool
from functools import partial



class ManipulatorInterface(Node):
    def __init__(self):
        super().__init__('manipulator_interface')
        self.comm_client = self.create_client(Communication, "/comm_server")
        self.init_manipulator()
        self.dof1 = 0.0
        self.dof2 = 0.0
        self.dof3 = 0.0
        self.dof4 = 0.0
        self.dof5 = 0.0
        self.dof6 = 0.0

        self.create_subscription(JointState, '/joint_states', self.joint_callback, 10)
        self.create_subscription(String, '/gripper', self.gripper_callback, 10)
        self.create_subscription(Bool, '/init_manipulator', self.init_manipulator_remotely,10)
        self.init_manipulator()
        self.init_manipulator()

    def init_manipulator(self):
        self.get_logger().info('Initializing manipulator...')
        self.call_service(128, [1 & 0xff, 1 & 0xff, 2 & 0xff, 2 & 0xff, 0 & 0xff, 0 & 0xff, 0 & 0xff, 0 & 0xff])
    def init_manipulator_remotely(self,msg):
        self.get_logger().info('Initializing manipulator...')
        self.call_service(128, [1 & 0xff, 1 & 0xff, 2 & 0xff, 2 & 0xff, 0 & 0xff, 0 & 0xff, 0 & 0xff, 0 & 0xff])


    def gripper_callback(self, msg: String):
        if str(msg.data) == "open":
            self.call_service(157, [2 & 0xff, 0 & 0xff, 0 & 0xff, 0 & 0xff, 0 & 0xff, 0 & 0xff, 0 & 0xff, 0 & 0xff])
        if str(msg.data) == "stop":
            self.call_service(157, [0 & 0xff, 0 & 0xff, 0 & 0xff, 0 & 0xff, 0 & 0xff, 0 & 0xff, 0 & 0xff, 0 & 0xff])
        if str(msg.data) == "close":
            self.call_service(157, [1 & 0xff, 0 & 0xff, 0 & 0xff, 0 & 0xff, 0 & 0xff, 0 & 0xff, 0 & 0xff, 0 & 0xff])

    

    def joint_callback(self, msg: JointState):
        dof1 = msg.position[0]
        dof2 = msg.position[1]
        dof3 = msg.position[2]
        dof4 = msg.position[3]
        dof5 = msg.position[4]
        dof6 = msg.position[5]

        dof1 = self.format_conversion(dof1)
        dof2 = self.format_conversion(dof2)
        self.call_service(129, [dof1[0] & 0xff, dof1[1] & 0xff, dof1[2] & 0xff, dof1[3] & 0xff,
                                dof2[0] & 0xff, dof2[1] & 0xff, dof2[2] & 0xff, dof2[3] & 0xff ])
        
        dof3 = self.format_conversion(dof3)
        dof4 = self.format_conversion(dof4)
        self.call_service(130, [dof3[0] & 0xff, dof3[1] & 0xff, dof3[2] & 0xff, dof3[3] & 0xff,
                                dof4[0] & 0xff, dof4[1] & 0xff, dof4[2] & 0xff, dof4[3] & 0xff ])
        dof5 = self.format_conversion(dof5)
        dof6 = self.format_conversion(dof6)
        self.call_service(131, [dof5[0] & 0xff, dof5[1] & 0xff, dof5[2] & 0xff, dof5[3] & 0xff,
                                dof6[0] & 0xff, dof6[1] & 0xff, dof6[2] & 0xff, dof6[3] & 0xff ]) 


    
    def call_service(self, req_id, data):
        while not self.comm_client.wait_for_service(timeout_sec=1.0):
            self.get_logger().warn(f'/comm_server not available, waiting...')

        request = Communication.Request()
        request.comm_mode = 0
        request.id = req_id
        request.data = data

        future = self.comm_client.call_async(request)
        future.add_done_callback(partial(self.future_callback))
    def future_callback(self, future):
        if future.result().success:
            self.get_logger().info(f'Service response sucessfull')
        else:
            self.get_logger().error(f'Service response unsucessfull')


    def format_conversion(self,number: float):
        num = struct.pack('>f',number).hex()
        num1 = int(num[0:2],16)
        num2 = int(num[2:4],16)
        num3 = int(num[4:6],16)
        num4 = int(num[6:8],16)
        return [num1,num2,num3,num4]

        


def main(args=None):
    rclpy.init(args=args)
    node = ManipulatorInterface()
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass
    finally:
        node.destroy_node()
        rclpy.shutdown()


if __name__ == '__main__':
    main()
