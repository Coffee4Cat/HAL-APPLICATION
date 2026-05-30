#!/usr/bin/env python3
import rclpy
import tkinter as tk
import threading
import socket



class Teleoperator:
    def __init__(self):
        self.server_ip = '192.168.2.69'
        self.server_port = 8890
        self.sock = None
        self.last_response = None
        self.connect_socket()
        print('[TELEOPERATOR] LABO EDITION')

    def connect_socket(self):
        if self.sock:
            try:
                self.sock.close()
            except OSError:
                pass
            self.sock = None

        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.sock.settimeout(3.0)
        try:
            self.sock.connect((self.server_ip, self.server_port))
            print(f'Connected to {self.server_ip}:{self.server_port}')
        except (socket.timeout, socket.error) as exc:
            print(f'Could not connect to {self.server_ip}:{self.server_port}: {exc}')
            self.sock = None

    def ensure_connected(self):
        if self.sock is None:
            self.connect_socket()
        return self.sock is not None

    def build_command_string(self, command_id, data_bytes):
        hex_code = f'{command_id:02X}'
        payload = ''.join(f'{byte:02X}' for byte in data_bytes)
        return f'#{hex_code}{payload}'

    def send_command(self, command_str):
        if not command_str.endswith('\n'):
            command_str = command_str + '\n'

        if not self.ensure_connected():
            print('Socket is not connected, cannot send command')
            return None

        try:
            self.sock.sendall(command_str.encode('utf-8'))
            print(f'Sent: {command_str.strip()}')
            response = self.sock.recv(4096).decode('utf-8', errors='replace').strip()
            self.last_response = response
            print(f'Received: {response}')
            return response
        except (socket.timeout, socket.error) as exc:
            print(f'Socket error: {exc}')
            try:
                self.sock.close()
            except OSError:
                pass
            self.sock = None
            return None



    def flash_light(self):
        print('FLASHING LIGHT')
        command = self.build_command_string(197, [1, 0, 0, 0, 0, 0, 0, 0])
        return self.send_command(command)

    def close_fiolka(self):
        print('CLOSE FIOLKA')
        command = self.build_command_string(197, [2, 0, 0, 0, 0, 0, 0, 0])
        return self.send_command(command)

    def wyciskacz(self, num):
        print(f'WYCISKACZ {num}')
        command = self.build_command_string(197, [0, num, 0, 0, 0, 0, 0, 0])
        return self.send_command(command)

    def sonda(self, num):
        print(f'SONDA {num}')
        command = self.build_command_string(197, [0, 0, num, 0, 0, 0, 0, 0])
        return self.send_command(command)

    def dynamixel_reset(self, num):
        print(f'DYNAMIXEL RESET {num}')
        command = self.build_command_string(197, [0, 0, 0, num, 0, 0, 0, 0])
        return self.send_command(command)


    def sys_commands(self, num):
        print(f'SYS COMMAND {num}')
        command = self.build_command_string(192, [0, num, 0, 0, 0, 0, 0, 0])
        return self.send_command(command)

    def automatic_control(self, num):
        print(f'AUTOMATIC CONTROL {num}')
        command = self.build_command_string(192, [num, 0, 0, 0, 0, 0, 0, 0])
        return self.send_command(command)

    def manual_control(self, num):
        print(f'MANUAL CONTROL {num}')
        command_map = {
            1: [0, 0, 0, 3, 128, 0, 0, 0],
            2: [0, 0, 0, 4, 255, 0, 0, 0],
            3: [0, 0, 0, 0, 0, 0, 0, 0],
            4: [0, 0, 0, 0, 0, 1, 128, 0],
            5: [0, 0, 0, 0, 0, 2, 255, 0],
            6: [0, 0, 0, 1, 255, 0, 0, 0],
            7: [0, 0, 0, 2, 128, 0, 0, 0],
        }
        command = self.build_command_string(193, command_map[num])
        return self.send_command(command)


    def uv_lamp(self):
        print(f'UV LAMP')
        command = self.build_command_string(194, [0, 0, 0, 0, 0, 0, 0, 0])
        return self.send_command(command)





#     Oś Pionowa Wiertła (Opuszczanie/Podnoszenie):
# * Opuść wiertło (w dół) z prędkością 50% (0x80):
#   cansend can_mani 0C1#00 00 00 03 80 00 00 00

# * Podnieś wiertło (w górę) z prędkością 100% (0xFF):
#   cansend can_mani 0C1#00 00 00 04 FF 00 00 00

# * Zatrzymaj ruch pionowy wiertła:
#   cansend can_mani 0C1#00 00 00 00 00 00 00 00

