version_list = []
f = open('opt/releases.txt')
line = f.readline()
while line:
    version = line.split("\"")[3][1:]
    sub_release = version.split(".")[1]
    if int(sub_release) >= 12:
        version_list.append(version) 
    line = f.readline()
version_list.sort(reverse=True)

max_version_list = []
num = -1
current_release = ""
for version in version_list:
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

length = len(max_version_list)
num = 0
for e in max_version_list:
    if num != length-1:
        with open('/opt/list.txt','a') as f:
            f.write(e+',')
    else:
        with open('/opt/list.txt','a') as f:
            f.write(e)
    num += 1
    