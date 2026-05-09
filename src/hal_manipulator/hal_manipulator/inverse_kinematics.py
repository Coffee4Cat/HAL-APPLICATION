#!/usr/bin/env python

import sys
import os
import PyKDL
import numpy as np

sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), "..")))

class IKSolution:
    def __init__(
        self,
        id: int,
        ik_joints: list,
        joint_state: list = [],
        joint_limits: list = [],
    ):
        self._id = id
        self._ik_joints = ik_joints
        self._joint_limits = joint_limits
        self._value = (
            self.evaluate_local_solution(joint_state)
            if joint_state
            else self.evaluate_global_solution()
        )
        # self._value = self.evaluate_local_solution(joint_state)*0.9 + self.evaluate_global_solution()*0.1
        # self._value = self.evaluate_local_solution(joint_state)

    def get_solution(self):
        return self._ik_joints

    def evaluate(self, joint_states=[]):
        self._value = (
            self.evaluate_local_solution(joint_states)
            if joint_states
            else self.evaluate_global_solution()
        )

    def evaluate_global_solution(self):
        value = 0
        for id in range(len(self._joint_limits)):
            if not self._ik_joints[id]:
                return 1000000   # to może być zle
            lower, upper = self._joint_limits[id]
            normal = (self._ik_joints[id] - lower) / (upper - lower)
            # value += (normal - (lower + upper) / 2) ** 2
            value += (normal - 0.5) ** 2
        return value / len(self._joint_limits)

    def evaluate_local_solution(self, joint_state):
        return sum(
            [
                (joint_state[id] - self._ik_joints[id]) ** 2
                for id in range(len(joint_state))
            ]
        ) / len(joint_state)

    def id(self):
        return self._id

    def value(self):
        return self._value

    def __str__(self):
        str_list = [
            f"theta{i}: {self._ik_joints[i]}, {np.degrees(self._ik_joints[i])}"
            for i in range(len(self._ik_joints))
        ]
        return "\n".join(str_list)







