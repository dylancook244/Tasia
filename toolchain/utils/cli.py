import sys
from utils.project import create_project_scaffolding
# Import other utilities as needed

def cli_commands():
    """Main CLI implementation for Tasia"""
    if len(sys.argv) < 2:
        print("Tasia - The Programming Language")
        print_help()
        return 0

    command = sys.argv[1]
    
    # Handle different commands
    if command == "new" and len(sys.argv) > 2:
        project_name = sys.argv[2]
        create_project_scaffolding(project_name)
    elif command in ["help"]:
        print_help()
    else:
        print(f"Unknown command: {command}")
        print_help()
        return 1
    
    return 0

def print_help():
    """Print help information"""
    print("\nUsage:")
    print("  tasia new PROJECT_NAME   Create a new Tasia project")
    print("  tasia help               Show this help information")
            