import shutil
import os
import zipfile

# CLEAN DIST IF ANY
dest_dir = "./dist"
if os.path.exists(dest_dir):
    shutil.rmtree(dest_dir)
os.makedirs(dest_dir)

# CREATE DIST
src_dir = "./"
listOfFiles = os.listdir(src_dir)
for f in listOfFiles:
    if f.endswith("png") or f.endswith("TTF") or f.endswith("bmp"):
        f = src_dir+f
        print "Copying", f, "..."
        shutil.copy(f, dest_dir)
        
src_dir = "./bin/Debug/"
listOfFiles = os.listdir(src_dir)
for f in listOfFiles:
    if f.endswith("dll") or f.endswith("exe"):
        f = src_dir+f
        print "Copying", f, "..."
        shutil.copy(f, dest_dir)

# ZIP    
if os.path.exists("dist.zip"):    
    os.remove("dist.zip")        
zf = zipfile.ZipFile("dist.zip", mode='w')
src_dir = "./dist/"
listOfFiles = os.listdir(src_dir)
for f in listOfFiles:
    f = src_dir+f
    print "Zipping", f, "..."
    zf.write(f)
zf.close()