# not really needed anymore

import os
import json
from glob import glob

def extract_strings(obj, path=()):
    if isinstance(obj, dict):
        result = {}
        for k, v in obj.items():
            sub = extract_strings(v, path + (k,))
            if isinstance(sub, dict):
                result.update(sub)
            else:
                result[path + (k,)] = sub
        return result
    elif isinstance(obj, str):
        return obj
    return {}

def main():
    translations_dir = os.path.join("res", "translations")
    files = glob(os.path.join(translations_dir, "*.json"))
    with open(os.path.join(translations_dir, "en.json"), encoding="utf-8") as f:
        en_data = json.load(f)
    en_strings = extract_strings(en_data)
    en_flat = {v: v for v in en_strings.values()}

    for file in files:
        with open(file, encoding="utf-8") as f:
            data = json.load(f)
        strings = extract_strings(data)
        mapping = {}
        for path, value in strings.items():
            en_value = en_strings.get(path)
            if en_value:
                mapping[en_value] = value
        if os.path.basename(file) == "en.json":
            mapping = {k: k for k in mapping}
        with open(file, "w", encoding="utf-8") as f:
            json.dump(mapping, f, ensure_ascii=False, indent=2)

    fill_missing_keys()

def fill_missing_keys():
    translations_dir = os.path.join("res", "translations")
    files = glob(os.path.join(translations_dir, "*.json"))
    all_keys = set()
    file_data = {}
    for file in files:
        with open(file, encoding="utf-8") as f:
            data = json.load(f)
        file_data[file] = data
        all_keys.update(data.keys())
    for file, data in file_data.items():
        updated = False
        for key in all_keys:
            if key not in data:
                data[key] = f"[TODO]: {key}"
                updated = True
        if updated:
            with open(file, "w", encoding="utf-8") as f:
                json.dump(data, f, ensure_ascii=False, indent=2)
            print(f"Updated {os.path.basename(file)} with missing keys.")
        else:
            print(f"No missing keys in {os.path.basename(file)}.")

if __name__ == "__main__":
    main()
