<p align="left">
  <img width="100" alt="icon" src="https://github.com/user-attachments/assets/ebfb3248-54cf-4f95-bab2-e51b68f795ba"/><br>
  <h1>Phoca, a yt-dlp Qt6 frontend</h1>
</p>

A minimalistic program to download video and audio from most social media platforms.

## 📥 Download
AppImage binaries and Windows installer are available on [Releases](https://github.com/maxipardo/Phoca/releases).

## 📦 Compiling from source code
**Requirements (Ubuntu/Debian):**
```bash
sudo apt update
sudo apt install build-essential cmake qt6-base-dev ffmpeg
curl -fsSL https://deno.land/install.sh | sh
```
**Compilación:**
```bash
git clone https://github.com/maxipardo/Phoca.git
cd Phoca
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc)
```

## 📸 Screenshots ##
<table>
  <tr>
    <td valign="center">
      <img src="https://github.com/user-attachments/assets/199f649d-2fca-4640-bbfe-81ad0d2c23fc" width="550" />
    </td>
    <td valign="center">
      <img src="https://github.com/user-attachments/assets/68b6bacf-1897-4fed-a2d2-45908bef9f1b" width="550" />
    </td>
  </tr>
<tr>
  <td style="vertical-align: middle; text-align: center;">
    <p align="center">
      <img src="https://github.com/user-attachments/assets/c86d8f9f-7fa5-4bee-8245-e01c3e564c40" width="350" />
    </p>
  </td>
  <td style="vertical-align: middle; text-align: center;">
    <p align="center">
      <img src="https://github.com/user-attachments/assets/e83062ec-5f9b-4de9-94fb-60d511c6993d" width="350" />
    </p>
  </td>
</tr>
</table>
