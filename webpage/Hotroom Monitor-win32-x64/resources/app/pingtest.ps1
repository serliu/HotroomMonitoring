while($true){
    Write-Host "Running..."
    $ip_list = @()
    $ip_list_final = @()
    Get-Content  'iplist.txt' | %{
	Write-Host -NoNewLine "Pinging", "$_"
        If (-Not (Test-Connection $_.Trim() -Quiet -Count 1 -TimeToLive 10)) {
	   $ip_list += "$_"
           Write-Host " Not Online"
        }Else{
	   Write-Host " Online"
        }
    }
    Write-Host "First pass done; Running second pass..."
    foreach($ip in $ip_list){
	Write-Host -NoNewLine "Pinging", "$ip"
        If (-Not (Test-Connection $ip -Quiet -Count 1 -TimeToLive 10)) {
	   $ip_list_final += "$ip"
           Write-Host " Not Online"
        }Else{
	   Write-Host " Online"
        }
    }
    Clear-Content 'badips.txt'
    Write-Host "Saving bad IPs..."
    foreach($ip in $ip_list_final){
	$ip | Out-File 'badips.txt' -Append
    }

    Write-Host "Ping Test Completed"
    $a = Get-Date
        Write-Host $a, "Sleeping for 15 minutes"
        Start-Sleep -Seconds 900
}

Write-Host "Timed out, restart"