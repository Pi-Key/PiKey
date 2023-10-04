import sys

if len(sys.argv) == 2:
    try:
        with open(sys.argv[1], "r") as f:
            lines = f.readlines()
            print(''.join(lines).replace("\n", "\\n"))
    except:
        print("[ERR]: Could not open file '{sys.argv[1]}'")

else:
    print("[ERR]: You need to include a file.\n Example: python string_from_file.py input_file.py")
