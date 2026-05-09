#!/usr/bin/env python3
import rclpy
from rclpy.node import Node
from geometry_msgs.msg import Twist, Pose
from sensor_msgs.msg import JointState
import tkinter as tk
import threading
import numpy as np
from functools import partial
import pygame
import time
from hal_interfaces.srv import InverseKinematics
from hal_interfaces.srv import ForwardKinematics
from example_interfaces.msg import String



class Teleoperator(Node):
    def __init__(self):
        super().__init__('teleoperator')
        self.general_wheels_mode = True
        self.general_mani_mode = True
        self.send_mani_stuff = False
        self.forward = 0.0
        self.side = 0.0
        self.work_wheel = False
        self.cmd_pub = self.create_publisher(Twist, '/cmd_vel', 10)
        self.create_timer(0.1, self.publish_cmd)

        self.joint_names = ["dof1", "dof2", "dof3", "dof4", "dof5", "dof6"] 
        self.joint_angles = [0.0, 0.0, 0.0, 0.0, 0.0, 0.0]
        self.joint_increment = 0.1
        self.work_ik = False
        self.work_joints = True
        self.last_index = None
        self.last_direction = None

        self.joint_state_publisher = self.create_publisher(JointState, '/joint_states', 10)
        self.inverse_kinematics_client = self.create_client(InverseKinematics,'/inverse_kinematics')
        self.gripper_state_publisher = self.create_publisher(String, '/gripper',10)

        while not self.inverse_kinematics_client.wait_for_service(timeout_sec=1.0):
            self.get_logger().info('inverse_kinematics service down, waiting...')
        self.ik_req = InverseKinematics.Request()

        self.forward_kinematics_client = self.create_client(ForwardKinematics,'/forward_kinematics')
        while not self.inverse_kinematics_client.wait_for_service(timeout_sec=1.0):
            self.get_logger().info('forward_kinematics service down, waiting...')
        self.fk_req = ForwardKinematics.Request()


        self.create_timer(0.15,self.publish_joint_state)

        self.translation = [0.7, 0.0, 1.0] # x y z 
        self.rotation = [0.0,1.57,0.0]   # r p y
        self.translation_increment = 0.02
        self.rotation_increment = 0.02

        self.get_logger().info('[TELEOPERATOR] Have fun!')

    def toggle_mani_mode(self):
        self.work_joints = not self.work_joints
        self.work_ik = not self.work_ik

    def toggle_general_mode(self):
        self.general_mani_mode = not self.general_mani_mode
        self.general_wheels_mode = not self.general_wheels_mode

    def send_ik_request(self):
        if (self.work_ik):
            self.ik_req.pose.position.x = self.translation[0]
            self.ik_req.pose.position.y = self.translation[1]
            self.ik_req.pose.position.z = self.translation[2]
            # [x,y,z,w] = self.rpy_to_quaternion(self.rotation[0],self.rotation[1],self.rotation[2])
            [x,y,z,w] = self.tilt_twist_quat(self.rotation[0],self.rotation[1],self.rotation[2])
            self.get_logger().info(f"++++++++++++++++++\nx: {x}\ny: {y}\nz: {z}\n w: {w}\n+++++++++++++++++++++\n")
            self.ik_req.pose.orientation.x = x
            self.ik_req.pose.orientation.y = y
            self.ik_req.pose.orientation.z = z
            self.ik_req.pose.orientation.w = w
            
            ik_future = self.inverse_kinematics_client.call_async(self.ik_req)
            ik_future.add_done_callback(partial(self.set_joint_angles))
            self.get_logger().info(f"+++++++++++++++++++++++++++++++++++++++")
            self.send_mani_stuff = True
    def set_joint_angles(self, future):
        result = future.result().joints.position[0:6]
        self.joint_angles = result

    def send_fk_request(self):
        self.fk_req.joints.position = self.joint_angles
        #D1
        self.fk_req.joints.position[0] = self.fk_req.joints.position[0] + 0.08
        #D2
        self.fk_req.joints.position[1] = -1 * (self.fk_req.joints.position[1] - np.pi/2) + 0.03 - 0.505
        #D3
        self.fk_req.joints.position[2] = -1 * (self.fk_req.joints.position[2] - np.pi/2) - 0.785 #was -0.2
        #D4
        self.fk_req.joints.position[3] = self.fk_req.joints.position[3] + 2.79 
        #D5
        self.fk_req.joints.position[4] = self.fk_req.joints.position[4] + 0.02
        #D6
        self.fk_req.joints.position[5] = self.fk_req.joints.position[5] + 0.64 
        
        fk_future = self.forward_kinematics_client.call_async(self.fk_req)
        fk_future.add_done_callback(partial(self.set_point))
    def set_point(self, future):
        result = future.result().pose
        self.translation = [result.position.x, result.position.y, result.position.z]
        self.rotation = self.quaternion_to_rpy(result.orientation.x, result.orientation.y, result.orientation.z, result.orientation.w)



    def update_joint_angles(self, joint_index, direction):
        if self.work_joints and self.general_mani_mode:
            if direction == "up":
                self.joint_angles[joint_index] += self.joint_increment   
            elif direction == "down":
                self.joint_angles[joint_index] -= self.joint_increment

            

            self.send_fk_request()
            self.get_logger().info(f'Publishing joint angles: {self.joint_angles}')


    def update_direction(self, direction):
        if direction == "up":
            self.forward = 0.3
            self.side = 0.0
        elif direction == "down":
            self.forward = -0.3
            self.side = 0.0
        elif direction == "left":
            self.forward = 0.0
            self.side = 0.3
        elif direction == "right":
            self.forward = 0.0
            self.side = -0.3



    def update_ik_translation(self, num: float, direction: str):
        if self.work_ik and self.general_mani_mode:
            if direction == 'x':
                self.translation[0] += num
            elif direction == 'y':
                self.translation[1] += num
            elif direction == 'z':
                self.translation[2] += num
            self.send_ik_request()
            self.get_logger().info(f'Publishing pose: Translation = x: {self.translation[0]:.2f}, y: {self.translation[1]:.2f}, z: {self.translation[2]:.2f}')
            self.get_logger().info(f'Publishing pose: Rotation = r: {self.rotation[0]:.2f}, p: {self.rotation[1]:.2f}, y: {self.rotation[2]:.2f}')
            self.send_mani_stuff = True

    def update_ik_translation_button(self, num: float, direction: str):
        if self.work_ik:
            if direction == 'x':
                self.translation[0] += num
            elif direction == 'y':
                self.translation[1] += num
            elif direction == 'z':
                self.translation[2] += num
            self.send_ik_request()
            self.get_logger().info(f'Publishing pose: Translation = x: {self.translation[0]:.2f}, y: {self.translation[1]:.2f}, z: {self.translation[2]:.2f}')
            self.get_logger().info(f'Publishing pose: Rotation = r: {self.rotation[0]:.2f}, p: {self.rotation[1]:.2f}, y: {self.rotation[2]:.2f}')
        
    def update_ik_translation(self, x_num: float, y_num: float, z_num: float ):
        if self.work_ik:
            self.translation[0] += x_num
            self.translation[1] += y_num
            self.translation[2] += z_num
            self.send_ik_request()
            self.get_logger().info(f'Publishing pose: Translation = x: {self.translation[0]:.2f}, y: {self.translation[1]:.2f}, z: {self.translation[2]:.2f}')
            self.get_logger().info(f'Publishing pose: Rotation = r: {self.rotation[0]:.2f}, p: {self.rotation[1]:.2f}, y: {self.rotation[2]:.2f}')
            self.send_mani_stuff = True


    def update_ik_rotation_button(self, num: float, direction: str):
        if self.work_ik:
            if direction == 'r':
                self.rotation[0] += num
            elif direction == 'p':
                self.rotation[1] += num
            elif direction == 'y':
                self.rotation[2] += num
            self.send_ik_request()
            self.get_logger().info(f'Publishing pose: Translation = x: {self.translation[0]:.2f}, y: {self.translation[1]:.2f}, z: {self.translation[2]:.2f}')
            self.get_logger().info(f'Publishing pose: Rotation = r: {self.rotation[0]:.2f}, p: {self.rotation[1]:.2f}, y: {self.rotation[2]:.2f}')




    def update_ik_rotation(self, r_num: float, p_num: float, y_num: float):
        if self.work_ik:
            self.rotation[0] += r_num
            self.rotation[1] += p_num
            self.rotation[2] += y_num
            self.send_ik_request()
            self.get_logger().info(f'Publishing pose: Translation = x: {self.translation[0]:.2f}, y: {self.translation[1]:.2f}, z: {self.translation[2]:.2f}')
            self.get_logger().info(f'Publishing pose: Rotation = r: {self.rotation[0]:.2f}, p: {self.rotation[1]:.2f}, y: {self.rotation[2]:.2f}')
            self.send_mani_stuff = True


    def rpy_to_quaternion(self, roll: float, pitch: float, yaw: float):
        cy = np.cos(yaw * 0.5)
        sy = np.sin(yaw * 0.5)
        cp = np.cos(pitch * 0.5)
        sp = np.sin(pitch * 0.5)
        cr = np.cos(roll * 0.5)
        sr = np.sin(roll * 0.5)

        w = cr * cp * cy + sr * sp * sy
        x = sr * cp * cy - cr * sp * sy
        y = cr * sp * cy + sr * cp * sy
        z = cr * cp * sy - sr * sp * cy

        return [x, y, z, w]
    

    def tilt_twist_quat(self, psi, theta, phi):
        def quat_from_angle_axis(angle, axis):
            axis = axis / np.linalg.norm(axis)
            w = np.cos(angle/2)
            xyz = np.sin(angle/2) * axis
            return np.array([w, *xyz])
        
        qz = quat_from_angle_axis(psi, np.array([0,0,1]))
        qy = quat_from_angle_axis(theta, np.array([0,1,0]))


        def apply_q(q, v):
            w, x, y, z = q
            uv = np.cross([x,y,z], v)
            uuv = np.cross([x,y,z], uv)
            return v + 2*(w*uv + uuv)
        # combine them: q = qz · qy · qx_body
        # quaternion multiplication:
        def quat_mul(a, b):
            w1, x1, y1, z1 = a
            w2, x2, y2, z2 = b
            return np.array([
                w1*w2 - x1*x2 - y1*y2 - z1*z2,
                w1*x2 + x1*w2 + y1*z2 - z1*y2,
                w1*y2 - x1*z2 + y1*w2 + z1*x2,
                w1*z2 + x1*y2 - y1*x2 + z1*w2
            ])
        x_body = apply_q(quat_mul(qz, qy), np.array([1,0,0]))
        qx_body = quat_from_angle_axis(phi, x_body)
        return quat_mul(quat_mul(qz, qy), qx_body)
    

    def quaternion_to_rpy(self, x: float, y: float, z: float, w: float):
        sinr_cosp = 2 * (w * x + y * z)
        cosr_cosp = 1 - 2 * (x * x + y * y)
        roll = np.arctan2(sinr_cosp, cosr_cosp)
        sinp = 2 * (w * y - z * x)
        if abs(sinp) >= 1:
            pitch = np.pi/2 * np.sign(sinp)
        else:
            pitch = np.arcsin(sinp)
        siny_cosp = 2 * (w * z + x * y)
        cosy_cosp = 1 - 2 * (y * y + z * z)
        yaw = np.arctan2(siny_cosp, cosy_cosp)

        return [roll, pitch, yaw]
    

    def home(self):
        self.joint_angles = [0.0, 0.0, 0.0, 0.0, 0.0, 0.0]
        self.translation = [0.7, 0.0, 1.0]
        self.rotation = [0.0,np.pi/2,0.0]
        joint_state = JointState()
        joint_state.header.stamp = self.get_clock().now().to_msg()
        joint_state.name = self.joint_names
        joint_state.position = self.joint_angles
        self.joint_state_publisher.publish(joint_state)
        self.get_logger().info(f'HOME')
        

    def set_gripper_pose_one(self):
        self.rotation = [0.0,np.pi/2,0.0]
        joint_state = JointState()
        joint_state.header.stamp = self.get_clock().now().to_msg()
        joint_state.name = self.joint_names
        joint_state.position = self.joint_angles
        self.joint_state_publisher.publish(joint_state)

    def set_gripper_pose_two(self):
        self.rotation = [0.0,0.021,0.0]
        joint_state = JointState()
        joint_state.header.stamp = self.get_clock().now().to_msg()
        joint_state.name = self.joint_names
        joint_state.position = self.joint_angles
        self.joint_state_publisher.publish(joint_state)

    def update_gripper_state(self, gripper):
        msg = String()
        if gripper == 0:
            msg.data = "stop"
        if gripper == 1:
            msg.data = "close"
        if gripper == -1:
            msg.data = "open"
        self.get_logger().info(f"{msg.data}")
        self.gripper_state_publisher.publish(msg)

    def publish_cmd(self):
        if (self.work_wheel):
            cmd = Twist()
            cmd.linear.x = self.forward
            cmd.angular.z = self.side

            self.cmd_pub.publish(cmd)
            self.get_logger().info(f'Publishing cmd: linear.x={cmd.linear.x:.2f}, angular.z={cmd.angular.z:.2f}')

    def publish_joint_state(self):
        # if self.send_mani_stuff:
        self.get_logger().info("SENDIN MANI STUFF")
        joint_state = JointState()
        joint_state.header.stamp = self.get_clock().now().to_msg()
        joint_state.name = self.joint_names
        joint_state.position = self.joint_angles
        self.joint_state_publisher.publish(joint_state)
        # self.send_mani_stuff = False
        





