'''
VirusTotal API
vtapi.c
'''

import requests

url = 'https://www.virustotal.com/vtapi/v2/file/scan'

params = {'apikey': '<apikey>'}

files = {'file': ('myfile.exe', open('myfile.exe', 'rb'))}

response = requests.post(url, files=files, params=params)

print(response.json()) 
