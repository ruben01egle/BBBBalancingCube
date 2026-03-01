import sys
import ctypes
import queue
import threading
from pathlib import Path
from datetime import datetime

from PyQt5.QtWidgets import QApplication, QMainWindow
from PyQt5.QtCore import QTimer

from data.cdata import CContent
from data.client import TCPClient
from plot.plotter import PlotManager
from data.recorder import DataRecorder

def main():
    
    print(ctypes.sizeof(CContent))

    app = QApplication(sys.argv)

    client = TCPClient("localhost", 40000)
    if not client.connect():
        print("Could not connect to server.")
        return

    plotter = PlotManager(mode="state")

    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    file_path = Path(f"ExperimentsData/experiment_data{timestamp}.csv")
    file_path.parent.mkdir(parents=True, exist_ok=True)
    recorder = DataRecorder(file_path, mode="imu+state")

    data_queue = queue.Queue()

    def receive_loop():
        try:
            while True:
                data = client.recv_bytes(ctypes.sizeof(CContent))
                msg = CContent.from_bytes(data)
                recorder.record(msg)
                data_queue.put(msg)
        except Exception as e:
            print("Error while receiving data:", e)
        except KeyboardInterrupt:
            print("End program")

    thread = threading.Thread(target=receive_loop, daemon=True)
    thread.start()

    main_window = QMainWindow()
    main_window.setCentralWidget(plotter.get_widget())
    main_window.show()

    def timer_callback():
        while not data_queue.empty():
            msg = data_queue.get_nowait()
            plotter.update(msg)

    timer = QTimer()
    timer.timeout.connect(timer_callback)
    timer.start(10)

    ret = app.exec()

    client.close()
    return ret

if __name__ == "__main__":
    sys.exit(main())
