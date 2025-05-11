## Tasia - The Next Generation Programming Language

**Mac/Linux**
```bash
curl -sSf https://raw.githubusercontent.com/dylancook244/tasia/production/scripts/install.sh | bash
```

**Windows**

Make sure you have Visual Studio with C++ tools
```bash
iwr -useb https://raw.githubusercontent.com/dylancook244/tasia/production/scripts/install.ps1 | iex
```

## Usage

```bash
# Compile a file
tasia build source.sia

# Compile and run
tasia run source.sia
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