class InverseKinematics:
    """
    Class representation of the inverse kinematic task.
    """

    def __init__(self, dh_params: dict, joint_limits: list, tolerance=5*1e-4) -> None:
        """
        Initialize the InverseKinematics class.

        Parameters:
            dh_params: parameters of the Denavit-Hartenberg notation
            joint_limits:
        """
        self._dh_params_dict = dh_params
        self._dh_params = self.extract_dh_params(dh_params)
        self._joint_limits = joint_limits
        self._tolerance = tolerance

    def extract_dh_params(self, params):
        extracted_params = {}

        dh_param_indices = [
            ("a", 0),
            ("alpha", 0),
            ("d", 1),
            ("d_theta", 1),
        ]

        for key, start in dh_param_indices:
            for i in range(len(params[key])):
                extracted_params[f"{key}{i+start}"] = params[key][i]

        return extracted_params

    def extract_rotation(self, rotation: PyKDL.Rotation):
        params = {
            "r11": rotation[0, 0],
            "r12": rotation[0, 1],
            "r13": rotation[0, 2],
            "r21": rotation[1, 0],
            "r22": rotation[1, 1],
            "r23": rotation[1, 2],
            "r31": rotation[2, 0],
            "r32": rotation[2, 1],
            "r33": rotation[2, 2],
        }
        return params

    def inverse_kinematics(
        self, goal_pose: PyKDL.Frame, joint_state: list = []
    ) -> list:
        """
        Solves the inverse kinematics problem for a given goal pose.

        Parameters:
            goal_pose: The desired pose of the end-effector.
            joint_state: The current joint states of the manipulator.

        Returns:
            A sorted list of possible joint configurations (solutions) that
            achieve the goal pose.
        """

        position = goal_pose.p
        rotation = goal_pose.M

        params = self.extract_rotation(rotation=rotation)

        r13 = params["r13"]
        r23 = params["r23"]
        r33 = params["r33"]
        d6 = self._dh_params["d6"]
        d1 = self._dh_params["d1"]

        position_origin_5_in_0 = position - PyKDL.Vector(r13, r23, r33) * d6
        distance_origin_5_0 = np.sqrt(
            (position_origin_5_in_0[2] - d1) ** 2
            + position_origin_5_in_0[0] ** 2
            + position_origin_5_in_0[1] ** 2
        )

        params["p05"] = position_origin_5_in_0
        params["b"] = distance_origin_5_0

        # #######################################
        potential_solutions = self.compute_solutions(params=params)
        solutions = []

        id = 1
        if len(potential_solutions) == 0:
            return None
        for solution in potential_solutions:
            is_solution = True


            for i in range(len(solution)):
                if not solution[i]:
                    is_solution = False
                    break

                # remap_theta = self.wrap_to_pi(
                #     solution[i] - self._dh_params[f"d_theta{i+1}"]
                # )
                remap_theta = self.wrap_to_pi_ranged(
                    solution[i], i
                )

                if (
                    remap_theta >= self._joint_limits[i][0]
                    and remap_theta <= self._joint_limits[i][1]
                ):
                    solution[i] = remap_theta
                else:
                    is_solution = False
                    break

            if is_solution:
                solutions.append(
                    IKSolution(
                        id=id,
                        ik_joints=solution,
                        joint_limits=self._joint_limits,
                        joint_state=joint_state,
                    )
                )
            id += 1

        return self.sort_solutions(solutions=solutions)

    def compute_solutions(self, params):
        solutions = []

        theta1_solutions = self.__get_theta1(params=params)
        theta2_solutions = self.__get_theta2(params=params)
        theta3_solutions = self.__get_theta3(params=params)

        solutions_theta123 = [
            [theta1_solutions[0], theta2_solutions[0], theta3_solutions[0]],
            [theta1_solutions[0], theta2_solutions[1], theta3_solutions[1]],
            [theta1_solutions[1], theta2_solutions[2], theta3_solutions[0]],
            [theta1_solutions[1], theta2_solutions[3], theta3_solutions[1]],
        ]

        for theta1, theta2, theta3 in solutions_theta123:
            solutions_theta456 = self.__get_theta456(
                goal_theta1=theta1,
                goal_theta2=theta2,
                goal_theta3=theta3,
                params=params,
            )

            for theta4, theta5, theta6 in solutions_theta456:
                solutions.append([theta1, theta2, theta3, theta4, theta5, theta6])
        return solutions

    def __get_theta1(self, params):
        theta1_solutions = []

        p05 = params["p05"]
        d2 = self._dh_params["d2"]
        d3 = self._dh_params["d3"]
        d5 = self._dh_params["d5"]

        arctan = np.arctan2(p05[1], p05[0])
        arcsin = np.arcsin((d2 + d3 + d5) / np.sqrt(p05[1] ** 2 + p05[0] ** 2))

        goal_theta1 = arctan + arcsin
        theta1_solutions.append(goal_theta1)

        goal_theta1 = np.pi + arctan - arcsin
        theta1_solutions.append(goal_theta1)

        return theta1_solutions

    def __get_theta3(self, params):
        theta3_solutions = []

        a2 = self._dh_params["a2"]
        d4 = self._dh_params["d4"]
        b = params["b"]

        cos_beta = (a2**2 + d4**2 - b**2) / (2 * a2 * d4)

        # ****************
        if cos_beta <= 1 + self._tolerance and cos_beta >= -1 - self._tolerance:
            cos_beta = np.clip(cos_beta, -1.0, 1.0)
        # print(cos_beta)
        # if cos_beta > 1 + self._tolerance or cos_beta < -1 - self._tolerance:
        #     raise ValueError(f"Invalid value for arccos: {cos_beta} (outside acceptable range)")
        # cos_beta = np.clip(cos_beta, -1.0, 1.0)

        # cos_beta = np.clip(cos_beta, -1.0, 1.0)
        # ****************

        beta = np.arccos(cos_beta)

        if cos_beta < 0:
            goal_theta3 = beta - np.pi / 2
        elif cos_beta > 0:
            goal_theta3 = beta + 3 * np.pi / 2

        theta3_solutions.append(goal_theta3)

        goal_theta3 = -beta + 3 * np.pi / 2
        theta3_solutions.append(goal_theta3)

        return theta3_solutions

    def __get_theta2(self, params):
        theta2_solutions = []

        b = params["b"]
        p05 = params["p05"]
        a2 = self._dh_params["a2"]
        d4 = self._dh_params["d4"]
        d1 = self._dh_params["d1"]

        arcsin = np.arcsin((p05[2] - d1) / b)
        arccos = np.arccos((a2**2 + b**2 - d4**2) / (2 * a2 * b))

        # ********************
        value = (a2**2 + b**2 - d4**2) / (2 * a2 * b)
        # print(value)

        # if value > 1 + self._tolerance or value < -1 - self._tolerance:
        #     raise ValueError(f"Invalid value for arccos: {value} (outside acceptable range)")
        if value <= 1 + self._tolerance and value >= -1 - self._tolerance:
            value = np.clip(value, -1.0, 1.0)
        #     arccos = np.arccos(value)

        # value = np.clip(value, -1.0, 1.0)
        arccos = np.arccos(value)
        # print(f"arcsin {arcsin}")
        # print(f"arccos {arccos}")
        # ********************

        # solution 1
        goal_theta2 = arcsin + arccos
        theta2_solutions.append(goal_theta2)

        # solution 2
        goal_theta2 = 2 * np.pi + arcsin - arccos
        theta2_solutions.append(goal_theta2)

        # solution 3
        goal_theta2 = np.pi - arcsin + arccos
        theta2_solutions.append(goal_theta2)

        # solution 4
        goal_theta2 = np.pi - arcsin - arccos
        theta2_solutions.append(goal_theta2)

        return theta2_solutions

    def __get_theta456(self, goal_theta1, goal_theta2, goal_theta3, params):
        solutions = []

        r11 = params["r11"]
        r12 = params["r12"]
        r13 = params["r13"]
        r21 = params["r21"]
        r22 = params["r22"]
        r23 = params["r23"]
        r31 = params["r31"]
        r32 = params["r32"]
        r33 = params["r33"]

        # --------------- THETA 4 ---------------

        numerator = np.sin(goal_theta1) * r13 - np.cos(goal_theta1) * r23

        denominator = (
            np.cos(goal_theta2 + goal_theta3)
            * (np.cos(goal_theta1) * r13 + np.sin(goal_theta1) * r23)
            + np.sin(goal_theta2 + goal_theta3) * r33
        )
        goal_theta4 = np.arctan2(numerator, denominator)

        # --------------- THETA 5 ---------------

        goal_theta5 = np.arccos(
            np.sin(goal_theta2 + goal_theta3)
            * (np.cos(goal_theta1) * r13 + np.sin(goal_theta1) * r23)
            - np.cos(goal_theta2 + goal_theta3) * r33
        )

        # --------------- THETA 6 ---------------

        num1 = -np.cos(goal_theta1) * np.sin(goal_theta2 + goal_theta3) * r12
        num2 = -np.sin(goal_theta1) * np.sin(goal_theta2 + goal_theta3) * r22
        num3 = np.cos(goal_theta2 + goal_theta3) * r32

        den1 = np.cos(goal_theta1) * np.sin(goal_theta2 + goal_theta3) * r11
        den2 = np.sin(goal_theta1) * np.sin(goal_theta2 + goal_theta3) * r21
        den3 = -np.cos(goal_theta2 + goal_theta3) * r31

        goal_theta6 = np.pi + np.arctan2(num1 + num2 + num3, den1 + den2 + den3)

        solutions.append([goal_theta4, goal_theta5, goal_theta6])
        solutions.append([np.pi + goal_theta4, -goal_theta5, np.pi + goal_theta6])

        return solutions

    def sort_solutions(self, solutions: list):
        return sorted(solutions, key=lambda x: x.value())

    def wrap_to_pi(self, angle):
        return (angle + np.pi) % (2 * np.pi) - np.pi

    def wrap_to_pi_ranged(self, angle, i):
        return (angle - self._joint_limits[i][0] - self._dh_params[f"d_theta{i+1}"]) % (2 * np.pi) + self._joint_limits[i][0]


