import sys

# entry point for cli commands
# commands: help, new
def cli_commands():

    if len(sys.argv) > 1:
        command = sys.argv[1]
            