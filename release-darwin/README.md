pipx uninstall tasia && pipx install . && tasia help



# Tasia Language

A lightweight, ergonomic programming language that combines Rust's memory safety with Go's simplicity. 

Currently still in development, not all features added yet. 

## Features 

- Memory safety through Rust-inspired ownership model
- Implicit lifetimes for cleaner syntax
- Procedural programming with lightweight OOP
- LLVM-powered optimized compilation

### Installation -- Make sure you have Python

Download the pre-built binary for your platform:

- [Windows](https://raw.githubusercontent.com/dylancook244/tasia/install/install.ps1)
- [macOS](https://raw.githubusercontent.com/dylancook244/tasia/install/install.sh)
- [Linux](https://raw.githubusercontent.com/dylancook244/tasia/install/install.sh)

### Instructions after downloading:

**Windows:**
1. Right-click the downloaded file and select "Save link as..." to save it
2. Open PowerShell as Administrator
3. Navigate to where you saved the file
4. Run: `.\install.ps1`

**macOS/Linux:**
1. Right-click the downloaded file and select "Save link as..." to save it
2. Open Terminal
3. Navigate to where you saved the file
4. Run: `chmod +x install.sh && ./install.sh`
```

## Usage

```bash
# Compile a file
tasia build source.sia

# Compile and run
tasia run source.sia

# Debug output
tasia run source.sia --compilerOutput
```

## Example

```sia
# Simple program
func square(x) {
  x * x
}

func main() {
  42 * square(2)
}
```

## License

MIT License