# Makefile for LIWIT - Linux-Windows Text Editor

# Compiler settings
CC = gcc
CFLAGS = -Wall -g
#gcc liwit.c -o liwit -lncurses -Wall -g
#$(CC) $(SOURCES) -o $(TARGET) $(LDFLAGS) $(CFLAGS)
LDFLAGS = -lncurses

# Target executable
TARGET = liwit

# Source files
SOURCES = liwit.c

# Installation directories
PREFIX ?= /usr/local
BINDIR = $(PREFIX)/bin
MANDIR = $(PREFIX)/share/man/man1

# ============================================================================
# Build Targets
# ============================================================================

# Default target - build the editor
all: $(TARGET)
	@echo "========================================="
	@echo "LIWIT built successfully!"
	@echo "Run with: ./$(TARGET)"
	@echo "Or try: ./$(TARGET) test.txt"
	@echo "========================================="

# Build the executable
$(TARGET): $(SOURCES)
	@echo "Compiling LIWIT..."
#before	$(CC) $(CFLAGS) $(SOURCES) -o $(TARGET) $(LDFLAGS)
	$(CC) $(SOURCES) -o $(TARGET) $(LDFLAGS) $(CFLAGS)
# Debug build with symbols
debug: CFLAGS += -g -DDEBUG
debug: clean $(TARGET)
	@echo "Debug build complete. Run with: gdb ./$(TARGET)"

# ============================================================================
# Installation
# ============================================================================

# Install system-wide
install: $(TARGET)
	@echo "Installing LIWIT to $(BINDIR)..."
	install -d "$(BINDIR)"
	install -m 0755 "$(TARGET)" "$(BINDIR)/$(TARGET)"
	@echo "Installation complete!"
	@echo "You can now run 'liwit' from anywhere."

# Uninstall from system
uninstall:
	@echo "Uninstalling LIWIT..."
	rm -f "$(BINDIR)/$(TARGET)"
	@echo "Uninstall complete."

# ============================================================================
# Debian/Ubuntu Package
# ============================================================================

# Create .deb package for Ubuntu/Debian
deb: $(TARGET)
	@echo "Creating Debian package..."
	# Create package structure
	mkdir -p debian-pkg/DEBIAN
	mkdir -p debian-pkg/usr/bin
	mkdir -p debian-pkg/usr/share/doc/liwit
	mkdir -p debian-pkg/usr/share/applications

	# Copy executable
	cp "$(TARGET)" debian-pkg/usr/bin/

	# Create control file
	@echo "Package: liwit" > debian-pkg/DEBIAN/control
	@echo "Version: 1.0" >> debian-pkg/DEBIAN/control
	@echo "Section: editors" >> debian-pkg/DEBIAN/control
	@echo "Priority: optional" >> debian-pkg/DEBIAN/control
	@echo "Architecture: amd64" >> debian-pkg/DEBIAN/control
	@echo "Depends: libncursesw6 | libncurses6" >> debian-pkg/DEBIAN/control
	@echo "Maintainer: Khud Bakhtiyar Iqbal Sofi <your-email@example.com>" >> debian-pkg/DEBIAN/control
	@echo "Description: Linux-Windows Text Editor" >> debian-pkg/DEBIAN/control
	@echo " A beginner-friendly terminal text editor with familiar Windows shortcuts." >> debian-pkg/DEBIAN/control
	@echo " Features Ctrl+S/O/Q, visual interface, line numbers, and more." >> debian-pkg/DEBIAN/control

	# Copyright
	@echo "Format: https://www.debian.org/doc/packaging-manuals/copyright-format/1.0/" > debian-pkg/usr/share/doc/liwit/copyright
	@echo "Upstream-Name: liwit" >> debian-pkg/usr/share/doc/liwit/copyright
	@echo "Source: https://github.com/kbakhtiyaris/LIWIT" >> debian-pkg/usr/share/doc/liwit/copyright
	@echo "" >> debian-pkg/usr/share/doc/liwit/copyright
	@echo "Files: *" >> debian-pkg/usr/share/doc/liwit/copyright
	@echo "Copyright: 2026 Khud Bakhtiyar Iqbal Sofi" >> debian-pkg/usr/share/doc/liwit/copyright
	@echo "License: GPL-3.0+" >> debian-pkg/usr/share/doc/liwit/copyright

	# Build package
	dpkg-deb --build debian-pkg liwit_1.0_amd64.deb

	@echo "========================================="
	@echo "Package created: liwit_1.0_amd64.deb"
	@echo "Install with: sudo dpkg -i liwit_1.0_amd64.deb"
	@echo "========================================="

# ============================================================================
# Testing and Development
# ============================================================================

# Create test file for testing the editor
test-file:
	@echo "Creating test file..."
	@echo "Hello from LIWIT!" > test.txt
	@echo "This is line 2." >> test.txt
	@echo "Line 3 with more text." >> test.txt
	@echo "Test file created: test.txt"

# Run the editor with test file
run: $(TARGET) test-file
	./$(TARGET) test.txt

# Check for memory leaks
valgrind: debug test-file
	valgrind --leak-check=full --show-leak-kinds=all ./$(TARGET) test.txt

# ============================================================================
# Cleanup
# ============================================================================

# Clean build artifacts
clean:
	@echo "Cleaning build files..."
	rm -f $(TARGET)
	rm -f *.o
	rm -f core
	rm -rf debian-pkg
	rm -f liwit_*.deb
	@echo "Clean complete."

# Clean everything including test files
cleanall: clean
	rm -f test.txt
	rm -f *.txt~
	@echo "All files cleaned."

# ============================================================================
# Help
# ============================================================================

help:
	@echo "LIWIT - Linux-Windows Text Editor Makefile"
	@echo ""
	@echo "Available targets:"
	@echo "  make              - Build LIWIT"
	@echo "  make debug        - Build with debug symbols"
	@echo "  make install      - Install system-wide"
	@echo "  make uninstall    - Remove from system"
	@echo "  make deb          - Create .deb package"
	@echo "  make test-file    - Create test.txt for testing"
	@echo "  make run          - Build and run with test file"
	@echo "  make valgrind     - Check for memory leaks"
	@echo "  make clean        - Remove build files"
	@echo "  make cleanall     - Remove all generated files"
	@echo "  make help         - Show this help"
	@echo ""
	@echo "Usage examples:"
	@echo "  make && ./liwit"
	@echo "  make && ./liwit myfile.txt"
	@echo "  make deb && sudo dpkg -i liwit_1.0_amd64.deb"

# Mark targets that don't produce files
.PHONY: all debug install uninstall deb test-file run valgrind clean cleanall help
