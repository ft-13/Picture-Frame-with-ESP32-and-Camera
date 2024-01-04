import sys
import subprocess
import errno
import os
import shutil


ext = [".jpg", ".JPG", ".jpeg", ".JPEG"]

# folder path
dir_path = r'./'

# list to store files
temp_files = []
# Iterate directory
for file in os.listdir(dir_path):
    # check only text files
    if file.endswith(tuple(ext)):
        output_gray = os.path.splitext(os.path.basename(file))[0] + '_gray' + os.path.splitext(os.path.basename(file))[1]
        
        process = subprocess.Popen('magick convert' + ' ' + file + ' ' + '-colorspace Gray' + ' ' + output_gray, stdout=subprocess.PIPE, creationflags=0x08000000)
        process.wait()
        temp_files.append(output_gray)
        
        input_gray = output_gray
        output_resized = os.path.splitext(os.path.basename(input_gray))[0] + '_resized' + os.path.splitext(os.path.basename(input_gray))[1]
        
        process = subprocess.Popen('magick convert' + ' ' + input_gray + ' ' + '-resize 1024x768!' + ' ' + output_resized, stdout=subprocess.PIPE, creationflags=0x08000000)
        process.wait()
        temp_files.append(output_resized)
        
        input_resized = output_resized
        output_raw = os.path.splitext(os.path.basename(file))[0] + '.gray'
        
        process = subprocess.Popen('magick convert' + ' ' + input_resized + ' ' + '-depth 8' + ' ' + output_raw, stdout=subprocess.PIPE, creationflags=0x08000000)
        process.wait()


# getting length of list
length = len(temp_files)
  
# Iterating the index
# same as 'for i in range(len(list))'
#for i in range(length):
    #os.remove(temp_files[i])


#move created .gray files to output folder and rename extension to .txt
for file in os.listdir(dir_path):
    # check only .gray files
    if file.endswith('.gray'):
        src = './' + file
        dest = './output/' + os.path.splitext(os.path.basename(file))[0] + '.txt'
        try:
            shutil.move(src, dest)
        except IOError as e:
            # ENOENT(2): file does not exist, raised also on missing dest parent dir
            if e.errno != errno.ENOENT:
                raise
            # try creating parent directories
            os.makedirs(os.path.dirname(dest))
            shutil.move(src, dest)





