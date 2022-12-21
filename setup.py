import zipfile
import urllib.request
import os
import sys
import subprocess
import tarfile
import hashlib

def install(what):
    subprocess.check_call([sys.executable, '-m', 'pip', 'install', what])

if not os.path.exists("odyssey.nso"):
    print("odyssey.nso does not exist. Place a Super Mario Odyssey v1.0.0 NSO named odyssey.nso in the root.")
    sys.exit(1)
else:
    print("Found odyssey.nso, checking hash...")

    with open("odyssey.nso", "rb") as f:
        b = f.read()
        if hashlib.sha256(b).hexdigest().upper() == "E21692D90F8FD2DEF2D2D22D983D62AC81DF3B8B3C762D1F2DCA9D9AB7B3053A":
            print("Hash is correct, continuing...")
        else:
            print("Hash is NOT correct. All checks will fail!")

try:
    import capstone
    print("Found capstone, continuing...")
except ModuleNotFoundError:
    print("Module 'capstone' not installed. Installing...")
    install("capstone")

try:
    import elftools
    print("Found elftools, continuing...")
except ModuleNotFoundError:
    print("Module `elftools` not found. Installing...")
    install("pyelftools")

try:
    import lz4
    print("Found lz4, continuing...")
except ModuleNotFoundError:
    print("Module `lz4` not found. Installing...")
    install("lz4")

try:
    import ninja
    print("Found ninja, continuing...")
except ModuleNotFoundError:
    print("Module 'ninja' not found. Installing...")
    install("ninja")

try:
    import colorama
    print("Found colorama, continuing...")
except ModuleNotFoundError:
    print("Module 'colorama' not found. Installing...")
    install("colorama")



if not os.path.exists("tools"):
    print("Making tools directory...")
    os.mkdir("tools")

if not os.path.exists("tools/clang"):
    print("Downloading Clang 3.9.1...")

    with urllib.request.urlopen("https://releases.llvm.org/3.9.1/clang+llvm-3.9.1-x86_64-linux-gnu-debian8.tar.xz") as response, open("compiler.tar.xz", "wb") as out:
        data = response.read()
        out.write(data)

    print("Extracting files...")
    ex = tarfile.open("compiler.tar.xz")
    ex.extractall("tools/")

    os.rename("tools/clang+llvm-3.9.1-x86_64-linux-gnu-debian8", "tools/clang")
else:
    print("Clang already exist, continuing...")



if os.path.exists("compiler.tar.xz"):
    print("Deleting temp files...")
    os.remove("compiler.tar.xz")

print("Done.")
sys.exit(1)