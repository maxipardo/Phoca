<p align="left">
  <img width="100" alt="icon" src="https://github.com/user-attachments/assets/82f886fd-2ac8-4ba4-9e9e-5b1aa35bca84"/><br>
  <h1>Phoca</h1>
</p>

**Video and Audio Downloader for Desktop**

Download multiple videos and audio simultaneously, convert files to your preferred formats, and easily download entire playlists. Powered by yt-dlp, Phoca provides a clean, lightweight experience that gets the job done.

## 📥 Download
GNU/Linux and Windows binaries are available on [Releases](https://github.com/maxipardo/Phoca/releases).

## 📦 Compiling from source code
**Requirements (Ubuntu/Debian):**
```bash
sudo apt update
sudo apt install build-essential cmake qt6-base-dev ffmpeg
curl -fsSL https://deno.land/install.sh | sh
```
**Compiling:**
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
      <img src="https://github.com/user-attachments/assets/c7b86c81-7272-4d9f-924f-0effe6412408" width="550" />
    </td>
    <td valign="center">
      <img src="https://github.com/user-attachments/assets/17d0f12c-10e0-442f-a6cc-078c57daf98d" width="550" />
    </td>
  </tr>
<tr>
  <td style="vertical-align: middle; text-align: center;">
    <p align="center">
      <img src="https://github.com/user-attachments/assets/7cb6eb72-7e8b-4eb3-80ac-c7ab884d45ab" width="550" />
    </p>
  </td>
  <td style="vertical-align: middle; text-align: center;">
    <p align="center">
      <img src="https://github.com/user-attachments/assets/b45043da-06e2-40e0-b447-25d8e359538e" width="550" />
    </p>
  </td>
</tr>
</table>
