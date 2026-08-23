import pandas as pd

# Pfad zur CSV-Datei
# !!! Orientation of Imu: cube in zero position
filename = "ExperimentsData/ImuCalib.csv"

# CSV Datei einlesen
df = pd.read_csv(filename)

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

offsets_s1 = calibrate_sensor(df, 'S1')
offsets_s2 = calibrate_sensor(df, 'S2')

print("        // IMU 1")
print(f"        .mImu1AccelScaleX  = {accel_scale_x:.6f},")
print(f"        .mImu1AccelScaleY  = {accel_scale_y:.6f},")
print(f"        .mImu1AccelOffsetX = {offsets_s1['accel_x']:.6f},")
print(f"        .mImu1AccelOffsetY = {offsets_s1['accel_y']:.6f},")
print(f"        .mImu1GyroScale    = {gyro_scale:.6f},")
print(f"        .mImu1GyroOffset   = {offsets_s1['gyro_z']:.6f},")
print()
print("        // IMU 2")
print(f"        .mImu2AccelScaleX  = {accel_scale_x:.6f},")
print(f"        .mImu2AccelScaleY  = {accel_scale_y:.6f},")
print(f"        .mImu2AccelOffsetX = {offsets_s2['accel_x']:.6f},")
print(f"        .mImu2AccelOffsetY = {offsets_s2['accel_y']:.6f},")
print(f"        .mImu2GyroScale    = {gyro_scale:.6f},")
print(f"        .mImu2GyroOffset   = {offsets_s2['gyro_z']:.6f},")