What is it
----------

Leonie is a bytecode interpreter for a HyperTalk-like programming language named Hammer. It is written in plain C for portability and control.


How to build
------------

There is currently only an Xcode project for MacOS X test app in the macosx subfolder, however that code should be easily portable to command-line Unix with a few changes.


Better test application
-----------------------

If you want to play around with Leonie in the context of actual bytecode, a good idea would be to check out the Forge and ForgeDebugger projects, which include a command line tool that compiles Hammer source code into Leonie bytecode and runs it.


License
-------

	Copyright 2003-2014 by Uli Kusterer.
	
	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.
	
	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:
	
	   1. The origin of this software must not be misrepresented; you must not
	   claim that you wrote the original software. If you use this software
	   in a product, an acknowledgment in the product documentation would be
	   appreciated but is not required.
	
	   2. Altered source versions must be plainly marked as such, and must not be
	   misrepresented as being the original software.
	
	   3. This notice may not be removed or altered from any source
	   distribution.
     