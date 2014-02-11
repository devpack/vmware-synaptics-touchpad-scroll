Use Synaptics Touchpad Scroll in VMware Guest
=============================================

Updated Andrew Zabavnikov fix to work also with VMware Player.

1. First fix the registry: http://blog.alexou.net/2010/07/scrolling-in-gtk-apps-with-synaptics-driver
2. http://superuser.com/questions/131297/use-synaptics-touchpad-scroll-in-vmware-guest/683726#683726

To use with VMware Workstation:

> vmware_scroll_start.exe

To use with VMware Player:

> vmware_scroll_start.exe 0

NB: Compiled with VS C++ 2010, so "Package redistribuable Microsoft Visual C++ 2010" needed if you use provided .exe (run the corresponding executable when a VM is running).