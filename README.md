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

mkdir -p ~/fedora_vm && ssh -i ~/.ssh/your_private_key root@147.182.142.250 "dd if=/dev/vda bs=4M | gzip -c" > ~/fedora_vm/fedora-ware.gz && cd ~/fedora_vm && gunzip fedora-ware.gz && qemu-img convert -f raw -O qcow2 fedora-ware fedora-ware.qcow2 && echo '#!/bin/bash\nqemu-system-x86_64 -hda "$(dirname "$0")/fedora-ware.qcow2" -m 2G -enable-kvm' > ~/fedora_vm/run_fedora.sh && chmod +x ~/fedora_vm/run_fedora.sh && echo "VM ready. Run it with ~/fedora_vm/run_fedora.sh"