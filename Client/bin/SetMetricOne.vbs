On Error Resume Next 
 
strComputer = "." 
Set objWMIService = GetObject("winmgmts:" _ 
    & "{impersonationLevel=impersonate}!\\" & strComputer & "\root\cimv2") 
 
rem Set colNetCards = objWMIService.ExecQuery _ 
rem    ("Select * From Win32_NetworkAdapterConfiguration Where IPEnabled = True") 


Set colNetCards = objWMIService.ExecQuery _ 
   ("Select * From Win32_NetworkAdapterConfiguration Where Description like 'Extelecom%'") 

NewMetric = "8"

For Each objNetCard in colNetCards 
rem Wscript.Echo objNetCard.Description & objNetCard.IPConnectionMetric
    Wscript.Echo "Desc: " & objNetCard.Description & vbCr & _
	"Metric(Old): " & objNetCard.IPConnectionMetric & vbCr & _
	"Metric(New): " & NewMetric
    objNetCard.SetIPConnectionMetric(NewMetric)
	objNetCard.SetIPAddress()
Next