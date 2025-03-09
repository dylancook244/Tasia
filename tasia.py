import sys

def main():
    from toolchain.utils.cli import cli_commands
    return cli_commands()

if __name__ == "__main__":
    sys.exit(main())

# run to create new binary
# pyinstaller --onefile --name=tasia --clean tasia.py
