Cat Mother Source Code Release

Copyright (C) 2003 Cat Mother, Ltd.
See below for license information.


Briefly
-------
Game development company Cat Mother Ltd. (www.catmother.com) has now closed 
its  offices, but in their last meeting the company board decided to 
publish all company  source code as open source. Also large part of the 
content is published. Published  material includes fully playable prototype 
of a 3rd person action/adventure game and commercial quality in-house 
3D-engine (C++/DirectX9). The source code is  published under BSD license 
and the content is published under GPL license. All  material can be 
downloaded from catmother.sourceforge.net.


About This Package
------------------
This package contains source code release only. For executable game demo 
and associated data files please download cm-deadjustice-demo-YYYYMMDD.zip 
from catmother.sourceforge.net. Example 3dsmax files and other content can 
be found from cm-gfx-YYYYMMDD.zip, also at catmother.sourceforge.net.


License
-------
Source code is freely available for personal, academic and commercial 
purposes. See license_src.txt for details. (based on BSD license from 
www.opensource.org)


Source Code Overview
--------------------

anim/
-- Low level animation classes

bsp/
-- BSP tree classes

config/
-- Configuration headers for MSVC6

crypt/
-- Simple encryption library to protect game content

deadjustice/
-- Prototype of 3rd person action/adventure game

dev/
-- Development helper classes for profiling

docs/
-- File formats, programming conventions, etc. shared documents

fsm/
-- Finite state machine library

gd/
-- Graphics device layer library (DirectX Graphics -wrapper)

id/
-- Input device library (DirectInput -wrapper)

io/
-- Input/output streaming library. Designed to match closely java.io.*

lang/
-- Core classes, e.g. Unicode support and thread-safe reference counting. 
-- Designed to match closely java.lang.*

math/
-- Low-level math classes

maxexport/
-- In-house 3dsmax5 exporter. See maxexport/docs/sgexport.txt for usage.

mb/
-- Mesh builder library used for geometry processing

mem/
-- Memory allocation support library

music/
-- Music (mp3) playback support

pix/
-- Low level pixel processing library, supports pixel format conversions etc.

ps/
-- Particle system library

script/
-- Scripting support (Lua wrapper)

sd/
-- Sound device layer library (DSound wrapper)

sg/
-- Scene graph library

sgu/
-- Scene graph utility classes

sgviewer/
-- Scene (.sg) file viewer. See sgviewer/docs/sgviewer.txt for usage.

snd/
-- High level sound library with scene graph binding

tester/
-- Testing framework

tools/crypta
-- Small command line encryption utility to be used with 'crypt' library

tools/imgpager
-- Small command line utility, packs sequence of images to single image.

util/
-- Utility classes. Designed to match closely java.util.*

win/
-- Simple Win32 wrapper library


External Library Dependencies
-----------------------------
Source code depends on libjpeg v6b, Lua v4 and zlib v1.1.3 libraries. 
Those libraries are not distributed with this release to avoid large 
download size and possible license issues. However, libjpeg can be found 
from www.ijg.org, Lua from www.lua.org and zlib from www.gzip.org/zlib/. 
Unzip those to external/libjpeg, external/lua and external/zlib folders.


Dead Justice Game Prototype Credits
-----------------------------------
Game design....... Olli Sorjonen, Sami Sorjonen
Programming....... Jani Kajala (lead), Toni Aittoniemi
Graphic content... Olli Sorjonen, Sami Sorjonen
Level scripting... Olli Sorjonen, Jani Kajala, Toni Aittoniemi
Testing........... Olli Sorjonen, Jani Kajala, Toni Aittoniemi

Dead Justice game prototype uses in-house core technology and 3D-engine,
developed by Jani Kajala. (All source code is included in this package)
