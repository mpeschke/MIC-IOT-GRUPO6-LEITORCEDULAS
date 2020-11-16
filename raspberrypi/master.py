import os
import smbus
import time

bus = smbus.SMBus(1)
address = 0x04
keepalive = 0x01
heartbeat = 3

def main():
	while 1:
		bus.write_byte(address, keepalive)
		cedula = int(bus.read_byte(address))
		if cedula == 0:
			print("Microcontrolador ocioso.")
		elif cedula == 1:
			print("cedula indefinida")
			os.system("mpg123 1_desconhecida.mp3")
		elif cedula == 2:
			print("2 reais")
			os.system("mpg123 2_reais.mp3")
		elif cedula == 10:
			print("10 reais")
			os.system("mpg123 10_reais.mp3")
		else:
			print("codigo desconhecido {}".format(cedula))
		time.sleep(heartbeat)


if __name__ == '__main__':
	try:
		main()
	except KeyboardInterrupt:
		print("CTRL C")
		sys.exit(0)