# Wrzeciono Wiertła (Ruch obrotowy):
# * Kręć do przodu (wiercenie) z prędkością 50%:
#   cansend can_mani 0C1#00 00 00 00 00 01 80 00

# * Kręć do tyłu z prędkością 100%:
#   cansend can_mani 0C1#00 00 00 00 00 02 FF 00

# Mieszadło / Wirówka:
# * Włącz mieszadło (Kierunek 1) z prędkością rozruchową ~100% (0xFF):
#   cansend can_mani 0C1#00 00 00 01 FF 00 00 00

# * Włącz mieszadło (Kierunek 2) ze średnią prędkością (0x80):
#   cansend can_mani 0C1#00 00 00 02 80 00 00 00



def initialize_gui(node: Teleoperator):


    root = tk.Tk()
    root.title("TELEOPERATOR")
    root.configure(bg="#7a9dde")
    root.geometry("1200x800")
    root.resizable(False, False)

    frame = tk.Frame(root)
    frame.configure(bg="#494b4f")
    frame.pack(expand=True, fill=tk.BOTH)

    # Group for manual controls (drawn as a rectangle/labelled frame)
    manual_group = tk.LabelFrame(frame, text="C0 STUFF", bg="#494b4f", fg="#ffffff", bd=2, relief=tk.GROOVE, font=("Arial", 12))
    manual_group.grid(row=1, column=1, rowspan=4, columnspan=6, padx=10, pady=10, sticky="nsew")


    flash_light_button = tk.Button(manual_group, bg="#fa3a42", fg="#ffffff", text="FLASH LIGHT", width=10, height=2)
    flash_light_button.bind('<ButtonPress-1>', lambda e: node.flash_light())
    flash_light_button.grid(row=1, column=1, padx=5, pady=5)

    close_fiolka_button = tk.Button(manual_group, bg="#fa3a42", fg="#ffffff", text="ZAMKNIJ FIOLKE", width=10, height=2)
    close_fiolka_button.bind('<ButtonPress-1>', lambda e: node.close_fiolka())
    close_fiolka_button.grid(row=2, column=1, padx=5, pady=5)

    wyciskacz_1_button = tk.Button(manual_group, bg="#da3afa", fg="#ffffff", text="WYCISKACZ UP", width=13, height=2)
    wyciskacz_1_button.bind('<ButtonPress-1>', lambda e: node.wyciskacz(1))
    wyciskacz_1_button.grid(row=1, column=2, padx=5, pady=5)
    wyciskacz_2_button = tk.Button(manual_group, bg="#da3afa", fg="#ffffff", text="WYCISKACZ 2ml", width=13, height=2)
    wyciskacz_2_button.bind('<ButtonPress-1>', lambda e: node.wyciskacz(2))
    wyciskacz_2_button.grid(row=2, column=2, padx=5, pady=5)
    wyciskacz_3_button = tk.Button(manual_group, bg="#da3afa", fg="#ffffff", text="WYCISKACZ 3ml", width=13, height=2)
    wyciskacz_3_button.bind('<ButtonPress-1>', lambda e: node.wyciskacz(3))
    wyciskacz_3_button.grid(row=3, column=2, padx=5, pady=5)
    wyciskacz_4_button = tk.Button(manual_group, bg="#da3afa", fg="#ffffff", text="WYCISKACZ FULL", width=13, height=2)
    wyciskacz_4_button.bind('<ButtonPress-1>', lambda e: node.wyciskacz(4))
    wyciskacz_4_button.grid(row=4, column=2, padx=5, pady=5)

    sonda_1_button = tk.Button(manual_group, bg="#e49b14", fg="#ffffff", text="SONDA GLEBA UP", width=15, height=2)
    sonda_1_button.bind('<ButtonPress-1>', lambda e: node.sonda(1))
    sonda_1_button.grid(row=1, column=3, padx=5, pady=5)
    sonda_2_button = tk.Button(manual_group, bg="#e49b14", fg="#ffffff", text="SONDA GLEBA DOWN", width=15, height=2)
    sonda_2_button.bind('<ButtonPress-1>', lambda e: node.sonda(2))
    sonda_2_button.grid(row=2, column=3, padx=5, pady=5)

    dynamixel_reset_1_button = tk.Button(manual_group, bg="#14e49b", fg="#140808", text="DYNAMIXEL RESET STRZYK", width=20, height=2)
    dynamixel_reset_1_button.bind('<ButtonPress-1>', lambda e: node.dynamixel_reset(1))
    dynamixel_reset_1_button.grid(row=1, column=4, padx=5, pady=5)
    dynamixel_reset_2_button = tk.Button(manual_group, bg="#14e49b", fg="#140808", text="DYNAMIXEL RESET WIDŁY", width=20, height=2)
    dynamixel_reset_2_button.bind('<ButtonPress-1>', lambda e: node.dynamixel_reset(2))
    dynamixel_reset_2_button.grid(row=2, column=4, padx=5, pady=5)

    sys_command_1_button = tk.Button(manual_group, bg="#e41475", fg="#140808", text="MANUAL ENTER", width=15, height=2)
    sys_command_1_button.bind('<ButtonPress-1>', lambda e: node.sys_commands(3))
    sys_command_1_button.grid(row=1, column=5, padx=5, pady=5)
    sys_command_2_button = tk.Button(manual_group, bg="#e41475", fg="#140808", text="IDLE / CZYŚĆ BŁĘDY", width=15, height=2)
    sys_command_2_button.bind('<ButtonPress-1>', lambda e: node.sys_commands(4))
    sys_command_2_button.grid(row=2, column=5, padx=5, pady=5)
    sys_command_3_button = tk.Button(manual_group, bg="#ff0303", fg="#FFFFFF", text="SCRAM", width=15, height=2)
    sys_command_3_button.bind('<ButtonPress-1>', lambda e: node.sys_commands(2))
    sys_command_3_button.grid(row=3, column=5, padx=5, pady=5)

    auto_group = tk.LabelFrame(frame, text="AUTO STUFF", bg="#494b4f", fg="#ffffff", bd=2, relief=tk.GROOVE, font=("Arial", 12))
    auto_group.grid(row=1, column=7, rowspan=4, columnspan=1, padx=10, pady=10, sticky="nsew")
    automatic_control_1_button = tk.Button(frame, bg="#14e49f", fg="#140808", text="AUTOMATIC HOMING", width=20, height=2)
    automatic_control_1_button.bind('<ButtonPress-1>', lambda e: node.automatic_control(1))
    automatic_control_1_button.grid(row=2, column=7, padx=5, pady=5)
    automatic_control_2_button = tk.Button(frame, bg="#14e49f", fg="#140808", text="AUTOMATIC WIERCENIE", width=20, height=2)
    automatic_control_2_button.bind('<ButtonPress-1>', lambda e: node.automatic_control(2))
    automatic_control_2_button.grid(row=3, column=7, padx=5, pady=5)
    automatic_control_3_button = tk.Button(frame, bg="#14e49f", fg="#140808", text="AUTOMATIC OTRZEPYTANIE", width=20, height=2)
    automatic_control_3_button.bind('<ButtonPress-1>', lambda e: node.automatic_control(3))
    automatic_control_3_button.grid(row=4, column=7, padx=5, pady=5)


    manual_label = tk.Label(frame, text=f"MANUAL", font=("Arial", 16))
    manual_label_list = ["OPUSZCZANIE 50%", "PODNOSZENIE 100%", "ZATRZYMAJ PION WIERTŁO", "RUCH WRZECIONA", "KRĘĆ DO TYŁU 100%", "MIESZADŁO K1 100%", "MIESZADŁO K2 50%", "STOP MIESZADŁO"]
    for i in range(8):
        manual_control_button = tk.Button(frame, bg="#e41475", fg="#140808", text=f"{manual_label_list[i]}", width=20, height=2)
        manual_control_button.bind('<ButtonPress-1>', lambda e, num=i+1: node.manual_control(num))
        manual_control_button.grid(row=i+1, column=8, padx=5, pady=5)

    # Przykład suwaka 0-255
    slider_value = tk.IntVar(value=128)

    def on_slider_change(value):
        slider_label.config(text=f"Wartość suwaka: {int(float(value))}")

    slider = tk.Scale(frame,
                      from_=0,
                      to=255,
                      orient=tk.HORIZONTAL,
                      length=500,
                      label="Suwak 0-255",
                      variable=slider_value,
                      command=on_slider_change)
    slider.grid(row=6, column=1, columnspan=6, padx=10, pady=10, sticky="ew")

    slider_label = tk.Label(frame, text=f"Wartość suwaka: {slider_value.get()}", font=("Arial", 12), bg="#494b4f", fg="#ffffff")
    slider_label.grid(row=7, column=1, columnspan=6, padx=10, pady=5)


    root.mainloop()





def main(args=None):
    rclpy.init(args=args)
    teleop_labo = Teleoperator()

    gui_thread = threading.Thread(target=initialize_gui, args=(teleop_labo,))
    gui_thread.start()

    gui_thread.join()

if __name__ == '__main__':
    main()