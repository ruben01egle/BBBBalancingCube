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

def extract_imu_calib_data(msg):
    if msg is None:
        return []
    return [
        msg.mSensor1DataCalib.mDotPhi, msg.mSensor1DataCalib.mDDotX, msg.mSensor1DataCalib.mDDotY,
        msg.mSensor2DataCalib.mDotPhi, msg.mSensor2DataCalib.mDDotX, msg.mSensor2DataCalib.mDDotY
    ]

def imu_calib_headers():
    return [
        'S1_mDotPhi', 'S1_mDDotX', 'S1_mDDotY',
        'S2_mDotPhi', 'S2_mDDotX', 'S2_mDDotY'
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

def extract_adc_data(msg):
    if msg is None:
        return []
    return [msg.mADCValue]

def adc_headers():
    return ['mADCValue']

PART_MAP = {
    'imuraw': (extract_imu_data, imu_headers),
    'imucalib': (extract_imu_calib_data, imu_calib_headers),
    'state': (extract_state_data, state_headers),
    'adc': (extract_adc_data, adc_headers),
}

class DataRecorder:
    def __init__(self, filename, mode: str):
        self.file = open(filename, 'w', newline='')
        self.writer = csv.writer(self.file)
        self.selected_parts = mode.lower().split('+')  # e.g. ['imuraw', 'imucalib', 'state']

        if 'all' in self.selected_parts:
            self.selected_parts = list(PART_MAP.keys())

        self.extractors = []
        self.headers = ['Timestamp_us']

        for part in self.selected_parts:
            if part not in PART_MAP:
                raise ValueError(f"Unknown mode part: '{part}'")
            extractor, headers_fn = PART_MAP[part]
            self.extractors.append(extractor)
            self.headers += headers_fn()

        self.writer.writerow(self.headers)

    def record(self, msg: CContent):
        row = [msg.mTimeUs]
        for extractor in self.extractors:
            row += extractor(msg)
        self.writer.writerow(row)

    def close(self):
        self.file.close()
