import sys
import os
import subprocess

def run_cmd(cmd):
    print(f"Running: {' '.join(cmd)}")
    res = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    if res.returncode != 0:
        print(f"Error executing command: {cmd}")
        print(f"stdout: {res.stdout.decode(errors='ignore')}")
        print(f"stderr: {res.stderr.decode(errors='ignore')}")
        sys.exit(1)

def main():
    if len(sys.argv) < 2:
        print("Usage: gen_ramdisk.py <output_file>")
        sys.exit(1)

    out_file = sys.argv[1]

    # Set ramdisk size to 48MB so it has plenty of space but remains lightweight
    size = 48 * 1024 * 1024

    print(f"Creating unformatted ramdisk file of size {size // (1024*1024)}MB...")
    with open(out_file, 'wb') as f:
        f.write(b'\x00' * size)

    # Format the ramdisk as FAT32
    print("Formatting ramdisk as FAT32 using mformat...")
    run_cmd(["mformat", "-F", "-i", out_file, "::"])

    # Create root level directories
    dirs = [
        "::/bin",
        "::/etc",
        "::/home",
        "::/home/Administrator",
        "::/home/Administrator/Desktop",
        "::/system",
        "::/system/wallpapers",
        "::/registry"
    ]
    print("Creating directory structure on ramdisk...")
    for d in dirs:
        run_cmd(["mmd", "-i", out_file, d])

    # Copy applications to root and /bin
    apps_map = {
        "apps/shell/app.bin": ["shell.bin", "bin/shell.bin", "home/Administrator/Desktop/shell.bin"],
        "apps/lab/app.bin": ["lab.bin", "bin/lab.bin", "home/Administrator/Desktop/lab.bin"],
        "apps/notepad/app.bin": ["notepad.bin", "bin/notepad.bin", "home/Administrator/Desktop/notepad.bin"],
        "apps/studio/app.bin": ["studio.bin", "bin/studio.bin", "home/Administrator/Desktop/studio.bin"],
        "apps/tests/app.bin": ["tests.bin", "bin/tests.bin", "home/Administrator/Desktop/tests.bin"],
    }

    print("Copying application binaries to ramdisk...")
    for src, targets in apps_map.items():
        if os.path.exists(src):
            for tgt in targets:
                run_cmd(["mcopy", "-o", "-i", out_file, src, f"::/{tgt}"])
        else:
            print(f"Warning: application source not found: {src}")

    # Copy wallpapers from /tmp/file_attachments/
    wallpapers = {
        "pawel-czerwinski-1A_dO4TFKgM-unsplash.jpg": "pawel-czerwinski.jpg",
        "sebastian-svenson-d2w-_1LJioQ-unsplash.jpg": "sebastian-svenson.jpg",
        "anders-jilden-cYrMQA7a3Wc-unsplash.jpg": "anders-jilden.jpg",
        "cubes.png": "cubes.png",
        "glassy.png": "glassy.png",
        "1.png": "1.png",
    }

    print("Copying wallpaper assets to ramdisk...")
    attachments_dir = "/tmp/file_attachments"
    for src_name, tgt_name in wallpapers.items():
        src_path = os.path.join(attachments_dir, src_name)
        if os.path.exists(src_path):
            run_cmd(["mcopy", "-o", "-i", out_file, src_path, f"::/system/wallpapers/{tgt_name}"])
        else:
            print(f"Warning: wallpaper asset not found at: {src_path}")

    # Write /etc/passwd with baseline Administrator credentials
    print("Creating baseline /etc/passwd on ramdisk...")
    passwd_path = "passwd_temp.txt"
    with open(passwd_path, "w") as f:
        f.write("Administrator:password\n")
    run_cmd(["mcopy", "-o", "-i", out_file, passwd_path, "::/etc/passwd"])
    os.remove(passwd_path)

    # Write initial registry setting defaults
    print("Creating initial /registry/registry.conf on ramdisk...")
    reg_path = "registry_temp.txt"
    with open(reg_path, "w") as f:
        f.write("SESSION/CurrentUser=Administrator\n")
        f.write("USERS/Administrator/Role=Administrator\n")
        f.write("HKCU\\ControlPanel\\Desktop\\Wallpaper=/system/wallpapers/pawel-czerwinski.jpg\n")
    run_cmd(["mcopy", "-o", "-i", out_file, reg_path, "::/registry/registry.conf"])
    os.remove(reg_path)

    print(f"Successfully generated populated ramdisk: {out_file}")

if __name__ == "__main__":
    main()
