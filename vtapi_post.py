'''
VirusTotal API
vtapi_post.py
'''

import requests
import sys
import json

url = 'https://www.virustotal.com/vtapi/v2/file/scan'
# My api key # Don't use this key on anything program else.
api_key = '0f839cbc51dbca7de87e34c963743592b03b47d4230a0c96316f89ba512e2f57'
file_path_name = sys.argv[1] # file path

params = {'apikey': api_key}

file_to_send = open(file_path_name, 'rb')
files = {'file': (file_path_name, file_to_send)}

response = requests.post(url, files=files, params=params)
data = json.loads(response.text)
keys = data.keys()

for key in keys:
	print "%-13s : %s" % (key, data[key])

#print(response.json())
