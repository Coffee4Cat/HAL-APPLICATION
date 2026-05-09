#!/usr/bin/env python3
import rclpy
from rclpy.node import Node
import math
from geometry_msgs.msg import Twist
from hal_interfaces.srv import Communication
from functools import partial
from example_interfaces.msg import Bool
from example_interfaces.msg import String


# TODO precise measurements
WHEEL_RAD = 0.1575
ROVER_WIDTH = 0.91


class WheelInterface(Node):
    def __init__(self):
        super().__init__('wheel_interface')
        self.comm_client = self.create_client(Communication, "/comm_server")
        self.init_light()
        self.init_engine()
        self.create_subscription(Twist, '/mux/cmd_vel', self.spd_callback, 10)
        self.status_pub = self.create_publisher(Bool, '/comm_status',10)
        self.create_timer(0.5, self.alive_callback)
        self.create_subscription(Bool, "/init_chassis", self.init_chassis,10)
        self.create_subscription(String, "/light", self.light_callback,10)




    def init_chassis(self, msg):
        self.init_light()
        self.init_engine()

    def light_callback(self, msg: String):
        if (msg.data == "red"):
            # self.call_service(50, [1 & 0xff, 1 & 0xff, 0 & 0xff, 0 & 0xff])
            pass
        elif (msg.data == "green"):
            self.call_service(50, [1 & 0xff, 0 & 0xff, 0 & 0xff, 1 & 0xff, 0 & 0xff, 0 & 0xff, 0 & 0xff, 0 & 0xff])
        elif (msg.data == "blue"):
            self.call_service(50, [0 & 0xff, 0 & 0xff, 1 & 0xff, 1 & 0xff, 0 & 0xff, 0 & 0xff, 0 & 0xff, 0 & 0xff])



    def alive_callback(self):
        msg = Bool()
        msg.data = True
        self.status_pub.publish(msg)


    def init_light(self):
        self.get_logger().info('Initializing light...')
        self.call_service(50, [0 & 0xff, 0 & 0xff, 1 & 0xff, 1 & 0xff, 0 & 0xff, 0 & 0xff, 0 & 0xff, 0 & 0xff])


    def init_engine(self):
        self.get_logger().info('Initializing engine...')
        self.call_service(22, [1 & 0xff, 1 & 0xff, 1 & 0xff, 0 & 0xff, 0 & 0xff, 0 & 0xff, 0 & 0xff, 0 & 0xff])

    def spd_callback(self, msg):
        spd_lin = msg.linear.x  # m/s
        spd_ang = msg.angular.z  # rad/s
        spd_l = (spd_lin - spd_ang * ROVER_WIDTH / 2 * 2) / WHEEL_RAD  # rad/s
        spd_r = (spd_lin + spd_ang * ROVER_WIDTH / 2 * 2) / WHEEL_RAD  # rad/s
        spd_mb_unit_l = round(spd_l / (2 * math.pi) * 100)
        spd_mb_unit_r = round(spd_r / (2 * math.pi) * 100)

        if abs(spd_mb_unit_l) <= 1 and abs(spd_mb_unit_r) > 1:
            spd_mb_unit_l = -2
        if abs(spd_mb_unit_r) <= 1 and abs(spd_mb_unit_l) > 1:
            spd_mb_unit_r = -2

        self.get_logger().info(f'Sending speed L: {spd_mb_unit_l}, R: {spd_mb_unit_r}')
        self.call_service(20, [spd_mb_unit_l & 0xff, spd_mb_unit_r & 0xff, 0 & 0xff, 0 & 0xff, 0 & 0xff, 0 & 0xff, 0 & 0xff, 0 & 0xff])

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


def main(args=None):
    rclpy.init(args=args)
    node = WheelInterface()
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass
    finally:
        node.destroy_node()
        rclpy.shutdown()


if __name__ == '__main__':
    main()
