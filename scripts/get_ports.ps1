Get-CimInstance -ClassName Win32_PnPEntity | Where-Object { $_.Name -match '\(COM\d+\)' } | ForEach-Object {
    if ($_.Name -match '\((COM\d+)\)') {
        $comPort = $matches[1]
        Write-Output "$comPort|$($_.Name)"
    }
}
