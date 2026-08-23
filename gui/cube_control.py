import sys
from typing import Optional
from PyQt5.QtGui import QCloseEvent
import cv2
from PyQt5.QtWidgets import (
    QApplication, QWidget, QVBoxLayout, QHBoxLayout, QLabel, QGroupBox, QRadioButton
)
from PyQt5.QtCore import QTimer, Qt
from PyQt5.QtGui import QImage, QPixmap

from servo_control.servo_control import ServoClient

# -------------------- GUI Hauptfenster --------------------
class MainWindow(QWidget):
    def __init__(self, rtsp_url="rtsp://admin:admin@127.0.0.1:49000/12"):
        super().__init__()
        self.setWindowTitle("Servo + Kamera GUI")
        self.resize(800, 600)

        self.servo = ServoClient()
        self.servo.connect()

        # ---------------- Layout ----------------
        main_layout = QHBoxLayout()
        self.setLayout(main_layout)

        # Video Display
        self.video_label = QLabel("Kamera wird geladen...")
        self.video_label.setAlignment(Qt.AlignmentFlag.AlignCenter)
        main_layout.addWidget(self.video_label, 4)

        # Steuerung
        control_widget = QWidget()
        control_layout = QVBoxLayout()
        control_widget.setLayout(control_layout)
        main_layout.addWidget(control_widget, 1)

        # Radio Buttons für Events
        radio_group = QGroupBox("Cube Position")
        radio_layout = QVBoxLayout()
        radio_group.setLayout(radio_layout)
        control_layout.addWidget(radio_group)

        self.buttons = {}
        # TODO: events are named in line with old coordinate system -> display current
        for event_name, button_text in [
            ("Set90Deg", "-90°"),
            ("Set10Deg", "-10°"),
            ("Set0Deg", "0°"),
            ("SetM10Deg", "10°"),
            ("SetM90Deg", "90°"),
            ("Set_5deg_gap", "5° Gap"),
            ("Set_release", "Release")
        ]:
            btn = QRadioButton(button_text)       # Sichtbarer Text
            btn.event_name = event_name           # eigenes Attribut für Event
            btn.toggled.connect(lambda checked, b=btn: self.on_event(b.event_name, checked))
            radio_layout.addWidget(btn)
            self.buttons[event_name] = btn

        radio_layout.addStretch()

        # ---------------- Video Capture ----------------
        self.cap = cv2.VideoCapture(rtsp_url)
        self.timer = QTimer()
        self.timer.timeout.connect(self.update_frame)
        self.timer.start(30)  # ~30 FPS

    def on_event(self, event_name, checked):
        if checked:
            self.servo.send_event(event_name)

    def update_frame(self):
        if not self.cap.isOpened():
            return
        ret, frame = self.cap.read()
        if not ret:
            return
        frame = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
        h, w, ch = frame.shape
        bytes_per_line = ch * w
        qt_image = QImage(frame.data, w, h, bytes_per_line, QImage.Format_RGB888)

        pixmap = QPixmap.fromImage(qt_image)
        self.video_label.setPixmap(pixmap.scaled(
            self.video_label.size(),
            Qt.AspectRatioMode.KeepAspectRatio,
            Qt.TransformationMode.SmoothTransformation
        ))


    def closeEvent(self, a0: Optional[QCloseEvent]) -> None:
        if a0 is not None:
            self.servo.disconnect()
            a0.accept()

# -------------------- Start --------------------
if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = MainWindow()
    window.show()
    sys.exit(app.exec_())
