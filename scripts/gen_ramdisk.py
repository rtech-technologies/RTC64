import sys
import os

def main():
    if len(sys.argv) < 2:
        print("Usage: gen_ramdisk.py <output_file>")
        sys.exit(1)

    out_file = sys.argv[1]
    # 32MB Ramdisk
    size = 32 * 1024 * 1024
    with open(out_file, 'wb') as f:
        f.write(b'\x00' * size)

    # Add userland apps to ramdisk
    # This is a dummy ramdisk generator, we should actually format it and add files.
    # For now, let's just make sure the file exists.
    # To truly be useful, we'd use mtools to add shell.bin
    print(f"Created industrial ramdisk: {out_file} ({size} bytes)")

if __name__ == "__main__":
    main()
