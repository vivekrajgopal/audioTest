# AudioDeviceSwitcher CI Builder

This repo builds the Windows DLL via **GitHub Actions** (no local compiler needed).

## Use
1. Create a new **private GitHub repo** and upload these files.
2. In GitHub → **Actions** → enable workflows.
3. Run the manual workflow: **Build AudioDeviceSwitcher DLL**.
4. When it finishes, download the artifact **AudioDeviceSwitcher_x64_Release** → it contains `AudioDeviceSwitcher.dll`.
5. Copy the DLL to your Unity project:
   `Assets/Plugins/x86_64/Windows/AudioDeviceSwitcher.dll`

Works with Windows 10/11.

If you still prefer local build:
- Open `AudioDeviceSwitcher.sln` in Visual Studio 2022 (with Desktop development with C++ installed), choose **Release|x64**, Build → you’ll get the DLL in `x64/Release/`.
