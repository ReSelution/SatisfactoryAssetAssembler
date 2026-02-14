import pandas as pd
import matplotlib
import matplotlib.pyplot as plt
import seaborn as sns

def analyze():
    # Lädt die CSV und ignoriert das "us" Suffix in der Spalte
    df = pd.read_csv("benchmark_results.csv")

    # "us" entfernen und zu Zahl konvertieren
    df['duration_us'] = df['duration_us'].astype(str).str.replace('us', '').astype(int)
    df["buffer_size"] /= 1000
    # Statistik berechnen
    stats = df.groupby('buffer_size')['duration_us'].agg(['median', 'min', 'max', 'std']).reset_index()

    print("--- Benchmark Ergebnisse (in ms) ---")
    stats['median_ms'] = stats['median'] / 1000
    stats['std_ms'] = stats['std'] / 1000
    print(stats[['buffer_size', 'median_ms', 'std_ms']].sort_values('median_ms'))

    # Plotting
    plt.figure(figsize=(10, 6))
    sns.boxplot(data=df, x='buffer_size', y='duration_us')
    plt.title('Performance Verteilung pro Buffer-Größe')
    plt.ylabel('Zeit in Mikrosekunden')
    plt.savefig("benchmark_boxplots.png")
    print("\nBoxplot gespeichert als 'benchmark_boxplots.png'")
    plt.show()


if __name__ == "__main__":
    analyze()