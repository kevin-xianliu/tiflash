# Copyright 2022 PingCAP, Ltd.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

add_subdirectory (System)
add_subdirectory (Page)
add_subdirectory (DeltaMerge/File/dtpb)
add_subdirectory (DeltaMerge/workload)
add_subdirectory (Page/workload)

if (ENABLE_TESTS)
    add_subdirectory (tests EXCLUDE_FROM_ALL)
    add_subdirectory (Transaction/tests EXCLUDE_FROM_ALL)
    add_subdirectory (Page/V2/tests EXCLUDE_FROM_ALL)
    add_subdirectory (Page/V3 EXCLUDE_FROM_ALL)
    add_subdirectory (Page/V3/tests EXCLUDE_FROM_ALL)
    add_subdirectory (DeltaMerge/tests EXCLUDE_FROM_ALL)
endif ()

