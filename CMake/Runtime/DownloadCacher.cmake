#
#  DownloadCacher.cmake
#  Copyright 2021 ItJustWorksTM
#
#  Licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
#

# initialize the cache, call it from within cached_download
function (cache_init)
  # allocate folder for where to cache the plugins
  # if it exists, clear cache content?
endfunction ()

# function for cleaning the cache
function (clear_download_cache)
  # delete all contents in the cached folder
  # (add option for deleting single cached folder?)
endfunction ()

# function for downloading URI with optional force (force re-download)
# dest: input file name, output absolute real path to download location
# needs to work 3.12+
function (cached_download URL DEST FORCE_UPDATE ERROR_PARAM)
  # check if plugin already exists
  # if !exists or FORCE_UPDATE
    # download and install
  # update CACHE_LIST
  # return status
endfunction ()