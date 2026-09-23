from PyQt5.QtWidgets import QTabWidget
from .imu_plot import IMUPlot
from .imu_calib_plot import IMUCalibPlot
from .state_plot import StatePlot

PLOT_TYPES = {
    "imuraw": IMUPlot,
    "imucalib": IMUCalibPlot,
    "state": StatePlot,
}

class PlotManager:
    def __init__(self):
        self.tabs = QTabWidget()
        self.plots = []

        for name, plot_cls in PLOT_TYPES.items():
            plot = plot_cls()
            self.plots.append(plot)
            self.tabs.addTab(plot.get_widget(), name)

    def get_widget(self):
        return self.tabs

    def update(self, ccontent):
        for plot in self.plots:
            plot.update(ccontent)
