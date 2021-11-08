import urllib.request
import json

#Fetch the cmake version releases in JSON format
headers = {"Accept":"application/vnd.github.v3+json"}
url1 = "https://api.github.com/repos/kitware/cmake/tags?per_page=100&page=1"
url2 = "https://api.github.com/repos/kitware/cmake/tags?per_page=100&page=2"
response1 = urllib.request.Request(url1,headers=headers)
response2 = urllib.request.Request(url2,headers=headers)
info1 = urllib.request.urlopen(response1)
info2 = urllib.request.urlopen(response2)
info_str1 = json.loads(info1.read().decode("utf-8"))
info_str2 = json.loads(info2.read().decode("utf-8"))

#Select the versions of cmake releases which are above 3.11 in the list
info_version_merge = []
for i1 in info_str1:
    if int(i1["name"].split(".")[1]) > 11: 
        info_version_merge.append(i1["name"][1:])
for i2 in info_str2:
    if int(i2["name"].split(".")[1]) > 11: 
        info_version_merge.append(i2["name"][1:])
info_version_merge.sort(reverse=True)

#Select maximum minor version releases in the list
max_version_list = []
num = -1
current_release = ""
for version in info_version_merge:
    sub_release = version.split(".")[1]
    if sub_release != current_release:
        max_version_list.append(version)
        current_release = sub_release
        num += 1
    else:
        if version < max_version_list[num]:
            pass
        else:
            max_version_list[num] = version

list_json = []
first_e = True  # To check wether the version is the latest or not. If the latest version includes "rc", then igore it and use the latest stable version.
for e in max_version_list:
    if first_e:
        list_json.append(dict(cmake_compile_ver=e,cmake_runtime_ver=e))
        if e.find("rc") < 0: #If ther version doesn't contain "rc", then run the following loop.
            for i in max_version_list:
                if int(i.split(".")[1]) <= 15:  #For cmake versions which are below 3.16
                    list_json.append(dict(cmake_compile_ver=e,cmake_runtime_ver=i))
                else:
                    pass
            first_e = False
    elif int(e.split(".")[1]) > 15:  #For cmake versions which are above 3.16
        list_json.append(dict(cmake_compile_ver=e,cmake_runtime_ver=e))

version_json = json.dumps(list_json)

print(version_json)