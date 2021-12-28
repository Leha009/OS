import os

actionList = []

for fileName in os.listdir("readers"):
	logFile = open("readers/" + fileName, "r")
	for line in logFile:
		time, logAction = line.split(" | ")
		actionList.append((time, logAction))
	logFile.close();
	
for fileName in os.listdir("writers"):
	logFile = open("writers/" + fileName, "r")
	for line in logFile:
		time, logAction = line.split(" | ")
		actionList.append((time, logAction))
	logFile.close();
	
actionList.sort()

outLog = open("actions.log", "w")
for action in actionList:
	outLog.write(action[0] + " | " + action[1])
outLog.close()