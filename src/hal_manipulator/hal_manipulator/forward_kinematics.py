#!/usr/bin/env python

import PyKDL


class ForwardKinematics:
    """
    Class representation of the forward kinematics task.
    """

    def __init__(self, dh_params: dict):
        """
        Initialize the ForwardKinematics class.

        Parameters:
            a: List of distances along the x-axis of frame {i-1},
                between the z-axes of frames {i-1} and {i}.

            alpha: List of angles around the x-axis of frame {i-1},
                between the z-axes of frames {i-1} and {i}.

            d: List of distances along the z-axis of frame {i},
                between the x-axes of frames {i-1} and {i}.

            basic_theta: List of angles around the z-axis of frame {i},
                between the x-axes of frames {i-1} and {i} in the zero
                position.
        """

        self._a = dh_params["a"]
        self._alpha = dh_params["alpha"]
        self._d = dh_params["d"]
        self._basic_theta = dh_params["d_theta"]

    def forward_kinematics(self, theta: list):
        """
        Function that resolves the forward kinematics task based on the
        current joint states and D-H notation parameters.

        Parameters:
            theta: List of current joint states (angles for revolute joints).
        """

        theta = [theta[i] + self._basic_theta[i] for i in range(len(theta))]
        R1x = PyKDL.Frame(PyKDL.Rotation.RotX(self._alpha[0]))
        T1x = PyKDL.Frame(PyKDL.Vector(self._a[0], 0, 0))
        R1z = PyKDL.Frame(PyKDL.Rotation.RotZ(theta[0]))
        T1z = PyKDL.Frame(PyKDL.Vector(0, 0, self._d[0]))
        A1 = R1x * T1x * R1z * T1z
        # print(A1)

        R2x = PyKDL.Frame(PyKDL.Rotation.RotX(self._alpha[1]))
        T2x = PyKDL.Frame(PyKDL.Vector(self._a[1], 0, 0))
        R2z = PyKDL.Frame(PyKDL.Rotation.RotZ(theta[1]))
        T2z = PyKDL.Frame(PyKDL.Vector(0, 0, self._d[1]))
        A2 = R2x * T2x * R2z * T2z

        R3x = PyKDL.Frame(PyKDL.Rotation.RotX(self._alpha[2]))
        T3x = PyKDL.Frame(PyKDL.Vector(self._a[2], 0, 0))
        R3z = PyKDL.Frame(PyKDL.Rotation.RotZ(theta[2]))
        T3z = PyKDL.Frame(PyKDL.Vector(0, 0, self._d[2]))
        A3 = R3x * T3x * R3z * T3z

        R4x = PyKDL.Frame(PyKDL.Rotation.RotX(self._alpha[3]))
        T4x = PyKDL.Frame(PyKDL.Vector(self._a[3], 0, 0))
        R4z = PyKDL.Frame(PyKDL.Rotation.RotZ(theta[3]))
        T4z = PyKDL.Frame(PyKDL.Vector(0, 0, self._d[3]))
        A4 = R4x * T4x * R4z * T4z

        R5x = PyKDL.Frame(PyKDL.Rotation.RotX(self._alpha[4]))
        T5x = PyKDL.Frame(PyKDL.Vector(self._a[4], 0, 0))
        R5z = PyKDL.Frame(PyKDL.Rotation.RotZ(theta[4]))
        T5z = PyKDL.Frame(PyKDL.Vector(0, 0, self._d[4]))
        A5 = R5x * T5x * R5z * T5z

        R6x = PyKDL.Frame(PyKDL.Rotation.RotX(self._alpha[5]))
        T6x = PyKDL.Frame(PyKDL.Vector(self._a[5], 0, 0))
        R6z = PyKDL.Frame(PyKDL.Rotation.RotZ(theta[5]))
        T6z = PyKDL.Frame(PyKDL.Vector(0, 0, self._d[5]))
        A6 = R6x * T6x * R6z * T6z

        A = A1 * A2 * A3 * A4 * A5 * A6
        # print(A.M)
        # print(f"position: {A.M*PyKDL.Vector()+A.p}")
        # print(f"orientation: {A.M.GetRPY()}")
        return A

    def get_pose(self, A):
        """
        Return position and orientation from a transformation matrix.

        Parameters:
            A (PyKDL.Frame): A transformation matrix represented by
            a PyKDL.Frame object.

        Returns:
            - position (PyKDL.Vector): A Vector object representing the
            position (translation) extracted from the matrix.

            - orientation (tuple): A tuple representing the orientation in RPY
            (Roll, Pitch, Yaw) angles.
        """

        position = A.p
        orientation = A.M.GetRPY()
        return type(position), type(orientation)


if __name__ == "__main__":
    fk = ForwardKinematics({
        "a": [
            0.0,
            0.0,
            0.55,
            0.0,
            0.0,
            0.0,
        ],
        "alpha": [
            0.0,
            1.570796,
            0.0,
            1.570796,
            -1.570796,
            1.570796,
        ],
        "d": [
            0.669490,
            0.07149,
            -0.05749,
            0.50949,
            0.0,
            0.04634,
        ],
        "d_theta": [
            0.0,
            2.61667,
            -0.785,
            0.0,
            0.0,
            0.0,
        ]
        })
    a = fk.forward_kinematics(
        theta=[0.7851443195818182, -0.4662773937783138, 1.9442312695808983, -2.4112676287536994, 1.516430682058859, 1.3758039386306429]

        # theta=[
        #     1.5379955746725207,
        #     -1.9310746503128506,
        #     -0.2118129031998386,
        #     -1.790487571910695,
        #     1.3482513187260174,
        #     1.8629513913681413
        # ]
    )
    # A = PyKDL.Frame()
    # print(fk.get_pose(A))
    print(a.p)
    print(a.M.GetQuaternion())
    # print(a.M.GetRPY())
