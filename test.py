import sys
import pycfb

file = pycfb.CFBFile("xxx")

if not file.open(): # required
    sys.exit(-1)

entries: dict[str, pycfb.CFBEntry] = file.entries()

for name, entry in entries.items():
    print(f"{name}-{entry.getDepth()}")

with open("name", "wb") as f: # extract one entry named `name`
    f.write(file.get("name"))
