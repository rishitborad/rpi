cmd_/home/pi/Documents/simple_driver/modules.order := {   echo /home/pi/Documents/simple_driver/hello_world.ko; :; } | awk '!x[$$0]++' - > /home/pi/Documents/simple_driver/modules.order
