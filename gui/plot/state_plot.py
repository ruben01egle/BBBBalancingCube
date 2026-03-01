from PyQt5.QtWidgets import QWidget, QGridLayout
from PyQt5.QtCore import Qt
import pyqtgraph as pg
import math
from collections import deque
from .base_plot import BasePlot

class StatePlot(BasePlot):
    def __init__(self, maxlen=500):
        super().__init__(None)
        self.maxlen = maxlen
        self.timestamps = deque(maxlen=maxlen)
        self.phi_c = deque(maxlen=maxlen)
        self.phi_a = deque(maxlen=maxlen)
        self.phi_d = deque(maxlen=maxlen)
        self.psi_d = deque(maxlen=maxlen)
        self.motor_torque = deque(maxlen=maxlen)

        self.widget = QWidget()
        self.layout = QGridLayout()
        self.widget.setLayout(self.layout)

        # 4 PlotWidgets erstellen
        self.plot_angles = pg.PlotWidget(title="Phi")
        self.plot_dot_angles = pg.PlotWidget(title="Dot Phi")
        self.plot_disk = pg.PlotWidget(title="Dot Psi")
        self.plot_motor = pg.PlotWidget(title="Tm")

        # Achsenbeschriftungen usw.
        self.plot_angles.setLabel('left', 'Angle (°)')
        self.plot_angles.setLabel('bottom', 'Time (µs)')
        self.plot_angles.addLegend()

        self.plot_dot_angles.setLabel('left', 'Angular Velocity (°/s)')
        self.plot_dot_angles.setLabel('bottom', 'Time (µs)')
        self.plot_dot_angles.addLegend()

        self.plot_disk.setLabel('left', 'Angular Velocity (°/s)')
        self.plot_disk.setLabel('bottom', 'Time (µs)')
        self.plot_disk.addLegend()

        self.plot_motor.setLabel('left', 'Torque (Nm)')
        self.plot_motor.setLabel('bottom', 'Time (µs)')
        self.plot_motor.addLegend()

        # Plots im Raster (2x2) anordnen
        self.layout.addWidget(self.plot_angles,    0, 0)  # Zeile 0, Spalte 0
        self.layout.addWidget(self.plot_dot_angles,0, 1)  # Zeile 0, Spalte 1
        self.layout.addWidget(self.plot_disk,      1, 0)  # Zeile 1, Spalte 0
        self.layout.addWidget(self.plot_motor,     1, 1)  # Zeile 1, Spalte 1

        # Kurven
        self.curves = {
            'phi_c': self.plot_angles.plot(pen=pg.mkPen(color=(255, 0, 0), width=2), name='Phi_C'),
            'phi_a': self.plot_angles.plot(pen=pg.mkPen(color=(255, 128, 128), style=Qt.PenStyle.DashLine), name='Phi_A'),

            'phi_d': self.plot_dot_angles.plot(pen=pg.mkPen(color=(255, 0, 0), width=2), name='DotPhi'),
            'psi_d': self.plot_disk.plot(pen=pg.mkPen(color=(0, 0, 255), width=2), name='DotPsi'),
            'motor_torque': self.plot_motor.plot(pen=pg.mkPen(color=(0, 255, 0), width=2), name='Torque'),
        }

    def get_widget(self):
        return self.widget

    def update(self, ccontent):
        ts = ccontent.mTimeUs
        state = ccontent.mStateData
        self.timestamps.append(ts)
        self.phi_c.append(math.degrees(state.mPhi_C))
        self.phi_a.append(math.degrees(state.mPhi_A))
        self.phi_d.append(math.degrees(state.mPhi_d))
        self.psi_d.append(math.degrees(state.mPsi_d))
        self.motor_torque.append(ccontent.mMotorTorque)

        t = list(self.timestamps)
        self.curves['phi_c'].setData(t, list(self.phi_c))
        self.curves['phi_a'].setData(t, list(self.phi_a))
        self.curves['phi_d'].setData(t, list(self.phi_d))
        self.curves['psi_d'].setData(t, list(self.psi_d))
        self.curves['motor_torque'].setData(t, list(self.motor_torque))
