from abc import ABC, abstractmethod
from PyQt5.QtWidgets import QWidget

class BasePlot(ABC):
    def __init__(self, plot_widget):
        self.plot_widget = plot_widget

    @abstractmethod
    def update(self, ccontent) -> None:
        pass

    @abstractmethod
    def get_widget(self) -> QWidget:
        pass
