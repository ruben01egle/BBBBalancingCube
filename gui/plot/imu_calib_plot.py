from PyQt5.QtWidgets import QWidget, QHBoxLayout
import pyqtgraph as pg
from collections import deque
from .base_plot import BasePlot

class IMUCalibPlot(BasePlot):
    def __init__(self, maxlen=500):
        super().__init__(None)
        self.maxlen = maxlen
        self.timestamps = deque(maxlen=maxlen)
        self.accel_sensor1 = {'x': deque(maxlen=maxlen), 'y': deque(maxlen=maxlen)}
        self.gyro_sensor1 = deque(maxlen=maxlen)
        self.accel_sensor2 = {'x': deque(maxlen=maxlen), 'y': deque(maxlen=maxlen)}
        self.gyro_sensor2 = deque(maxlen=maxlen)

        self.widget = QWidget()
        self.layout = QHBoxLayout()
        self.widget.setLayout(self.layout)

        self.plot_accel = pg.PlotWidget(title="Calibrated Acceleration")
        self.plot_accel.setLabel('left', 'Acceleration (m/s²)')
        self.plot_accel.setLabel('bottom', 'Time (µs)')
        self.plot_accel.addLegend()

        self.plot_gyro = pg.PlotWidget(title="Calibrated Angular Velocity")
        self.plot_gyro.setLabel('left', 'Angular Velocity (rad/s)')
        self.plot_gyro.setLabel('bottom', 'Time (µs)')
        self.plot_gyro.addLegend()

        self.layout.addWidget(self.plot_accel)
        self.layout.addWidget(self.plot_gyro)

        self.curves_accel1 = {
            'x': self.plot_accel.plot(pen='r', name='S1 Accel X'),
            'y': self.plot_accel.plot(pen='g', name='S1 Accel Y'),
        }
        self.curves_accel2 = {
            'x': self.plot_accel.plot(pen='m', name='S2 Accel X'),
            'y': self.plot_accel.plot(pen='c', name='S2 Accel Y'),
        }
        self.curve_gyro1 = self.plot_gyro.plot(pen='r', name='S1 Gyro Z')
        self.curve_gyro2 = self.plot_gyro.plot(pen='b', name='S2 Gyro Z')

    def get_widget(self):
        return self.widget

    def update(self, ccontent):
        ts = ccontent.mTimeUs
        imu1 = ccontent.mSensor1DataCalib
        imu2 = ccontent.mSensor2DataCalib

        self.timestamps.append(ts)
        self.accel_sensor1['x'].append(imu1.mDDotX)
        self.accel_sensor1['y'].append(imu1.mDDotY)
        self.gyro_sensor1.append(imu1.mDotPhi)

        self.accel_sensor2['x'].append(imu2.mDDotX)
        self.accel_sensor2['y'].append(imu2.mDDotY)
        self.gyro_sensor2.append(imu2.mDotPhi)

        t = list(self.timestamps)
        for axis in 'xy':
            self.curves_accel1[axis].setData(t, list(self.accel_sensor1[axis]))
            self.curves_accel2[axis].setData(t, list(self.accel_sensor2[axis]))
        self.curve_gyro1.setData(t, list(self.gyro_sensor1))
        self.curve_gyro2.setData(t, list(self.gyro_sensor2))