# if __name__ == "__main__":

#     # DH params and joint limits are in ../config/dh_parameters.yaml

#     joint_limits = [
#         [-1.57, 1.57],                          # MEASURED
#         [-2.9, 0.3],                            # TEST REQUIRED [!]
#         [-0.44, 3.14],                          # TEST REQUIRED [!]
#         [-1.57, 1.57],                          # SAFETY [mani]
#         [-1.57, 1.57],                          # SAFETY [kable]
#         [-3.141592653589793, 3.141592653589793],# THIS CAN SPIN A LOT... So we leave it like this
#     ]

#     ik = InverseKinematics(
#         dh_params={
#             "a": [
#                 0.0,
#                 0.0,
#                 0.55,
#                 0.0,
#                 0.0,
#                 0.0,
#             ],
#             "alpha": [
#                 0.0,
#                 1.570796,
#                 0.0,
#                 1.570796,
#                 -1.570796,
#                 1.570796,
#             ],
#             "d": [
#                 0.669490,
#                 0.07149,
#                 -0.05749,
#                 0.50949,
#                 0.0,
#                 0.04634,
#             ],
#             "d_theta": [
#                 0.0,
#                 2.61667,
#                 -0.785,
#                 0.0,
#                 0.0,
#                 0.0,
#             ],
#         },
#         joint_limits=joint_limits,
#     )

#     orientation = [
#         0.678808985057852,
#         0.1294341384768194,
#         0.37429717702520887,
#         0.6183581396508433,
#     ]

#     # position = [-0.890984458094262, -0.3731057457347144, 1.0689133153666328]   # HOME?
#     position = [0.8, 0.0, 1.12]
#     rot = PyKDL.Rotation.Quaternion(            # HERE IT IS!
#         orientation[0],
#         orientation[1],
#         orientation[2],
#         orientation[3],
#     )
#     pos = PyKDL.Vector(
#         position[0],
#         position[1],
#         position[2],
#     )
#     frame = PyKDL.Frame(rot, pos)
#     sols = ik.inverse_kinematics(frame)
#     print(ik.inverse_kinematics(frame))
#     print([sol.get_solution() for sol in sols])
#     print([sol.id() for sol in sols])
