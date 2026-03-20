# PGViewer — PostgreSQL Database Viewer & Editor

A lightweight, production-quality PostgreSQL GUI built with **C++17** and **Qt6**
(Qt5 compatible). Designed as a clean, fast alternative to DBeaver/pgAdmin for
everyday database work.

---

## Features

| Category          | Details |
|-------------------|---------|
| **Authentication** | Login dialog, credential persistence in JSON, auto-fill on startup |
| **Theme**          | Light & Dark mode, toggle at runtime (Ctrl+Shift+D) |
| **Explorer**       | Schema tree: Schemas → Tables / Views, live search, refresh |
| **Table Viewer**   | Paginated grid, column sort, column filter, inline cell editing |
| **CRUD**           | Insert rows, update cells (double-click), delete with confirmation |
| **Query Editor**   | Multi-line SQL editor, syntax highlighting, F5 to run, selection-run |
| **Results**        | Tabular results display, execution time, error details |
| **Export**         | Export current page to CSV |
| **Connection**     | Status indicator, 30-second ping, manual reconnect |
| **Error Handling** | No crashes — all DB errors surfaced as user messages |

---

## Requirements

| Tool | Version |
|------|---------|
| Qt   | 6.x (or Qt 5.15+) |
| CMake | 3.16+ |
| PostgreSQL dev headers | `libpq-dev` / `postgresql-devel` |
| C++ compiler | GCC 10+, Clang 12+, MSVC 2019+ (C++17) |

---

## Build Instructions

### Linux (Ubuntu / Debian)

```bash
# 1. Install dependencies
sudo apt update
sudo apt install -y \
    cmake build-essential \
    qt6-base-dev qt6-tools-dev \
    libpq-dev

# 2. Clone / extract the project
cd pgviewer

# 3. Configure & build
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)

# 4. Run
./pgviewer
```

### Linux (Fedora / RHEL)

```bash
sudo dnf install cmake gcc-c++ \
    qt6-qtbase-devel \
    libpq-devel
```

### Windows (MSVC + vcpkg)

```powershell
# Prerequisites:
#   Qt 6.x installed via Qt Installer at C:\Qt
#   PostgreSQL installed at C:\Program Files\PostgreSQL\16

cd pgviewer
mkdir build && cd build

cmake .. `
  -G "Visual Studio 17 2022" -A x64 `
  -DCMAKE_PREFIX_PATH="C:\Qt\6.7.0\msvc2019_64" `
  -DPostgreSQL_ROOT="C:\Program Files\PostgreSQL\16"

cmake --build . --config Release

# Copy Qt DLLs next to the .exe
windeployqt Release\pgviewer.exe
```

### macOS

```bash
brew install cmake qt@6 postgresql@16

cmake .. \
  -DCMAKE_PREFIX_PATH="$(brew --prefix qt@6)" \
  -DPostgreSQL_ROOT="$(brew --prefix postgresql@16)"

make -j$(sysctl -n hw.logicalcpu)
```

---

## Project Structure

```
pgviewer/
├── CMakeLists.txt          # Build system
├── README.md
├── sample_config.json      # Example config format
├── resources/
│   └── resources.qrc       # Qt resource file
└── src/
    ├── main.cpp            # Entry point
    │
    ├── core/               # Pure C++ domain layer (no UI)
    │   ├── DatabaseConnection.h/.cpp   # QSqlDatabase wrapper
    │   ├── ConfigManager.h/.cpp        # JSON config read/write
    │   └── QueryResult.h/.cpp          # Data transfer struct
    │
    ├── services/           # Business logic layer (no UI)
    │   ├── DatabaseService.h/.cpp      # Schema browsing + CRUD
    │   └── ExportService.h/.cpp        # CSV export
    │
    └── ui/                 # Qt Widgets layer
        ├── LoginDialog.h/.cpp          # Connection credentials dialog
        ├── MainWindow.h/.cpp           # Main application window
        ├── DatabaseExplorer.h/.cpp     # Left-panel schema tree
        ├── TableViewer.h/.cpp          # Paginated table grid + CRUD
        ├── QueryEditor.h/.cpp          # SQL editor + results
        ├── CellEditDelegate.h/.cpp     # Inline cell editor delegate
        ├── ThemeManager.h/.cpp         # Light/dark palette & stylesheet
        └── StatusBar.h/.cpp            # Custom status bar
```

---

## Architecture

```
┌─────────────────────────────────────────────────────────┐
│                    UI Layer (Qt Widgets)                 │
│  LoginDialog → MainWindow                               │
│    ├── DatabaseExplorer  (sidebar schema tree)          │
│    ├── TableViewer       (paginated grid + CRUD)        │
│    └── QueryEditor       (SQL editor + results)         │
├─────────────────────────────────────────────────────────┤
│                  Services Layer                         │
│  DatabaseService  (schema + CRUD queries)               │
│  ExportService    (CSV export)                          │
├─────────────────────────────────────────────────────────┤
│                   Core Layer                            │
│  DatabaseConnection  (Qt SQL / QPSQL driver)            │
│  ConfigManager       (JSON config, QStandardPaths)      │
│  QueryResult         (POD data transfer struct)         │
└─────────────────────────────────────────────────────────┘
```

**Design principles:**
- UI layer never writes SQL directly
- Service layer never includes Qt widget headers
- Smart pointers (`std::unique_ptr`) throughout — no manual `delete`
- `m_ignoreChanges` guard prevents feedback loops on `itemChanged`

---

## Config File Location

The config is written automatically on first successful login.

| Platform | Path |
|----------|------|
| Linux    | `~/.config/PGViewer/pgviewer_config.json` |
| Windows  | `%APPDATA%\PGViewer\pgviewer_config.json` |
| macOS    | `~/Library/Preferences/PGViewer/pgviewer_config.json` |

### Config Format

```json
{
    "connection": {
        "host": "localhost",
        "port": 5432,
        "database": "mydb",
        "username": "postgres",
        "password": "yourpassword"
    },
    "settings": {
        "theme": "light",
        "lastSchema": "public",
        "lastTable": "users"
    }
}
```

> **Security note:** Passwords are stored in plaintext. For production use,
> integrate with the platform keychain:
> `libsecret` (Linux), Windows Credential Manager, or macOS Keychain.

---

## Keyboard Shortcuts

| Shortcut | Action |
|----------|--------|
| `F5` | Refresh explorer / Run query |
| `Ctrl+R` | Reconnect to database |
| `Ctrl+Shift+D` | Toggle dark mode |
| `Ctrl+Q` | Quit |
| `Double-click cell` | Edit cell inline |
| `Enter` (filter box) | Apply filter |

---

## Troubleshooting

**"QPSQL driver not found"**
```bash
# Install Qt SQL PostgreSQL plugin
sudo apt install libqt6sql6-psql   # Qt6
sudo apt install libqt5sql5-psql   # Qt5
```

**"PostgreSQL not found" (CMake)**
```bash
# Point CMake to your PostgreSQL installation
cmake .. -DPostgreSQL_ROOT=/usr/pgsql-16
```

**Connection timeout on Windows**
- Ensure PostgreSQL service is running: `services.msc` → `postgresql-x64-16`
- Check `pg_hba.conf` allows connections from `localhost`
