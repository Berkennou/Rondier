from datetime import datetime
from pathlib import Path
import pydbus
from gi.repository import GLib
import socket
import time


IP = "192.168.43.89"
PORT = 6565
ADDR = (IP, PORT)
SIZE = 1024
FORMAT = "utf-8"





discovery_time = 60

macServer = 'F8:5E:A0:1F:A9:C2'

bus = pydbus.SystemBus()
mainloop = GLib.MainLoop()

class DeviceMonitor:
    def __init__(self, path_obj):
        self.device = bus.get('org.bluez', path_obj)
        self.device.onPropertiesChanged = self.prop_changed
        rssi = self.device.GetAll('org.bluez.Device1').get('RSSI')

    def prop_changed(self, iface, props_changed, props_removed):
        rssi = props_changed.get('RSSI', None)
        if rssi is not None and self.device.Address == macServer:
            print(f'\tRondier detecté : {self.device.Address} @ {rssi} dBm\n')
            if rssi > -50:
                end_discovery()

def end_discovery():
    mainloop.quit()
    adapter.StopDiscovery()

def new_iface(path, iface_props):
    device_addr = iface_props.get('org.bluez.Device1', {}).get('Address')
    if device_addr:
        DeviceMonitor(path)


client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
client.connect(ADDR)
print('[Connected]\n')


"""Identification"""
client.send("1".encode(FORMAT))
num = input("num : ")
client.send(num.encode(FORMAT))
print('[Identification done]\n')

"""Waiting for my turn"""
data = client.recv(SIZE).decode(FORMAT)

print(f'[waiting for the Rondsman]\n')

# BlueZ object manager
mngr = bus.get('org.bluez', '/')
mngr.onInterfacesAdded = new_iface

# Connect to the DBus api for the Bluetooth adapter
adapter = bus.get('org.bluez', '/org/bluez/hci0')
adapter.DuplicateData = False

# Iterate around already known devices and add to monitor

mng_objs = mngr.GetManagedObjects()
for path in mng_objs:
    device = mng_objs[path].get('org.bluez.Device1', {}).get('Address', [])
    if device:
        DeviceMonitor(path)

# Run discovery for discovery_time
adapter.StartDiscovery()
GLib.timeout_add_seconds(discovery_time, end_discovery)
try:
    mainloop.run()
except KeyboardInterrupt:
    end_discovery()

num = 'P' #PASSED
client.send(num.encode(FORMAT))
print('[Checkpoint validated :) ]\n')

""" Close connection """
client.close()
