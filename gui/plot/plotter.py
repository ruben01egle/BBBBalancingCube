from PyQt5.QtWidgets import QWidget, QGridLayout
from .imu_plot import IMUPlot
from .imu_calib_plot import IMUCalibPlot
from .state_plot import StatePlot

PLOT_TYPES = {
    "imuraw": IMUPlot,
    "imucalib": IMUCalibPlot,
    "state": StatePlot,
}

class PlotManager:
    def __init__(self, mode):
        self.widget = QWidget()
        self.layout = QGridLayout()
        self.widget.setLayout(self.layout)
        self.plots = []

        parts = mode.lower().split('+')
        for col, part in enumerate(parts):
            if part not in PLOT_TYPES:
                raise ValueError(f"Unknown mode: {mode}")
            plot = PLOT_TYPES[part]()
            self.plots.append(plot)
            self.layout.addWidget(plot.get_widget(), 0, col)

    def get_widget(self):
        return self.widget

    def update(self, ccontent):
        for plot in self.plots:
            plot.update(ccontent)
