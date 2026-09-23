import sys
import argparse
import ctypes
import queue
import threading
import time
from pathlib import Path
from datetime import datetime

from data.cdata import CContent
from data.client import TCPClient
from data.recorder import DataRecorder

CALIB_CONNECT_TIMEOUT_S = 60

def calib_main(out_file):
    """Headless mode: record raw IMU data of a `--calibrate` run until the server closes the connection.

    The SSH tunnel accepts connections before the app listens, so a connection that
    closes before any data arrived is retried until the timeout.
    """
    if out_file is None:
        timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
        out_file = f"ExperimentsData/ImuCalib_{timestamp}.csv"
    file_path = Path(out_file)
    file_path.parent.mkdir(parents=True, exist_ok=True)

    deadline = time.monotonic() + CALIB_CONNECT_TIMEOUT_S
    while time.monotonic() < deadline:
        client = TCPClient("localhost", 40000)
        if not client.connect():
            time.sleep(0.5)
            continue
        recorder = DataRecorder(file_path, mode="imuraw")
        count = 0
        try:
            while True:
                data = client.recv_bytes(ctypes.sizeof(CContent))
                recorder.record(CContent.from_bytes(data))
                count += 1
        except ConnectionError:
            pass  # server closed the connection: end of calibration run
        finally:
            recorder.close()
            client.close()
        if count > 0:
            print(f"Recorded {count} samples to {file_path}")
            return 0
        time.sleep(0.5)

    print("Calibration run: no data received")
    return 1

def main():
    from PyQt5.QtWidgets import QApplication, QMainWindow
    from PyQt5.QtCore import QTimer
    from plot.plotter import PlotManager

    print(ctypes.sizeof(CContent))

    app = QApplication(sys.argv)

    client = TCPClient("localhost", 40000)
    if not client.connect():
        print("Could not connect to server.")
        return

    plotter = PlotManager()

    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    file_path = Path(f"ExperimentsData/experiment_data{timestamp}.csv")
    file_path.parent.mkdir(parents=True, exist_ok=True)
    recorder = DataRecorder(file_path, mode="all")

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
    parser = argparse.ArgumentParser()
    parser.add_argument("--calib", action="store_true",
                        help="headless: record raw IMU data of a --calibrate run and exit")
    parser.add_argument("--out", help="output csv file for --calib")
    args = parser.parse_args()
    sys.exit(calib_main(args.out) if args.calib else main())
