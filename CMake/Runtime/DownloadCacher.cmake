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

# function for cleaning the cache
function (clear_download_cache)
  # delete all contents in the cached folder
  file (REMOVE_RECURSE "${SMCE_DIR}/cached_downloads/")
  message (STATUS "Cache cleared")
endfunction (clear_download_cache)

#[================================================================================================================[.rst:
cached_download
--------------------

Function to allow for caching of various downloads.

Usage:
.. code-block:: cmake

  cached_download (URL <url> DEST <dest-var> [RESULT_VARIABLE <res-var>] [FORCE_UPDATE])

Where ``<URL>`` is the URL to the file to be downloaded, ``<DEST>`` is the name of the variable to store the absolute
real path to the download location passed to the parent scope by the function, ``<RESULT_VARIABLE>``
is the name of the variable to store the result of all processes passed to the parent scope by
the function, ``<FORCE_UPDATE>`` will define whether an already cached download should be re-downloaded and cached.


Note:
No additional arguments except for the ones defined are allowed in the function call.
Uses SHA256 to create a uniquely hashed download location for each download.
Download file is locked until the file has been downloaded and cached, this is done in order to avoid
possible race conditions.
#]================================================================================================================]
function (cached_download)
  # initialize the cache download directory
  file (MAKE_DIRECTORY "${SMCE_DIR}/cached_downloads")

  # parse args
  set (options FORCE_UPDATE)
  set (oneValueArgs URL RESULT_VARIABLE DEST)
  set (multiValueArgs)
  cmake_parse_arguments ("ARG" "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  if (ARG_UNPARSED_ARGUMENTS)
    set (ARG_RESULT_VARIABLE "Function was called with too many arguments" PARENT_SCOPE)
    message (FATAL_ERROR "Function was called with too many arguments")
  endif ()
  
  # use SHA256 hash to URL to create unique identifier, lock download location mutex-style
  set ("${ARG_RESULT_VARIABLE}" "" PARENT_SCOPE)
  string (SHA256 HASHED_DEST "${ARG_URL}")
  set (DOWNLOAD_PATH "${SMCE_DIR}/cached_downloads/${HASHED_DEST}")
  file (TOUCH "${DOWNLOAD_PATH}.lock")
  file (LOCK "${DOWNLOAD_PATH}.lock")

  # check if plugin has already been downloaded and cached before  
  if (EXISTS "${DOWNLOAD_PATH}")
    set (INDEX 1)
  else ()
    set (INDEX -1)
  endif ()
  
  # if download has been cached previously and is requested a forced re-download, clean previous download and re-cache 
  if (${INDEX} GREATER -1 AND ${ARG_FORCE_UPDATE})
    file (REMOVE "${DOWNLOAD_PATH}")
    set (${INDEX} -1)
  endif ()

  # if download has not been cached, download. Otherwise pass.
  if (${INDEX} LESS 0)
    message (DEBUG "Downloading")

    file (DOWNLOAD "${ARG_URL}" "${DOWNLOAD_PATH}" STATUS DLSTATUS)
    list (GET DLSTATUS 0 DLSTATUS_CODE)
    if (DLSTATUS_CODE)
      list (GET DLSTATUS 1 DLSTATUS_MESSAGE)
      file (REMOVE "${DOWNLOAD_PATH}")
      file (REMOVE "${DOWNLOAD_PATH}.lock")
      set (ARG_RESULT_VARIABLE "${DLSTATUS}" PARENT_SCOPE)
      message (FATAL_ERROR "Download failed: (${DLSTATUS_CODE}) ${DLSTATUS_MESSAGE}")
    endif ()
    message (DEBUG "Download complete")
    message (DEBUG "Cached!")
  else ()
    message (DEBUG "Has already been cached!")
  endif ()

  # Unlock file and output absolute real path to download location
  file (LOCK "${DOWNLOAD_PATH}.lock" RELEASE)
  file (REMOVE "${DOWNLOAD_PATH}.lock")
  
  if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.19")
    file (REAL_PATH "${DOWNLOAD_PATH}" DOWNLOAD_PATH)
  else ()
    get_filename_component (DOWNLOAD_PATH "${DOWNLOAD_PATH}" REALPATH)
  endif ()
  
  set ("${ARG_DEST}" "${DOWNLOAD_PATH}" PARENT_SCOPE)

endfunction (cached_download)
