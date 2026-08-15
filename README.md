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
      <img src="https://github.com/user-attachments/assets/26890c36-f6ce-447d-b7a8-05524412e9b9" width="400" />
    </td>
    <td style="vertical-align: middle; text-align: center;">
      <img src="https://github.com/user-attachments/assets/2e00a79a-792c-4794-94ca-b98e316c6c82" width="400" />
    </td>
  </tr>
</table>
