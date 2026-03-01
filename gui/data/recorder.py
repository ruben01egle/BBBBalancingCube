import csv
from .cdata import CContent

def extract_imu_data(msg):
    if msg is None:
        return []
    return [
        msg.mSensor1Data.mWx, msg.mSensor1Data.mWy, msg.mSensor1Data.mWz,
        msg.mSensor1Data.mAx, msg.mSensor1Data.mAy, msg.mSensor1Data.mAz,
        msg.mSensor2Data.mWx, msg.mSensor2Data.mWy, msg.mSensor2Data.mWz,
        msg.mSensor2Data.mAx, msg.mSensor2Data.mAy, msg.mSensor2Data.mAz
    ]

def imu_headers():
    return [
        'S1_mWx', 'S1_mWy', 'S1_mWz', 'S1_mAx', 'S1_mAy', 'S1_mAz',
        'S2_mWx', 'S2_mWy', 'S2_mWz', 'S2_mAx', 'S2_mAy', 'S2_mAz'
    ]

def extract_state_data(msg: CContent):
    if msg is None:
        return []
    state = msg.mStateData
    return [
        state.mPhi_A, state.mPhi_G, state.mPhi_C, state.mDotPhi, state.mDotPsi,
        msg.mMotorTorque   # Motormoment ans Ende
    ]

def state_headers():
    return ['mPhi_A', 'mPhi_G', 'mPhi_C', 'mDotPhi', 'mDotPsi', 'mMotorTorque']

class DataRecorder:
    def __init__(self, filename, mode: str):
        self.file = open(filename, 'w', newline='')
        self.writer = csv.writer(self.file)
        self.selected_parts = mode.lower().split('+')  # z. B. ['imu', 'state']

        self.extractors = []
        self.headers = ['Timestamp_us']

        for part in self.selected_parts:
            if part == 'imu':
                self.extractors.append(lambda msg: extract_imu_data(msg))
                self.headers += imu_headers()
            elif part == 'state':
                self.extractors.append(lambda msg: extract_state_data(msg))
                self.headers += state_headers()
            else:
                raise ValueError(f"Unknown mode part: '{part}'")

        self.writer.writerow(self.headers)

    def record(self, msg: CContent):
        row = [msg.mTimeUs]
        for extractor in self.extractors:
            row += extractor(msg)
        self.writer.writerow(row)

    def close(self):
        self.file.close()
