import re

def extract_unique_properties(input_file):
    # Regex sucht nach dem Namen zwischen den Doppelpunkten
    prop_regex = re.compile(r": (\w+) :")
    unique_props = set()

    try:
        with open(input_file, 'r') as f:
            for line in f:
                match = prop_regex.search(line)
                if match:
                    unique_props.add(match.group(1))
    except FileNotFoundError:
        print(f"Datei {input_file} nicht gefunden.")
        return

    for prop in sorted(unique_props):
        var_name = prop
        if prop.startswith('m_') and len(prop) > 2:
            var_name = prop[2:]
        elif prop.startswith('m') and len(prop) > 1 and prop[1].isupper():
            var_name = prop[1:]
        elif prop.startswith('b') and len(prop) > 1 and prop[1].isupper():
            var_name = prop[1:]

        # print(f'static constexpr uint32_t {var_name} = fnv1a("{prop}");')
        print(f' DBG_MARK_HANDLED(m_className, Base::{var_name});')


    print(f"\n// Gesamtanzahl einzigartiger Properties: {len(unique_props)}")

if __name__ == "__main__":
    extract_unique_properties("data.txt")