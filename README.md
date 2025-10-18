# 🕵️‍♀️ NavigationRecorder: Web Navigation Recorder and Visualizer

NavigationRecorder is a Qt-based application designed to record, replay, and visualize web navigation sessions. It captures main page navigations, AJAX calls,  saves screenshots, and generates a visual navigation graph. The application provides a user-friendly interface with a toolbar and menu, as well as command-line options for enhanced flexibility.

![screenshot](https://github.com/zebulon75018/navigationRecorder/blob/main/img/navigationrecorder.png?raw=true)

## ✨ Features

* **Record Web Navigation**:
  * Captures main page navigations (e.g., link clicks, form submissions, typed URLs).
  * Intercepts AJAX.
  * Records navigation events with URLs, timestamps, HTTP methods, parameters, and optional screenshots.

* **Screenshots**:
  * Optionally captures screenshots of the web view for each navigation event.
  * Screenshots are stored in a customizable directory (default: `screenshots/`).
  * Screenshot capture can be enabled/disabled via the `--screenshots` command-line option.

* **Replay Navigation**:
  * Replays recorded navigation sessions by loading URLs in sequence (skipping AJAX requests).
  * Allows stopping the replay at any time using the "Stop" button.

* **Save and Load Sessions**:
  * Saves navigation events (including URLs, methods, parameters, and screenshot paths) to a JSON file.
  * Loads JSON files to restore and replay previous sessions.

* **Visual Navigation Graph**:
  * Generates a `.dot` file for visualization with [Graphviz](https://graphviz.org/).
  * Nodes display screenshots (if enabled) with labels showing the URL and HTTP method.
  * Edges represent transitions between navigation events, with dashed lines for AJAX requests.

* **User Interface**:
  * Toolbar with buttons for "Record" (turns red when active), "Play", "Stop", and "Save".
  * Menu with actions to start/stop recording, replay, save/load JSON files, save `.dot` files, and quit the application.

* **Command-Line Options**:
  * `--url <url>`: Specifies the initial URL to load (e.g., `--url https://www.example.com`).
  * `--replay <file>`: Loads and replays a JSON file (e.g., `--replay navigation.json`).
  * `--screenshot-dir <dir>`: Sets the directory for screenshots (default: `screenshots`).
  * `--screenshots`: Enables screenshot capture (disabled by default).
  * *At least one of `--url`, `--replay`, or `--help` must be provided, otherwise the application exits with an error.*

## 📋 Prerequisites

* **Qt**: Version 5 or 6 with the `QtWebEngine`, `QtWidgets` modules.
* **Compiler**: A C++17-compatible compiler (e.g., g++, MSVC).
* **Graphviz**: Required to visualize `.dot` files as graphs.
* **Icons (optional)**: Provide custom icons for the toolbar (`record.png`, `record_active.png`, `play.png`, `stop.png`, `save.png`) or use Qt standard icons.

## 📦 Installation

1. **Clone the Repository**:
   ```bash
   git clone https://github.com/zebulon75018/navigationRecorder.git
   cd navigationRecorder
