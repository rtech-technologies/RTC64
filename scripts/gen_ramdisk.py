import sys
import os

def main():
    if len(sys.argv) < 2:
        print("Usage: gen_ramdisk.py <output_file>")
        sys.exit(1)

    out_file = sys.argv[1]
    # Simple empty ramdisk for now
    size = 32 * 1024 * 1024
    with open(out_file, 'wb') as f:
        f.write(b'\x00' * size)
    print(f"Created industrial ramdisk: {out_file}")

if __name__ == "__main__":
    main()
