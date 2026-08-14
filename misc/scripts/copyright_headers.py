#!/usr/bin/env python3

import os
import sys
import re

pattern = r"(.*\n){3}(.*This file is part of:.*\n)(.*GODOT ENGINE.*\n)(.*https://godotengine.org.*\n)(.*\n){23}"
header = "// Copyright Godot Engine contributors (see AUTHORS.md).\n"

if len(sys.argv) < 2:
    print("Invalid usage of copyright_headers.py, it should be called with a path to one or multiple files.")
    sys.exit(1)

for f in sys.argv[1:]:
    with open(f.strip(), "r", encoding="utf-8") as fileread:
        text = fileread.read()

        if re.search(pattern, text):
            text = re.sub(pattern, "", text)
            text = header + text
        elif header not in text:
            text = header + text

    with open(f.strip(), "w", encoding="utf-8", newline="\n") as filewrite:
        filewrite.write(text)
