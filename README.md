# HAL-APPLICATION
Apka do sterowania łazikiem wyprodukowana przez Autonomy Department
## SETUP I ODPALANIE
### Wymagania
Oprócz [ROS2-HUMBLE](https://docs.ros.org/en/humble/Installation/Ubuntu-Install-Debs.html) (Ubuntu 22.04) trzeba się upewnić że jest Qt5. Reszta rzeczy do instalacji pod spodem:

```bash
sudo apt install python3-pykdl
```

### Co tutaj jest
Są cztery paczki:  
- **hal_application** -> paczka od aplikacji do sterowania łazikiem.
- **hal_communication** -> paczka od konwersji ROS-TCP i wysyłki do łazika
- **hal_interfaces** -> paczka z customowymi wiadomościami rosowymi
- **hal_manipulator** -> paczka od odwrotnej kinematyki

### Jak to zbudować

```bash
cd HAL-APPLICATION
colcon build
source install/setup.bash
```
(Ten ostatni trzeba robić w każdym nowym terminalu, chyba że doda się to do ~/.bashrc)

### Jak to odpalić
1. Wszystko na raz:
```bash
ros2 launch hal_application fullstack.launch.py 
```
2. Po kolei (każda komenda w osobnym terminalu, pamiętając o tym *source install/setup.bash* )
```bash
ros2 launch hal_communication communication.launch.py 
ros2 launch hal_manipulator hal_kinematics.launch.py 
ros2 run hal_application application
ros2 run hal_application gamepad_interface.py
```

## PROBLEMY I ICH ROZWIĄZANIE

### Rekonfiguracja połączenia sieciowego
```bash
cd src/hal_communication/config
nano comm.yaml
# PO ZMIANIE
cd ../../..
colcon build
```
Generalnie najważniejsze z komunikacji jest to, że... Jak nie ma błędów łączności które wyskakują cały czas to znaczy że nie działa xD. Generalnie coś takiego oznacza że jest problem z łącznością:
```bash
[hal_communication_node-1] [INFO] [1778321764.978748814] [communication]: -1
```
To jest jedna wiadomość na początku działania węzła od komunikacji. Jak jest inna liczba to jest git

### Mechanika kręciła manipulator i kinematyka się rozjebała
**Nie polecamy tego robić bez 'szkolenia'. Tutorial bardzo awaryjny**  
To autonomia naprawia prawie cały czas ale jest to zadanie giga upierdliwe. Można softowo dodać offsety które kontrują te hardwareowe.
```bash
cd src/hal_manipulator/config
nano dh_parameters.yaml
```
Tutaj modyfikuje się limity. GŁÓWNIE D4_LIM.
```bash
cd src/hal_manipulator/scripts
nano hal_kinematics.py
```
```python
#To jest coś co trzeba tweakować żeby kinematyka się zgodziła przy okazji modyfikując tamte limity z wcześniejszego pliku
#D4
resp.joints.position[3] = resp.joints.position[3] + 2.79 - 3.14 - 2.35 - 0.10
```

### Gamepad
Pada trzeba podłączyć po USB. Czytany jest za pomocą pygame. W samej aplikacji trzeba kliknąć w głównym oknie odpowiedni przycisk w 'GAMEPAD ACTIVATION'.  
Jak się wypnie czy coś to najlepiej kliknąć tam none i przeresetować node związany z gamepadem. Zrobienie tego w odwrotnej kolejności daje segfault xD.

### Jak jeździć łazikiem
W 'GENERAL CONTROL SETTINGS' klikacjie ten guzik co ma 'deactivated'. Klikacie go, aż dioda obok zrobi się zielona. Wtedy można przejść do 'TELEOPERATOR'. Tam jest joystick 'chassis drive'.

### Jak ruszać manipulatorem
Prawie to samo co wyżej, tylko że klikacie guziki odpowiednie dla mani. W teleoperatorze po prawej jest przestrzeń dla mani. Tam są 3 zakładki.
1. Widgety które robią tyle samo co pad
2. Automatyzacja
3. Ustawienia wzmocnienia inputów pada oraz widgetów (mani robi się szybszy albo wolniejszy)

### Jak modyfikować Automatyzację 
```bash
cd src/hal_application/include
nano CONFIG.h
```
Tam na dole są rzeczy od automatyzacji. XYZ oraz orientacje można przeczytać w apce (lewy dolny róg teleoperatora).
