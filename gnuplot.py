from subprocess import Popen,PIPE
from threading import Thread

class Gnuplot():
	def __init__ (self, scriptfile=None):
		if scriptfile:
			self.temp_name = scriptfile
			print "Temporary gnuplot script file:"
			print self.temp_name+"\n"
		else:
			print "Not using a temp file\n"

	def plot(self, GP_name, export):
		if export:
			plot = Popen([GP_name,self.temp_name], bufsize=1000, stdout=PIPE).stdout
			print plot.read()
		else:
			args = ("-persist ",self.temp_name)
			plot = Popen([GP_name,args], bufsize=1000, stdout=PIPE).stdout
			print plot.read()

	def compose(self, data, temp_name):
		print "Composing the plot script:"
		plotscript = []
		plotscript.append("# gnuplot script created by Plotdrop")
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
		
		plotcommand = "plot "
		xlimcmd = "["
		if data.has_key("xmin"):
			xlimcmd += data['xmin']
		xlimcmd += ":"
		if data.has_key("xmax"):
			xlimcmd += data['xmax']
		xlimcmd += "]"
		
		ylimcmd = "["
		if data.has_key("ymin"):
			ylimcmd += data['ymin']
		ylimcmd += ":"
		if data.has_key("ymax"):
			ylimcmd += data['ymax']
		ylimcmd += "]"
		
		path = data['path']
		
		plotcommand += " \""+path+"\" "
		
		plotscript.append(plotcommand)
		
		outfile = open(self.temp_name,"w")
		for i in plotscript:
			print i
			outfile.write(i+"\n")
		outfile.close()

	def version(self, GP_name):
		command = GP_name + " --version"
		pipe = Popen(command, shell=True, bufsize=1000, stdout=PIPE).stdout
		return pipe.read()

def plotdrop_test(cmd,temp):
	print "gnuplot version:"
	print plot.version(gp_command)
	pltdata = ["0 0","1 2","2 4","3 9","4 16"]
	outfile = open("test.dat","w")
	for i in pltdata:
		outfile.write(i+"\n")
	outfile.close()
	data = {'path':"test.dat", 'xlabel': 'Time', 'ylabel':"Energy", 'title':"Energy vs. Time"}
	plot.compose(data, temp)
	plot.plot(cmd, False)

if __name__ == '__main__':
	gp_command = "gnuplot"
	temp_name = "test.gnuplot"
	
	plot = Gnuplot(temp_name)
	
	plotdrop_test(gp_command,temp_name)
