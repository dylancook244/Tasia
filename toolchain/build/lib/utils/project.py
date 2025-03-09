import os
import sys

def start_project_scaffolding():
    if len(sys.argv) > 1:
        # store the proj_name as an argument
        proj_name = sys.argv[1]

        print(f"Creating tasia project {proj_name}")

        create_project_scaffolding(proj_name)


def create_project_scaffolding(proj_name):

    # create our project directories
    target_dir = os.path.join(proj_name, "target")
    src_dir = os.path.join(proj_name, "src")

    # create our directories
    os.mkdir(proj_name)
    os.mkdir(target_dir)
    os.mkdir(src_dir)

    # create our starting project main and toml files
    main_sia = os.path.join(src_dir, "main.sia")
    tasia_toml = os.path.join(proj_name, "Tasia.toml")

    # put our starting tasia code in main_sia
    with open (main_sia, 'w') as file:
        file.write("func main() {\n")
        file.write("\t42*69\n")
        file.write("}")

    # write a basic tasia_toml file for project
    with open (tasia_toml, 'w') as file:
        file.write("[package]\n")
        file.write(f'name = "{proj_name}"\n')
        file.write('version = "0.1.0"\n')
        file.write('edition = "2024"\n')
        file.write("\n")
        file.write("[dependencies]")



