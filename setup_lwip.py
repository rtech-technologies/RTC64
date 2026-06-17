import urllib.request
import os
import tarfile

url = "https://download.savannah.gnu.org/releases/lwip/lwip-2.1.3.zip"
path = "lwip.zip"

try:
    print(f"Downloading {url}...")
    urllib.request.urlretrieve(url, path)
    print("Download complete.")
except Exception as e:
    print(f"Error: {e}")
