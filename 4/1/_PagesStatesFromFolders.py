pageLog = open("actions.log", "r")
parsedPageLog = open("octavePagesBusiness.m", "w", encoding="utf-8")

iPages = 20

timeOctave = [0]

pagesRead = []
pagesWrite = []
pagesFree = [i for i in range(iPages)]

pagesReadOctave = [0]
pagesWriteOctave = [0]
pagesFreeOctave = [iPages]

minTime = -1

lAll = []

for line in pageLog:
	state = ""
	if("reading page #" in line):
		state = "reading"
	elif("writing page number to page #" in line):
		state = "pwriting"
	#elif("release reader's semaphore #" in line or
	#		"read the page #" in line):
	elif("release reader's semaphore #" in line):
		state = "PWfree"
	elif("read the page #" in line):
		state = "Rfree"
	if(state != ""):
		line = line.replace("\n", "")
		time = int(line[:line.find(" ")])
		if(minTime == -1):
			minTime = time
		time = time - minTime
		line = line[line.find("#")+1:]
		if(line.find(".") >= 0):
			pageNumber = int(line[:line.find(".")])
		else:
			pageNumber = int(line)
		
		if(state == "reading"):
			if(pageNumber in pagesWrite):
				pagesWrite.remove(pageNumber)
			if(pageNumber in pagesFree):
				pagesFree.remove(pageNumber)
			pagesRead.append(pageNumber)
		elif(state == "pwriting"):
			if(pageNumber in pagesRead):
				pagesRead.remove(pageNumber)
			if(pageNumber in pagesFree):
				pagesFree.remove(pageNumber)
			pagesWrite.append(pageNumber)
		else:
			if(pageNumber in pagesWrite):
				pagesWrite.remove(pageNumber)
			if(pageNumber in pagesRead):
				pagesRead.remove(pageNumber)
			if(pageNumber not in pagesFree):
				pagesFree.append(pageNumber)
		
		timeOctave.append(time)
		pagesReadOctave.append(len(pagesRead))
		pagesWriteOctave.append(len(pagesWrite))
		pagesFreeOctave.append(len(pagesFree))
		
		lAll.append([state[0], time, pageNumber])
		
_pWrite = []
_pRead = []
_pFree = [i for i in range(iPages)]

_time = [0]
_poWrite = [0]
_poRead = [0]
_poFree = [iPages]
		
#print(lAll)
#lAll.sort()		# Интнресный результат ждет того, кто разкомментирует
lAll = sorted(lAll, key = lambda x: (x[1], x[0]))
#print("\nNEW\n", lAll)

outLog = open("bLog.log", "w")

for info in lAll:
	state = info[0]
	time = info[1]
	pageNumber = info[2]
	if("r" == state):
		if(pageNumber in _pWrite):
			_pWrite.remove(pageNumber)
		if(pageNumber in _pFree):
			_pFree.remove(pageNumber)
		_pRead.append(pageNumber)
	elif("p" == state):
		if(pageNumber in _pRead):
			_pRead.remove(pageNumber)
		if(pageNumber in _pFree):
			_pFree.remove(pageNumber)
		_pWrite.append(pageNumber)
	else:
		if(pageNumber in _pWrite):
			_pWrite.remove(pageNumber)
		if(pageNumber in _pRead):
			_pRead.remove(pageNumber)
		if(pageNumber not in _pFree):
			_pFree.append(pageNumber)
			
	_time.append(time)
	_poRead.append(len(_pRead))
	_poWrite.append(len(_pWrite))
	_poFree.append(len(_pFree))
	outLog.write(f"{time} | page #{pageNumber}, state = {state}\n")
	
outLog.close()

#parsedPageLog.write(f"time = {timeOctave};\nread = {pagesReadOctave};\nwrite = {pagesWriteOctave};\nfree = {pagesFreeOctave};\n")
parsedPageLog.write(f"time = {_time};\nread = {_poRead};\nwrite = {_poWrite};\nfree = {_poFree};\n")
parsedPageLog.write("figure;\nhold on;\nplot(time, read, \"r\");\nplot(time, write, \"b\");\nplot(time, free, \"g\");\nxlabel(\"Время, мс\")\nhold off;")
pageLog.close()
parsedPageLog.close()