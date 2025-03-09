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

- [Windows](https://github.com/dylancook244/tasia/releases/latest/download/tasia-Windows.exe)
- [macOS](https://github.com/dylancook244/tasia/releases/latest/download/tasia-macOS)
- [Linux](https://github.com/dylancook244/tasia/releases/latest/download/tasia-Linux)

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