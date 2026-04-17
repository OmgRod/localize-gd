import os
import json
from glob import glob

TRANSLATION_DIR = os.path.join(os.path.dirname(__file__), 'res', 'translations')
EN_FILE = os.path.join(TRANSLATION_DIR, 'en.json')

def load_json(path):
    if os.path.exists(path):
        with open(path, 'r', encoding='utf-8') as f:
            return json.load(f)
    return {}

def save_json(path, data):
    with open(path, 'w', encoding='utf-8') as f:
        json.dump(data, f, ensure_ascii=False, indent=2)

def add_string_to_translations(new_string):
    # Add to en.json
    en_data = load_json(EN_FILE)
    if new_string not in en_data:
        en_data[new_string] = new_string
        save_json(EN_FILE, en_data)
        print(f"Added '{new_string}' to en.json.")
    else:
        print(f"'{new_string}' already exists in en.json.")
    # Add to other translations as TODO
    files = glob(os.path.join(TRANSLATION_DIR, '*.json'))
    for file in files:
        if os.path.basename(file) == 'en.json':
            continue
        data = load_json(file)
        if new_string not in data:
            data[new_string] = f"[TODO]: {new_string}"
            save_json(file, data)
            print(f"Added TODO for '{new_string}' in {os.path.basename(file)}.")
        else:
            print(f"'{new_string}' already exists in {os.path.basename(file)}.")

def main():
    print("Type a string to add to en.json and all translations. Press Enter on an empty line to quit.")
    while True:
        s = input('String: ').strip()
        if not s:
            print("Done.")
            break
        add_string_to_translations(s)

if __name__ == '__main__':
    main()
