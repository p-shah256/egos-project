# Requirements
- Linux machine is required for the project to work.
- macOS does not provide support to create tap interfaces which is an essential requirement for this project to work.
- We recommend using Ubuntu. The following mentioned steps work for ubuntu but similar commands should work for other distros as well.

# Installation and Setup
- Clone this repository in proper OSI directory
- Ensure that you have commented out the qemu path in env.sh (We would need the custom qemu installation for this)
  
- Install qemu on your machine
  ```
  apt-get install qemu-system
  ```
  
- We would be using bridge helper to communicate from our qemu guest OS (EGOS) with the host OS via a bridge br0.
- Perform below mentioned commands to setup bridge helper properly
  ```
  sudo chmod u+s /usr/lib/qemu/qemu-bridge-helper
  mkdir /etc/qemu
  sudo chmod 755 /etc/qemu
  touch /etc/qemu/bridge.conf
  sudo chmod 644 /etc/qemu/bridge.conf
  echo "allow br0" | sudo tee -a /etc/qemu/bridge.conf
  ```

- Create a bridge on host OS
  ```
  sudo ip link add br0 type bridge
  sudo ip link set dev br0 up
  sudo ip addr add 192.168.0.1/24 dev br0
  ```

- Create tap2 on host (for testing)
  ```
  sudo apt-get install bridge-utils
  sudo ip tuntap add mode tap tap2
  sudo brctl addif br0 tap2
  sudo ip link set dev tap2 up
  ```

- Verify tap and bridge are setup properly
  ```
  sudo ip link
  sudo ip addr
  ```
- Compile tap tester
  ```
  gcc -o tap_tester tap_tester.c
  ```

- Refer the demo video for more info. Please let us know if there are issues while setting/testing it up.

  References:
  - https://github.com/u-boot/u-boot
  - https://github.com/adamdunkels/uip
