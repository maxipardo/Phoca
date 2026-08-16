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
      <img src="https://github.com/user-attachments/assets/1b9aa9a4-9a5b-4e9f-8c2e-227e2f6e7157" width="550" />
    </td>
    <td valign="center">
      <img src="https://github.com/user-attachments/assets/4962f1b5-1dfc-4be3-8140-5d83ba8c24b4" width="550" />
    </td>
  </tr>
<tr>
  <td style="vertical-align: middle; text-align: center;">
    <p align="center">
      <img src="https://github.com/user-attachments/assets/0e7692b9-3be2-4483-9259-c3ddb239dd10" width="550" />
    </p>
  </td>
  <td style="vertical-align: middle; text-align: center;">
    <p align="center">
      <img src="https://github.com/user-attachments/assets/76ab05bd-b78e-45f4-ac17-eb201b05d547" width="550" />
    </p>
  </td>
</tr>
</table>
