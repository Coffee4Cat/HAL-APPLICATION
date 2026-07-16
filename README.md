# HAL-APPLICATION
Aplikacja do sterowania łazikiem opracowana przez Autonomy Department. Repozytorium stworzone w celu udostępnienia aplikacji dla członków koła którzy nie są w autonomii.
<p align="center">
    <img src="presentation_gif.gif" width="480"/>
</p>

## SETUP I ODPALANIE

### Wymagania
Oprócz [ROS2-HUMBLE](https://docs.ros.org/en/humble/Installation/Ubuntu-Install-Debs.html) (Ubuntu 22.04) warto upewnić się, że jest zainstalowany Qt5. Pozostałe zależności opisano poniżej:

```bash
sudo apt install python3-pykdl
```

### Co tutaj jest
Są cztery pakiety:
- **hal_application** — pakiet aplikacji do sterowania łazikiem
- **hal_communication** — pakiet odpowiedzialny za konwersję ROS-TCP i wysyłkę danych do łazika
- **hal_interfaces** — pakiet z własnymi wiadomościami ROS
- **hal_manipulator** — pakiet odpowiedzialny za odwrotną kinematykę

### Jak to zbudować

```bash
cd HAL-APPLICATION
colcon build
source install/setup.bash
```
Ten ostatni krok trzeba powtarzać w każdym nowym terminalu, chyba że doda się go do ~/.bashrc.

### Jak to uruchomić
1. Wszystko na raz:
```bash
ros2 launch hal_application fullstack.launch.py
```
2. Po kolei (każda komenda w osobnym terminalu, pamiętając o poleceniu `source install/setup.bash`):
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
Najważniejszą rzeczą w komunikacji jest to, że jeśli przez dłuższy czas nie pojawiają się błędy łączności, oznacza to problem z połączeniem.
Przykładowa wartość oznaczająca problem z łącznością:
```bash
[hal_communication_node-1] [INFO] [1778321764.978748814] [communication]: -1
```
To jest jedna z wiadomości na początku działania węzła komunikacyjnego. Jeśli pojawi się inna wartość, oznacza to problem z łącznością.

### Mechanika kręciła manipulatorem, a kinematyka przestała działać
**Nie zalecamy tego robić bez odpowiedniego szkolenia. Ten tutorial jest dość awaryjny.**
Autonomia często poprawia ten obszar, ale jest to zadanie dość uciążliwe. Można softwarowo dodać offsety, które kompensują problemy sprzętowe.
```bash
cd src/hal_manipulator/config
nano dh_parameters.yaml
```
Tutaj modyfikuje się limity, głównie `D4_LIM`.
```bash
cd src/hal_manipulator/scripts
nano hal_kinematics.py
```
```python
# To jest coś, co trzeba dostosować, aby kinematyka zgadzała się przy okazji modyfikowania limitów z wcześniejszego pliku
# D4
resp.joints.position[3] = resp.joints.position[3] + 2.79 - 3.14 - 2.35 - 0.10
```

### Gamepad
Pad należy podłączyć przez USB. Jest odczytywany za pomocą pygame. W samej aplikacji trzeba kliknąć w głównym oknie odpowiedni przycisk w sekcji `GAMEPAD ACTIVATION`.
Jeśli pojawią się problemy, najlepiej kliknąć tam `none` i zresetować węzeł związany z gamepadem. Zrobienie tego w odwrotnej kolejności może prowadzić do awarii procesu.

### Jak jeździć łazikiem
W sekcji `GENERAL CONTROL SETTINGS` należy kliknąć przycisk oznaczony jako `deactivated`. Klikajcie go, aż dioda obok zmieni kolor na zielony. Wtedy można przejść do `TELEOPERATOR`. Tam znajduje się joystick `chassis drive`.

### Jak ruszać manipulatorem
Prawie to samo co wyżej, tylko że należy kliknąć odpowiednie przyciski dla manipulatora. W teleoperatorze po prawej znajduje się obszar dla manipulatora. Tam są 3 zakładki:
1. Widgety, które robią to samo co pad
2. Automatyzacja
3. Ustawienia wzmocnienia wejść pada oraz widgetów (manipulator może być szybszy albo wolniejszy)

### Jak modyfikować automatyzację
```bash
cd src/hal_application/include
nano CONFIG.h
```
Na dole znajdują się ustawienia automatyzacji. Współrzędne XYZ oraz orientacje można odczytać w aplikacji (lewy dolny róg teleoperatora).
