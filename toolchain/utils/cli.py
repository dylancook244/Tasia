import os
import sys
import subprocess
from pathlib import Path
import shutil

# Path to the compiler directory relative to this script
COMPILER_DIR = Path(__file__).parent / "compiler"
COMPILER_EXEC_NAME = "tasia_compiler" + (".exe" if sys.platform.startswith("win") else "")

def ensure_compiler_built():
    """Ensure the compiler is built and ready to use"""
    compiler_path = COMPILER_DIR / COMPILER_EXEC_NAME
    
    # Check if compiler exists and is executable
    if not compiler_path.exists():
        print("Compiler not found. Building now...")
        return build_compiler()
    
    return str(compiler_path)

def build_compiler():
    """Build the compiler using the Makefile"""
    # Store the current directory
    original_dir = os.getcwd()
    
    try:
        # Change to the compiler directory
        os.chdir(COMPILER_DIR)
        
        # Run make
        print("Building compiler...")
        if sys.platform.startswith("win"):
            # Windows might use nmake or mingw32-make
            result = subprocess.run(["make", "-f", "Makefile"], 
                                  check=False, capture_output=True, text=True)
        else:
            # Unix-like systems
            result = subprocess.run(["make"], 
                                  check=False, capture_output=True, text=True)
        
        if result.returncode != 0:
            print("Compiler build failed:")
            print(result.stderr)
            return None
        
        # Verify the compiler was built
        compiler_path = Path(COMPILER_EXEC_NAME)
        if not compiler_path.exists():
            print(f"Build succeeded but compiler executable not found at {compiler_path}")
            return None
        
        return str(COMPILER_DIR / COMPILER_EXEC_NAME)
    
    finally:
        # Restore the original directory
        os.chdir(original_dir)

def compile_file(filepath):
    """Compile a Tasia source file using the compiler"""
    # Ensure the compiler is built
    compiler_path = ensure_compiler_built()
    if not compiler_path:
        print("Could not build or find the compiler")
        return False
    
    # Run the compiler on the file
    print(f"Compiling {filepath}...")
    result = subprocess.run([compiler_path, "compile", filepath], 
                          check=False, capture_output=True, text=True)
    
    if result.returncode != 0:
        print("Compilation failed:")
        print(result.stderr)
        return False
    
    return True

def run_file(filepath):
    """Compile and run a Tasia source file"""
    # First compile
    if not compile_file(filepath):
        return False
    
    # Get the output executable path (strip extension)
    output_path = os.path.splitext(filepath)[0]
    
    # Run the executable
    print(f"Running {output_path}...")
    result = subprocess.run([output_path], check=False)
    
    return result.returncode == 0

def create_project_scaffolding(project_name):
    """Create a new Tasia project"""
    print(f"Creating tasia project {project_name}")
    
    # Create project directories
    os.makedirs(os.path.join(project_name, "target"), exist_ok=True)
    os.makedirs(os.path.join(project_name, "src"), exist_ok=True)
    
    # Create main.sia file
    main_path = os.path.join(project_name, "src", "main.sia")
    with open(main_path, 'w') as f:
        f.write("func main() {\n")
        f.write("\t42*69\n")
        f.write("}")
    
    # Create Tasia.toml file
    toml_path = os.path.join(project_name, "Tasia.toml")
    with open(toml_path, 'w') as f:
        f.write("[package]\n")
        f.write(f'name = "{project_name}"\n')
        f.write('version = "0.1.0"\n')
        f.write('edition = "2024"\n')
        f.write("\n")
        f.write("[dependencies]")
    
    print(f"Project {project_name} created successfully")
    return True

def print_help():
    """Print help information"""
    print("\nUsage:")
    print("  tasia new PROJECT_NAME   Create a new Tasia project")
    print("  tasia compile FILE       Compile a Tasia source file")
    print("  tasia run FILE           Compile and run a Tasia source file")
    print("  tasia help               Show this help information")

def cli_commands():
    """Main CLI implementation for Tasia"""
    if len(sys.argv) < 2:
        print("Tasia - The Programming Language")
        print_help()
        return 0

    command = sys.argv[1]
    
    if command == "compile":
        if len(sys.argv) < 3:
            print("Missing filename: tasia compile <filename>")
            return 1
        
        filepath = sys.argv[2]
        success = compile_file(filepath)
        return 0 if success else 1
        
    elif command == "run":
        if len(sys.argv) < 3:
            print("Missing filename: tasia run <filename>")
            return 1
        
        filepath = sys.argv[2]
        success = run_file(filepath)
        return 0 if success else 1
        
    elif command == "new":
        if len(sys.argv) < 3:
            print("Missing project name: tasia new <project_name>")
            return 1
        
        project_name = sys.argv[2]
        create_project_scaffolding(project_name)
        return 0
        
    elif command == "help":
        print_help()
        return 0
        
    else:
        print(f"Unknown command: {command}")
        print_help()
        return 1