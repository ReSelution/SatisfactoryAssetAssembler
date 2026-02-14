import re
import statistics

def analyze_assets(data):
    # Regex findet: Pfad: Zahl
    pattern = re.compile(r'(.*?):\s*(\[?\d+\]?)')

    sizes = []
    uasset_sizes = []
    max_path_len = 0
    longest_path = ""

    for line in data.strip().split('\n'):
        match = pattern.search(line)
        if match:
            path = match.group(1).strip()
            # Pfadlänge berechnen
            current_len = len(path)
            if current_len > max_path_len:
                max_path_len = current_len
                longest_path = path

            # Größe extrahieren
            val = int(match.group(2).replace('[', '').replace(']', ''))
            sizes.append(val)

            # Spezielle Statistik nur für .uasset
            if ".uasset" in path.lower():
                uasset_sizes.append(val)

    if not sizes:
        print("Keine Daten gefunden!")
        return

    def print_stats(name, data_list):
        if not data_list: return
        print(f"--- Statistik für {name} ({len(data_list)} Dateien) ---")
        print(f"Minimum:  {min(data_list):>12} Bytes")
        print(f"Maximum:  {max(data_list):>12} Bytes")
        print(f"Schnitt:  {int(statistics.mean(data_list)):>12} Bytes")
        print(f"Median:   {int(statistics.median(data_list)):>12} Bytes")
        # P95 berechnen
        data_sorted = sorted(data_list)
        p95_idx = int(len(data_sorted) * 0.95)
        print(f"P95:      {data_sorted[p95_idx]:>12} Bytes")
        print()

    print_stats("Alle Dateien", sizes)
    print_stats(".uasset Dateien", uasset_sizes)

    print("--- Pfad-Analyse ---")
    print(f"Maximale Pfadlänge: {max_path_len} Zeichen")
    print(f"Längster Pfad:      {longest_path}")
    if max_path_len > 260:
        print("\nWARNUNG: Pfade über 260 Zeichen gefunden! Prüfe dein VFS auf Long-Path Support.")

if __name__ == "__main__":
    try:
        with open("output/filesEntry.txt", "r") as f:
            log_data = f.read()
        analyze_assets(log_data)
    except FileNotFoundError:
        print("Datei output/filesEntry.txt nicht gefunden!")