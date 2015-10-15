import os
import socket
import pylibgv

class gv_socket(object):
    def __init__(self):
	
	self.so_data = socket.socket(socket.AF_UNIX,socket.SOCK_STREAM)
	self.so_ctrl = socket.socket(socket.AF_UNIX,socket.SOCK_STREAM)
	self.local_address = # Crear la direccion local

	self.gvapi   = pylibgv.pylibgv(self.so_ctrl.fileno())

	server_address = os.getenv()


	try:
	    self.so_ctrl.connect(server_address)
	except:
	    raise exception

	
	try:
	    self.so_data.bind(self.local_address)
	except:
	    raise exception

	try:
	    self.so_data.listen(1)
	except:
	    raise exception


    def close(self):
	self.gvapi.close(self.so_ctrl.fileno())
	self.so_data.close()
	self.so_ctrl.close()


    def connect(self, addr, port, vport):
	try:
	    self.gvapi.connect(addr,port,vport)
	except:
	    raise 

    def accept(self):


    def listen(self, backlog):


    def bind(self, addr, port, vport):


    def setsockopt(self):
	pass


    def getsockopt(self):
	pass


    def send(self, data):
	#
	# Check if the socket have connection
	#
	return self.so_data.send(data)


    def recv(self, datalen):
	#
	# Check if the socket have connection
	#
	return self.so_data.recv(datalen)