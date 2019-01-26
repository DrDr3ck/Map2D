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
    if f.endswith("png") or f.endswith("TTF") or f.endswith("bmp") or f.endswith("opt"):
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
        
        
src_dir = "./buttons/"
dest_dir = "./dist/buttons"
os.makedirs(dest_dir)
listOfFiles = os.listdir(src_dir)
for f in listOfFiles:
    if f.endswith("png"):
        f = src_dir+f
        print "Copying", f, "..."
        shutil.copy(f, dest_dir)
        
src_dir = "./items/"
dest_dir = "./dist/items"
os.makedirs(dest_dir)
listOfFiles = os.listdir(src_dir)
for f in listOfFiles:
    if f.endswith("png"):
        f = src_dir+f
        print "Copying", f, "..."
        shutil.copy(f, dest_dir)        
        
src_dir = "./objects/"
dest_dir = "./dist/objects"
os.makedirs(dest_dir)
listOfFiles = os.listdir(src_dir)
for f in listOfFiles:
    if f.endswith("png"):
        f = src_dir+f
        print "Copying", f, "..."
        shutil.copy(f, dest_dir)                
        
src_dir = "./language/"
dest_dir = "./dist/language"
os.makedirs(dest_dir)
listOfFiles = os.listdir(src_dir)
for f in listOfFiles:
    if f.endswith("txt"):
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
src_dir = "./dist/buttons/"
listOfFiles = os.listdir(src_dir)
for f in listOfFiles:
    f = src_dir+f
    print "Zipping", f, "..."
    zf.write(f)    
src_dir = "./dist/items/"
listOfFiles = os.listdir(src_dir)
for f in listOfFiles:
    f = src_dir+f
    print "Zipping", f, "..."
    zf.write(f)      
src_dir = "./dist/objects/"
listOfFiles = os.listdir(src_dir)
for f in listOfFiles:
    f = src_dir+f
    print "Zipping", f, "..."
    zf.write(f)          
src_dir = "./dist/language/"
listOfFiles = os.listdir(src_dir)
for f in listOfFiles:
    f = src_dir+f
    print "Zipping", f, "..."
    zf.write(f)         
zf.close()