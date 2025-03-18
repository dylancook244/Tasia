## Tasia - The Next Generation

**Mac/Linux**
```bash
curl -sSf https://raw.githubusercontent.com/dylancook244/tasia/production/scripts/install.sh | bash
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
func square(x) {
  x * x
}

func main() {
  42 * square(2)
}
```