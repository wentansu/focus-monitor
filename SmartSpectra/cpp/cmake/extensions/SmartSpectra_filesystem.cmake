###########################################################
# filesystem.cmake
# Created by Greg on 8/19/2024.
# Copyright (C) 2024 Presage Security, Inc.
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

function(create_directory_with_placeholder_if_does_not_exist directory_path)
    if (NOT EXISTS ${directory_path})
        file(MAKE_DIRECTORY ${directory_path})
        file(WRITE ${directory_path}/placeholder "")
    endif ()
endfunction()

function(make_if_does_not_exist directory_path)
    if (NOT EXISTS ${directory_path})
        file(MAKE_DIRECTORY ${directory_path})
    endif ()
endfunction()
