###########################################################
# get_venv_base.py
# Created by Greg on 2/14/2025.
# Copyright (C) 2025 Presage Security, Inc.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 3 of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
###########################################################
import os
import sys

PROGRAM_EXIT_FAILURE = 1

def activate_venv(venv_path):
    # Validate and sanitize the input path to prevent path traversal
    venv_path = os.path.abspath(os.path.normpath(venv_path))
    activate_script = os.path.join(venv_path, 'bin', 'activate_this.py')
    if os.path.exists(activate_script):
        # Validate that this is actually a virtual environment directory
        if not os.path.isdir(venv_path) or not os.path.exists(os.path.join(venv_path, 'pyvenv.cfg')):
            print(f"Path {venv_path} does not appear to be a valid virtual environment.")
            sys.exit(PROGRAM_EXIT_FAILURE)

        # More secure approach: check if we can update sys.path instead of executing arbitrary code
        # Try multiple possible site-packages paths for cross-platform compatibility
        possible_paths = [
            # Unix/Linux/macOS
            os.path.join(venv_path, 'lib', f'python{sys.version_info.major}.{sys.version_info.minor}', 'site-packages'),
            # Windows
            os.path.join(venv_path, 'Lib', 'site-packages'),
            # Some conda environments
            os.path.join(venv_path, 'lib', 'site-packages')
        ]

        site_packages = None
        for path in possible_paths:
            if os.path.exists(path):
                site_packages = path
                break

        if site_packages:
            # Insert at beginning of sys.path to ensure venv packages take priority
            sys.path.insert(0, site_packages)
            # Set virtual env environment variables manually
            os.environ['VIRTUAL_ENV'] = venv_path
            if 'PYTHONHOME' in os.environ:
                del os.environ['PYTHONHOME']
        else:
            print(f"Site-packages directory not found in {venv_path}. Tried: {', '.join(possible_paths)}")
            sys.exit(PROGRAM_EXIT_FAILURE)
    else:
        print(f"Activation script {activate_script} not found.")
        sys.exit(PROGRAM_EXIT_FAILURE)

def get_venv_base():
    if hasattr(sys, 'real_prefix') or (hasattr(sys, 'base_prefix') and sys.base_prefix != sys.prefix):
        return sys.prefix
    elif 'VIRTUAL_ENV' in os.environ:
        return os.environ['VIRTUAL_ENV']
    else:
        return None

def main():
    venv_base = get_venv_base()
    if not venv_base:
        # Assuming the virtual environment path is passed as an environment variable
        venv_path = os.getenv('VIRTUAL_ENV_PATH')
        if venv_path:
            activate_venv(venv_path)
            venv_base = get_venv_base()

    if venv_base:
        print(venv_base)
    else:
        print("Not running inside a virtual environment")
        sys.exit(PROGRAM_EXIT_FAILURE)

if __name__ == "__main__":
    main()
