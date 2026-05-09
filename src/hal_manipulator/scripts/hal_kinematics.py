#! /usr/bin/env python3

import rclpy
from rclpy.node import Node
import rclpy.publisher
from sensor_msgs.msg import JointState
from geometry_msgs.msg import Pose
from std_msgs.msg import Float64
import sys, os, PyKDL
import numpy as np
from hal_interfaces.srv import InverseKinematics
from hal_interfaces.srv import ForwardKinematics
from rclpy.qos import QoSProfile

sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), "..")))
from hal_manipulator.forward_kinematics import ForwardKinematics as FK
from hal_manipulator.inverse_kinematics import InverseKinematics as IK


class HalKinematics(Node):
    def __init__(self):
        super().__init__("hal_kinematics")
        self.get_logger().info("[hal_kinematics][VERSION 1.0]")
        
        # PARAM LOADING
        try:
            dh_param_names = [
                'a0','a1','a2','a3','a4','a5','alpha0','alpha1','alpha2','alpha3',
                'alpha4','alpha5','d1','d2','d3','d4','d5','d6','d_theta1','d_theta2',
                'd_theta3','d_theta4','d_theta5','d_theta6'    
            ]
            limit_param_names = ['d1_lim','d2_lim','d3_lim','d4_lim','d5_lim','d6_lim']
            self.declare_parameters(
                namespace='',
                parameters=[(name, rclpy.Parameter.Type.DOUBLE) for name in dh_param_names]
            )
            self.declare_parameters(
                namespace='',
                parameters=[(name, rclpy.Parameter.Type.DOUBLE_ARRAY) for name in limit_param_names]
            )
            for param_name in dh_param_names:
                value = self.get_param_double_value(param_name)
                setattr(self, param_name, value)
            for param_name in limit_param_names:
                value = self.get_param_double_array_value(param_name)
                setattr(self, param_name, value)
            self.get_logger().info("[hal_kinematics] PARAM-LOAD SUCCESSFULL")
        except:
            self.get_logger().fatal("[hal_kinematics] PARAM-LOAD FAILED")

        # PARAM to VARIABLE
        self.a = [self.a0,self.a1,self.a2,self.a3,self.a4,self.a5]
        self.alpha = [self.alpha0, self.alpha1, self.alpha2, self.alpha3, self.alpha4, self.alpha5]
        self.d = [self.d1, self.d2, self.d3, self.d4, self.d5, self.d6]
        self.d_theta = [self.d_theta1, self.d_theta2, self.d_theta3, self.d_theta4, self.d_theta5, self.d_theta6]
        self._dh_params = {"a":self.a,"alpha":self.alpha,"d":self.d,"d_theta":self.d_theta}
        self._joint_limits = [
            [self.d1_lim[0], self.d1_lim[1]],                          
            [self.d2_lim[0], self.d2_lim[1]],                           
            [self.d3_lim[0], self.d3_lim[1]],                          
            [self.d4_lim[0], self.d4_lim[1]],                         
            [self.d5_lim[0], self.d5_lim[1]],                         
            [self.d6_lim[0], self.d6_lim[1]]
        ]

        # SOLVERS
        self._fk_solver = FK(self._dh_params)
        self._ik_solver = IK(self._dh_params,self._joint_limits)
        self._joint_state_memory = [0.0, 0.0, 0.0, 0.0, 0.0, 0.0]
        # ROS2 STUFF

        self._ik_service = self.create_service(InverseKinematics,'/inverse_kinematics',self.inverse_kinematics_callback,)
        self._fk_service = self.create_service(ForwardKinematics,'/forward_kinematics',self.forward_kinematics_callback)

    # JUST SOME PARAMLOADERS
    def get_param_double_value(self, param_name):
        return self.get_parameter(param_name).get_parameter_value().double_value
    def get_param_double_array_value(self, param_name):
        return self.get_parameter(param_name).get_parameter_value().double_array_value



    def inverse_kinematics_callback(self, req, resp):
        """get point and do the magic"""
        self.get_logger().info("[hal_manipulator] got req")
        # GET GOAL POINT
        goal_position = PyKDL.Vector(req.pose.position.x,req.pose.position.y,req.pose.position.z)
        goal_rotation = PyKDL.Rotation.Quaternion(req.pose.orientation.x,req.pose.orientation.y,req.pose.orientation.z,req.pose.orientation.w)
        self._goal_pose = PyKDL.Frame(goal_rotation,goal_position)
        # CALCULATE ANGLES
        try:
            solutions = self._ik_solver.inverse_kinematics(self._goal_pose, self._joint_state_memory)   # it should be a list
            self._goal_theta = solutions[0]
            self.get_logger().info(f"[hal_manipulator] new: {self._goal_theta._ik_joints}")
            self.get_logger().info(f"[hal_manipulator] Found {len(solutions)} solutions")

            # SEND JOINT STATES
            if self._goal_theta is not None:
                now = self.get_clock().now()
                resp.joints.header.stamp = now.to_msg()
                resp.joints.position = [float(self._goal_theta._ik_joints[i]) for i in range(6)]
                #D1
                resp.joints.position[0] = resp.joints.position[0] - 0.08 + 0.025    #calibrated
                #D2
                resp.joints.position[1] = -1 * (resp.joints.position[1] + np.pi/2) - 0.03 + 0.505
                #D3
                resp.joints.position[2] = -1 * (resp.joints.position[2] - np.pi/2) -0.785 #was -0.2
                #D4
                resp.joints.position[3] = resp.joints.position[3] + 2.79 - 3.14 - 2.35 - 0.10   #calibrated
                #D5
                resp.joints.position[4] = resp.joints.position[4] + 0.02
                #D6
                resp.joints.position[5] = resp.joints.position[5] + 0.64 - 0.436 + 0.20   #calibrated
                resp.joints.name = ['dof1','dof2','dof3','dof4','dof5','dof6']
                self._joint_state_memory = resp.joints.position
                resp.success = True
            else:
                now = self.get_clock().now()
                resp.joints.header.stamp = now.to_msg()
                resp.joints.position = self._joint_state_memory
                resp.joints.name = ['dof1','dof2','dof3','dof4','dof5','dof6']
                resp.success = False
                self.get_logger().info(f"[hal_manipulator] IT DIDNT WORK")
        except:
            now = self.get_clock().now()
            resp.joints.header.stamp = now.to_msg()
            resp.joints.position = self._joint_state_memory
            resp.joints.name = ['dof1','dof2','dof3','dof4','dof5','dof6']
            resp.success = False
            self.get_logger().info(f"[hal_manipulator] IT DIDNT WORK")
        finally:
            self.get_logger().info(f"------------------------")
            self.get_logger().info(f"[d1] {resp.joints.position[0]}")
            self.get_logger().info(f"[d2] {resp.joints.position[1]}")
            self.get_logger().info(f"[d3] {resp.joints.position[2]}")
            self.get_logger().info(f"[d4] {resp.joints.position[3]}")
            self.get_logger().info(f"[d5] {resp.joints.position[4]}")
            self.get_logger().info(f"[d6] {resp.joints.position[5]}")
            self.get_logger().info(f"------------------------")
            return resp
        

    def forward_kinematics_callback(self, req, resp):
        angles = req.joints.position[0:6]
        self._joint_state_memory = angles
        solution = self._fk_solver.forward_kinematics(angles)
        translation = solution.p
        rotation = solution.M.GetQuaternion()
        resp.pose.position.x = float(translation[0])
        resp.pose.position.y = float(translation[1])
        resp.pose.position.z = float(translation[2])
        resp.pose.orientation.x = float(rotation[0])
        resp.pose.orientation.y = float(rotation[1])
        resp.pose.orientation.z = float(rotation[2])
        resp.pose.orientation.w = float(rotation[3])
        return resp


def main():
    
    rclpy.init()
    node = HalKinematics()
    rclpy.spin(node)

    node.get_logger().info("[hal_manipulator] Shutting down")
    node.destroy_node()

    rclpy.shutdown()

if __name__ == "__main__":
    main()
    
            
