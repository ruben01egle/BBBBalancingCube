import csv
import argparse
import matplotlib.pyplot as plt
import os

def calculate_jitter(csv_path, expected_period, save=False):
    timestamps = []

    # CSV einlesen
    with open(csv_path, 'r') as file:
        reader = csv.DictReader(file)
        for row in reader:
            timestamps.append(int(row['Timestamp_us']))

    # Perioden berechnen
    periods = [t2 - t1 for t1, t2 in zip(timestamps[:-1], timestamps[1:])]

    if not periods:
        print("Nicht genügend Daten zur Berechnung.")
        return

    mean_period = sum(periods) / len(periods)
    jitters = [p - expected_period for p in periods]
    jitters_abs = [abs(p - expected_period) for p in periods]
    jitters_mean_period = [abs(p - mean_period) for p in periods]

    average_jitter = sum(jitters_abs) / len(jitters_abs)
    max_jitter = max(jitters_abs)
    average_jitter_mean_period = sum(jitters_mean_period) / len(jitters_mean_period)
    max_jitter_mean_period = max(jitters_mean_period)

    print(f"Datei: {csv_path}")
    print(f"Vgl mit erwarteter Periode: {expected_period:.2f} µs")
    print(f"Durchschnittlicher Jitter: {average_jitter:.2f} µs")
    print(f"Maximaler Jitter: {max_jitter:.2f} µs")
    print(f"Vgl mit mittlerer Periode: {mean_period:.2f} µs")
    print(f"Durchschnittlicher Jitter: {average_jitter_mean_period:.2f} µs")
    print(f"Maximaler Jitter: {max_jitter_mean_period:.2f} µs")

    # Professionelles Diagramm-Layout
    plt.figure(figsize=(10, 5))
    plt.hist(jitters, bins=80, color="steelblue", edgecolor="black", linewidth=0.5)

    plt.title("Histogramm Jitter")
    plt.xlabel("Abweichung von erwarteter Periode [µs]")
    plt.ylabel("Anzahl Perioden")

    plt.grid(True, linestyle="--", alpha=0.6)
    plt.tight_layout()

    if save:
        pdf_path = os.path.splitext(csv_path)[0] + ".pdf"
        plt.savefig(pdf_path, format="pdf")
        print(f"Diagramm gespeichert unter: {pdf_path}")
    else:
        plt.show()

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Berechnet Jitter aus IMU-Timestamps in einer CSV-Datei.")
    parser.add_argument("csv_path", help="Pfad zur CSV-Datei mit IMU-Daten")
    parser.add_argument("expected_period", type=int, help="Erwartete Periodendauer in Mikrosekunden")
    parser.add_argument("--save", action="store_true", help="Speichert das Diagramm als PDF mit gleichem Namen wie die CSV-Datei")

    args = parser.parse_args()
    calculate_jitter(args.csv_path, args.expected_period, save=args.save)
