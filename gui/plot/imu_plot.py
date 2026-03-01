from PyQt5.QtWidgets import QWidget, QVBoxLayout, QHBoxLayout
import pyqtgraph as pg
from collections import deque
from .base_plot import BasePlot

class IMUPlot(BasePlot):
    def __init__(self, maxlen=500):
        super().__init__(None)
        self.maxlen = maxlen
        self.timestamps = deque(maxlen=maxlen)
        self.accel_sensor1 = {'x': deque(maxlen=maxlen), 'y': deque(maxlen=maxlen), 'z': deque(maxlen=maxlen)}
        self.gyro_sensor1 = {'x': deque(maxlen=maxlen), 'y': deque(maxlen=maxlen), 'z': deque(maxlen=maxlen)}
        self.accel_sensor2 = {'x': deque(maxlen=maxlen), 'y': deque(maxlen=maxlen), 'z': deque(maxlen=maxlen)}
        self.gyro_sensor2 = {'x': deque(maxlen=maxlen), 'y': deque(maxlen=maxlen), 'z': deque(maxlen=maxlen)}

        # Das Haupt-Widget, das wir nach außen geben
        self.widget = QWidget()
        self.layout = QVBoxLayout()
        self.widget.setLayout(self.layout)

        # Zwei separate Plots
        self.plot_accel_sensor1 = pg.PlotWidget(title="Acceleration Sensor 1")
        self.plot_accel_sensor1.setLabel('left', 'Acceleration (m/s²)')
        self.plot_accel_sensor1.setLabel('bottom', 'Time (µs)')
        self.plot_accel_sensor1.addLegend()

        self.plot_gyro_sensor1 = pg.PlotWidget(title="Gyroscope Sensor 1")
        self.plot_gyro_sensor1.setLabel('left', 'Angular Velocity (°/s)')
        self.plot_gyro_sensor1.setLabel('bottom', 'Time (µs)')
        self.plot_gyro_sensor1.addLegend()

        self.plot_accel_sensor2 = pg.PlotWidget(title="Acceleration Sensor 2")
        self.plot_accel_sensor2.setLabel('left', 'Acceleration (m/s²)')
        self.plot_accel_sensor2.setLabel('bottom', 'Time (µs)')
        self.plot_accel_sensor2.addLegend()

        self.plot_gyro_sensor2 = pg.PlotWidget(title="Gyroscope Sensor 2")
        self.plot_gyro_sensor2.setLabel('left', 'Angular Velocity (°/s)')
        self.plot_gyro_sensor2.setLabel('bottom', 'Time (µs)')
        self.plot_gyro_sensor2.addLegend()

        # --- Zeile 1: Acceleration-Plots nebeneinander ---
        accel_layout = QHBoxLayout()
        accel_layout.addWidget(self.plot_accel_sensor1)
        accel_layout.addWidget(self.plot_accel_sensor2)

        # --- Zeile 2: Gyro-Plots nebeneinander ---
        gyro_layout = QHBoxLayout()
        gyro_layout.addWidget(self.plot_gyro_sensor1)
        gyro_layout.addWidget(self.plot_gyro_sensor2)

        # --- Alles in die Hauptspalte ---
        self.layout.addLayout(accel_layout)
        self.layout.addLayout(gyro_layout)

        self.curves_accel1 = {
            'x': self.plot_accel_sensor1.plot(pen='r', name='Accel X'),
            'y': self.plot_accel_sensor1.plot(pen='g', name='Accel Y'),
            'z': self.plot_accel_sensor1.plot(pen='b', name='Accel Z'),
        }
        self.curves_accel2 = {
            'x': self.plot_accel_sensor2.plot(pen='r', name='Accel X'),
            'y': self.plot_accel_sensor2.plot(pen='g', name='Accel Y'),
            'z': self.plot_accel_sensor2.plot(pen='b', name='Accel Z'),
        }
        self.curves_gyro1 = {
            'x': self.plot_gyro_sensor1.plot(pen='r', name='Gyro X'),
            'y': self.plot_gyro_sensor1.plot(pen='g', name='Gyro Y'),
            'z': self.plot_gyro_sensor1.plot(pen='b', name='Gyro Z'),
        }
        self.curves_gyro2 = {
            'x': self.plot_gyro_sensor2.plot(pen='r', name='Gyro X'),
            'y': self.plot_gyro_sensor2.plot(pen='g', name='Gyro Y'),
            'z': self.plot_gyro_sensor2.plot(pen='b', name='Gyro Z'),
        }

    def get_widget(self):
        return self.widget

    def update(self, ccontent):
        ts = ccontent.mTimeUs
        imu1 = ccontent.mSensor1Data
        imu2 = ccontent.mSensor2Data

        self.timestamps.append(ts)
        self.accel_sensor1['x'].append(imu1.mAx)
        self.accel_sensor1['y'].append(imu1.mAy)
        self.accel_sensor1['z'].append(imu1.mAz)
        self.gyro_sensor1['x'].append(imu1.mWx)
        self.gyro_sensor1['y'].append(imu1.mWy)
        self.gyro_sensor1['z'].append(imu1.mWz)

        self.accel_sensor2['x'].append(imu2.mAx)
        self.accel_sensor2['y'].append(imu2.mAy)
        self.accel_sensor2['z'].append(imu2.mAz)
        self.gyro_sensor2['x'].append(imu2.mWx)
        self.gyro_sensor2['y'].append(imu2.mWy)
        self.gyro_sensor2['z'].append(imu2.mWz)

        t = list(self.timestamps)
        for axis in 'xyz':
            self.curves_accel1[axis].setData(t, list(self.accel_sensor1[axis]))
            self.curves_gyro1[axis].setData(t, list(self.gyro_sensor1[axis]))
            self.curves_accel2[axis].setData(t, list(self.accel_sensor2[axis]))
            self.curves_gyro2[axis].setData(t, list(self.gyro_sensor2[axis]))