# 🚀 git-ez

> **The Cozy, Ultra-Lightweight Git & GitHub Launcher in Pure C**  
> *Native Windows GUI • Zero Terminal Windows • ~71 KB Executable*

`git-ez` is a fast, lightweight, and native Windows GUI application written in pure standard C (C99) using the Win32 API. It completely automates initializing local projects, staging files, generating `.gitignore` templates, creating commits, authenticating with GitHub, and publishing secure **Private** (or Public) repositories in seconds — all without opening or flashing any terminal windows.

---

## ✨ Key Features

- 🔒 **Private by Default**: Protects your code automatically by defaulting to private repository creation.
- 🖥️ **Native Win32 GUI**: Clean, modern desktop interface with Segoe UI typography, Common Controls v6 visual styling, and Windows dark mode support (`DWMWA_USE_IMMERSIVE_DARK_MODE`).
- 🔕 **Zero Terminal Windows**: Runs `git` and `gh` operations silently in the background using native Win32 `CreateProcess` with `CREATE_NO_WINDOW` and pipe redirection. No black command prompts flash or pop up.
- ⚡ **Ultra-Lightweight (~71 KB)**: Compiled in pure C directly against Windows system libraries — no Electron, Qt, .NET, or heavy runtime bloat. Instant startup (<10 ms).
- 📁 **Robust Folder Picker**: Pick any directory on any drive using the native Windows folder browser ("Browse..." button) or type/paste paths directly. Rock-solid path normalization handles root drives (`C:\`), trailing slashes, quotes, and paths with spaces seamlessly.
- 🧠 **Smart Context Detection**:
  - **New Project Mode**: Scans project files, weighs the folder, detects the programming language (C/C++, Python, Node/Web, Rust, Go, General), auto-generates `.gitignore`, suggests a clean repository name, and publishes directly to GitHub.
  - **Sync & Push Mode**: Automatically detects existing Git repositories, displays the active branch, tallies staged/modified/untracked files, and allows one-click commit and push.
- 📋 **Automatic Clipboard & Browser Integration**: Copies the live repository URL straight to your clipboard and opens the repo in your default web browser upon completion.
- 🧵 **Non-Blocking Background Worker**: Multi-threaded execution (`CreateThread`) keeps the interface responsive while streaming real-time status and logs into the activity window.

---

## 🎮 How to Use

### 1. Launch `git-ez`
Double-click `bin/git-ez.exe` in Windows Explorer, or launch it from PowerShell / CMD:
```powershell
.\bin\git-ez.exe
```

### 2. Choose Your Project Folder
- Click **"Browse..."** to select your project folder with the native Windows folder dialog, or type/paste the directory path into the folder box.
- `git-ez` will immediately inspect the directory:
  - If it's a **new folder**, it scans files, detects your tech stack, auto-fills a clean repository name, and selects the matching `.gitignore` recipe.
  - If it's an **existing Git repository**, it switches to **Sync & Push** mode and reports your branch name and uncommitted changes.

### 3. Review Settings & Take Off!
- Choose **Private (Safe)** or **Public**.
- Customize your commit message (default: `"Initial commit"` or `"Update project files"`).
- Click **"🚀 Publish to GitHub"** (or **"✨ Sync & Push to GitHub"**).
- Watch live progress in the status log. When finished, your repository URL is automatically copied to your clipboard and opened in your web browser!

---

## 🛠️ CLI Options

You can also launch `git-ez` pre-targeted at a specific folder or view version info:

```text
USAGE:
  git-ez.exe [OPTIONS]

OPTIONS:
  -d, --dir <path>       Pre-load target project directory
  -n, --name <name>      Pre-fill GitHub repository name
  -v, --version          Show version information
  -h, --help             Show command-line help
```

*Example:*
```cmd
bin\git-ez.exe -d "C:\Users\Username\Projects\my-cool-app"
```

---

## 📦 Building from Source

### Prerequisites
A compatible C99 compiler on Windows:
- **GCC** (MinGW-w64 / w64devkit) *(Recommended)*
- **Zig** (`zig cc`)
- **Clang** / LLVM
- **MSVC** (`cl.exe`)

### Build with `build.bat`
Run the provided automated build script:
```cmd
build.bat
```
The script compiles the Windows manifest resource and builds an optimized, stripped executable in `bin\git-ez.exe` (~71 KB).

### Build with `make`
```cmd
make
```

### Build with CMake
```cmd
mkdir build && cd build
cmake ..
cmake --build . --config Release
```

---

## 📄 License
[MIT License](https://opensource.org/license/mit).  
Free to use, modify, and distribute.

---

## 📋 Notes
- The source code comments are whimsical, crafted with care, caffeine, and mild confusion.
- Built with pure Win32 API for the smallest executable size, instant launch speed, and zero external runtime dependencies.
- Includes [HowTo - Git CLI Instructions.md](HowTo%20-%20Git%20CLI%20Instructions.md) for handy CLI reference.