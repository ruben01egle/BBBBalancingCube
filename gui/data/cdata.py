import ctypes

class CImuData(ctypes.Structure):
    _fields_ = [
        ("mWx", ctypes.c_int16),
        ("mWy", ctypes.c_int16),
        ("mWz", ctypes.c_int16),
        ("mAx", ctypes.c_int16),
        ("mAy", ctypes.c_int16),
        ("mAz", ctypes.c_int16),
    ]

class CStateVectorData(ctypes.Structure):
    _fields_ = [
        ("mPhi_A", ctypes.c_float),
        ("mPhi_G", ctypes.c_float),
        ("mPhi_C", ctypes.c_float),
        ("mPhi_d", ctypes.c_float),
        ("mPsi_d", ctypes.c_float),
    ]

class CContent(ctypes.Structure):
    _fields_ = [
        ("mTimeUs", ctypes.c_int64),
        ("mSensor1Data", CImuData),
        ("mSensor2Data", CImuData),
        ("mStateData", CStateVectorData),
        ("mMotorTorque", ctypes.c_float),
        ("mADCValue", ctypes.c_uint16),
        ("mPadding", ctypes.c_uint16),
    ]

    @classmethod
    def from_bytes(cls, data: bytes) -> "CContent":
        if len(data) != ctypes.sizeof(cls):
            raise ValueError(f"Expected {ctypes.sizeof(cls)} bytes, got {len(data)}")
        return cls.from_buffer_copy(data)
