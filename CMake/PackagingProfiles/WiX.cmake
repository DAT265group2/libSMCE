#
#  PackagingProfiles/WiX.cmake
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

set (CPACK_GENERATOR WIX)
#Generate by online website : https://www.guidgenerator.com/online-guid-generator.aspx
set (CPACK_WIX_UPGRADE_GUID C32F838C-EBF1-42D1-AC8C-DB918F25CD94)
set (CPACK_WIX_ROOT_FEATURE_TITLE "SMCE libSMCE")
#set (CPACK_WIX_PATCH_FILE "${PROJECT_SOURCE_DIR}/build/Debug/SMCE.dll")
set (CPACK_WIX_PATCH_FILE "C:/Dev/libSMCE/patch.xml")
