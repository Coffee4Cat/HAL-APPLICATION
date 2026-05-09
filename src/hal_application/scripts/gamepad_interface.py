#!/usr/bin/env python3
import rclpy
import pygame
from rclpy.node import Node
from hal_interfaces.msg import GamepadInterface
from std_msgs.msg import Bool
import threading
import time




class GamepadInterfaceNode(Node):
    def __init__(self):
        super().__init__("gamepad_interface")
        self.get_logger().info("[GAMEPAD INTERFACE][VERSION 1.0]")
        self.gamepad_pub = self.create_publisher(GamepadInterface, "/gamepad_data",10)
        self.status_pub = self.create_publisher(Bool, "/gamepad_status",10)

        self.gamepad_status = False
        self.status_timer = self.create_timer(1.0, lambda: self.status_pub.publish(Bool(data=self.gamepad_status)))

        self.joystick = None
        self.gamepad_thread = threading.Thread(target=self.gamepad_loop)
        self.gamepad_thread.daemon = True
        self.gamepad_thread.start()

    
    def publish_gamepad_status(self):
        msg = Bool()
        msg.data = self.gamepad_status
        self.status_pub.publish(msg)
        if self.gamepad_status:
            self.get_logger().info("[GAMEPAD WORKS FINE]")


    def gamepad_loop(self):
        while rclpy.ok():
            try:
                pygame.event.pump()
                msg = GamepadInterface()
                msg.axes = [self.joystick.get_axis(i) for i in range(self.joystick.get_numaxes())]
                msg.buttons = [self.joystick.get_button(i) for i in range(self.joystick.get_numbuttons())]
                msg.hat_one = self.joystick.get_hat(0)[0]
                msg.hat_two = self.joystick.get_hat(0)[1]

                self.gamepad_pub.publish(msg)
                self.gamepad_status = True
                time.sleep(1/15)
            except Exception as e:
                self.joystick = None
                self.get_logger().warn("[GAMEPAD NOT DETECTED]")
                self.gamepad_status = False
                msg = GamepadInterface()
                self.gamepad_pub.publish(msg)
                time.sleep(1)
                pygame.joystick.quit()
                pygame.joystick.init()
                if pygame.joystick.get_count() == 0:
                    self.joystick = None
                    self.get_logger().warn("[COULDNT INITIALIZE GAMEPAD]")
                    self.gamepad_status = False
                    time.sleep(1)
                else:
                    self.joystick = pygame.joystick.Joystick(0)
                    self.joystick.init()
                    self.get_logger().info("[GAMEPAD INITIALIZED]")
                    self.get_logger().info(f"[{self.joystick.get_name()}]")
                    self.gamepad_status = True


def main():
    rclpy.init()
    pygame.init()
    node = GamepadInterfaceNode()
    rclpy.spin(node)
    pygame.quit()
    rclpy.shutdown()

if __name__ == "__main__":
    main()
        
