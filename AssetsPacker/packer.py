import argparse
import os
from collections import defaultdict

ASSETS_HEADER_FILE = "resources.hpp"
CMAKE_INCLUDE_NAME = "CMakeInclude.txt"

def get_this_file_path():

    full_path = os.path.realpath(__file__)
    dir_path = os.path.dirname(full_path)
    return dir_path

class Resource:
    def __init__(self, disk_path, target_path, res_type):
        self.disk_path = disk_path
        self.target_path = target_path
        self.res_type = res_type

    def escaped_name(self):
        return self.target_path.replace('/','___').replace(".",'__').replace('-','_')

    def c_name(self):
        asset_name = self.escaped_name()
        asset_name = "ASSET__" + asset_name
        return asset_name
    def c_file_name(self):
        file_name = self.escaped_name()
        file_name = "asset__" + file_name + ".cpp"
        return file_name
    def c_variable_name(self):
        asset_name = self.c_name()
        return asset_name + "__data"

    def c_size_name(self):
        asset_name = self.c_name()
        return asset_name + "__size"
    def c_path_name(self):
        asset_name = self.c_name()
        return asset_name + "__path"

    def print_exports(self, file_handle):
        f = file_handle
        print(f'extern "C" const size_t {self.c_size_name()};', file=f)
        print(f'extern "C" const char* {self.c_path_name()};', file=f)
        print(f'extern "C" const unsigned char {self.c_variable_name()}[];', file=f)

    def write_c_file(self, output_directory):

        file_data = self.read_file()

        with open(os.path.join(output_directory, self.c_file_name()),'w') as f:
            print("#include <cstddef>", file = f)
            self.print_exports(f)
            print("", file=f)

            print(f'const char* {self.c_path_name()} = "{self.target_path}";', file=f)
            print(f'const unsigned char {self.c_variable_name()}[] =', file=f)
            self.write_file_content(f, file_data)
            print("",file=f)
            print(f'const size_t {self.c_size_name()} = sizeof({self.c_variable_name()})/sizeof(unsigned char);', file=f)

    def read_file(self):
        return open(self.disk_path, 'rb').read()

    def write_file_content(self, file_handle, file_content):
        f = file_handle
        f.write('{')
        transform = hex_chars = list(map(hex, file_content))

        pos = 0
        while pos < len(transform):
            for item in transform[pos:pos+20]:
                f.write(item)
                f.write(',')
            f.write('\n')
            pos += 20
        f.write('};')


def write_header_file(output_dir: str, resources: dict[str,Resource]):
    with open(os.path.join(output_dir, ASSETS_HEADER_FILE), 'w') as f:
        print("#include <cstddef>", file=f)
        print('#include "AssetStruct.hpp"', file=f)
        print("",file=f)
        for item_name, resource in resources.items():
            resource.print_exports(f)
            print("",file=f)

        print("const AssetStruct assets_listing[] = {", file = f)
        for item_name, resource in resources.items():
            print(f'{{{resource.c_size_name()},{resource.c_path_name()},{resource.c_variable_name()}}},',file=f)
        print("};", file=f)


def write_cmake_file(output_dir: str, resources: dict[str,Resource]):
    with open(os.path.join(output_dir,CMAKE_INCLUDE_NAME), 'w') as f:
        print("target_sources(AssetsManager PRIVATE", file=f)
        print(f"./data/{ASSETS_HEADER_FILE}", file=f)
        for target, resource in resources.items():
            print(f"./data/{resource.c_file_name()}", file=f)
        print(")",file=f)

if __name__ == "__main__":
    current_file_dir = get_this_file_path()
    source_directory = os.path.join(current_file_dir,"../assets")
    output_directory = os.path.join(current_file_dir, "../src/AssetsManager/data")
    print("source_dir", source_directory)
    print("output_dir", output_directory)
    print("script_dir = ", current_file_dir)

    if not os.path.isdir(source_directory):
        raise RuntimeError(f"source directory doesn't exist: {source_directory}")
    if not os.path.isdir(output_directory):
        raise RuntimeError(f"output directory doesn't exist: {output_directory}")

    folder_data = defaultdict(list)
    for root, dirs, files in os.walk(source_directory):
        root = os.path.relpath(root,source_directory)
        root = root.replace('\\','/')
        for file in files:
            folder_data[root].append(file)
    resources = {}

    for folder_name, folder_list in folder_data.items():
        folder_name = folder_name.strip()
        if len(folder_list) == 0:
            print(f"warning: folder {folder_name} is empty!")

        for item in folder_list:
            item = item.strip()
            item_stored = {}
            item_disk_path = os.path.join(source_directory, folder_name, item)
            item_asset_path = '/'.join([folder_name,item])
            item_asset_path = 'assets/' + item_asset_path[2:]
            item_type = os.path.splitext(item)
            if not os.path.isfile(item_disk_path):
                raise ValueError(f"item: {item} in folder {folder_name} doesn't exists at path {item_disk_path}")
            if item_asset_path in resources:
                raise ValueError(f"duplicate resource: {item_asset_path}")
            if ' ' in item_asset_path:
                raise ValueError(f"asset path cannot have spaces: '{item_asset_path}'")
            resources[item_asset_path] = Resource(item_disk_path, item_asset_path, item_type)

    print("Done!")

    for resource in resources.values():
        resource.write_c_file(output_directory)
    write_header_file(output_directory, resources)
    write_cmake_file(output_directory, resources)
