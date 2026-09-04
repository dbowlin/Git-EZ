# 📘 Complete Step-by-Step Guide: Upload a Private Repository to GitHub Using GitHub CLI

## Before You Begin

### 🪟 Windows Installation
- Ensure you have the GitHub CLI installed. In an Administrator terminal, run:
  ```powershell
  winget install GitHub.cli
  ```
- Close and reopen your terminal after installation.

### 🐧 Linux Installation
Install `git` and `gh` using your distribution's package manager:

- **Debian / Ubuntu / Linux Mint / WSL:**
  ```bash
  sudo apt update
  sudo apt install gh git -y
  ```
- **Fedora / RHEL / CentOS:**
  ```bash
  sudo dnf install gh git -y
  ```
- **Arch Linux / Manjaro:**
  ```bash
  sudo pacman -S github-cli git --noconfirm
  ```
- **openSUSE:**
  ```bash
  sudo zypper install gh git
  ```

---

## Step 1: Navigate to Your Project Folder
- Open your terminal.
- Navigate to your project folder:
  - **Windows (PowerShell/CMD):**
    ```powershell
    cd "C:\path\to\your\project-folder"
    ```
  - **Linux / macOS (Bash/Zsh):**
    ```bash
    cd "/path/to/your/project-folder"
    # or if in your home directory:
    cd ~/path/to/your/project-folder
    ```
  *(Replace the path with the actual location of your project folder)*
- **Tip:** You can type `cd ` followed by a space, then drag and drop your folder from your file manager into the terminal window to auto-fill the path.
- Verify you're in the right folder:
  - **Windows:** `dir`
  - **Linux / macOS:** `ls -la`

---

## Step 2: Initialize Git in Your Folder
- Type this command to create a new Git repository:
  ```bash
  git init
  ```
- You should see: `Initialized empty Git repository in /path/to/your/project-folder/.git/`
- **What this does:** Creates a hidden `.git` folder that tracks all your changes.

---

## Step 3: Stage All Your Files
- Type this command to prepare all files for the commit:
  ```bash
  git add .
  ```
- **What this does:** The dot `.` means "everything in this folder" — it stages all files so they are ready to be saved.
- **No output** usually means it worked successfully.

---

## Step 4: Commit Your Files
- Type this command to save your files with a descriptive message:
  ```bash
  git commit -m "Initial commit"
  ```
- You'll see output showing how many files were changed and inserted.
- **What this does:** Takes a permanent "snapshot" of your files at this moment.

---

## Step 5: Authenticate with GitHub (One-Time Setup)
- If you haven't logged in yet, type:
  ```bash
  gh auth login
  ```
- When prompted:
  - **What account do you want to log into?** → Select `GitHub.com` (use arrow keys, press `Enter`)
  - **What is your preferred protocol for Git operations?** → Select `HTTPS` (or `SSH` if you have configured SSH keys)
  - **Authenticate Git with your GitHub credentials?** → Select `Yes`
  - **How would you like to authenticate?** → Select `Login with a web browser` (or `Paste an authentication token` on headless Linux servers)
- The terminal will display a one-time code (e.g., `ABCD-1234`).
- Press `Enter` to open a browser window (or manually open https://github.com/login/device if on a headless server without a desktop GUI).
- In the browser:
  - Enter the code from your terminal.
  - Click **Authorize github**.
- Return to your terminal — you'll see a success message: `Authentication complete`.

---

## Step 6: Create a Private GitHub Repository and Push Your Code
- Type this command using the `--private` flag:
  ```bash
  gh repo create <repository-name> --private --source=. --push
  ```
  *(Replace `<repository-name>` with your desired repository name, e.g., `my-project`)*

- **What this command does:**
  - `gh repo create <repository-name>`: Creates a new repository on GitHub under your account.
  - `--private`: Ensures the repository is **Private** (only you and invited collaborators can view or access it).
  - `--source=.`: Specifies the current directory as the source of the repository.
  - `--push`: Automatically pushes your local committed files and branches to GitHub.

- You will see output indicating that the private repository was created and files were pushed:
  ```text
  ✓ Created repository <your-username>/<repository-name> on GitHub
  ✓ Added remote https://github.com/<your-username>/<repository-name>.git
  ✓ Pushed commits to https://github.com/<your-username>/<repository-name>.git
  ```

---

## Step 7: Verify Your Upload
- Open your browser.
- Go to: `https://github.com/<your-username>/<repository-name>`
- Confirm all your files are visible and that the repository has the **Private** badge next to the repository title.
- *(Replace `<your-username>` with your GitHub username and `<repository-name>` with your repository name)*

---

## Step 8: How to Update Your Repository in the Future
After making changes to your files locally:

1. Navigate to your project folder (if not already there):
   - **Windows:** `cd "C:\path\to\your\project-folder"`
   - **Linux / macOS:** `cd "/path/to/your/project-folder"`
2. Stage all changes:
   ```bash
   git add .
   ```
3. Commit with a description:
   ```bash
   git commit -m "Description of what you changed"
   ```
4. Push to GitHub:
   ```bash
   git push
   ```

---

## 🛠️ Common Troubleshooting

| Problem | Solution |
| :--- | :--- |
| `'gh' is not recognized / command not found` | Install GitHub CLI using `winget` (Windows) or `sudo apt / dnf / pacman install gh` (Linux) |
| `'git' is not recognized / command not found` | Install Git from [git-scm.com](https://git-scm.com) or via package manager |
| `current directory is not a git repository` | Run `git init` (Step 2) |
| `authentication failed` | Run `gh auth login` again (Step 5) |
| `fatal: remote origin already exists` | Run `git remote remove origin` then repeat Step 6 |
| `Repo was created as public accidentally` | Run `gh repo edit <your-username>/<repository-name> --visibility private` |
| `No browser available on headless Linux server` | In `gh auth login`, choose token auth or visit `https://github.com/login/device` on another machine |

---

## ✅ Quick Reference

### 🪟 Windows

#### Step-by-Step Commands
```powershell
cd "C:\path\to\your\project-folder"
git init
git add .
git commit -m "Initial commit"
gh repo create <repository-name> --private --source=. --push
```

#### ⚡ All-in-One Command (PowerShell)
```powershell
git init && git add . && git commit -m "Initial commit" && gh repo create <repository-name> --private --source=. --push
```

---

### 🐧 Linux / macOS

#### Step-by-Step Commands
```bash
cd "/path/to/your/project-folder"
git init
git add .
git commit -m "Initial commit"
gh repo create <repository-name> --private --source=. --push
```

#### ⚡ All-in-One Command (Bash / Zsh)
```bash
git init && git add . && git commit -m "Initial commit" && gh repo create <repository-name> --private --source=. --push
```

> **Note (Navigate & Run from anywhere in one line):**
> ```bash
> cd "/path/to/your/project-folder" && git init && git add . && git commit -m "Initial commit" && gh repo create <repository-name> --private --source=. --push
> ```

---

**That's it! Your private repository is now live and secure on GitHub. 🔒🎉**