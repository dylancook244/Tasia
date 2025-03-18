# Tasia Language

A lightweight, ergonomic programming language that combines Rust's memory safety with Go's simplicity. 

Currently still in development, not all features added yet. 

## Features 

- Memory safety through Rust-inspired ownership model
- Implicit lifetimes for cleaner syntax
- Procedural programming with lightweight OOP
- LLVM-powered optimized compilation

### Installation

**Mac/Linux**
```bash
curl -sSf https://raw.githubusercontent.com/dylancook244/tasia/production/scripts/install.sh | sh
```

**Windows**
```bash
iwr -useb https://raw.githubusercontent.com/dylancook244/tasia/production/scripts/install.ps1 | iex
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