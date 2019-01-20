#!/usr/bin/python2.6
# coding: utf-8

import sys
import pygame
import os.path
from pygame.locals import *

class Transparency:
    def __init__(self):
        pass

if __name__ == "__main__":
    pygame.init()
    
    argc = len(sys.argv)
    
    if argc != 2:
        print "Error: need 1 argument (image filename)"
        sys.exit()
    
    filename = sys.argv[1]
        
    print "Put transparency for file", filename
    
    if not os.path.exists(filename):
        print "Error:", filename, "is not a file"
        sys.exit()
    
    window = pygame.display.set_mode((200, 200))
    myimage = pygame.image.load(filename).convert()
    myimage.set_colorkey((255,255,255),RLEACCEL)
    
    filename = "result_"+filename
    print "Result:", filename
    
    pygame.image.save(myimage, filename)