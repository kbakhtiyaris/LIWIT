# LIWIT - Linux-Windows Text Editor

You can use it simply by cloning the repo, installing the required dependencies, follow the steps below to get it running in no time. if you face any problems feel free to contact.

**Making Linux terminal editing as easy as Windows Notepad!**

[![License](https://img.shields.io/badge/license-GPL--3.0-blue.svg)](LICENSE)
[![Platform](https://img.shields.io/badge/platform-Linux-orange.svg)]()
[![Language](https://img.shields.io/badge/language-C-green.svg)]()

## What is LIWIT?

LIWIT (Linux-Windows Text Editor) is a beginner-friendly terminal text editor designed for people transitioning from Windows to Linux. If you find vim/emacs intimidating but need a powerful terminal editor, LIWIT is for you!

### Why LIWIT?

- ✅ **Familiar shortcuts**: Ctrl+S to save, Ctrl+O to open, Ctrl+Q to quit, Ctrl+x to cut,
                             Ctrl+c to copy, Ctrl+v to paste
- ✅ **Visual interface**: Menu bar showing available shortcuts, not all of them
- ✅ **No learning curve**: Works like Windows Notepad but in the terminal
- ✅ **Lightweight**: Pure C with ncurses - fast and minimal
- ✅ **Beginner-friendly**: Perfect for Linux newcomers

## Quick Start

### Installation

```bash
# Clone the repository
git clone https://github.com/kbakhtiyaris/LIWIT.git
cd LIWIT

# Install dependencies (Ubuntu/Debian)
sudo apt-get install libncurses6-dev libncursesw6-dev

# Build
make

# OR if you want to  Install System-Wide
sudo make install
# If you face any problem in installing it system-wide follow the complete steps below

# Run
./liwit

# OR Run(if you installed it system-wide)
liwit
```

### First Time Use

```bash
# Create a new file
./liwit

# Open an existing file
./liwit myfile.txt

# The interface shows all shortcuts - no memorization needed!
```

## ⌨️ Keyboard Shortcuts

All the shortcuts you already know from Windows:

| Shortcut | Action | Windows Equivalent |
|----------|--------|-------------------|
| **Ctrl+S** | Save file | Same |
| **Ctrl+O** | Open file | Same |
| **Ctrl+Q** | Quit editor | Alt+F4 |
| **Ctrl+X** | Copy line | Same |
| **Ctrl+C** | Cut line | Same |
| **Ctrl+V** | Paste | Same |
| **Arrow Keys** | Move cursor | Same |
| **Home** | Line start | Same |
| **End** | Line end | Same |
| **Page Up/Down** | Scroll | Same |
| **Insert** | Toggle insert/overwrite | Same |
| **Backspace** | Delete back | Same |
| **Delete** | Delete forward | Same |
| **Enter** | New line | Same |
| **f2** | Start selection | Not same | (for this version, will be updated in future version)

## Features

### Current Features (v1.0)
- ✅ Windows-like keyboard shortcuts
- ✅ Visual menu bar with shortcuts displayed
- ✅ Status bar with file info and cursor position
- ✅ Line numbers
- ✅ Color-coded interface
- ✅ Insert/Overwrite mode toggle (limited for this version)
- ✅ Smooth scrolling (vertical and horizontal)
- ✅ Copy/Paste/Cut support
- ✅ File save/open with prompts
- ✅ Modified file indicator
- ✅ Tab support (converts to spaces)

### Planned Features (Future)
- 🔜 Undo/Redo (Ctrl+Z, Ctrl+Y)
- 🔜 Find/Replace (Ctrl+F, Ctrl+H)
- 🔜 Syntax highlighting
- 🔜 Multiple file tabs
- 🔜 Configuration file
- 🔜 Auto-save
- 🔜 Mouse support

## 📖 Usage Guide

### Creating a New File

```bash
./liwit
# Type your content
# Press Ctrl+S to save
# Enter filename when prompted
```

### Editing an Existing File

```bash
./liwit document.txt
# Make changes
# Press Ctrl+S to save
# Changes are automatically tracked
```

### Navigation Tips

- **Arrow keys**: Basic cursor movement
- **Home/End**: Jump to line start/end
- **Page Up/Down**: Fast scrolling
- **Ctrl+Home**: Jump to file start (planned)
- **Ctrl+End**: Jump to file end (planned)

### Copy/Paste

```bash
# Position cursor on line to copy
# Press Ctrl+C
# Move to destination
# Press Ctrl+V
```

## Building from Source

### Prerequisites

```bash
# Ubuntu/Debian
sudo apt-get update
sudo apt-get install build-essential libncurses6-dev

# Fedora/RHEL
sudo dnf install gcc ncurses-devel

# Arch Linux
sudo pacman -S base-devel ncurses
```

### Compile

```bash
# Standard build
make

# Debug build (with symbols)
make debug

# Create .deb package
make deb
```

### Install System-Wide

```bash
# Install to /usr/local/bin
sudo make install

# Now you can run from anywhere
liwit myfile.txt

# Uninstall
sudo make uninstall
```

## 📦 Package Installation

### Debian/Ubuntu (.deb)

```bash
# Download the .deb package or build it
make deb

# Install
sudo dpkg -i liwit_1.0_amd64.deb

# If dependencies missing
sudo apt-get install -f
```

### From PPA (Coming Soon)

```bash
```bash
~~sudo add-apt-repository ppa:kbakhtiyaris/liwit~~
~~sudo apt-get update~~
~~sudo apt-get install liwit~~
```
sudo apt-get update
sudo apt-get install liwit
```

## 🎓 For Students & Beginners

LIWIT is perfect if you:
- 🎓 Are learning Linux for the first time
- 💻 Need to edit config files via SSH
- 📝 Want a simple editor without vim/emacs complexity
- 🔧 Are working on embedded systems (Raspberry Pi, etc.)
- 🚀 Want something faster than nano with better UX

## 🏗️ Project Structure

```
LIWIT/
├── liwit.c              # Main source code (well-commented!)
├── Makefile             # Build configuration
├── README.md            # This file
├── LICENSE              # GPL-3.0 License
├── .gitignore          # Git ignore rules
```

## 🤝 Contributing

Contributions are welcome! This is a learning project, so beginner-friendly PRs are especially appreciated.

### How to Contribute

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Make your changes
4. Test thoroughly
5. Commit (`git commit -m 'Add amazing feature'`)
6. Push to branch (`git push origin feature/amazing-feature`)
7. Open a Pull Request

### Good First Issues

- Add Ctrl+Z undo functionality
- Implement Ctrl+F find feature
- Add line wrap toggle
- Create man page
- Add more color themes

## 🐛 Bug Reports

Found a bug? Please open an issue with:
- Operating system and version
- Terminal emulator used
- Steps to reproduce
- Expected vs actual behavior

## 📝 License

LIWIT is licensed under the **GNU GPL v3.0 or later**.  
See the [LICENSE](LICENSE) file for details.

## 👨‍💻 Author

**Khud Bakhtiyar Iqbal Sofi**
- 🎓 Mechatronics Engineering Student @ Istanbul Gedik University, Turkey
- 🔧 Passionate about embedded systems and user-friendly software
- 💼 [GitHub](https://github.com/kbakhtiyaris)

## 🙏 Acknowledgments

- Built with [ncurses](https://invisible-island.net/ncurses/) library
- Inspired by nano's simplicity and Windows Notepad's familiarity
- Thanks to everyone who helped test and provide feedback

## 📊 Project Stats

- **Language**: C (100%)
- **Lines of Code**: ~800
- **Dependencies**: ncurses
- **Platforms**: Linux (Ubuntu, Debian, Fedora, Arch)

## 🎯 Roadmap

### Version 1.0 (Current)
- ✅ Basic editing
- ✅ File operations
- ✅ Windows shortcuts
- ✅ Visual interface

### Version 1.1 (Next)
- 🔜 Undo/Redo
- 🔜 Find/Replace
- 🔜 Better error messages

### Version 2.0 (Future)
- 🔜 Syntax highlighting
- 🔜 Multiple tabs
- 🔜 Plugin system

## 💬 Community

- **Issues**: Report bugs or request features
- **Discussions**: Ask questions or share ideas
- **Wiki**: Documentation and tutorials (coming soon)

## ⭐ Show Your Support

If LIWIT helped you, please consider:
- ⭐ Starring the repository
- 🐛 Reporting bugs
- 💡 Suggesting features
- 🤝 Contributing code
- 📢 Sharing with friends

---

**Made with ❤️ for Linux beginners everywhere!**

*"Because everyone deserves a text editor they can actually use."*
