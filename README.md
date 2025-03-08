# Tasia Language

A lightweight, ergonomic programming language that combines Rust's memory safety with Go's simplicity. 

Currently still in development, not all features added yet. 

## Features 

- Memory safety through Rust-inspired ownership model
- Implicit lifetimes for cleaner syntax
- Procedural programming with lightweight OOP
- LLVM-powered optimized compilation

### Installation -- Make sure you have Python

Windows: 
```bash
git clone https://github.com/dylancook244/tasia.git; cd tasia; python build.py; cd ..; if (Test-Path "C:\Program Files\tasia\bin\tasia.exe") { Remove-Item -Recurse -Force tasia; Write-Host "Tasia installed and repo cleaned." } else { Write-Host "WARNING: Installation incomplete. Repo kept." }
```

Mac/Linux: 
```bash
git clone https://github.com/dylancook244/tasia.git && cd tasia && python build.py && cd .. && { command -v tasia >/dev/null 2>&1 && rm -rf tasia && echo "Tasia installed and repo cleaned." || echo "WARNING: Installation incomplete. Repo kept."; }
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