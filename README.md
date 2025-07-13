# TerminalAI

TerminalAI is a simple terminal emulator that integrates local LLMs via [Ollama](https://ollama.com) to provide intelligent command-line assistance using `/help`. Use your terminal as normal — and get smart suggestions right when you need them.

---

## ✨ Features

* Embedded terminal interface with ImGui
* Local LLM integration via Ollama (e.g., `deepseek-coder`, `phi3`, `llama3`)
* Instantly get shell command suggestions with `/help`
* Offline-first: No need for OpenAI API keys
* Configurable backend LLM

---

## 📦 Installation

You can either **compile from source** or **download a prebuilt binary**.

---

### 🔧 Compile from Source

#### 1. Clone the Repository

```bash
git clone https://github.com/nigelloh15/termai.git
cd terminalai
```

#### 2. Configure Python Version

* Open `CMakeLists.txt`
* Update the Python version reference to match the Python version installed on your system (e.g., `Python3.12` or `Python3.11`)

#### 3. Set Preferred LLM

* Open `integration.py`
* Set the variable or hardcoded model name to the local LLM you want to use (e.g., `"deepseek-coder"` or `"phi3"`)

#### 4. Build the App

```bash
mkdir build
cd build
cmake ..
make
```

* This will generate a `.app` file for MacOS.

---

### 📁 Prebuilt Binary

Download the latest `.app` file from the [Releases](https://github.com/nigelloh15/termai/releases) tab.

* The prebuilt version uses:

  * **Python**: 3.12.1
  * **Default LLM**: `deepseek-coder` via Ollama

No setup needed beyond ensuring `ollama` and the model are available on your machine.

---

## 🚀 Usage

### 1. Install Ollama

If you haven’t already, install [Ollama](https://ollama.com):

```bash
curl -fsSL https://ollama.com/install.sh | sh
```

### 2. Pull a Model

Pull the model you'd like to use (must match the one specified in `integration.py` if building from source):

```bash
ollama run deepseek-coder
# or for other models:
ollama pull phi3
```

> 💡 Make sure the model is running or available before launching TerminalAI.

### 3. Launch TerminalAI

#### If Built Locally:

```bash
./build/TerminalAI.app  # macOS .app
```

#### If Prebuilt:

Double-click the `.app` file or run it from terminal.

---

## 💬 How to Use

* Type `/help <your question>` to ask for CLI assistance
  Example:

  ```
  /help how to list all hidden files
  ```

* You will get a suggestion from the local LLM.

* Use the terminal like any other shell.

---

## 🛠️ Configuration

You can swap out the LLM used by modifying `integration.py` and restarting the app. Make sure the model is installed via `ollama`.

---

