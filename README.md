bss
===

(GPL) Basic serial sniffer is a FREE software.

This software is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY.

Instructions
============


download the software :

	mkdir devel
	cd devel
	
	git clone git://github.com/Keidan/bss.git
	cd bss
	git submodule init
	git submodule update
	cmake -DCMAKE_BUILD_TYPE=[x86|x86_64|xscale] -DDISTRIBUTION=[debug|release] .
	make
  

License (like GPL)
==================

	You can:
		- Redistribute the sources code and binaries.
		- Modify the Sources code.
		- Use a part of the sources (less than 50%) in an other software, just write somewhere "bss is great" visible by the user (on your product or on your website with a link to my page).
		- Redistribute the modification only if you want.
		- Send me the bug-fix (it could be great).
		- Pay me a beer or some other things.
		- Print the source code on WC paper ...
	You can NOT:
		- Earn money with this Software (But I can).
		- Add malware in the Sources.
		- Do something bad with the sources.
		- Use it to travel in the space with a toaster.
	
	I reserve the right to change this licence. If it change the version of the copy you have keep its own license
