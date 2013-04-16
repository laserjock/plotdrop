from subprocess import Popen,PIPE
from threading import Thread

class Gnuplot():
	def __init__ (self, scriptfile=None):
		self.fields = {
			"xlabel": "set x label ###",
			"ylabel": "set y label ###",
			"title": "set title ###",
			"logscaley": "set logscale y",
			"logscalex": "set logscale x",
			"zeroaxis": "set zeroaxis",
			"grid": "set grid",
			"xmin": "###",
			"xmax": "###",
			"ymin": "###",
			"ymax": "###"}

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

	def return_field(self, data, field):
		if data.has_key(field):
			return self.fields[field].replace("###", data[field])

	def return_header(self, data):
		fields = ["xlabel", "ylabel", "title", "logscaley", "logscalex",
			"zeroaxis", "grid"]
		script = []
		for field in fields:
			if self.return_field(data, field):
				script.append(self.return_field(data, field))
		return script
			
		

	def compose(self, data, temp_name):		
		
		print "Composing the plot script:"
		plotscript = []
		plotscript += self.return_header(data)

		plotcommand = "plot "
		xlimcmd = "[" + return_field(data, "xmin")
		xlimcmd += ":" + + return_field(data, "xmax")
		xlimcmd += "]"
		
		ylimcmd = "[" + return_field(data, "ymin")
		ylimcmd += ":" + + return_field(data, "ymax")
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
