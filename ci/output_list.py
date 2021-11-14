import urllib.request
import json
import pandas as pd

# fetch repo info
def fetch_repo_info():
    #Fetch the cmake version releases in JSON format
    headers = {"Accept":"application/vnd.github.v3+json"}
    url1 = "https://api.github.com/repos/kitware/cmake/releases?per_page=100&page=1"
    url2 = "https://api.github.com/repos/kitware/cmake/releases?per_page=100&page=2"
    response1 = urllib.request.Request(url1,headers=headers)
    response2 = urllib.request.Request(url2,headers=headers)
    info1 = urllib.request.urlopen(response1)
    info2 = urllib.request.urlopen(response2)
    info_str1 = json.loads(info1.read().decode("utf-8"))
    info_str2 = json.loads(info2.read().decode("utf-8"))

    return info_str1+info_str2[::-1]

# Select the versions of cmake releases which are above 3.11 in the list
# and sort them. Use least version tuple variable to update the least 
# active cmake version.
def sort_vers(repo_info_list, least_ver=(3,11)):
    
    ind = 0
    version_ = {}
    
    for x in repo_info_list:
        tag = x.get('tag_name')
        tag = tag.replace('v','')
        tag_ = tag.replace('-','.')
        tag_vec = map(int, tag_.split(".")[0:3])
        
        first = next(tag_vec)
        second = next(tag_vec) 
        third = next(tag_vec)
        i = tag.find('rc')
        if(i<0):
            fourth = 0
        else:
            fourth = int(tag[i+2:])
        
        if(first>=least_ver[0] and second>least_ver[1]):
            ind +=1
            version_[ind] = [first, second, third, fourth, tag]
            
    df = pd.DataFrame(version_.values(), columns=['Main', 'Sub', 'Subsub', 'RC', 'Tag'])
    
    return df.sort_values(['Main', 'Sub', 'Subsub', 'RC'], ascending=False)

# Select maximum minor version releases in the list
def max_min_vers(vers_df):
    i = -1
    mnv_dict = {}
    for index, row in vers_df.iterrows():
       if(i != row.iloc[1]):
           mnv_dict[i] = row
           i = row.iloc[1]

    return pd.DataFrame(mnv_dict.values(), columns=['Main', 'Sub', 'Subsub', 'RC', 'Tag'])

def vers_json(mv_df):
    list_json = []
    ind = 0

    #If the latest version contains "rc", add the version into the final version list first, then delete the version from the max_version_list
    if mv_df.iloc[0][3] > 0:  
        list_json.append(dict(cmake_compile_ver=mv_df.iloc[0][4],cmake_runtime_ver=mv_df.iloc[0][4]))
        ind = 1
        
    for i in range(ind,len(mv_df.index)):
        if mv_df.iloc[i][1] <= 15:  #For cmake versions which are below 3.16
            list_json.append(dict(cmake_compile_ver=mv_df.iloc[ind][4],cmake_runtime_ver=mv_df.iloc[i][4]))
        else: #For cmake versions which are above 3.16
            list_json.append(dict(cmake_compile_ver=mv_df.iloc[i][4],cmake_runtime_ver=mv_df.iloc[i][4]))
        
    return json.dumps(list_json)



def main():
    repo_info = fetch_repo_info()
    sorted_vers = sort_vers(repo_info)
    max_version_df = max_min_vers(sorted_vers)
    version_json = vers_json(max_version_df)
    print(version_json)

if __name__ == "__main__":
    main()