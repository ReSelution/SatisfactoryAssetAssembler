import re

SAP_LOG = "./logs/sap.log"
CUE_LOG = "./logs/cue.log"

PATTERN = re.compile(r"(\w+Property)\s+@\s+(\d+)")
offsetDict = {}

def parseCueLog():
    with open(CUE_LOG, 'r') as f:
        for line in f:
            match = PATTERN.search(line)
            if match:
                offsetDict[match.group(2)] = match.group(1)

def main():
    parseCueLog()
    with open(SAP_LOG, 'r') as f:
        for line in f:
            match = PATTERN.search(line)
            if match:
                my_type = match.group(1)   # z.B. "FloatProperty"
                my_offset = match.group(2) # z.B. "28386" (als String)

                # Hol den Typ, den CUE an diesem Offset hatte
                cue_type = offsetDict.get(my_offset)

                if cue_type:
                    if cue_type == my_type:
                        pass
                    else:
                        print(f"CONFLICT at @{my_offset}: CUE says {cue_type}, but I say {my_type}")
                else:
                    print(f"MISSING in CUE: My parser found {my_type} at @{my_offset}")


if __name__ == "__main__":
    main()