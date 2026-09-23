import sys

import pandas as pd

# Pfad zur CSV-Datei
# !!! Orientation of Imu: cube in zero position
DEFAULT_FILENAME = "ExperimentsData/ImuCalib.csv"

# Full-scale ranges of the IMU: must match IMU_SETUP in include/CubeConstants.hpp
GYRO_RANGE_DPS = 2000    # 250, 500, 1000 or 2000
ACCEL_RANGE_G = 4        # 2, 4, 8 or 16

# Sensitivities from the MPU-9250 datasheet (keep in sync with CubeConstants.hpp)
GYRO_LSB_PER_DPS = {250: 131.0, 500: 65.5, 1000: 32.8, 2000: 16.4}
ACCEL_LSB_PER_G = {2: 16384.0, 4: 8192.0, 8: 4096.0, 16: 2048.0}

GRAVITY = 9.81
PI = 3.14159265359

# Expected raw y reading in zero position (1 g)
y_raw_expected = ACCEL_LSB_PER_G[ACCEL_RANGE_G]

# Skalenfaktoren definieren (gelten für beide Sensoren gleich)
accel_scale_x = GRAVITY / ACCEL_LSB_PER_G[ACCEL_RANGE_G]
accel_scale_y = GRAVITY / ACCEL_LSB_PER_G[ACCEL_RANGE_G]
gyro_scale = PI / 180 / GYRO_LSB_PER_DPS[GYRO_RANGE_DPS]

def calibrate_sensor(df, prefix):
    """Berechnet Offsets für einen Sensor (prefix = 'S1' oder 'S2')."""
    accel_x = df[f'{prefix}_mAx'].mean()
    accel_y = df[f'{prefix}_mAy'].mean() - y_raw_expected
    gyro_z  = df[f'{prefix}_mWz'].mean()

    return {
        'accel_x': accel_x,
        'accel_y': accel_y,
        'gyro_z': gyro_z,
    }

def compute_calibration(df):
    """Returns all IMU calibration values, keyed like the entries in cube_calibration.json."""
    result = {}
    for imu, prefix in ((1, 'S1'), (2, 'S2')):
        offsets = calibrate_sensor(df, prefix)
        result[f'imu{imu}AccelScaleX'] = accel_scale_x
        result[f'imu{imu}AccelScaleY'] = accel_scale_y
        result[f'imu{imu}AccelOffsetX'] = float(offsets['accel_x'])
        result[f'imu{imu}AccelOffsetY'] = float(offsets['accel_y'])
        result[f'imu{imu}GyroScale'] = gyro_scale
        result[f'imu{imu}GyroOffset'] = float(offsets['gyro_z'])
    return result

def main():
    filename = sys.argv[1] if len(sys.argv) > 1 else DEFAULT_FILENAME
    cal = compute_calibration(pd.read_csv(filename))

    # Same layout as the "calibration" block in config/cube_calibration.json
    # (adcScale, adcOffset and phiOffset are not part of this calibration)
    for imu in (1, 2):
        print(f'        "imu{imu}AccelScaleX": {cal[f"imu{imu}AccelScaleX"]:.6f}, "imu{imu}AccelScaleY": {cal[f"imu{imu}AccelScaleY"]:.6f},')
        print(f'        "imu{imu}AccelOffsetX": {cal[f"imu{imu}AccelOffsetX"]:.6f}, "imu{imu}AccelOffsetY": {cal[f"imu{imu}AccelOffsetY"]:.6f},')
        print(f'        "imu{imu}GyroScale": {cal[f"imu{imu}GyroScale"]:.6f}, "imu{imu}GyroOffset": {cal[f"imu{imu}GyroOffset"]:.6f},')

if __name__ == "__main__":
    main()
