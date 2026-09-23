import sys

import pandas as pd

# Pfad zur CSV-Datei
# !!! Orientation of Imu: cube in zero position
DEFAULT_FILENAME = "ExperimentsData/ImuCalib.csv"

y_raw_expected = 8192.0

# Skalenfaktoren definieren (gelten für beide Sensoren gleich)
accel_scale_x = 9.81 / 8192.0
accel_scale_y = 9.81 / 8192.0
gyro_scale = 1.0 / 16.4 * 3.14159265359 / 180

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

    for imu in (1, 2):
        print(f"        // IMU {imu}")
        print(f"        .mImu{imu}AccelScaleX  = {cal[f'imu{imu}AccelScaleX']:.6f},")
        print(f"        .mImu{imu}AccelScaleY  = {cal[f'imu{imu}AccelScaleY']:.6f},")
        print(f"        .mImu{imu}AccelOffsetX = {cal[f'imu{imu}AccelOffsetX']:.6f},")
        print(f"        .mImu{imu}AccelOffsetY = {cal[f'imu{imu}AccelOffsetY']:.6f},")
        print(f"        .mImu{imu}GyroScale    = {cal[f'imu{imu}GyroScale']:.6f},")
        print(f"        .mImu{imu}GyroOffset   = {cal[f'imu{imu}GyroOffset']:.6f},")
        if imu == 1:
            print()

if __name__ == "__main__":
    main()
