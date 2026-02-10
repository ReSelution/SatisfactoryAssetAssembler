import json
import re


def compare_recipes(log_file_path, json_file_path):
    official_json = {}

    try:
        with open(json_file_path, "r", encoding='utf-16') as f:
            data = json.load(f)
            for item in data:
                if item.get("NativeClass") == "/Script/CoreUObject.Class'/Script/FactoryGame.FGRecipe'" or \
                    item.get("NativeClass") == "/Script/CoreUObject.Class'/Script/FactoryGame.FGItemDescriptor'" or \
                    item.get("NativeClass") == "/Script/CoreUObject.Class'/Script/FactoryGame.FGBuildingDescriptor'" or \
                    item.get("NativeClass") == "/Script/CoreUObject.Class'/Script/FactoryGame.FGItemDescriptorBiomass'":

                    for recipe in item["Classes"]:
                        official_json[recipe["ClassName"]] = recipe
    except FileNotFoundError:
        print(f"Fehler: JSON-Datei '{json_file_path}' nicht gefunden.")
        return

    recipe_pattern = re.compile(r"\[SAP\]\s+\[DATA_DUMP\]:\s+([\w_]+)")
    found_in_both = []
    only_in_crawler = []
    try:
        with open(log_file_path, "r", encoding='utf-8', errors='ignore') as f:
            for line in f:
                match = recipe_pattern.search(line)
                if match:
                    recipe_id = match.group(1)

                    if recipe_id in official_json:
                        found_in_both.append(recipe_id)
                    else:
                        only_in_crawler.append(recipe_id)
    except FileNotFoundError:
        print(f"Fehler: Log-Datei '{log_file_path}' nicht gefunden.")
        return

    # 3. Mengenvergleich

    crawler_set = set(found_in_both) | set(only_in_crawler)
    only_in_json = [name for name in official_json.keys() if name not in crawler_set]

    # Kurzes Fazit
    for recipe in only_in_crawler:
        print(f"only Crawler: {recipe}")

    for recipe in only_in_json:
        print(f"only JSON: {recipe}")
    print(f"Match: {len(found_in_both)} | Nur Crawler: {len(only_in_crawler)} | Nur JSON: {len(only_in_json)}")


if __name__ == '__main__':
    # Pfade hier anpassen
    compare_recipes("logs/sap.log", "tools/en-US.json")