def initialize_gui(node: Teleoperator):

    global joy

    def joystick_loop(node):
        global joy
        prev_buttons = [0] * joy.get_numbuttons()
        prev_joy = [0.0] * 6

        while True:
            pygame.event.pump()
            
            # MODES
            if joy.get_button(8) and not prev_buttons[8]:
                node.toggle_general_mode()
            if joy.get_button(6) and not prev_buttons[6]:
                node.work_joints = True
                node.work_ik = False
            if joy.get_button(7) and not prev_buttons[7]:
                node.work_joints = False
                node.work_ik = True
            if joy.get_button(2) and not prev_buttons[2]:
                node.set_gripper_pose_one()
            if joy.get_button(3) and not prev_buttons[3]:
                node.set_gripper_pose_two()

            # DRIVE MODE
            if node.general_wheels_mode:
                forward = joy.get_axis(1) * 0.4
                side = joy.get_axis(0) * 0.4
                node.forward = -forward if abs(forward) > 0.05 else 0.0
                node.side = -side if abs(side) > 0.05 else 0.0
                node.work_wheel = True
            else:
                node.work_wheel = False


            # MANI MODE
            if node.general_mani_mode:
                # IK MODE
                if node.work_ik:
                    x_motion = -joy.get_axis(4) if abs(joy.get_axis(4)) > 0.05 else 0.0
                    y_motion = -joy.get_axis(0) if abs(joy.get_axis(0)) > 0.05 else 0.0
                    z_motion = -joy.get_axis(1) if abs(joy.get_axis(1)) > 0.05 else 0.0
                    mul = 0.008
                    x_motion = x_motion * mul
                    y_motion = y_motion * mul
                    z_motion = z_motion * mul

                    if not (x_motion == 0.0 and y_motion == 0.0 and z_motion == 0.0):
                        node.update_ik_translation(x_motion, y_motion, z_motion)

                    rr_motion = 0.0
                    pp_motion = 0.0
                    yy_motion = 0.0
                    gripper = 0

                    mul = 0.03

                    hat = joy.get_hat(0)

                    if hat[0] == -1 and hat[1] == 0:
                        rr_motion = mul
                    if hat[0] == 1 and hat[1] == 0:
                        rr_motion = -mul

                    if hat[0] == 0 and hat[1] == 1:
                        pp_motion = mul
                    if hat[0] == 0 and hat[1] == -1:
                        pp_motion = -mul

                    if joy.get_axis(5) > 0.5:
                        yy_motion = mul
                    if joy.get_axis(2) > 0.5:
                        yy_motion = -mul


                    if not (rr_motion == 0 and yy_motion == 0 and pp_motion == 0):
                        node.update_ik_rotation(rr_motion, pp_motion, yy_motion)



                    if joy.get_button(4) != prev_buttons[4]:
                        if joy.get_button(4):
                            gripper = 1
                        if not joy.get_button(4):
                            gripper = 0
                        node.update_gripper_state(gripper)
                        
                    if joy.get_button(5) != prev_buttons[5]:
                        if joy.get_button(5):
                            gripper = -1
                        if not joy.get_button(5):
                            gripper = 0
                        node.update_gripper_state(gripper)

                    if joy.get_button(3):
                        node.set_gripper_pose_one()
                    if joy.get_button(0):
                        node.set_gripper_pose_two()


            prev_buttons = [joy.get_button(i) for i in range(joy.get_numbuttons())]

        
            time.sleep(0.1)


    def update_labels():
        dof1_label.config(text=f"{node.joint_angles[0]:.2}")
        dof2_label.config(text=f"{node.joint_angles[1]:.2}")
        dof3_label.config(text=f"{node.joint_angles[2]:.2}")
        dof4_label.config(text=f"{node.joint_angles[3]:.2}")
        dof5_label.config(text=f"{node.joint_angles[4]:.2}")
        dof6_label.config(text=f"{node.joint_angles[5]:.2}")
        x_label.config(text=f"{node.translation[0]:.4}")
        y_label.config(text=f"{node.translation[1]:.4}")
        z_label.config(text=f"{node.translation[2]:.4}")
        rr_label.config(text=f"{node.rotation[0]:.4}")
        pp_label.config(text=f"{node.rotation[1]:.4}")
        yy_label.config(text=f"{node.rotation[2]:.4}")
        mani_mode_text = "ENABLE IK" if node.work_joints else "ENABLE JOINTS"
        switch_mani_mode.config(text=mani_mode_text)
        mani_status_text = "Forward Mode" if node.work_joints else "Inverse Mode"
        manipulator_status.config(text=mani_status_text)
        root.after(100, update_labels)

    def press_wheel(node, direction):
        node.update_direction(direction)
        node.work_wheel = True
    def release_wheel(node):
        node.forward = 0.0
        node.side = 0.0
        node.publish_cmd()
        node.work_wheel = False


    def update_joint_increment(event, node):
        try:
            new_value = float(event.widget.get())
            if new_value > 0.0:
                node.joint_increment = new_value
                print(f"[JOINT_INCREMENT] {new_value}")
            else:
                print("[JOINT_INCREMENT] Lt0 - change unaccepted")
        except ValueError:
            print("[JOINT_INCREMENT] NaN - change unaccepted")
    def update_translation_increment(event, node):
        try:
            new_value = float(event.widget.get())
            if new_value > 0.0:
                node.translation_increment = new_value
                print(f"[TRANSLATION_INCREMENT] {new_value}")
            else:
                print("[TRANSLATION_INCREMENT] Lt0 - change unaccepted")
        except ValueError:
            print("[TRANSLATION_INCREMENT] NaN - change unaccepted")
    def update_rotation_increment(event, node):
        try:
            new_value = float(event.widget.get())
            if new_value > 0.0:
                node.rotation_increment = new_value
                print(f"[ROTATION_INCREMENT] {new_value}")
            else:
                print("[ROTATION_INCREMENT] Lt0 - change unaccepted")
        except ValueError:
            print("[ROTATION_INCREMENT] NaN - change unaccepted")

    def initialize_joystick():
        global joy
        pygame.init()
        pygame.joystick.init()

        if pygame.joystick.get_count() == 0:
            print("No controller detected")

        joy = pygame.joystick.Joystick(0)
        joy.init()
        node.general_mani_mode = False
        joystick_thread = threading.Thread(target=joystick_loop, args=(node,), daemon=True)
        joystick_thread.start()





    root = tk.Tk()
    root.title("TELEOPERATOR")
    root.configure(bg="#7a9dde")
    root.geometry("850x600")
    root.resizable(False, False)

    frame = tk.Frame(root)
    frame.configure(bg="#494b4f")
    frame.pack(expand=True, fill=tk.BOTH)

    wheel_title = tk.Label(frame, text="WHEELS", font=("Arial", 10))
    joints_title = tk.Label(frame, text="JOINTS", font=("Arial", 10))
    inverse_title = tk.Label(frame, text="INVERSE", font=("Arial", 10))
    manipulator_status = tk.Label(frame, text="Joint Mode", font=("Arial", 10))

    button_up_wheel = tk.Button(frame, bg="#fa3a42", fg="#ffffff", text="UP", width=10, height=2)
    button_down_wheel = tk.Button(frame, bg="#fa3a42", fg="#ffffff", text="DOWN", width=10, height=2)
    button_left_wheel = tk.Button(frame, bg="#fa3a42", fg="#ffffff", text="LEFT", width=10, height=2)
    button_right_wheel = tk.Button(frame, bg="#fa3a42", fg="#ffffff", text="RIGHT", width=10, height=2)

    switch_mani_mode = tk.Button(frame, bg="#eab676", fg="#ffffff", text="ENABLE IK", command=lambda: node.toggle_mani_mode(), width=10, height=2)
    init_joy = tk.Button(frame, bg="#eab676", fg="#ffffff", text="ENABLE JOYSTICK", command=lambda: initialize_joystick(), width=10, height=2)

    dof1_up = tk.Button(frame, bg="#fa3a42", fg="#ffffff", text="DOF1 UP", command=lambda: node.update_joint_angles(0,"up"), width=10, height=2)
    dof1_down = tk.Button(frame, bg="#fa3a42", fg="#ffffff", text="DOF1 DOWN", command=lambda: node.update_joint_angles(0,"down"), width=10, height=2)
    dof2_up = tk.Button(frame, bg="#fa3a42", fg="#ffffff", text="DOF2 UP", command=lambda: node.update_joint_angles(1,"up"), width=10, height=2)
    dof2_down = tk.Button(frame, bg="#fa3a42", fg="#ffffff", text="DOF2 DOWN", command=lambda: node.update_joint_angles(1,"down"), width=10, height=2)
    dof3_up = tk.Button(frame, bg="#fa3a42", fg="#ffffff", text="DOF3 UP", command=lambda: node.update_joint_angles(2,"up"), width=10, height=2)
    dof3_down = tk.Button(frame, bg="#fa3a42", fg="#ffffff", text="DOF3 DOWN", command=lambda: node.update_joint_angles(2,"down"), width=10, height=2)
    dof4_up = tk.Button(frame, bg="#fa3a42", fg="#ffffff", text="DOF4 UP", command=lambda: node.update_joint_angles(3,"up"), width=10, height=2)
    dof4_down = tk.Button(frame, bg="#fa3a42", fg="#ffffff", text="DOF4 DOWN", command=lambda: node.update_joint_angles(3,"down"), width=10, height=2)
    dof5_up = tk.Button(frame, bg="#fa3a42", fg="#ffffff", text="DOF5 UP", command=lambda: node.update_joint_angles(4,"up"), width=10, height=2)
    dof5_down = tk.Button(frame, bg="#fa3a42", fg="#ffffff", text="DOF5 DOWN", command=lambda: node.update_joint_angles(4,"down"), width=10, height=2)
    dof6_up = tk.Button(frame, bg="#fa3a42", fg="#ffffff", text="DOF6 UP", command=lambda: node.update_joint_angles(5,"up"), width=10, height=2)
    dof6_down = tk.Button(frame, bg="#fa3a42", fg="#ffffff", text="DOF6 DOWN", command=lambda: node.update_joint_angles(5,"down"), width=10, height=2)

    x_up = tk.Button(frame, bg="#eab676", fg="#ffffff", text="X UP", command=lambda: node.update_ik_translation_button(node.translation_increment,'x'), width=10, height=2)
    x_down = tk.Button(frame, bg="#eab676", fg="#ffffff", text="X DOWN", command=lambda: node.update_ik_translation_button(-node.translation_increment,'x'), width=10, height=2)
    y_up = tk.Button(frame, bg="#eab676", fg="#ffffff", text="Y UP", command=lambda: node.update_ik_translation_button(node.translation_increment,'y'), width=10, height=2)
    y_down = tk.Button(frame, bg="#eab676", fg="#ffffff", text="Y DOWN", command=lambda: node.update_ik_translation_button(-node.translation_increment,'y'), width=10, height=2)
    z_up = tk.Button(frame, bg="#eab676", fg="#ffffff", text="Z UP", command=lambda: node.update_ik_translation_button(node.translation_increment,'z'), width=10, height=2)
    z_down = tk.Button(frame, bg="#eab676", fg="#ffffff", text="Z DOWN", command=lambda: node.update_ik_translation_button(-node.translation_increment,'z') ,width=10, height=2)

    rr_up = tk.Button(frame, bg="#2596be", fg="#ffffff", text="R UP", command=lambda: node.update_ik_rotation_button(node.rotation_increment,'r'), width=10, height=2)
    rr_down = tk.Button(frame, bg="#2596be", fg="#ffffff", text="R DOWN", command=lambda: node.update_ik_rotation_button(-node.rotation_increment,'r'), width=10, height=2)
    pp_up = tk.Button(frame, bg="#2596be", fg="#ffffff", text="P UP", command=lambda: node.update_ik_rotation_button(node.rotation_increment,'p'), width=10, height=2)
    pp_down = tk.Button(frame, bg="#2596be", fg="#ffffff", text="P DOWN", command=lambda: node.update_ik_rotation_button(-node.rotation_increment,'p'), width=10, height=2)
    yy_up = tk.Button(frame, bg="#2596be", fg="#ffffff", text="Y UP", command=lambda: node.update_ik_rotation_button(node.rotation_increment,'y'), width=10, height=2)
    yy_down = tk.Button(frame, bg="#2596be", fg="#ffffff", text="Y DOWN", command=lambda: node.update_ik_rotation_button(-node.rotation_increment,'y') ,width=10, height=2)

    home = tk.Button(frame, bg="#8aba54", fg="#ffffff", text="HOME", command=node.home ,width=10, height=2)

    joint_entry = tk.Entry(frame, width=10)
    joint_entry.insert(0, "0.1")
    translation_entry = tk.Entry(frame, width=10)
    translation_entry.insert(0, "0.02")
    rotation_entry = tk.Entry(frame, width=10)
    rotation_entry.insert(0, "0.02")

    joint_label = tk.Label(frame, text=f"joint inc", font=("Arial", 10))
    translation_label = tk.Label(frame, text=f"translation inc", font=("Arial", 10))
    rotation_label = tk.Label(frame, text=f"rotation inc", font=("Arial", 10))
    dof1_label = tk.Label(frame, bg="#ff5733", fg="#ffffff", text=f"{node.joint_angles[0]:.4}", font=("Arial", 10))
    dof2_label = tk.Label(frame, bg="#ff5733", fg="#ffffff", text=f"{node.joint_angles[1]:.4}", font=("Arial", 10))
    dof3_label = tk.Label(frame, bg="#ff5733", fg="#ffffff", text=f"{node.joint_angles[2]:.4}", font=("Arial", 10))
    dof4_label = tk.Label(frame, bg="#ff5733", fg="#ffffff", text=f"{node.joint_angles[3]:.4}", font=("Arial", 10))
    dof5_label = tk.Label(frame, bg="#ff5733", fg="#ffffff", text=f"{node.joint_angles[4]:.4}", font=("Arial", 10))
    dof6_label = tk.Label(frame, bg="#ff5733", fg="#ffffff", text=f"{node.joint_angles[5]:.4}", font=("Arial", 10))
    x_label = tk.Label(frame, bg="#ff5733", fg="#ffffff", text=f"{node.translation[0]:.4}", font=("Arial", 10))
    y_label = tk.Label(frame, bg="#ff5733", fg="#ffffff", text=f"{node.translation[1]:.4}", font=("Arial", 10))
    z_label = tk.Label(frame, bg="#ff5733", fg="#ffffff", text=f"{node.translation[2]:.4}", font=("Arial", 10))
    rr_label = tk.Label(frame, bg="#ff5733", fg="#ffffff", text=f"{node.rotation[0]:.4}", font=("Arial", 10))
    pp_label = tk.Label(frame, bg="#ff5733", fg="#ffffff", text=f"{node.rotation[1]:.4}", font=("Arial", 10))
    yy_label = tk.Label(frame, bg="#ff5733", fg="#ffffff", text=f"{node.rotation[2]:.4}", font=("Arial", 10))

    button_up_wheel.bind('<ButtonPress-1>', lambda e: press_wheel(node,"up"))
    button_up_wheel.bind('<ButtonRelease>', lambda e: release_wheel(node))
    button_down_wheel.bind('<ButtonPress-1>', lambda e: press_wheel(node,"down"))
    button_down_wheel.bind('<ButtonRelease>', lambda e: release_wheel(node))
    button_left_wheel.bind('<ButtonPress-1>', lambda e: press_wheel(node,"left"))
    button_left_wheel.bind('<ButtonRelease>', lambda e: release_wheel(node))
    button_right_wheel.bind('<ButtonPress-1>', lambda e: press_wheel(node,"right"))
    button_right_wheel.bind('<ButtonRelease>', lambda e: release_wheel(node))

    joint_entry.bind("<Return>", lambda event: update_joint_increment(event, node))
    translation_entry.bind("<Return>", lambda event: update_translation_increment(event, node))
    rotation_entry.bind("<Return>", lambda event: update_rotation_increment(event, node))

    separator = tk.Label(frame, text="", width=10, bg="#494b4f")
    separator.grid(row=6, column=4)

    wheel_title.grid(row=0, column=1, padx=5, pady=5)
    joints_title.grid(row=5, column=1, padx=5, pady=5)
    inverse_title.grid(row=5, column=6, padx=5, pady=5)
    button_up_wheel.grid(row=1, column=1, padx=5, pady=5)
    button_left_wheel.grid(row=2, column=0, padx=5, pady=5)
    button_right_wheel.grid(row=2, column=2, padx=5, pady=5)
    button_down_wheel.grid(row=3, column=1, padx=5, pady=5)
    dof1_up.grid(row=6, column=0, padx=5, pady=5)
    dof1_down.grid(row=6, column=2, padx=5, pady=5)
    dof2_up.grid(row=7, column=0, padx=5, pady=5)
    dof2_down.grid(row=7, column=2, padx=5, pady=5)
    dof3_up.grid(row=8, column=0, padx=5, pady=5)
    dof3_down.grid(row=8, column=2, padx=5, pady=5)
    dof4_up.grid(row=9, column=0, padx=5, pady=5)
    dof4_down.grid(row=9, column=2, padx=5, pady=5)
    dof5_up.grid(row=10, column=0, padx=5, pady=5)
    dof5_down.grid(row=10, column=2, padx=5, pady=5)
    dof6_up.grid(row=11, column=0, padx=5, pady=5)
    dof6_down.grid(row=11, column=2, padx=5, pady=5)
    dof1_label.grid(row=6, column=1, padx=5, pady=5)
    dof2_label.grid(row=7, column=1, padx=5, pady=5)
    dof3_label.grid(row=8, column=1, padx=5, pady=5)
    dof4_label.grid(row=9, column=1, padx=5, pady=5)
    dof5_label.grid(row=10, column=1, padx=5, pady=5)
    dof6_label.grid(row=11, column=1, padx=5, pady=5)
    x_label.grid(row=6, column=6, padx=5, pady=5)
    y_label.grid(row=7, column=6, padx=5, pady=5)
    z_label.grid(row=8, column=6, padx=5, pady=5)
    rr_label.grid(row=9, column=6, padx=5, pady=5)
    pp_label.grid(row=10, column=6, padx=5, pady=5)
    yy_label.grid(row=11, column=6, padx=5, pady=5)

    x_up.grid(row=6, column=5, padx=5, pady=5)
    x_down.grid(row=6, column=7, padx=5, pady=5)
    y_up.grid(row=7, column=5, padx=5, pady=5)
    y_down.grid(row=7, column=7, padx=5, pady=5)
    z_up.grid(row=8, column=5, padx=5, pady=5)
    z_down.grid(row=8, column=7, padx=5, pady=5)

    rr_up.grid(row=9, column=5, padx=5, pady=5)
    rr_down.grid(row=9, column=7, padx=5, pady=5)
    pp_up.grid(row=10, column=5, padx=5, pady=5)
    pp_down.grid(row=10, column=7, padx=5, pady=5)
    yy_up.grid(row=11, column=5, padx=5, pady=5)
    yy_down.grid(row=11, column=7, padx=5, pady=5)

    home.grid(row=1, column=2, padx=5, pady=5)

    joint_entry.grid(row=1, column=6, padx=5, pady=5)
    translation_entry.grid(row=2, column=6, padx=5, pady=5)
    rotation_entry.grid(row=3, column=6, padx=5, pady=5)
    joint_label.grid(row=1,column=5,padx=5,pady=5)
    translation_label.grid(row=2,column=5,padx=5,pady=5)
    rotation_label.grid(row=3,column=5,padx=5,pady=5)

    switch_mani_mode.grid(row=3,column=2,padx=5,pady=5)
    init_joy.grid(row=1,column=0,padx=5,pady=5)
    manipulator_status.grid(row=0,column=5,padx=5,pady=5)


    update_labels()

    root.mainloop()





def main(args=None):
    rclpy.init(args=args)
    teleop_wheel = Teleoperator()

    gui_thread = threading.Thread(target=initialize_gui, args=(teleop_wheel,))
    gui_thread.start()

    try:
        rclpy.spin(teleop_wheel)
    except KeyboardInterrupt:
        pass
    
    gui_thread.join()
    teleop_wheel.destroy_node()
    rclpy.shutdown()

if __name__ == '__main__':
    main()