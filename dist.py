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
    if f.split(".")[-1] in ("png", "ttf", "bmp", "opt", "xml"):
        f = src_dir+f
        print "Copying", f, "..."
        shutil.copy(f, dest_dir)

if os.name == "nt":
    src_dir = "./bin/Debug/"
    listOfFiles = os.listdir(src_dir)
    for f in listOfFiles:
        if f.endswith("dll") or f.endswith("exe"):
            f = src_dir+f
            print "Copying", f, "..."
            shutil.copy(f, dest_dir)
else:
    shutil.copy("build/Map2D", dest_dir)

items_to_copy = (
    ("png", "buttons"),
    ("png", "images"),
    ("png", "items"),
    ("png", "objects"),
    ("txt", "language"),
)
for ext,dir in items_to_copy:
    src_dir = "./%s/" % dir
    dest_dir = "./dist/%s" % dir
    os.makedirs(dest_dir)
    listOfFiles = os.listdir(src_dir)
    for f in listOfFiles:
        if f.endswith(ext):
            f = src_dir+f
            print "Copying", f, "..."
            shutil.copy(f, dest_dir)


# ZIP
if os.path.exists("dist.zip"):
    os.remove("dist.zip")

shutil.make_archive("dist", 'zip', "./dist/")
