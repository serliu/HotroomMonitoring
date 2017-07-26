while($True){
    $J = Invoke-WebRequest -Uri http://192.168.100.100/json | ConvertFrom-Json
    $Thresh = 38.00
    if(($J.S0 -ge $Thresh)-or($J.S1 -ge $Thresh)){
        Write-Host "$J.S0", "$J.S1"
        rename-item "autoshutdown.txt" "autoshutdown.bat"
        break
    }
    Else{
	$a = Get-Date
    	Write-Host "$a", "Still Alive"
    }
    Start-Sleep -Seconds 10
}
Write-Host "Command sent"