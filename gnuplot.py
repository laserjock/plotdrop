from subprocess import Popen,PIPE
from threading import Thread

class Gnuplot():
	def __init__ (self, scriptfile=None):
		self.temp_name = scriptfile
		print scriptfile

	def plot(self, GP_name, export):
		if export:
			plot = Popen([GP_name,self.temp_name], bufsize=1000, stdout=PIPE).stdout
			print plot.read()
		else:
			args = ("-persist ",self.temp_name)
			plot = Popen([GP_name,args], bufsize=1000, stdout=PIPE).stdout
			print plot.read()

	def compose(self, data, temp_name):
		print "compose"
		plotscript = []
		if data.has_key("xlabel"):
			plotscript.append("set xlabel \"%s\"" % data['xlabel'])
		if data.has_key("ylabel"):
			plotscript.append("set ylabel \"%s\"" % data['ylabel'])
		if data.has_key("title"):
			plotscript.append("set title \"%s\"" % data['title'])
		if data.has_key("logscaley"):
			plotscript.append("set logscale y")
		if data.has_key("logscalex"):
			plotscript.append("set logscale x")
		if data.has_key("zeroaxis"):
			plotscript.append("set zeroaxis")
		if data.has_key("grid"):
			plotscript.append("set grid")
		
		for i in plotscript:
			print i

	def version(self, GP_name):
		command = GP_name + " --version"
		pipe = Popen(command, shell=True, bufsize=1000, stdout=PIPE).stdout
		return pipe.read()


if __name__ == '__main__':
	plot = Gnuplot("test.gnuplot")
	gp_command = "gnuplot"
	temp_name = "test.gnuplot"
	version = plot.version(gp_command)
	print version
	#plot.plot(gp_command,False)
	data = {'xlabel':"hello", 'ylabel':"goodbye", 'zeroaxis':"", 'title':"nona business"}
	plot.compose(data,temp_name)
