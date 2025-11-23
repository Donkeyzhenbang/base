cmd_/home/jym/linux-kernel-dev/modules.order := {   echo /home/jym/linux-kernel-dev/virt_input.ko; :; } | awk '!x[$$0]++' - > /home/jym/linux-kernel-dev/modules.order
