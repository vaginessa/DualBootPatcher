from fileinfo import FileInfo
import common as c
import re

file_info = FileInfo()

filename_regex           = r"^Liquid-JB-v[0-9\.]+-OFFICIAL-.*\.zip$"
file_info.ramdisk        = 'jflte/AOSP/AOSP.def'
file_info.patch          = c.auto_patch
file_info.extract        = c.files_to_auto_patch

def matches(filename):
  if re.search(filename_regex, filename):
    return True
  else:
    return False

def print_message():
  print("Detected Official LiquidSmooth ROM zip")

def get_file_info():
  return file_info