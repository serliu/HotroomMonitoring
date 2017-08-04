while($True){
    $J = Invoke-WebRequest -Uri http://192.168.100.100/json -TimeoutSec 3 -Method GET | ConvertFrom-Json
    $Thresh = 38.00
    if(($J.S0 -ge $Thresh)-or($J.S1 -ge $Thresh)){
        Write-Host "$J.S0", "$J.S1"
        rename-item "autoshutdown.txt" "autoshutdown.bat"
        Write-Host "Command sent, Press any button to Revert script..."
        cmd /c pause | out-null
        rename-item "autoshutdown.bat" "autoshutdown.txt"
    }
    Else{
	$a = Get-Date
    	Write-Host "$a", "Temperatures =", $J.S0,"&"  $J.S1
    }
    cmd /c timeout 10
}
